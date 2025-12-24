#!/bin/bash
set -e

# --- ROOT DIRECTORY ---
KERNEL_DIR=$(pwd)
# -----------------------

# --- Log Helpers ---
info() {
    echo -e "\n\e[1;36m==>\e[0m \e[1m$1\e[0m"
}

# Inicializar submódulos si existen
if [ -f .gitmodules ]; then
    UNINITIALIZED_SUBMODULES=$(git submodule status | grep '^-' || true)
    if [ -n "$UNINITIALIZED_SUBMODULES" ]; then
        info "Initializing submodules..."
        git submodule update --init --recursive
    fi
fi

# --- CONFIGURACIÓN FIJA (SIN PREGUNTAS) ---
model_choice="r8q"
PERMISSIVE=false
clang_selection="2" # AOSP Clang 20
# ------------------------------------------

# Configuración de Compilador AOSP Clang 20
TOOLCHAIN_ROOT="/home/ignacio/toolchains"
AOSP_TARGET_DIR="$TOOLCHAIN_ROOT/clang-r547379"
GOOGLE_REPO_URL="https://android.googlesource.com/platform/prebuilts/clang/host/linux-x86"
TARGET_VERSION_FOLDER="clang-r547379"

info "Selected: AOSP Clang 20 (Official Google)"
CHOSEN_CLANG_DIR="$AOSP_TARGET_DIR"

if [ ! -d "$AOSP_TARGET_DIR/bin" ]; then
    info "Compiler not detected. Starting download (Sparse Checkout)..."
    mkdir -p "$TOOLCHAIN_ROOT"
    TEMP_CLONE_DIR="/tmp/aosp_clang_sparse"
    rm -rf "$TEMP_CLONE_DIR" && mkdir -p "$TEMP_CLONE_DIR"
    cd "$TEMP_CLONE_DIR"
    git init
    git remote add origin "$GOOGLE_REPO_URL"
    git config core.sparseCheckout true
    echo "$TARGET_VERSION_FOLDER/" >> .git/info/sparse-checkout
    git pull --depth=1 origin master
    mv "$TARGET_VERSION_FOLDER" "$TOOLCHAIN_ROOT/"
    rm -rf "$TEMP_CLONE_DIR"
    cd "$KERNEL_DIR"
fi

# Build Environment Setup
PATH="${CHOSEN_CLANG_DIR}/bin:${PATH}"
KERNEL_LLVM_BIN="${CHOSEN_CLANG_DIR}/bin/clang"

export LC_ALL=C
export KBUILD_BUILD_TIMESTAMP=$(date -u "+%a %b %d %H:%M:%S UTC %Y")
export KBUILD_BUILD_USER="Queen-Unleashed"
export KBUILD_BUILD_HOST="Z3phery"

PRODUCT_OUT=out
KERNEL_OUT_DIR=$PRODUCT_OUT/obj/KERNEL_OBJ
ANYKERNEL_DIR="$KERNEL_DIR/AnyKernel3"

mkdir -p "$KERNEL_OUT_DIR"

# Target Parameters
MODEL=$model_choice
CHIPSET_NAME=kona
KERNEL_ARCH=arm64
export PROJECT_NAME="${MODEL}"
[ -z "${PLATFORM_VERSION}" ] && export PLATFORM_VERSION=11

KERNEL_DEFCONFIG="vendor/${CHIPSET_NAME}-queenunleashed_defconfig"
COMMON_DEFCONFIG="vendor/samsung/kona-sec-common.config"
PROJECT_CONFIG="vendor/samsung/${MODEL}.config"

# Export variables for Make
export CC="ccache clang"
export LLVM=1
export LLVM_IAS=1
export CLANG_TRIPLE=aarch64-linux-gnu-
export CROSS_COMPILE=aarch64-linux-gnu-
export CROSS_COMPILE_ARM32=arm-linux-gnueabi-
export DTC_OVERLAY_TEST_EXT="$KERNEL_DIR/tools/ufdt_apply_overlay"
BUILD_JOB_NUMBER=$(grep -c processor /proc/cpuinfo)

# Lógica de Larois
KERNEL_MAKE_ENV="DTC_EXT=$KERNEL_DIR/tools/dtc CONFIG_BUILD_ARM64_DT_OVERLAY=y"

# --- FUNCTION DEFINITIONS ---

