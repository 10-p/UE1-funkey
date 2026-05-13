# Unreal Engine 1 - FunKey S / Anbernic RG Nano Port

Fork of [fgsfdsfgs/UE1](https://github.com/fgsfdsfgs/UE1) with support for the **FunKey S** and **Anbernic RG Nano** (ARM Cortex-A7, Allwinner V3s, 64MB RAM, 240x240 screen, FunKey-OS).

Uses software rendering (SoftDrv) and OpenAL audio. No GPU required.

This is a vibe coded app for a nifty toy so don't expect much more than this.

This is the full fat Unreal. I recommend new players to go to options, enable auto aim, view spring and to customize the buttons if the defaults don't work for you.

There are some small issues such as the graphics shadows showing some horizontal streaks or the view and weapon bobbing being jerky.

## FunKey / RG Nano

### Running

1. Install the original retail v200 release of Unreal or the v205 demo onto your PC.
2. Copy the game folders to `/mnt/FunKey/Unreal/` on the device:
   * `System/` (`.u`, `.int` files)
   * `Maps/`, `Textures/`, `Sounds/`, `Music/`
3. Copy `FunKey/Default-FunKey.ini` from this repo to the device as `/mnt/FunKey/Unreal/System/Default.ini`.
4. Delete `Unreal.ini` from the device's `System/` folder (it will be regenerated from `Default.ini` on first launch).
5. Copy the `.opk` file to the device's applications folder.
6. Launch from the FunKey menu.
7. Logs are written to `/mnt/FunKey/Unreal/funkey.log`.

### Input

The included `Default-FunKey.ini` provides these default bindings:

| Button | Action | Fn + Button (L shoulder held) |
|--------|--------|-------------------------------|
| D-pad Up/Down | Move forward / backward | Look up / down |
| D-pad Left/Right | Turn left / right | Prev / next inventory |
| A | Fire | -- |
| B | Alt-fire | -- |
| X | Jump | Duck |
| Y | Next weapon | -- |
| R shoulder | Activate inventory | -- |
| Start | Enter (confirm) | -- |
| Select | Center view | -- |
| Power/Menu | Menu (Escape) | -- |
| L shoulder | Fn modifier (hold) | -- |

All bindings can be changed in-game via Preferences. The Fn modifier button is configurable in `Unreal.ini`:
```ini
[NSDLDrv.NSDLClient]
FnScanCode=16
```
Set to empty (`FnScanCode=`) to disable the Fn layer and use L shoulder as a regular key.

### Building (on Linux or WSL)

#### 1. Get the FunKey SDK

Download or build the [FunKey-sdk-2.0.0](https://github.com/FunKey-Project/FunKey-OS) toolchain. Set the environment variable:
```
export FUNKEY_SDK_PATH=/path/to/FunKey-sdk-2.0.0
```

#### 2. Get the SDL2 + DirectFB SDK

The FunKey SDK does not include SDL2. Download [joyrider3774's prebuilt FunKey SDL2 SDK](https://github.com/joyrider3774/sdks/releases/tag/v1.0):
```
wget https://github.com/joyrider3774/sdks/releases/download/v1.0/funkey-sdk-sdl2.tar.gz
tar xzf funkey-sdk-sdl2.tar.gz
```

Copy the SDL2 headers and libraries into a `funkey-deps/` directory next to `Source/`:
```
mkdir -p funkey-deps/include funkey-deps/lib
cp -r funkey-sdk/arm-funkey-linux-musleabihf/sysroot/usr/include/SDL2 funkey-deps/include/
cp -L funkey-sdk/arm-funkey-linux-musleabihf/sysroot/usr/lib/libSDL2* funkey-deps/lib/
cp -L funkey-sdk/arm-funkey-linux-musleabihf/sysroot/usr/lib/libdirectfb* funkey-deps/lib/
cp -L funkey-sdk/arm-funkey-linux-musleabihf/sysroot/usr/lib/libdirect-* funkey-deps/lib/
cp -L funkey-sdk/arm-funkey-linux-musleabihf/sysroot/usr/lib/libfusion* funkey-deps/lib/
cp -L funkey-sdk/arm-funkey-linux-musleabihf/sysroot/usr/lib/lib++dfb* funkey-deps/lib/
cp -L funkey-sdk/arm-funkey-linux-musleabihf/sysroot/usr/lib/libts* funkey-deps/lib/
cp -rL funkey-sdk/arm-funkey-linux-musleabihf/sysroot/usr/lib/directfb-1.7-7 funkey-deps/lib/
```

#### 3. Build libxmp

```
wget https://github.com/libxmp/libxmp/releases/download/libxmp-4.6.0/libxmp-4.6.0.tar.gz
tar xzf libxmp-4.6.0.tar.gz
cd libxmp-4.6.0
cmake -Bbuild \
    -DCMAKE_SYSTEM_NAME=Linux \
    -DCMAKE_SYSTEM_PROCESSOR=armv7l \
    -DCMAKE_C_COMPILER="$FUNKEY_SDK_PATH/bin/arm-funkey-linux-musleabihf-gcc" \
    -DCMAKE_SYSROOT="$FUNKEY_SDK_PATH/arm-funkey-linux-musleabihf/sysroot" \
    -DCMAKE_C_FLAGS="-mthumb -march=armv7-a+neon-vfpv4 -mtune=cortex-a7 -mfpu=neon-vfpv4" \
    -DCMAKE_INSTALL_PREFIX="$(pwd)/../funkey-deps" \
    -DBUILD_SHARED_LIBS=ON
cmake --build build -j
cmake --install build
cd ..
```

#### 4. Build UE1

```
cmake -G"Unix Makefiles" \
    -DCMAKE_TOOLCHAIN_FILE=cmake/funkey-arm.cmake \
    -DFUNKEY=ON \
    -DFUNKEY_DEPS_PREFIX="$(pwd)/funkey-deps" \
    -Bbuild-funkey Source
cmake --build build-funkey -j
```

The binary will be at `build-funkey/Unreal/Unreal.bin`.

#### 5. Package the OPK

Install `squashfs-tools`, then:
```
cmake --build build-funkey --target package-opk
```

The OPK will be at `build-funkey/unreal_funkey-s.opk`.

---

# Original README below

## What?

Unreal Engine 1 v200 source with modifications to make it run on modern systems.  
Requires assets from the original Unreal v200 retail release or from the v205 demo. Other versions have not been tested.

## Changes from original source

* Added SDL2 windowing/client driver (NSDLDrv).
* Added GLES2 and fixed pipeline GL graphics drivers (NOpenGLESDrv and NOpenGLDrv).
* Added OpenAL + libxmp audio driver (NOpenALDrv).
* Added GCC support and fixed a bunch of related bugs.
* Supported platforms: Windows (x86), Linux (x86, ARM32) and PSVita (ARM32).
* Editor UI is not supported.

## Running

### Linux and Windows
1. Install the original retail v200 release of Unreal or the v205 demo.
2. Copy over the new files:
   * If you downloaded a ZIP from the Releases section:
     1. Unzip said ZIP to the `Unreal` folder. Overwrite everything.
   * If you built the game yourself:
     1. Copy the .dll/.so/.exe/.bin files you built to `Unreal/System`. Overwrite everything.
     2. Copy the contents of `Engine/Config` to `Unreal/System`. Overwrite everything.
3. Run `System/Unreal.exe`.

### PSVita
1. Ensure you have libshacccg installed.
2. Install the original retail v200 release of Unreal or the v205 demo onto your PC.
3. Copy the contents of the `Unreal` folder to `ux0:/data/unreal/` on your PSVita.
4. Copy the `unreal` folder from `unreal-arm-psvita-gcc.zip` to `ux0:/data/`. Overwrite everything.
5. Install `unreal.vpk` from `unreal-arm-psvita-gcc.zip`.
6. Run Unreal.

## Building

### Windows x86 (MSYS2/MinGW)
1. Install MSYS2.
2. Open the `MINGW32` prompt. **Do not** use the `MINGW64` or `MSYS` prompts.
3. Install dependencies: `pacman -S git make mingw-w64-i686-toolchain mingw-w64-i686-cmake mingw-w64-i686-SDL2 mingw-w64-i686-openal mingw-w64-i686-libxmp`
4. Build:
   ```
   cmake -G"Unix Makefiles" -Bbuild Source
   cmake --build build -j4 -- -O && cmake --install build
   ```
5. The resulting files will be in `build/RelWithDebInfo` by default.

### Windows x86 (Visual Studio)
1. Install VS2019 or VS2022. Dependencies are included in the repo.
2. Build:
   ```
   cmake -Bbuild -G"Visual Studio 16 2019" -A Win32 Source # or -G"Visual Studio 17 2022"
   cmake --build build && cmake --install build --config RelWithDebInfo
   ```
3. The resulting files will be in `build/RelWithDebInfo` by default.

### Linux x86
1. Install git, make, cmake, gcc, g++, sdl2, libopenal, libxmp.
   * If cross-compiling from x86_64, also install 32-bit versions of the libraries and gcc-multilib/g++-multilib.
   * On Debian x86_64 this process looks something like this:
     ```
     sudo dpkg --add-architecture i386
     sudo apt-get -y update
     sudo apt-get -y install git gcc g++ gcc-multilib g++-multilib make cmake
     sudo apt-get -y install libsdl2-dev libopenal-dev libxmp-dev libsdl2-dev:i386 libopenal-dev:i386 libxmp-dev:i386
     ```
2. Build:
   ```
   cmake -G"Unix Makefiles" -DCMAKE_C_FLAGS=-m32 -DCMAKE_CXX_FLAGS=-m32 -Bbuild Source # if on x86_64
   cmake -G"Unix Makefiles" -Bbuild Source # if on i686
   cmake --build build -j4 -- -O && cmake --install build
   ```
3. The resulting files will be in `build/RelWithDebInfo` by default.

### Linux ARM
1. Install git, make, cmake, gcc, g++, sdl2, libopenal, libxmp.
   * If cross-compiling from ARM64, also install armhf versions of the libraries and arm-linux-gnueabihf-gcc/g++.
   * On Debian x86_64 or ARM64 this process looks something like this:
     ```
     sudo dpkg --add-architecture armhf
     sudo apt-get -y update
     sudo apt-get -y install git gcc g++ crossbuild-essential-armhf make cmake
     sudo apt-get -y install libsdl2-dev libopenal-dev libxmp-dev libsdl2-dev:armhf libopenal-dev:armhf libxmp-dev:armhf
     ```
2. Build:
   ```
   cmake -G"Unix Makefiles" -DCMAKE_C_COMPILER=arm-linux-gnueabihf-gcc-12 -DCMAKE_CXX_COMPILER=arm-linux-gnueabihf-g++-12 -Bbuild Source
   cmake --build build -j4 -- -O && cmake --install build
   ```
3. The resulting files will be in `build/RelWithDebInfo` by default.

### PSVita (on Linux or WSL)
1. Install VitaSDK with all VDPM packages and ensure the `VITASDK` environment variable is set and `$VITASDK/bin` is in your `PATH`.
2. Build and install vitaGL:
   ```
   git clone --recursive https://github.com/Rinnegatamante/vitaGL
   make -C vitaGL HAVE_GLSL_SUPPORT=1 CIRCULAR_VERTEX_POOL=2 MATH_SPEEDHACK=1 INDICES_SPEEDHACK=1 PRIMITIVES_SPEEDHACK=1 -j install
   ```
3. Build and install SDL2:
   ```
   git clone --recursive --branch vitagl https://github.com/Northfear/SDL
   pushd SDL
   cmake -S. -Bbuild -DCMAKE_TOOLCHAIN_FILE=${VITASDK}/share/vita.toolchain.cmake -DCMAKE_BUILD_TYPE=Release -DVIDEO_VITA_VGL=ON
   cmake --build build -- -j
   cmake --install build
   popd
   ```
4. Build the VPK:
   ```
   cmake -G"Unix Makefiles" -DCMAKE_TOOLCHAIN_FILE="${VITASDK}/share/vita.toolchain.cmake" -Bbuild -DCMAKE_BUILD_TYPE=RelWithDebInfo Source
   cmake --build build -j
   ```
5. The VPK will be in `build/Unreal/`.

## Note

Unreal Engine, Unreal and any related trademarks or copyrights are owned by Epic Games. This repository is not affiliated with or endorsed by Epic Games. 
This is based on the v200 source available elsewhere on the Internet, with assets and third party proprietary libraries removed. 
Do not use for commercial purposes.
