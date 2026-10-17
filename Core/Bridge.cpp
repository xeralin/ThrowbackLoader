#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#include <windows.h>

#include <bit>
#include <cstdint>
#include <cstring>

#include "Bridge.h"

namespace
{
constexpr DWORD BRIDGED_TIMESTAMPS[] = {0x5654d137, 0x56a7e86f, 0x5728f897, 0x57c9b536};
constexpr ULONG ADAPTER_BUFFER_BYTES = 32 * 1024;
constexpr int ADAPTER_ATTEMPTS = 3;
constexpr size_t SLOT_BYTES = 48;
constexpr size_t TRAMPOLINE_OFFSET = 16;

constexpr uint8_t PUSH_RBP_R15_R14[] = {0x55, 0x41, 0x57, 0x41, 0x56};

uint8_t g_mac[6];
decltype(&gethostname) g_gethostname;
decltype(&GetAdaptersAddresses) g_list_adapters;
decltype(&getaddrinfo) g_getaddrinfo;
decltype(&gethostbyname) g_gethostbyname;
decltype(&GetAdaptersInfo) g_get_adapters_info;

bool is_radmin(const wchar_t* name)
{
    for (; *name != L'\0'; ++name)
    {
        if (_wcsnicmp(name, L"radmin", 6) == 0)
        {
            return true;
        }
    }
    return false;
}

uint32_t radmin_ip()
{
    const HANDLE heap = GetProcessHeap();
    ULONG size = ADAPTER_BUFFER_BYTES;
    IP_ADAPTER_ADDRESSES* list = nullptr;
    ULONG error = ERROR_BUFFER_OVERFLOW;
    for (int attempt = 0; attempt < ADAPTER_ATTEMPTS && error == ERROR_BUFFER_OVERFLOW; ++attempt)
    {
        HeapFree(heap, 0, list);
        list = static_cast<IP_ADAPTER_ADDRESSES*>(HeapAlloc(heap, 0, size));
        if (list == nullptr)
        {
            return 0;
        }
        error = g_list_adapters(AF_INET, GAA_FLAG_SKIP_ANYCAST | GAA_FLAG_SKIP_MULTICAST | GAA_FLAG_SKIP_DNS_SERVER,
                                nullptr, list, &size);
    }
    uint32_t ip = 0;
    for (auto* adapter = error == NO_ERROR ? list : nullptr; adapter != nullptr && ip == 0; adapter = adapter->Next)
    {
        const auto* unicast = adapter->FirstUnicastAddress;
        if (adapter->OperStatus == IfOperStatusUp && unicast != nullptr && is_radmin(adapter->FriendlyName))
        {
            ip = reinterpret_cast<const sockaddr_in*>(unicast->Address.lpSockaddr)->sin_addr.s_addr;
        }
    }
    HeapFree(heap, 0, list);
    return ip;
}

bool is_own_host(const char* name)
{
    char host[256];
    return name != nullptr && _stricmp(name, "localhost") != 0 && g_gethostname(host, sizeof(host)) == 0 &&
           _stricmp(name, host) == 0;
}

int WSAAPI getaddrinfo_hook(const char* node, const char* service, const addrinfo* hints, addrinfo** result)
{
    const int error = g_getaddrinfo(node, service, hints, result);
    const uint32_t radmin = error == 0 && is_own_host(node) ? radmin_ip() : 0;
    sockaddr_in* first = nullptr;
    for (addrinfo* entry = radmin != 0 ? *result : nullptr; entry != nullptr; entry = entry->ai_next)
    {
        if (entry->ai_family != AF_INET)
        {
            continue;
        }
        auto* address = reinterpret_cast<sockaddr_in*>(entry->ai_addr);
        if (first == nullptr)
        {
            first = address;
        }
        else if (address->sin_addr.s_addr == radmin)
        {
            address->sin_addr.s_addr = first->sin_addr.s_addr;
            break;
        }
    }
    if (first != nullptr)
    {
        first->sin_addr.s_addr = radmin;
    }
    return error;
}

hostent* WSAAPI gethostbyname_hook(const char* name)
{
    hostent* host = g_gethostbyname(name);
    const uint32_t radmin =
        host != nullptr && host->h_addrtype == AF_INET && host->h_addr_list[0] != nullptr && is_own_host(name)
            ? radmin_ip()
            : 0;
    if (radmin == 0)
    {
        return host;
    }
    for (char** entry = host->h_addr_list + 1; *entry != nullptr; ++entry)
    {
        if (memcmp(*entry, &radmin, sizeof(radmin)) == 0)
        {
            memcpy(*entry, host->h_addr_list[0], sizeof(radmin));
            break;
        }
    }
    memcpy(host->h_addr_list[0], &radmin, sizeof(radmin));
    return host;
}

ULONG WINAPI get_adapters_info_hook(PIP_ADAPTER_INFO info, PULONG size)
{
    const ULONG error = g_get_adapters_info(info, size);
    for (auto* adapter = error == NO_ERROR ? info : nullptr; adapter != nullptr; adapter = adapter->Next)
    {
        if (adapter->AddressLength == sizeof(g_mac))
        {
            memcpy(adapter->Address, g_mac, sizeof(g_mac));
        }
    }
    return error;
}

void make_mac()
{
    LARGE_INTEGER counter;
    QueryPerformanceCounter(&counter);
    uint64_t x = static_cast<uint64_t>(counter.QuadPart) ^ (static_cast<uint64_t>(GetCurrentProcessId()) << 32) ^
                 GetTickCount64();
    for (uint8_t& byte : g_mac)
    {
        x ^= x << 13;
        x ^= x >> 7;
        x ^= x << 17;
        byte = static_cast<uint8_t>(x);
    }
    g_mac[0] = (g_mac[0] & 0xfc) | 0x02;
}

void write_jump(uint8_t* at, const void* target)
{
    at[0] = 0xFF;
    at[1] = 0x25;
    memset(at + 2, 0, 4);
    memcpy(at + 6, &target, sizeof(target));
}

uint8_t* allocate_near(HMODULE module, size_t bytes)
{
    SYSTEM_INFO info;
    GetSystemInfo(&info);
    const auto anchor = reinterpret_cast<uintptr_t>(module);
    const uintptr_t step = info.dwAllocationGranularity;
    for (uintptr_t address = anchor - 0x10000000; address > anchor - 0x70000000; address -= step)
    {
        if (void* memory = VirtualAlloc(reinterpret_cast<void*>(address), bytes, MEM_COMMIT | MEM_RESERVE,
                                        PAGE_READWRITE))
        {
            return static_cast<uint8_t*>(memory);
        }
    }
    return nullptr;
}

template <typename Function> void install_hook(HMODULE module, const char* name, Function detour, Function& original)
{
    auto* target = reinterpret_cast<uint8_t*>(GetProcAddress(module, name));
    if (target == nullptr || memcmp(target, PUSH_RBP_R15_R14, sizeof(PUSH_RBP_R15_R14)) != 0)
    {
        return;
    }
    uint8_t* const slot = allocate_near(module, SLOT_BYTES);
    if (slot == nullptr)
    {
        return;
    }
    uint8_t* const trampoline = slot + TRAMPOLINE_OFFSET;
    write_jump(slot, reinterpret_cast<const void*>(detour));
    memcpy(trampoline, target, sizeof(PUSH_RBP_R15_R14));
    write_jump(trampoline + sizeof(PUSH_RBP_R15_R14), target + sizeof(PUSH_RBP_R15_R14));
    DWORD protection;
    VirtualProtect(slot, SLOT_BYTES, PAGE_EXECUTE_READ, &protection);
    FlushInstructionCache(GetCurrentProcess(), slot, SLOT_BYTES);
    original = reinterpret_cast<Function>(trampoline);
    const int32_t displacement = static_cast<int32_t>(slot - (target + 5));
    VirtualProtect(target, 5, PAGE_EXECUTE_READWRITE, &protection);
    target[0] = 0xE9;
    memcpy(target + 1, &displacement, sizeof(displacement));
    VirtualProtect(target, 5, protection, &protection);
    FlushInstructionCache(GetCurrentProcess(), target, 5);
}

void install_hooks()
{
    const HMODULE ws2 = GetModuleHandleW(L"ws2_32.dll");
    const HMODULE iphlpapi = GetModuleHandleW(L"iphlpapi.dll");
    if (ws2 == nullptr || iphlpapi == nullptr)
    {
        return;
    }
    g_gethostname = std::bit_cast<decltype(g_gethostname)>(GetProcAddress(ws2, "gethostname"));
    g_list_adapters = std::bit_cast<decltype(g_list_adapters)>(GetProcAddress(iphlpapi, "GetAdaptersAddresses"));
    if (g_gethostname == nullptr || g_list_adapters == nullptr)
    {
        return;
    }
    make_mac();
    install_hook(ws2, "getaddrinfo", getaddrinfo_hook, g_getaddrinfo);
    install_hook(ws2, "gethostbyname", gethostbyname_hook, g_gethostbyname);
    install_hook(iphlpapi, "GetAdaptersInfo", get_adapters_info_hook, g_get_adapters_info);
}
}

void start_bridge()
{
    const auto base = reinterpret_cast<uintptr_t>(GetModuleHandleW(nullptr));
    const auto* headers =
        reinterpret_cast<const IMAGE_NT_HEADERS*>(base + reinterpret_cast<const IMAGE_DOS_HEADER*>(base)->e_lfanew);
    for (const DWORD timestamp : BRIDGED_TIMESTAMPS)
    {
        if (headers->FileHeader.TimeDateStamp == timestamp)
        {
            install_hooks();
        }
    }
}
