#ifndef CORE_H
#define CORE_H

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

#include "CloseGuard.h"
#include "Config.h"
#include "Consts.h"
#include "Logger.h"
#include "Utils.h"

#define DLLEXPORT extern "C" __declspec(dllexport)

#include "Version.h"

[[nodiscard]] const char* get_version_string();

#endif
