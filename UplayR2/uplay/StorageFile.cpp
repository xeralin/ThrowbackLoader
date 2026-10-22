#include "Uplay.h"

#include <fcntl.h>
#include <io.h>
#include <share.h>
#include <sys/stat.h>

#include <algorithm>
#include <chrono>
#include <cstring>
#include <vector>

constexpr uint32_t SAVE_HEADER_SIZE = 40;

struct FileData
{
    char* file_name;
    uint32_t* unknown;
    uint32_t file_size;
    uint32_t padding;
    uint64_t last_modified;
};

struct FileList
{
    uint32_t number;
    uint32_t padding = 0;
    struct FileData** files;
};

namespace
{
[[nodiscard]] bool is_valid_name(const char* name)
{
    return name && *name && !std::strpbrk(name, "/\\");
}

[[nodiscard]] int read_save(const std::filesystem::path& path, std::vector<char>& data)
{
    int file = -1;
    const errno_t open_error = _wsopen_s(&file, path.c_str(), _O_BINARY | _O_RDONLY, _SH_DENYNO, _S_IREAD);
    if (open_error != 0)
    {
        return open_error == ENOENT ? -6 : -10;
    }

    const long long file_size = _filelengthi64(file);
    int result = file_size < 0 ? -10 : -6;
    if (file_size > SAVE_HEADER_SIZE)
    {
        data.resize(static_cast<size_t>(file_size - SAVE_HEADER_SIZE));
        const bool loaded =
            _lseek(file, SAVE_HEADER_SIZE, SEEK_SET) == static_cast<long>(SAVE_HEADER_SIZE) &&
            _read(file, data.data(), static_cast<unsigned>(data.size())) == static_cast<int>(data.size());
        result = loaded ? 0 : -10;
    }
    _close(file);
    return result;
}

[[nodiscard]] bool write_save(const std::filesystem::path& path, const std::vector<char>& data)
{
    int file = -1;
    if (_wsopen_s(&file, get_temporary_path(path).c_str(), _O_BINARY | _O_CREAT | _O_TRUNC | _O_WRONLY,
                  _SH_DENYWR, _S_IREAD | _S_IWRITE) != 0)
    {
        return false;
    }

    const char header[SAVE_HEADER_SIZE] = {};
    if (_write(file, header, sizeof(header)) != static_cast<int>(sizeof(header)) ||
        _write(file, data.data(), static_cast<unsigned>(data.size())) != static_cast<int>(data.size()))
    {
        _close(file);
        return false;
    }
    return commit_file(file, path);
}
}

DLLEXPORT int UPC_StorageFileClose(ContextData* context, int in_handle)
{
    if (!context)
    {
        return -2;
    }

    std::lock_guard<std::mutex> lock(context->storage_mutex);
    const auto file = context->storage_files.find(in_handle);
    if (file == context->storage_files.end())
    {
        return -2;
    }
    if (file->second.flags == 0x2 && !file->second.data.empty() && !write_save(file->second.path, file->second.data))
    {
        return -10;
    }

    context->storage_files.erase(file);
    return 0;
}

DLLEXPORT int UPC_StorageFileDelete(ContextData* context, char* in_file_name_utf8)
{
    if (!context || !is_valid_name(in_file_name_utf8))
    {
        return -2;
    }

    std::error_code error_code;
    if (std::filesystem::remove(get_saves_path(g_uplay_config.GetAppId()) / in_file_name_utf8, error_code))
    {
        return 0;
    }
    return error_code ? -10 : -6;
}

DLLEXPORT int UPC_StorageFileListFree(ContextData* context, FileList* in_storage_file_list)
{
    if (!in_storage_file_list)
    {
        return 0;
    }

    for (unsigned i = 0; i < in_storage_file_list->number; ++i)
    {
        delete[] in_storage_file_list->files[i]->file_name;
        delete in_storage_file_list->files[i]->unknown;
        delete in_storage_file_list->files[i];
    }

    delete[] in_storage_file_list->files;
    delete in_storage_file_list;
    return 0;
}

