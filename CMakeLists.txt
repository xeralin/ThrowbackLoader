cmake_minimum_required(VERSION 3.20)
project(ThrowbackLoader LANGUAGES CXX RC)

if(NOT CMAKE_SYSTEM_NAME STREQUAL "Windows")
    message(FATAL_ERROR
        "This build cross-compiles to Windows. Configure with:\n"
        "  -DCMAKE_TOOLCHAIN_FILE=toolchain-x86_64-w64-mingw32.cmake")
endif()

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

add_link_options(-static -static-libgcc -static-libstdc++)
add_link_options($<$<NOT:$<CONFIG:Debug>>:-s>)
add_link_options(-Wl,--no-insert-timestamp)

set(TBL_DEPS_INC "${CMAKE_BINARY_DIR}/third_party")
set(TBL_TOML_URL "https://raw.githubusercontent.com/marzer/tomlplusplus/v3.4.0/toml.hpp")
set(TBL_TOML_HPP "${TBL_DEPS_INC}/toml++/toml.hpp")
if(NOT EXISTS "${TBL_TOML_HPP}")
    file(DOWNLOAD "${TBL_TOML_URL}" "${TBL_TOML_HPP}" TLS_VERIFY ON STATUS _st)
    list(GET _st 0 _code)
    if(NOT _code EQUAL 0)
        list(GET _st 1 _msg)
        file(REMOVE "${TBL_TOML_HPP}")
        message(FATAL_ERROR "Download failed (${TBL_TOML_URL}): ${_msg}")
    endif()
endif()

add_library(tbl_core STATIC
    Core/CloseGuard.cpp
    Core/Config.cpp
    Core/Core.cpp
    Core/Logger.cpp
    Core/Utils.cpp)
target_include_directories(tbl_core PUBLIC Core "${TBL_DEPS_INC}")

function(tbl_add_dll target out_name)
    add_library(${target} SHARED ${ARGN})
    target_link_libraries(${target} PRIVATE tbl_core)
    set_target_properties(${target} PROPERTIES
        PREFIX ""
        OUTPUT_NAME "${out_name}"
        SUFFIX ".dll")
endfunction()

tbl_add_dll(uplay_r1 uplay_r1_loader64
    Core/DllMain.cpp
    UplayR1/uplay/Ach.cpp
    UplayR1/uplay/Avatar.cpp
    UplayR1/uplay/Chat.cpp
    UplayR1/uplay/Friends.cpp
    UplayR1/uplay/Installer.cpp
    UplayR1/uplay/Options.cpp
    UplayR1/uplay/Overlay.cpp
    UplayR1/uplay/Party.cpp
    UplayR1/uplay/Presence.cpp
    UplayR1/uplay/Save.cpp
    UplayR1/uplay/Store.cpp
    UplayR1/uplay/Uplay.cpp
    UplayR1/uplay/User.cpp
    UplayR1/uplay/Win.cpp)
add_custom_command(TARGET uplay_r1 POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "$<TARGET_FILE:uplay_r1>"
        "$<TARGET_FILE_DIR:uplay_r1>/upc_r1_loader64.dll")

tbl_add_dll(uplay_r2 upc_r2_loader64
    Core/DllMain.cpp
    UplayR2/uplay/Achievement.cpp
    UplayR2/uplay/Friend.cpp
    UplayR2/uplay/InstallChunks.cpp
    UplayR2/uplay/Multiplayer.cpp
    UplayR2/uplay/Overlay.cpp
    UplayR2/uplay/Product.cpp
    UplayR2/uplay/StorageFile.cpp
    UplayR2/uplay/Store.cpp
    UplayR2/uplay/Streaming.cpp
    UplayR2/uplay/Uplay.cpp)
add_custom_command(TARGET uplay_r2 POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "$<TARGET_FILE:uplay_r2>"
        "$<TARGET_FILE_DIR:uplay_r2>/uplay_r2_loader64.dll")

tbl_add_dll(defaultargs defaultargs
    DefaultArgs/DllMain.cpp
    DefaultArgs/exports.def)

set_source_files_properties(Core/Version.rc PROPERTIES OBJECT_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/Core/Version.h")
add_executable(launch_r6 WIN32 Launcher/LaunchR6.cpp Core/Version.rc)
set_target_properties(launch_r6 PROPERTIES OUTPUT_NAME "LaunchR6")
target_include_directories(launch_r6 PRIVATE Core "${TBL_DEPS_INC}")
target_link_libraries(launch_r6 PRIVATE user32 shell32)