FUNC_BUILD_KERNEL() {
    local __dts_dir="${KERNEL_OUT_DIR}/arch/${KERNEL_ARCH}/boot/dts"

    info "Starting Kernel compilation para $MODEL..."
    
    make -C "$KERNEL_DIR" O="$KERNEL_OUT_DIR" ARCH="$KERNEL_ARCH" \
        $KERNEL_DEFCONFIG $COMMON_DEFCONFIG $PROJECT_CONFIG

    info "Building Device Tree Overlay (dtbo.img)..."
    make -C "$KERNEL_DIR" -j"$BUILD_JOB_NUMBER" O="$KERNEL_OUT_DIR" $KERNEL_MAKE_ENV \
         ARCH="$KERNEL_ARCH" CC="clang --target=aarch64-linux-gnu" dtbo.img

    info "Building Kernel Image and dtbs..."
    make -C "$KERNEL_DIR" -j"$BUILD_JOB_NUMBER" O="$KERNEL_OUT_DIR" $KERNEL_MAKE_ENV \
         ARCH="$KERNEL_ARCH" CC="clang --target=aarch64-linux-gnu" Image dtbs

    if [ ! -f "$KERNEL_OUT_DIR/arch/arm64/boot/Image" ]; then
        echo "CRITICAL ERROR: Compilation failed."
        exit 1
    fi
    
    cat "$__dts_dir/vendor/qcom"/*.dtb > "$PRODUCT_OUT/dtb.img"
    
    if [ -f "$KERNEL_OUT_DIR/arch/arm64/boot/dtbo.img" ]; then
        cp "$KERNEL_OUT_DIR/arch/arm64/boot/dtbo.img" "$PRODUCT_OUT/dtbo.img"
    fi

    rm -rf "$__dts_dir/samsung/*"
    rsync -cv "$KERNEL_OUT_DIR/arch/arm64/boot/Image" "$PRODUCT_OUT/Image"
}

FUNC_BUILD_BOOTIMG() {
    info "Preparing boot.img..."
    BUILD_ENV="$KERNEL_DIR/build_env/WORK_DIR"
    TARGET="$KERNEL_DIR/build_env/$MODEL"
    BOOT_IMG_REPO="https://github.com/ata-kaner/r8q_archive/releases/download/stock_kernel/boot_r8q.img"

    mkdir -p "$BUILD_ENV" "$TARGET"

    if ! [ -f "$BUILD_ENV/boot.img" ]; then
        curl -L -s -o "$BUILD_ENV/boot.img" "$BOOT_IMG_REPO"
    fi

    cd "$BUILD_ENV"
    if [ -f "./magiskboot-x86" ]; then
        ./magiskboot-x86 unpack boot.img
        cp "$KERNEL_DIR/$PRODUCT_OUT/dtb.img" ./dtb
        rsync -cv "$KERNEL_DIR/$PRODUCT_OUT/Image" ./kernel
        ./magiskboot-x86 repack boot.img "queen_$MODEL.img"
        rsync -cv "./queen_$MODEL.img" "$TARGET/boot.img"
        ./magiskboot-x86 cleanup
        rm "./queen_$MODEL.img"
    fi
    
    if [ -f "$KERNEL_DIR/$PRODUCT_OUT/dtbo.img" ]; then
        cp "$KERNEL_DIR/$PRODUCT_OUT/dtbo.img" "$TARGET/dtbo.img"
    fi
    cd "$KERNEL_DIR"
}

FUNC_BUILD_ANYKERNEL() {
    info "Creating AnyKernel3 flashable package..."

    if [ ! -d "$ANYKERNEL_DIR" ]; then
        git clone https://github.com/osm0sis/AnyKernel3 "$ANYKERNEL_DIR"
    fi

    rm -f "$ANYKERNEL_DIR/Image" "$ANYKERNEL_DIR/dtb" "$ANYKERNEL_DIR/dtbo.img" "$ANYKERNEL_DIR"/*.zip

    cp "$PRODUCT_OUT/Image" "$ANYKERNEL_DIR/Image"
    cp "$PRODUCT_OUT/dtb.img" "$ANYKERNEL_DIR/dtb"
    
    if [ -f "$PRODUCT_OUT/dtbo.img" ]; then
        info "Copiando dtbo.img a AnyKernel3..."
        cp "$PRODUCT_OUT/dtbo.img" "$ANYKERNEL_DIR/dtbo.img"
    fi

    gitsha=$(git rev-parse --short HEAD)
    DATE=$(date +"%Y%m%d")
    ZIP_NAME="Queen-Unleashed-ONEUI-${MODEL}-${gitsha}-${DATE}.zip"

    cd "$ANYKERNEL_DIR"
    zip -r9 "$ZIP_NAME" * -x .git README.md *placeholder .gitignore .github
    
    if [ -f "$ZIP_NAME" ]; then
        mv "$ZIP_NAME" "$KERNEL_DIR/"
        info "COMPLETED!"
        echo "File: $KERNEL_DIR/$ZIP_NAME"
    fi
    cd "$KERNEL_DIR"
}

# --- EXECUTION ---
(
    FUNC_BUILD_KERNEL
    FUNC_BUILD_BOOTIMG
    FUNC_BUILD_ANYKERNEL
)