DLLEXPORT int UPC_StorageFileListGet(ContextData* context, FileList** out_storage_file_list)
{
    if (!context || !out_storage_file_list)
    {
        return -2;
    }

    std::error_code error_code;
    const auto directory = std::filesystem::directory_iterator(get_saves_path(g_uplay_config.GetAppId()), error_code);

    std::vector<FileData*> files;
    for (const auto& file : directory)
    {
        if (!file.is_regular_file(error_code) || file.path().extension() == ".tmp")
        {
            continue;
        }
        const uintmax_t file_size = std::filesystem::file_size(file, error_code);
        if (error_code || file_size <= SAVE_HEADER_SIZE)
        {
            continue;
        }
        const auto last_write_time = file.last_write_time(error_code);
        if (error_code)
        {
            continue;
        }

        const std::string file_name = file.path().filename().string();

        FileData* f_data = new FileData();
        f_data->file_name = new char[file_name.size() + 1];
        file_name.copy(f_data->file_name, file_name.size());
        f_data->file_name[file_name.size()] = 0;
        f_data->unknown = new uint32_t{};
        f_data->file_size = static_cast<uint32_t>(file_size - SAVE_HEADER_SIZE);
        const auto unix_time = std::chrono::clock_cast<std::chrono::system_clock>(last_write_time);
        f_data->last_modified =
            std::chrono::duration_cast<std::chrono::milliseconds>(unix_time.time_since_epoch()).count();
        files.push_back(f_data);
    }

    FileList* fl = new FileList();
    fl->number = static_cast<uint32_t>(files.size());
    fl->files = new FileData*[files.size()];
    std::copy(files.begin(), files.end(), fl->files);

    *out_storage_file_list = fl;
    return 0;
}

DLLEXPORT int UPC_StorageFileOpen(ContextData* context, char* in_file_name_utf8, unsigned in_flags, int* out_handle)
{
    if (!context || !is_valid_name(in_file_name_utf8) || !out_handle || (in_flags != 0x1 && in_flags != 0x2))
    {
        return -2;
    }

    const auto file_path = get_saves_path(g_uplay_config.GetAppId()) / in_file_name_utf8;
    std::vector<char> data;
    if (in_flags == 0x1)
    {
        const int result = read_save(file_path, data);
        if (result != 0)
        {
            return result;
        }
    }

    std::lock_guard<std::mutex> lock(context->storage_mutex);
    const int handle = context->next_storage_handle++;
    context->storage_files.emplace(handle, StorageHandle{in_flags, file_path, std::move(data)});
    *out_handle = handle;
    return 0;
}

DLLEXPORT int UPC_StorageFileRead(ContextData* context, int in_handle, int in_bytes_to_read,
                                  unsigned in_bytes_read_offset, void* out_data, int* out_bytes_read, void* in_callback,
                                  void* in_callback_data)
{
    if (!context || !out_data || !out_bytes_read || in_bytes_to_read <= 0)
    {
        return -2;
    }

    int out = 0;
    {
        std::lock_guard<std::mutex> lock(context->storage_mutex);
        const auto file = context->storage_files.find(in_handle);
        if (file == context->storage_files.end())
        {
            return -2;
        }
        if (file->second.flags != 0x1)
        {
            return -9;
        }

        const auto& data = file->second.data;
        const size_t available = in_bytes_read_offset < data.size() ? data.size() - in_bytes_read_offset : 0;
        const size_t count = std::min(static_cast<size_t>(in_bytes_to_read), available);
        if (count > 0)
        {
            std::copy_n(data.begin() + in_bytes_read_offset, count, static_cast<char*>(out_data));
        }
        *out_bytes_read = static_cast<int>(count);
        if (count == 0)
        {
            out = -7;
        }
    }

    context->push_callback(in_callback, in_callback_data, out);
    return UPC_ASYNC_PENDING;
}

DLLEXPORT int UPC_StorageFileWrite(ContextData* context, int in_handle, void* in_data, int in_size, void* in_callback,
                                   void* in_callback_data)
{
    if (!context || !in_data || in_size <= 0)
    {
        return -2;
    }

    {
        std::lock_guard<std::mutex> lock(context->storage_mutex);
        const auto file = context->storage_files.find(in_handle);
        if (file == context->storage_files.end())
        {
            return -2;
        }
        if (file->second.flags != 0x2)
        {
            return -9;
        }

        auto& data = file->second.data;
        const auto* bytes = static_cast<const char*>(in_data);
        data.insert(data.end(), bytes, bytes + in_size);
    }

    context->push_callback(in_callback, in_callback_data, 0);
    return UPC_ASYNC_PENDING;
}
