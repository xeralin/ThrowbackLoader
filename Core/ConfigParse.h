#ifndef CORE_CONFIG_PARSE_H
#define CORE_CONFIG_PARSE_H

#include <algorithm>
#include <fstream>
#include <sstream>
#include <string>

#define TOML_EXCEPTIONS 0
#include <toml++/toml.hpp>

namespace tbl
{
inline toml::parse_result parse_config(const std::string& path)
{
    auto result = toml::parse_file(path);
    if (result)
    {
        return result;
    }

    std::ifstream file(path, std::ios::binary);
    if (!file)
    {
        return result;
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string text = buffer.str();
    std::replace(text.begin(), text.end(), '"', '\'');
    return toml::parse(text);
}
}

#endif
