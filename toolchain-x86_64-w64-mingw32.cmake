set(CMAKE_SYSTEM_NAME Windows)
set(CMAKE_SYSTEM_PROCESSOR x86_64)

set(TOOLCHAIN_PREFIX x86_64-w64-mingw32)
find_program(CMAKE_CXX_COMPILER ${TOOLCHAIN_PREFIX}-g++)

if(NOT CMAKE_CXX_COMPILER)
    message(FATAL_ERROR
        "${TOOLCHAIN_PREFIX}-g++ not found on PATH. Install it via Fedora "
        "'mingw64-gcc-c++' (distrobox) or Homebrew 'mingw-w64'.")
endif()
