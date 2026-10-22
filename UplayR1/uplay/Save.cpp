#include "Uplay.h"

#include <fcntl.h>
#include <io.h>
#include <share.h>
#include <sys/stat.h>

#include <algorithm>
#include <array>
#include <charconv>
#include <fstream>
#include <map>
#include <mutex>
#include <vector>

constexpr uint32_t SAVE_HEADER_SIZE = 0x228;
constexpr uint32_t SAVE_NAME_OFFSET = 0x28;
constexpr uint32_t SAVE_NAME_SIZE = 0x200;

struct SaveGame
{
    DWORD slot;
    char* name;
    DWORD size;
};

struct SaveGameList
{
    ULONG_PTR count;
    SaveGame** games;
};

namespace
{
struct SaveHandle
{
    uint32_t mode;
    int file;
    std::string name;
    bool failed = false;
};

std::mutex g_saves_mutex;
std::map<uint32_t, SaveHandle> g_open_saves;

[[nodiscard]] std::filesystem::path get_save_path(uint32_t handle)
{
    return get_saves_path() / std::to_string(handle);
}

[[nodiscard]] std::optional<uint32_t> parse_save_handle(const std::filesystem::path& filepath)
{
    const std::string file_name = filepath.filename().string();
    uint32_t file_number = 0;
    const auto result = std::from_chars(file_name.data(), file_name.data() + file_name.size(), file_number);
    if (result.ec != std::errc{} || result.ptr != file_name.data() + file_name.size())
    {
        return std::nullopt;
    }

    return file_number;
}

[[nodiscard]] int open_save(uint32_t handle, uint32_t mode)
{
    if (handle == 0)
    {
        return 6;
    }
    if (g_open_saves.contains(handle))
    {
        return 8;
    }
    if (mode > 1)
    {
        return 1;
    }

    const auto save_path = get_save_path(handle);
    int file = -1;
    if (mode == 0)
    {
        const errno_t open_error = _wsopen_s(&file, save_path.c_str(), _O_BINARY | _O_RDONLY, _SH_DENYNO, _S_IREAD);
        if (open_error != 0)
        {
            return open_error == ENOENT ? 3 : 6;
        }
        const long long file_size = _filelengthi64(file);
        if (file_size <= SAVE_HEADER_SIZE)
        {
            _close(file);
            return file_size < 0 ? 6 : 3;
        }
        g_open_saves.emplace(handle, SaveHandle{mode, file, {}});
        return 0;
    }

    if (_wsopen_s(&file, get_temporary_path(save_path).c_str(), _O_BINARY | _O_CREAT | _O_TRUNC | _O_WRONLY,
                  _SH_DENYWR, _S_IREAD | _S_IWRITE) != 0)
    {
        return 6;
    }
    if (_lseek(file, SAVE_HEADER_SIZE, SEEK_SET) != static_cast<long>(SAVE_HEADER_SIZE))
    {
        _close(file);
        return 6;
    }
    g_open_saves.emplace(handle, SaveHandle{mode, file, "Unnamed"});
    return 0;
}
}

DLLEXPORT int UPLAY_SAVE_GetSavegames(SaveGameList** out_games_list, UplayOverlapped* overlapped)
{
    if (!out_games_list || !overlapped)
    {
        return 0;
    }

    std::vector<std::unique_ptr<SaveGame>> saves_found;
    std::error_code error_code;
    for (const auto& entry : std::filesystem::directory_iterator(get_saves_path(), error_code))
    {
        const auto save_handle = parse_save_handle(entry.path());
        if (!entry.is_regular_file(error_code) || !save_handle.has_value() || save_handle.value() == 0)
        {
            continue;
        }

        std::ifstream save_file(entry.path(), std::ios::binary | std::ios::ate);
        if (!save_file.is_open())
        {
            continue;
        }

        const auto file_size = static_cast<DWORD>(save_file.tellg());
        if (file_size <= SAVE_HEADER_SIZE)
        {
            continue;
        }
        save_file.seekg(SAVE_NAME_OFFSET, std::ios::beg);

        std::array<wchar_t, SAVE_NAME_SIZE / sizeof(wchar_t)> wide_name{};
        if (!save_file.read(reinterpret_cast<char*>(wide_name.data()), SAVE_NAME_SIZE))
        {
            continue;
        }
        wide_name.back() = L'\0';
        auto name = std::make_unique<char[]>(SAVE_NAME_SIZE);
        WideCharToMultiByte(CP_UTF8, 0, wide_name.data(), -1, name.get(), SAVE_NAME_SIZE - 1, nullptr, nullptr);

        auto save_game = std::make_unique<SaveGame>();
        save_game->slot = save_handle.value();
        save_game->name = name.release();
        save_game->size = file_size - SAVE_HEADER_SIZE;
        saves_found.push_back(std::move(save_game));
    }

    auto* games_list = new SaveGameList{saves_found.size(), new SaveGame*[saves_found.size()]};
    for (size_t i = 0; i < saves_found.size(); ++i)
    {
        games_list->games[i] = saves_found[i].release();
    }

    *out_games_list = games_list;
    overlapped->set_result();
    return 1;
}

