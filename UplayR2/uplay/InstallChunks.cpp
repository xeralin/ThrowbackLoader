#include "Uplay.h"

struct ChunkList
{
    uint32_t number_chunks;
    uint32_t padding = 0;
    uint32_t* chunk_list;
};

DLLEXPORT int UPC_InstallChunkListFree(ContextData* context, ChunkList* chunk_list)
{
    LOGGER_INFO(__FUNCTION__);
    if (!context || !chunk_list)
    {
        return -2;
    }
    delete[] chunk_list->chunk_list;
    delete chunk_list;
    return 0;
}

DLLEXPORT int UPC_InstallChunkListGet(ContextData* context, ChunkList** out_chunk_list)
{
    LOGGER_INFO(__FUNCTION__);
    if (!context || !out_chunk_list)
    {
        return -2;
    }
    *out_chunk_list = new ChunkList();
    return 0;
}

DLLEXPORT int UPC_InstallChunksOrderUpdate()
{
    LOGGER_INFO(__FUNCTION__);
    return 0;
}

DLLEXPORT int UPC_InstallChunksOrderUpdate_Extended()
{
    LOGGER_INFO(__FUNCTION__);
    return 0;
}

DLLEXPORT int UPC_InstallChunksPresenceCheck()
{
    LOGGER_INFO(__FUNCTION__);
    return 0;
}
