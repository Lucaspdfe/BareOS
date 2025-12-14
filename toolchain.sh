#!/bin/sh

# Fixed install path
TOOLCHAIN_PREFIX="/opt/i686-elf-toolchain/i686-elf"
export PATH="$TOOLCHAIN_PREFIX/bin:$PATH"

BINUTILS_VERSION=2.42
GCC_VERSION=13.3.0

# Variables that still depend on environment
BINUTILS_SRC="toolchain/binutils-$BINUTILS_VERSION"
BINUTILS_BUILD="toolchain/binutils-build-$BINUTILS_VERSION"

GCC_SRC="toolchain/gcc-$GCC_VERSION"
GCC_BUILD="toolchain/gcc-build-$GCC_VERSION"

BINUTILS_URL="https://ftp.gnu.org/gnu/binutils/binutils-$BINUTILS_VERSION.tar.xz"
GCC_URL="https://ftp.gnu.org/gnu/gcc/gcc-$GCC_VERSION/gcc-$GCC_VERSION.tar.xz"

#----------------------------------------
# Binutils
#----------------------------------------

fetch_binutils() {
    mkdir -p toolchain
    cd toolchain && wget -nc "$BINUTILS_URL"
}

toolchain_binutils() {
    tar -xf "toolchain/binutils-$BINUTILS_VERSION.tar.xz" -C "toolchain/"
    mkdir -p "$BINUTILS_BUILD"
    cd "$BINUTILS_BUILD" && ../"binutils-$BINUTILS_VERSION"/configure \
        --prefix="$TOOLCHAIN_PREFIX" \
        --target="i686-elf" \
        --with-sysroot \
        --disable-nls \
        --disable-werror

    cd ../..
    make -j8 -C "$BINUTILS_BUILD"
    sudo make -C "$BINUTILS_BUILD" install
}

#----------------------------------------
# GCC
#----------------------------------------

fetch_gcc() {
    mkdir -p toolchain
    cd toolchain && wget -nc "$GCC_URL"
}

toolchain_gcc() {
    tar -xf "toolchain/gcc-$GCC_VERSION.tar.xz" -C "toolchain/"
    mkdir -p "$GCC_BUILD"
    cd "$GCC_BUILD" && ../"gcc-$GCC_VERSION"/configure \
        --prefix="$TOOLCHAIN_PREFIX" \
        --target="i686-elf" \
        --disable-nls \
        --enable-languages=c,c++ \
        --without-headers
    cd ../..
    make -j8 -C "$GCC_BUILD" all-gcc all-target-libgcc
    sudo make -C "$GCC_BUILD" install-gcc install-target-libgcc
}

#----------------------------------------
# Cleaning
#----------------------------------------

clean_toolchain() {
    rm -rf "$GCC_BUILD" "$GCC_SRC" "$BINUTILS_BUILD" "$BINUTILS_SRC"
}

clean_toolchain_all() {
    rm -rf toolchain/*
}

#----------------------------------------
# Dispatch
#----------------------------------------

case "$1" in
    toolchain)
        toolchain_binutils
        toolchain_gcc
        ;;
    toolchain_binutils)
        toolchain_binutils
        ;;
    toolchain_gcc)
        toolchain_gcc
        ;;
    fetch_binutils)
        fetch_binutils
        ;;
    fetch_gcc)
        fetch_gcc
        ;;
    clean-toolchain)
        clean_toolchain
        ;;
    clean-toolchain-all)
        clean_toolchain_all
        ;;
    *)
        echo "Unknown command"
        exit 1
        ;;
esac
