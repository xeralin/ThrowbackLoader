#include "Core.h"

#ifndef NDEBUG
Logger& Logger::Instance()
{
    static Logger instance;
    return instance;
}

void Logger::print_level(LOG_LEVEL level) const
{
    switch (level)
    {
    case LOG_LEVEL::ERR:
        std::cout << "[ERROR]";
        return;
    case LOG_LEVEL::WARN:
        std::cout << "[WARNING]";
        return;
    case LOG_LEVEL::INFO:
        std::cout << "[INFO]";
        return;
    }
}

void Logger::print_timestamp() const
{
    std::cout << std::format(" [{:%Y-%m-%d %H:%M:%S}] ", std::chrono::system_clock::now());
}
#endif

static void create_console()
{
#ifndef NDEBUG
    AllocConsole();
    FILE* console = nullptr;
    freopen_s(&console, "CONOUT$", "w", stdout);
    SetConsoleTitleA("ThrowbackLoader");
#endif
}

static void free_console()
{
#ifndef NDEBUG
    fclose(stdout);
    FreeConsole();
#endif
}

void handle_windows_calls(int reason)
{
    if (reason == DLL_PROCESS_ATTACH)
    {
        create_console();
        LOGGER_INFO("ThrowbackLoader version {}", get_version_string());
    }
    else if (reason == DLL_PROCESS_DETACH)
    {
        free_console();
    }
}
