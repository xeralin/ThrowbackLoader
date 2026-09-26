#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <shellapi.h>
#include <tlhelp32.h>

#include <array>
#include <string>
#include <string_view>
#include <vector>

#include "ConfigParse.h"
#include "Consts.h"

namespace
{
constexpr std::array<const wchar_t*, 4> EXECUTABLES = {
    L"RainbowSixGame.exe",
    L"RainbowSix.exe",
    L"RainbowSix_DX11.exe",
    L"RainbowSix_Vulkan.exe",
};

constexpr DWORD SAFETY_POLL_INTERVAL_MS = 3000;
constexpr int SAFETY_MISSING_POLLS = 2;

std::wstring widen(std::string_view text)
{
    if (text.empty())
    {
        return {};
    }
    const int length = MultiByteToWideChar(CP_UTF8, 0, text.data(), static_cast<int>(text.size()), nullptr, 0);
    std::wstring result(static_cast<size_t>(length), L'\0');
    MultiByteToWideChar(CP_UTF8, 0, text.data(), static_cast<int>(text.size()), result.data(), length);
    return result;
}

struct LaunchConfig
{
    std::wstring args;
    std::vector<std::wstring> autorun;
};

LaunchConfig read_launch_config()
{
    LaunchConfig config;
    config.args = widen(CONSTS::DEFAULT_ARGS);

    const auto parsed = tbl::parse_config(CONSTS::DEFAULT_CONFIG_FILENAME);
    if (!parsed)
    {
        return config;
    }

    const auto& table = parsed.table();
    if (const auto value = table["Launch"]["args"].value<std::string>(); value && !value->empty())
    {
        config.args = widen(*value);
    }
    if (const auto* autorun = table["Launch"]["autorun"].as_array())
    {
        for (const auto& entry : *autorun)
        {
            if (const auto path = entry.value<std::string>(); path && !path->empty())
            {
                config.autorun.push_back(widen(*path));
            }
        }
    }
    return config;
}

void start_program(const std::wstring& path)
{
    std::wstring command_line = L"\"" + path + L"\"";

    STARTUPINFOW startup_info{};
    startup_info.cb = sizeof(startup_info);
    PROCESS_INFORMATION process_info{};

    if (CreateProcessW(nullptr, command_line.data(), nullptr, nullptr, FALSE, 0, nullptr, nullptr, &startup_info,
                       &process_info) != FALSE)
    {
        CloseHandle(process_info.hThread);
        CloseHandle(process_info.hProcess);
        return;
    }

    if (GetLastError() != ERROR_ELEVATION_REQUIRED)
    {
        return;
    }

    wchar_t full_path[MAX_PATH];
    const wchar_t* file = path.c_str();
    const DWORD full_path_length = GetFullPathNameW(path.c_str(), MAX_PATH, full_path, nullptr);
    if (full_path_length != 0 && full_path_length < MAX_PATH)
    {
        file = full_path;
    }

    SHELLEXECUTEINFOW shell_info{};
    shell_info.cbSize = sizeof(shell_info);
    shell_info.lpVerb = L"runas";
    shell_info.lpFile = file;
    shell_info.nShow = SW_SHOWNORMAL;
    ShellExecuteExW(&shell_info);
}

void set_working_directory_to_executable()
{
    wchar_t path[MAX_PATH];
    const DWORD length = GetModuleFileNameW(nullptr, path, MAX_PATH);
    if (length == 0 || length >= MAX_PATH)
    {
        return;
    }
    std::wstring directory(path, length);
    const auto separator = directory.find_last_of(L"\\/");
    if (separator != std::wstring::npos)
    {
        directory.resize(separator);
        SetCurrentDirectoryW(directory.c_str());
    }
}

bool is_game_process(const wchar_t* executable)
{
    for (const auto name : EXECUTABLES)
    {
        if (lstrcmpiW(executable, name) == 0)
        {
            return true;
        }
    }
    return false;
}

struct WindowQuery
{
    const std::vector<DWORD>* pids;
    bool present;
};

BOOL CALLBACK on_window(HWND window, LPARAM parameter)
{
    auto* query = reinterpret_cast<WindowQuery*>(parameter);
    if (!IsWindowVisible(window) || GetWindow(window, GW_OWNER) != nullptr)
    {
        return TRUE;
    }
    DWORD pid = 0;
    GetWindowThreadProcessId(window, &pid);
    for (const DWORD game_pid : *query->pids)
    {
        if (game_pid == pid)
        {
            query->present = true;
            return FALSE;
        }
    }
    return TRUE;
}

bool game_window_present()
{
    std::vector<DWORD> pids;
    const HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snapshot == INVALID_HANDLE_VALUE)
    {
        return false;
    }
    PROCESSENTRY32W entry{};
    entry.dwSize = sizeof(entry);
    if (Process32FirstW(snapshot, &entry))
    {
        do
        {
            if (is_game_process(entry.szExeFile))
            {
                pids.push_back(entry.th32ProcessID);
            }
        } while (Process32NextW(snapshot, &entry));
    }
    CloseHandle(snapshot);

