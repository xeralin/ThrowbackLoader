#ifndef CORE_LOGGER_H
#define CORE_LOGGER_H

#ifndef NDEBUG

#include <chrono>
#include <format>
#include <iostream>

class Logger
{
    enum class LOG_LEVEL
    {
        INFO,
        WARN,
        ERR
    };

    void print_level(LOG_LEVEL level) const;
    void print_timestamp() const;

    template <typename... Args> void log(LOG_LEVEL level, std::format_string<Args...> fmt, Args&&... args) const
    {
        print_level(level);
        print_timestamp();
        std::cout << std::format(fmt, std::forward<Args>(args)...) << "\n";
    }

  public:
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;
    Logger(Logger&&) = delete;
    Logger& operator=(Logger&&) = delete;

    static Logger& Instance();

    template <typename... Args> void info(std::format_string<Args...> fmt, Args&&... args) const
    {
        log(LOG_LEVEL::INFO, fmt, std::forward<Args>(args)...);
    }

    template <typename... Args> void warn(std::format_string<Args...> fmt, Args&&... args) const
    {
        log(LOG_LEVEL::WARN, fmt, std::forward<Args>(args)...);
    }

    template <typename... Args> void err(std::format_string<Args...> fmt, Args&&... args) const
    {
        log(LOG_LEVEL::ERR, fmt, std::forward<Args>(args)...);
    }

  private:
    Logger() = default;
};

template <typename... Args> void LOGGER_INFO(std::format_string<Args...> fmt, Args&&... args)
{
    Logger::Instance().info(fmt, std::forward<Args>(args)...);
}

template <typename... Args> void LOGGER_WARNING(std::format_string<Args...> fmt, Args&&... args)
{
    Logger::Instance().warn(fmt, std::forward<Args>(args)...);
}

template <typename... Args> void LOGGER_ERROR(std::format_string<Args...> fmt, Args&&... args)
{
    Logger::Instance().err(fmt, std::forward<Args>(args)...);
}

#else

#define LOGGER_INFO(...)
#define LOGGER_WARNING(...)
#define LOGGER_ERROR(...)

#endif

void handle_windows_calls(int reason);

#endif
