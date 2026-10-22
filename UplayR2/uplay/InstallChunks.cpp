#include "Uplay.h"

struct ChunkList
{
    uint32_t number_chunks;
    uint32_t padding = 0;
    uint32_t* chunk_list;
};

DLLEXPORT int UPC_InstallChunkListFree(ContextData* context, ChunkList* chunk_list)
{
    if (!context || !chunk_list)
    {
        return -2;
    }
    delete chunk_list;
    return 0;
}

DLLEXPORT int UPC_InstallChunkListGet(ContextData* context, ChunkList** out_chunk_list)
{
    if (!context || !out_chunk_list)
    {
        return -2;
    }
    *out_chunk_list = new ChunkList();
    return 0;
}

DLLEXPORT int UPC_InstallChunksOrderUpdate()
{
    return 0;
}

DLLEXPORT int UPC_InstallChunksOrderUpdate_Extended()
{
    return 0;
}

DLLEXPORT int UPC_InstallChunksPresenceCheck()
{
    return 0;
}