    if (pids.empty())
    {
        return false;
    }

    WindowQuery query{&pids, false};
    EnumWindows(on_window, reinterpret_cast<LPARAM>(&query));
    return query.present;
}

void terminate_game_processes()
{
    const HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snapshot == INVALID_HANDLE_VALUE)
    {
        return;
    }
    PROCESSENTRY32W entry{};
    entry.dwSize = sizeof(entry);
    if (Process32FirstW(snapshot, &entry))
    {
        do
        {
            if (is_game_process(entry.szExeFile))
            {
                const HANDLE process = OpenProcess(PROCESS_TERMINATE, FALSE, entry.th32ProcessID);
                if (process != nullptr)
                {
                    TerminateProcess(process, 0);
                    CloseHandle(process);
                }
            }
        } while (Process32NextW(snapshot, &entry));
    }
    CloseHandle(snapshot);
}

const wchar_t* find_installed_executable()
{
    for (const auto executable : EXECUTABLES)
    {
        const DWORD attributes = GetFileAttributesW(executable);
        if (attributes != INVALID_FILE_ATTRIBUTES && (attributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
        {
            return executable;
        }
    }
    return nullptr;
}

struct GameJob
{
    HANDLE job;
    HANDLE port;
};

GameJob create_game_job()
{
    GameJob game_job{CreateJobObjectW(nullptr, nullptr), nullptr};
    if (game_job.job == nullptr)
    {
        return game_job;
    }
    game_job.port = CreateIoCompletionPort(INVALID_HANDLE_VALUE, nullptr, 0, 1);
    if (game_job.port == nullptr)
    {
        CloseHandle(game_job.job);
        game_job.job = nullptr;
        return game_job;
    }
    JOBOBJECT_ASSOCIATE_COMPLETION_PORT info{};
    info.CompletionKey = game_job.job;
    info.CompletionPort = game_job.port;
    if (SetInformationJobObject(game_job.job, JobObjectAssociateCompletionPortInformation, &info, sizeof(info)) ==
        FALSE)
    {
        CloseHandle(game_job.port);
        CloseHandle(game_job.job);
        game_job = {nullptr, nullptr};
    }
    return game_job;
}

HANDLE start_game(const wchar_t* executable, const std::wstring& args, HANDLE job)
{
    std::wstring command_line = L"\"";
    command_line += executable;
    command_line += L"\" ";
    command_line += args;

    STARTUPINFOW startup_info{};
    startup_info.cb = sizeof(startup_info);
    PROCESS_INFORMATION process_info{};

    if (CreateProcessW(nullptr, command_line.data(), nullptr, nullptr, FALSE, CREATE_SUSPENDED, nullptr, nullptr,
                       &startup_info, &process_info) == FALSE)
    {
        return nullptr;
    }
    if (job != nullptr)
    {
        AssignProcessToJobObject(job, process_info.hProcess);
    }
    ResumeThread(process_info.hThread);
    CloseHandle(process_info.hThread);
    return process_info.hProcess;
}

void wait_for_game_exit(const GameJob& game_job)
{
    bool window_seen = false;
    int missing_polls = 0;
    for (;;)
    {
        DWORD code = 0;
        ULONG_PTR key = 0;
        LPOVERLAPPED overlapped = nullptr;
        if (GetQueuedCompletionStatus(game_job.port, &code, &key, &overlapped, SAFETY_POLL_INTERVAL_MS) != FALSE)
        {
            if (code == JOB_OBJECT_MSG_ACTIVE_PROCESS_ZERO)
            {
                return;
            }
            continue;
        }
        if (GetLastError() != WAIT_TIMEOUT)
        {
            return;
        }
        if (game_window_present())
        {
            window_seen = true;
            missing_polls = 0;
        }
        else if (window_seen && ++missing_polls >= SAFETY_MISSING_POLLS)
        {
            TerminateJobObject(game_job.job, 0);
            return;
        }
    }
}

void show_error(const std::wstring& message)
{
    MessageBoxW(nullptr, message.c_str(), L"ThrowbackLoader", MB_OK | MB_ICONERROR);
}
}

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
    set_working_directory_to_executable();
    terminate_game_processes();

    const LaunchConfig config = read_launch_config();

    const wchar_t* executable = find_installed_executable();
    if (executable == nullptr)
    {
        show_error(L"No R6S executable was found in this folder");
        return 1;
    }

    for (const auto& program : config.autorun)
    {
        start_program(program);
    }

    const GameJob game_job = create_game_job();
    const HANDLE game_process = start_game(executable, config.args, game_job.job);
    if (game_process == nullptr)
    {
        show_error(std::wstring(L"Failed to launch ") + executable);
        return 1;
    }

    if (game_job.job != nullptr)
    {
        CloseHandle(game_process);
        wait_for_game_exit(game_job);
    }
    else
    {
        WaitForSingleObject(game_process, INFINITE);
        CloseHandle(game_process);
        terminate_game_processes();
    }
    return 0;
}
