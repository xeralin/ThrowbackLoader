#include "Core.h"

#include <io.h>

#include <filesystem>
#include <fstream>
#include <optional>

std::filesystem::path get_saves_path(std::optional<uint32_t> app_id)
{
    std::error_code error_code;
    auto saves_path = std::filesystem::current_path(error_code) / "Saves";
    if (app_id.has_value())
    {
        saves_path /= std::to_string(app_id.value());
    }

    std::filesystem::create_directories(saves_path, error_code);
    return saves_path;
}

std::filesystem::path get_temporary_path(std::filesystem::path path)
{
    path += ".tmp";
    return path;
}

bool commit_file(int file, const std::filesystem::path& path)
{
    const bool flushed = _commit(file) == 0;
    return _close(file) == 0 && flushed &&
           MoveFileExW(get_temporary_path(path).c_str(), path.c_str(),
                       MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH);
}

void reset_streaminginstall()
{
    std::error_code error_code;
    auto streaminginstall_path = std::filesystem::current_path(error_code) / "streaminginstall.ini";

    if (error_code || !std::filesystem::exists(streaminginstall_path, error_code))
    {
        return;
    }

    std::ofstream si_file(streaminginstall_path);
    si_file << "[MissionToChunk]\n[FileToChunk]";
}

void remove_sen_client()
{
    std::error_code error_code;
    auto sen_client_path = std::filesystem::current_path(error_code) / "sen_client.dll";
    if (!error_code)
    {
        std::filesystem::remove(sen_client_path, error_code);
    }
}
