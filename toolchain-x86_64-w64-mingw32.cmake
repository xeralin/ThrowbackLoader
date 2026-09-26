set(CMAKE_SYSTEM_NAME Windows)
set(CMAKE_SYSTEM_PROCESSOR x86_64)

set(TOOLCHAIN_PREFIX x86_64-w64-mingw32)
find_program(CMAKE_CXX_COMPILER ${TOOLCHAIN_PREFIX}-g++)
find_program(CMAKE_RC_COMPILER  ${TOOLCHAIN_PREFIX}-windres)

if(NOT CMAKE_CXX_COMPILER)
    message(FATAL_ERROR
        "${TOOLCHAIN_PREFIX}-g++ not found on PATH. Install it via Fedora "
        "'mingw64-gcc-c++' (distrobox) or Homebrew 'mingw-w64'.")
endif()

execute_process(
    COMMAND ${CMAKE_CXX_COMPILER} -print-sysroot
    OUTPUT_VARIABLE _mingw_sysroot
    OUTPUT_STRIP_TRAILING_WHITESPACE)
if(_mingw_sysroot AND IS_DIRECTORY "${_mingw_sysroot}")
    set(CMAKE_FIND_ROOT_PATH "${_mingw_sysroot}")
endif()

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)
