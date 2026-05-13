#!/bin/sh
# Launch script for Unreal on FunKey/RG Nano
# Packaged inside the OPK squashfs

# Find game directory
GAME_DIR=""
for d in /mnt/FunKey/Unreal /mnt/Funkey/Unreal /mnt/funkey/Unreal; do
    if [ -d "$d" ]; then
        GAME_DIR="$d"
        break
    fi
done

LOG_FILE="/tmp/unreal_funkey.log"
if [ -n "$GAME_DIR" ]; then
    LOG_FILE="$GAME_DIR/funkey.log"
fi

echo "Starting Unreal" > "$LOG_FILE"
echo "Date: $(date)" >> "$LOG_FILE"

if [ -z "$GAME_DIR" ]; then
    echo "ERROR: Game directory not found" >> "$LOG_FILE"
    echo "Tried: /mnt/FunKey/Unreal /mnt/Funkey/Unreal /mnt/funkey/Unreal" >> "$LOG_FILE"
    exit 1
fi

if [ ! -d "$GAME_DIR/System" ]; then
    echo "ERROR: System/ subdirectory not found in $GAME_DIR" >> "$LOG_FILE"
    exit 1
fi

# Find OPK mount directory
OPK_MNT=""
if [ -x "$(pwd)/Unreal.bin" ]; then
    OPK_MNT="$(pwd)"
fi
if [ -z "$OPK_MNT" ]; then
    for d in /tmp/.mount_* /tmp/opk-* /mnt/opk-* /opk; do
        if [ -x "$d/Unreal.bin" ]; then
            OPK_MNT="$d"
            break
        fi
    done
fi
if [ -z "$OPK_MNT" ]; then
    echo "ERROR: Could not find OPK mount with Unreal.bin" >> "$LOG_FILE"
    exit 1
fi

# Engine expects CWD = System/
cd "$GAME_DIR/System"

# Bundled libs from OPK
export LD_LIBRARY_PATH="$OPK_MNT/lib:$OPK_MNT/lib/directfb-1.7-7:$OPK_MNT/lib/directfb-1.7-7/systems:$OPK_MNT/lib/directfb-1.7-7/wm:$OPK_MNT/lib/directfb-1.7-7/interfaces/IDirectFBFont:$OPK_MNT/lib/directfb-1.7-7/interfaces/IDirectFBImageProvider:$OPK_MNT/lib/directfb-1.7-7/interfaces/IDirectFBVideoProvider:$OPK_MNT/lib/directfb-1.7-7/inputdrivers:$OPK_MNT/lib/directfb-1.7-7/gfxdrivers:$LD_LIBRARY_PATH"
export DFBARGS="module-dir=$OPK_MNT/lib/directfb-1.7-7,system=fbdev,fbdev=/dev/fb0,no-linux-input-grab,no-cursor"
unset AUDIODEV

"$OPK_MNT/Unreal.bin" >> "$LOG_FILE" 2>&1
