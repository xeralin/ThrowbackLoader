#include "Config.h"

#include "ConfigParse.h"
#include "Logger.h"

#include <filesystem>
#include <format>
#include <fstream>

static void write_default_config_file(const std::filesystem::path& config_path)
{
    std::ofstream file(config_path);
    file << std::format(R"([Config]
username = '{}' # Max 16 characters
user_id = ''

[Launch]
args = '{}'
autorun = []
)",
                        CONSTS::DEFAULT_USERNAME, CONSTS::DEFAULT_ARGS);
}

static void assign_if_value(std::string& var, const std::optional<std::string_view>& opt)
{
    if (opt.has_value() && !opt->empty())
    {
        var = *opt;
    }
}

void UplayConfig::Initialize()
{
    std::error_code error_code;
    const auto current_path = std::filesystem::current_path(error_code);
    const auto config_path = current_path / CONSTS::DEFAULT_CONFIG_FILENAME;
    LOGGER_INFO("Config path: {}", config_path.string());

    const auto game_folder = current_path.filename().string();
    if (!error_code && !game_folder.empty())
    {
        m_UserId = std::format("{}{}", CONSTS::USER_ID_PREFIX, game_folder);
    }

    if (error_code || !std::filesystem::exists(config_path, error_code))
    {
        LOGGER_WARNING("Config.toml not found, writing the default");
        write_default_config_file(config_path);
        return;
    }

    const auto parse_result = tbl::parse_config(config_path.string());
    if (!parse_result)
    {
        LOGGER_WARNING("Config.toml is malformed, {}", parse_result.error().description());
        return;
    }

    const auto& toml_table = parse_result.table();
    assign_if_value(m_Username, toml_table["Config"]["username"].value<std::string_view>());
    assign_if_value(m_UserId, toml_table["Config"]["user_id"].value<std::string_view>());

    LOGGER_INFO("Username: {}", m_Username);
}
