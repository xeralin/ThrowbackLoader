#include "Uplay.h"

DLLEXPORT int UPC_StoreCheckout()
{
    return 0;
}

DLLEXPORT bool UPC_StoreIsEnabled()
{
    return false;
}

DLLEXPORT int UPC_StoreIsEnabled_Extended(ContextData* context, bool* out_enabled)
{
    if (!context || !out_enabled)
    {
        return -2;
    }

    *out_enabled = false;
    return 0;
}

DLLEXPORT int UPC_StoreLanguageSet()
{
    return 0;
}

DLLEXPORT int UPC_StorePartnerGet()
{
    return 0;
}

DLLEXPORT int UPC_StorePartnerGet_Extended()
{
    return 0;
}

DLLEXPORT int UPC_StoreProductDetailsShow()
{
    return 0;
}

DLLEXPORT int UPC_StoreProductListFree()
{
    return 0;
}

DLLEXPORT int UPC_StoreProductListGet()
{
    return 0;
}

DLLEXPORT int UPC_StoreProductsShow()
{
    return 0;
}