DLLEXPORT int UPLAY_SAVE_Remove(uint32_t in_handle, UplayOverlapped* overlapped)
{
    if (!overlapped)
    {
        return 0;
    }

    std::lock_guard<std::mutex> lock(g_saves_mutex);
    std::error_code error_code;
    int result = 0;
    if (in_handle == 0)
    {
        result = 1;
    }
    else if (g_open_saves.contains(in_handle))
    {
        result = 8;
    }
    else if (!std::filesystem::remove(get_save_path(in_handle), error_code))
    {
        result = error_code ? 6 : 3;
    }

    overlapped->set_result(result);
    return result == 0;
}

DLLEXPORT int UPLAY_SAVE_SetName(uint32_t in_handle, const char* in_name_utf8)
{
    std::lock_guard<std::mutex> lock(g_saves_mutex);
    const auto save = g_open_saves.find(in_handle);
    if (!in_name_utf8 || save == g_open_saves.end() || save->second.mode != 1)
    {
        return 0;
    }

    save->second.name = in_name_utf8;
    return 1;
}

DLLEXPORT int UPLAY_SAVE_Open(uint32_t in_handle, uint32_t in_flags, uint32_t* out_handle, UplayOverlapped* overlapped)
{
    if (!out_handle || !overlapped)
    {
        return 0;
    }

    std::lock_guard<std::mutex> lock(g_saves_mutex);
    const int result = open_save(in_handle, in_flags);
    if (result != 0)
    {
        overlapped->set_result(result);
        return 0;
    }

    *out_handle = in_handle;
    overlapped->set_result(out_handle);
    return 1;
}

DLLEXPORT int UPLAY_SAVE_Close(uint32_t in_handle)
{
    std::lock_guard<std::mutex> lock(g_saves_mutex);
    const auto save = g_open_saves.extract(in_handle);
    if (save.empty())
    {
        return 0;
    }

    const int file = save.mapped().file;
    if (save.mapped().mode == 0)
    {
        return _close(file) == 0;
    }

    const auto save_path = get_save_path(in_handle);
    if (save.mapped().failed || _lseek(file, 0, SEEK_CUR) <= static_cast<long>(SAVE_HEADER_SIZE))
    {
        _close(file);
        std::error_code error_code;
        std::filesystem::remove(get_temporary_path(save_path), error_code);
        return !save.mapped().failed;
    }

    const std::string& name = save.mapped().name;
    const int length = MultiByteToWideChar(CP_UTF8, 0, name.data(), static_cast<int>(name.size()), nullptr, 0);
    std::wstring wide_name(static_cast<size_t>(length), L'\0');
    MultiByteToWideChar(CP_UTF8, 0, name.data(), static_cast<int>(name.size()), wide_name.data(), length);
    std::array<wchar_t, SAVE_NAME_SIZE / sizeof(wchar_t)> name_field{};
    std::copy_n(wide_name.begin(), std::min(wide_name.size(), name_field.size() - 1), name_field.begin());

    if (_lseek(file, SAVE_NAME_OFFSET, SEEK_SET) != static_cast<long>(SAVE_NAME_OFFSET) ||
        _write(file, name_field.data(), SAVE_NAME_SIZE) != static_cast<int>(SAVE_NAME_SIZE))
    {
        _close(file);
        return 0;
    }
    return commit_file(file, save_path);
}

DLLEXPORT int UPLAY_SAVE_Read(uint32_t in_handle, uint32_t in_bytes_to_read, uint32_t in_bytes_read_offset,
                              uint8_t** out_data, uint32_t* out_bytes_read, UplayOverlapped* overlapped)
{
    if (!out_data || !*out_data || !out_bytes_read || !overlapped)
    {
        return 0;
    }

    std::lock_guard<std::mutex> lock(g_saves_mutex);
    const auto save = g_open_saves.find(in_handle);
    if (save == g_open_saves.end() || save->second.mode != 0)
    {
        overlapped->set_result(save == g_open_saves.end() ? 1 : 8);
        return 0;
    }

    if (_lseek(save->second.file, SAVE_HEADER_SIZE + in_bytes_read_offset, SEEK_SET) < 0)
    {
        overlapped->set_result(6);
        return 0;
    }
    const int bytes_read = _read(save->second.file, *out_data, in_bytes_to_read);
    if (bytes_read < 0)
    {
        overlapped->set_result(6);
        return 0;
    }

    *out_bytes_read = static_cast<uint32_t>(bytes_read);
    overlapped->set_result(*out_data);
    return 1;
}

DLLEXPORT int UPLAY_SAVE_Write(uint32_t in_handle, uint32_t in_size, char** in_data, UplayOverlapped* overlapped)
{
    if (!in_data || !overlapped)
    {
        return 0;
    }

    std::lock_guard<std::mutex> lock(g_saves_mutex);
    const auto save = g_open_saves.find(in_handle);
    if (save == g_open_saves.end() || save->second.mode != 1)
    {
        overlapped->set_result(save == g_open_saves.end() ? 3 : 8);
        return 0;
    }

    if (_write(save->second.file, *in_data, in_size) != static_cast<int>(in_size))
    {
        save->second.failed = true;
        overlapped->set_result(6);
        return 0;
    }

    overlapped->set_result(*in_data);
    return 1;
}

DLLEXPORT int UPLAY_SAVE_ReleaseGameList(SaveGameList* games_list)
{
    if (!games_list)
    {
        return 0;
    }

    for (size_t i = 0; i < games_list->count; ++i)
    {
        delete[] games_list->games[i]->name;
        delete games_list->games[i];
    }

    delete[] games_list->games;
    delete games_list;
    return 1;
}
