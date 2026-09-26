#include "Uplay.h"

#include <charconv>
#include <fstream>
#include <vector>

constexpr uint32_t SAVE_PADDING_SIZE = 0x228;
constexpr uint32_t SAVE_HEADER_SIZE = 0x200;

namespace
{
[[nodiscard]] std::optional<std::vector<std::filesystem::path>> get_save_files()
{
    std::error_code error_code;
    const auto iterator = std::filesystem::directory_iterator(get_saves_path(), error_code);

    if (error_code)
    {
        return std::nullopt;
    }

    std::vector<std::filesystem::path> save_files{};

    for (const auto& entry : iterator)
    {
        if (entry.is_regular_file(error_code) && !entry.path().has_extension())
        {
            save_files.push_back(entry.path());
        }
    }

    return save_files;
}

[[nodiscard]] std::filesystem::path get_save_path(uint32_t handle)
{
    const auto saves_path = get_saves_path();
    return saves_path / std::to_string(handle);
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
}

DLLEXPORT int UPLAY_SAVE_GetSavegames(UplayList** out_games_list, UplayOverlapped* overlapped)
{
    LOGGER_INFO(__FUNCTION__);

    if (!out_games_list || !overlapped)
    {
        LOGGER_ERROR("Called with a null argument");
        return 0;
    }

    const auto savefile_paths = get_save_files();

    if (!savefile_paths.has_value() || savefile_paths->size() == 0)
    {
        return 0;
    }

    std::vector<std::unique_ptr<FileList>> saves_found;
    for (const auto& save_path : savefile_paths.value())
    {
        const auto save_handle = parse_save_handle(save_path);
        if (!save_handle.has_value())
        {
            continue;
        }

        std::ifstream save_file(save_path, std::ios::binary | std::ios::ate);
        if (!save_file.is_open())
        {
            continue;
        }

        const auto file_size = static_cast<DWORD>(save_file.tellg());
        if (file_size < SAVE_PADDING_SIZE)
        {
            continue;
        }
        save_file.seekg(0x28, std::ios::beg);

        auto data_buffer = std::make_unique_for_overwrite<char[]>(SAVE_HEADER_SIZE);
        if (!save_file.read(data_buffer.get(), SAVE_HEADER_SIZE))
        {
            continue;
        }

        auto file_list_item = std::make_unique<FileList>();
        file_list_item->bufferstring = data_buffer.release();
        file_list_item->num = save_handle.value();
        file_list_item->pointer = file_size - SAVE_PADDING_SIZE;

        saves_found.push_back(std::move(file_list_item));
    }

    if (saves_found.empty())
    {
        return 0;
    }

    auto game_ptr_array = std::make_unique<FileList*[]>(saves_found.size());

    for (size_t i = 0; i < saves_found.size(); ++i)
    {
        game_ptr_array[i] = saves_found[i].release();
    }

    auto final_list = std::make_unique<UplayList>();
    final_list->count = saves_found.size();
    final_list->games = game_ptr_array.release();

    *out_games_list = final_list.release();
    return 1;
}

DLLEXPORT int UPLAY_SAVE_Remove(uint32_t in_handle, UplayOverlapped* overlapped)
{
    LOGGER_INFO("{} {}", __FUNCTION__, in_handle);

    if (!overlapped)
    {
        return 0;
    }

    const auto save_path = get_save_path(in_handle);
    std::error_code error_code;
    if (!std::filesystem::remove(save_path, error_code))
    {
        LOGGER_ERROR("Failed to remove the save file");
        return 0;
    }

    overlapped->set_result();
    return 1;
}

DLLEXPORT int UPLAY_SAVE_SetName()
{
    LOGGER_INFO(__FUNCTION__);

    return 0;
}

DLLEXPORT int UPLAY_SAVE_Open(uint32_t in_handle, uint32_t in_flags, uint32_t* out_handle, UplayOverlapped* overlapped)
{
    LOGGER_INFO("{} {} {}", __FUNCTION__, in_handle, in_flags);

    if (!out_handle || !overlapped)
    {
        LOGGER_ERROR("Called with a null argument");
        return 0;
    }

    *out_handle = in_handle;

    overlapped->set_zeroes();
    overlapped->set_result(out_handle);
    return 1;
}

DLLEXPORT int UPLAY_SAVE_Close(uint32_t in_handle)
{
    LOGGER_INFO("{} {}", __FUNCTION__, in_handle);

    return 1;
}

DLLEXPORT int UPLAY_SAVE_Read(uint32_t in_handle, uint32_t in_bytes_to_read, uint32_t in_bytes_read_offset,
                              uint8_t** out_data, uint32_t* out_bytes_read, UplayOverlapped* overlapped)
{
    LOGGER_INFO("{} {} {} {}", __FUNCTION__, in_handle, in_bytes_to_read, in_bytes_read_offset);

    if (!out_data || !*out_data || !out_bytes_read || !overlapped)
    {
        LOGGER_ERROR("Called with a null argument");
        return 0;
    }

    overlapped->set_zeroes();

    std::ifstream file(get_save_path(in_handle), std::ios::binary | std::ios::ate);
    if (!file.is_open())
    {
        return 0;
    }

    const auto file_size = static_cast<uint64_t>(file.tellg());
    if (file_size < SAVE_PADDING_SIZE)
    {
        return 0;
    }

    const size_t available_bytes = file_size - SAVE_PADDING_SIZE;
    const size_t bytes_to_copy = std::min(static_cast<size_t>(in_bytes_to_read), available_bytes);

    file.seekg(SAVE_PADDING_SIZE, std::ios::beg);
    if (bytes_to_copy > 0 && !file.read(reinterpret_cast<char*>(*out_data + in_bytes_read_offset), bytes_to_copy))
    {
        return 0;
    }

    *out_bytes_read = static_cast<uint32_t>(bytes_to_copy);
    overlapped->set_result(static_cast<void*>(*out_data));
    return 1;
}

DLLEXPORT int UPLAY_SAVE_Write(uint32_t in_handle, uint32_t in_size, char** in_data, UplayOverlapped* overlapped)
{
    LOGGER_INFO("{} {} {}", __FUNCTION__, in_handle, in_size);

    if (!in_data || !overlapped)
    {
        LOGGER_ERROR("Called with a null argument");
        return 0;
    }

    overlapped->set_zeroes();

    const auto file_path = get_save_path(in_handle);

    const size_t total_size = SAVE_PADDING_SIZE + in_size;
    std::vector<uint8_t> output_buffer(total_size, 0);

    const uint8_t* source_data = reinterpret_cast<const uint8_t*>(*in_data);
    std::copy(source_data, source_data + in_size, output_buffer.begin() + SAVE_PADDING_SIZE);

    std::ofstream save_file(file_path, std::ios::binary);
    if (!save_file.is_open())
    {
        return 0;
    }

    save_file.write(reinterpret_cast<const char*>(output_buffer.data()), total_size);
    if (save_file.fail())
    {
        return 0;
    }

    overlapped->set_result(*in_data);
    return 1;
}

DLLEXPORT int UPLAY_SAVE_ReleaseGameList(UplayList* games_list)
{
    LOGGER_INFO(__FUNCTION__);

    if (!games_list)
    {
        LOGGER_ERROR("Called with a null argument");
        return 0;
    }

    for (size_t i = 0; i < games_list->count; ++i)
    {
        if (games_list->games[i])
        {
            delete[] games_list->games[i]->bufferstring;
            delete games_list->games[i];
        }
    }

    delete[] games_list->games;
    delete games_list;
    return 1;
}
