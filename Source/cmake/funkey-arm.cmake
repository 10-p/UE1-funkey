# CMake toolchain file for FunKey-OS SDK (Anbernic RG Nano / FunKey S)
# ARM Cortex-A7, Allwinner V3s, no GPU, 240x240 screen, 64MB RAM
#
# Usage:
#   cmake -DCMAKE_TOOLCHAIN_FILE=cmake/funkey-arm.cmake -DFUNKEY=ON ...

# Locate the SDK root via environment variable or default path.
if(DEFINED ENV{FUNKEY_SDK_PATH})
    set(FUNKEY_SDK "$ENV{FUNKEY_SDK_PATH}")
elseif(DEFINED FUNKEY_SDK_PATH)
    set(FUNKEY_SDK "${FUNKEY_SDK_PATH}")
else()
    message(FATAL_ERROR "FunKey SDK not found. Set FUNKEY_SDK_PATH env var or pass -DFUNKEY_SDK_PATH=<path>.")
endif()

if(NOT EXISTS "${FUNKEY_SDK}/bin/arm-funkey-linux-musleabihf-gcc")
    message(FATAL_ERROR "FunKey SDK not found at ${FUNKEY_SDK}. Set FUNKEY_SDK_PATH.")
endif()

set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR armv7l)

set(CMAKE_C_COMPILER "${FUNKEY_SDK}/bin/arm-funkey-linux-musleabihf-gcc")
set(CMAKE_CXX_COMPILER "${FUNKEY_SDK}/bin/arm-funkey-linux-musleabihf-g++")

set(CMAKE_C_FLAGS_INIT "-D_LARGEFILE_SOURCE -D_LARGEFILE64_SOURCE -D_FILE_OFFSET_BITS=64 -O2 -fno-strict-aliasing -mthumb -march=armv7-a+neon-vfpv4 -mtune=cortex-a7 -mfpu=neon-vfpv4 -DFUNKEY_DEVICE")
set(CMAKE_CXX_FLAGS_INIT "${CMAKE_C_FLAGS_INIT}")

set(CMAKE_SYSROOT "${FUNKEY_SDK}/arm-funkey-linux-musleabihf/sysroot")
set(CMAKE_FIND_ROOT_PATH "${FUNKEY_SDK}/arm-funkey-linux-musleabihf/sysroot")
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

# Also search the local funkey-deps prefix for SDL2/libxmp we built ourselves
if(DEFINED FUNKEY_DEPS_PREFIX)
    list(APPEND CMAKE_PREFIX_PATH "${FUNKEY_DEPS_PREFIX}")
    list(APPEND CMAKE_FIND_ROOT_PATH "${FUNKEY_DEPS_PREFIX}")
endif()
