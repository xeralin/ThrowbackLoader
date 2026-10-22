#ifndef CORE_UTILS_H
#define CORE_UTILS_H

#include <cstdint>
#include <filesystem>
#include <optional>

[[nodiscard]] std::filesystem::path get_saves_path(std::optional<uint32_t> app_id = std::nullopt);
[[nodiscard]] std::filesystem::path get_temporary_path(std::filesystem::path path);
[[nodiscard]] bool commit_file(int file, const std::filesystem::path& path);
void reset_streaminginstall();
void remove_sen_client();

#endif
