#include "Consts.h"
#include "Utils.h"
#include "Logger.h"

#include <filesystem>
#include <fstream>
#include <optional>

std::filesystem::path get_saves_path(std::optional<uint32_t> app_id)
{
    std::error_code error_code;
    auto saves_path = std::filesystem::current_path(error_code) / std::filesystem::path(CONSTS::SAVE_FILES_FOLDER);
    if (app_id.has_value())
    {
        saves_path /= std::to_string(app_id.value());
    }

    if (!std::filesystem::exists(saves_path, error_code))
    {
        std::filesystem::create_directories(saves_path, error_code);
    }
    return saves_path;
}

static constexpr const char* DEFAULT_STREAMINGINSTALL_FILE = R"([MissionToChunk]
[FileToChunk])";

void reset_streaminginstall()
{
    std::error_code error_code;
    auto streaminginstall_path =
        std::filesystem::current_path(error_code) / std::filesystem::path("streaminginstall.ini");

    if (error_code || !std::filesystem::exists(streaminginstall_path, error_code))
    {
        LOGGER_INFO("Missing streaminginstall.ini");
        return;
    }

    std::ofstream si_file(streaminginstall_path);
    if (!si_file.is_open())
    {
        LOGGER_ERROR("Failed to open streaminginstall.ini for writing");
        return;
    }

    LOGGER_INFO("Applying streaminginstall.ini workaround");
    si_file << DEFAULT_STREAMINGINSTALL_FILE;

    if (!si_file)
    {
        LOGGER_ERROR("Failed to write to streaminginstall.ini");
    }
}

void remove_sen_client()
{
    std::error_code error_code;
    auto sen_client_path = std::filesystem::current_path(error_code) / std::filesystem::path("sen_client.dll");

    if (error_code || !std::filesystem::exists(sen_client_path, error_code))
    {
        LOGGER_INFO("Missing sen_client.dll");
        return;
    }

    LOGGER_INFO("Removing sen_client.dll");
    if (!std::filesystem::remove(sen_client_path, error_code))
    {
        LOGGER_ERROR("Failed to remove sen_client.dll");
    }
}
