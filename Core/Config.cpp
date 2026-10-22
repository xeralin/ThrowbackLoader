#include "Config.h"

#include "ConfigParse.h"

#include <filesystem>
#include <format>
#include <fstream>

static void write_default_config_file()
{
    std::ofstream file(CONSTS::DEFAULT_CONFIG_FILENAME);
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

    const auto game_folder = current_path.filename().string();
    if (!error_code && !game_folder.empty())
    {
        m_UserId = std::format("Throwback_{}", game_folder);
    }

    if (error_code || !std::filesystem::exists(CONSTS::DEFAULT_CONFIG_FILENAME, error_code))
    {
        write_default_config_file();
        return;
    }

    const auto parse_result = tbl::parse_config(CONSTS::DEFAULT_CONFIG_FILENAME);
    if (!parse_result)
    {
        return;
    }

    const auto& toml_table = parse_result.table();
    assign_if_value(m_Username, toml_table["Config"]["username"].value<std::string_view>());
    assign_if_value(m_UserId, toml_table["Config"]["user_id"].value<std::string_view>());
}
