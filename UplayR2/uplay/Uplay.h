#ifndef UPLAY_R2_H
#define UPLAY_R2_H

#include "Core.h"
#include <map>
#include <mutex>
#include <queue>
#include <vector>

inline UplayConfig g_uplay_config;

struct CallbackData
{
    CallbackData(void* fn, void* data, int value)
        : callback(reinterpret_cast<void (*)(int, void*)>(fn)), result(value), context_data(data)
    {
    }

    void (*callback)(int, void*);
    int result;
    void* context_data;
};

struct StorageHandle
{
    unsigned flags;
    std::filesystem::path path;
    std::vector<char> data;
};

struct ContextData
{
    std::queue<CallbackData> callbacks;
    std::mutex callbacks_mutex;
    std::map<int, StorageHandle> storage_files;
    int next_storage_handle = 1;
    std::mutex storage_mutex;

    void push_callback(void* callback, void* data, int result)
    {
        std::lock_guard<std::mutex> lock(callbacks_mutex);
        callbacks.emplace(callback, data, result);
    }
};

constexpr int UPC_ASYNC_PENDING = 0x10000;

#endif
