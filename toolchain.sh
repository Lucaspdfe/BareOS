#!/bin/sh
set -e

# ----------------------------
# Configuration
# ----------------------------

TARGET="i686-elf"

BINUTILS_VERSION="2.37"
GCC_VERSION="11.2.0"

TOOLCHAIN_PREFIX="/opt/i686-elf-toolchain"
export PATH="${TOOLCHAIN_PREFIX}/bin:${PATH}"

BINUTILS_URL="https://ftp.gnu.org/gnu/binutils/binutils-${BINUTILS_VERSION}.tar.xz"
GCC_URL="https://ftp.gnu.org/gnu/gcc/gcc-${GCC_VERSION}/gcc-${GCC_VERSION}.tar.xz"

BINUTILS_SRC="toolchain/binutils-${BINUTILS_VERSION}"
BINUTILS_BUILD="toolchain/binutils-build-${BINUTILS_VERSION}"

GCC_SRC="toolchain/gcc-${GCC_VERSION}"
GCC_BUILD="toolchain/gcc-build-${GCC_VERSION}"

JOBS=8

# ----------------------------
# Binutils
# ----------------------------

build_binutils() {
    mkdir -p toolchain
    cd toolchain

    if [ ! -f "binutils-${BINUTILS_VERSION}.tar.xz" ]; then
        wget "${BINUTILS_URL}"
    fi

    tar -xf "binutils-${BINUTILS_VERSION}.tar.xz"

    mkdir -p "../${BINUTILS_BUILD}"
    cd "../${BINUTILS_BUILD}"

    CFLAGS= ASMFLAGS= CC= CXX= LD= ASM= LINKFLAGS= LIBS= \
        ../binutils-${BINUTILS_VERSION}/configure \
            --prefix="${TOOLCHAIN_PREFIX}" \
            --target="${TARGET}" \
            --with-sysroot \
            --disable-nls \
            --disable-werror

    make -j"${JOBS}"
    make install
}

# ----------------------------
# GCC
# ----------------------------

build_gcc() {
    mkdir -p toolchain
    cd toolchain

    if [ ! -f "gcc-${GCC_VERSION}.tar.xz" ]; then
        wget "${GCC_URL}"
    fi

    tar -xf "gcc-${GCC_VERSION}.tar.xz"

    # ---- GMP / MPFR / MPC ----
    cd "gcc-${GCC_VERSION}"
    ./contrib/download_prerequisites
    cd ..

    mkdir -p "../${GCC_BUILD}"
    cd "../${GCC_BUILD}"

    CFLAGS= ASMFLAGS= CC= CXX= LD= ASM= LINKFLAGS= LIBS= \
        ../gcc-${GCC_VERSION}/configure \
            --prefix="${TOOLCHAIN_PREFIX}" \
            --target="${TARGET}" \
            --disable-nls \
            --enable-languages=c,c++ \
            --without-headers

    make -j"${JOBS}" all-gcc all-target-libgcc
    make install-gcc install-target-libgcc
}

# ----------------------------
# Clean
# ----------------------------

clean_toolchain() {
    rm -rf "${GCC_BUILD}" "${GCC_SRC}" "${BINUTILS_BUILD}" "${BINUTILS_SRC}"
}

clean_toolchain_all() {
    rm -rf toolchain/*
}

# ----------------------------
# Dispatch
# ----------------------------

case "$1" in
    toolchain)
        build_binutils
        build_gcc
        ;;
    binutils)
        build_binutils
        ;;
    gcc)
        build_gcc
        ;;
    clean)
        clean_toolchain
        ;;
    clean-all)
        clean_toolchain_all
        ;;
    *)
        echo "Usage: $0 {toolchain|binutils|gcc|clean|clean-all}"
        exit 1
        ;;
esac
