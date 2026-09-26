#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")"

major=$(grep -oP 'TB_LOADER_MAJOR\s+\K\d+' Core/Version.h)
minor=$(grep -oP 'TB_LOADER_MINOR\s+\K\d+' Core/Version.h)
patch=$(grep -oP 'TB_LOADER_PATCH\s+\K\d+' Core/Version.h)
metadata=$(grep -oP 'TB_LOADER_METADATA\s+"\K[^"]*' Core/Version.h || true)
version="${major}.${minor}.${patch}"
[ -n "${metadata}" ] && version="${version}-${metadata}"

emu="${STEAM_API64:-Bundle/steam_api64.dll}"
if [ ! -f "${emu}" ]; then
    echo "steam_api64.dll not found at ${emu}. Set STEAM_API64=/path to override." >&2
    exit 1
fi

cmake -B build -G Ninja -DCMAKE_TOOLCHAIN_FILE=toolchain-x86_64-w64-mingw32.cmake
cmake --build build

stage=$(mktemp -d)
trap 'rm -rf "${stage}"' EXIT

cp build/uplay_r1_loader64.dll \
   build/upc_r1_loader64.dll \
   build/upc_r2_loader64.dll \
   build/uplay_r2_loader64.dll \
   build/defaultargs.dll \
   "${stage}/"
cp "${emu}" "${stage}/steam_api64.dll"
cp Bundle/Config.toml "${stage}/Config.toml"
cp build/LaunchR6.exe "${stage}/LaunchR6.exe"

zip="${PWD}/ThrowbackLoader-${version}.zip"
rm -f "${zip}"
( cd "${stage}" && zip -q "${zip}" * )
