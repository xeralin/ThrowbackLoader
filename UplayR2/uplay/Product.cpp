#include "Uplay.h"

struct Product
{
    uint32_t appid;
    uint32_t type;
    uint32_t unknown1;
    uint32_t unk_3 = 3;
    uint32_t unk_0 = 0;
    uint32_t unk_1 = 1;
};

struct ProductList
{
    uint32_t number = 0;
    uint32_t padding = 0;
    Product** out;
};

DLLEXPORT int UPC_ProductConsume()
{
    LOGGER_INFO(__FUNCTION__);
    return 0;
}

DLLEXPORT int UPC_ProductConsumeSignatureFree()
{
    LOGGER_INFO(__FUNCTION__);
    return 0;
}

DLLEXPORT int UPC_ProductListFree(ContextData* context, ProductList* in_product_list)
{
    LOGGER_INFO(__FUNCTION__);

    if (in_product_list)
    {
        for (unsigned i = 0; i < in_product_list->number; ++i)
        {
            delete in_product_list->out[i];
        }

        delete[] in_product_list->out;
    }

    delete in_product_list;
    return 0;
}

DLLEXPORT int UPC_ProductListGet(ContextData* context, char* in_opt_user_id_utf8, unsigned in_filter,
                                 ProductList** out_product_list, void* in_callback,
                                 void* in_callback_data)
{
    LOGGER_INFO(__FUNCTION__);

    if (!context || !out_product_list)
    {
        return -2;
    }

    {
        std::lock_guard<std::mutex> lock(context->callbacks_mutex);
        context->callbacks.push(CallbackData(in_callback, in_callback_data, 0));
    }

    ProductList* pr_list = new ProductList();
    pr_list->out = new Product*[1];
    pr_list->out[0] = new Product{g_uplay_config.GetAppId(), 1, 1};
    pr_list->number = 1;

    *out_product_list = pr_list;
    return UPC_ASYNC_PENDING;
}
