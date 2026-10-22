#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")"

major=$(grep -oP 'TB_LOADER_MAJOR\s+\K\d+' Core/Version.h)
minor=$(grep -oP 'TB_LOADER_MINOR\s+\K\d+' Core/Version.h)
patch=$(grep -oP 'TB_LOADER_PATCH\s+\K\d+' Core/Version.h)
version="${major}.${minor}.${patch}"

cmake -B build -G Ninja -DCMAKE_TOOLCHAIN_FILE=toolchain-x86_64-w64-mingw32.cmake
cmake --build build

zip="ThrowbackLoader-${version}.zip"
rm -f "${zip}"
zip -qjMM "${zip}" build/uplay_r1_loader64.dll build/upc_r1_loader64.dll build/upc_r2_loader64.dll \
    build/uplay_r2_loader64.dll build/defaultargs.dll build/LaunchR6.exe Bundle/steam_api64.dll Bundle/Config.toml
