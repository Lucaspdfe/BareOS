#!/bin/sh
set -e

# ----------------------------
# Configuration
# ----------------------------

ARCH="i686-elf"
PREFIX="/opt/$ARCH-toolchain"
SYSROOT="$PREFIX/$ARCH/sysroot"

BINUTILS_VERSION=2.42
GCC_VERSION=13.3.0

JOBS=$(nproc)

export PATH="$PREFIX/bin:$PATH"

WORKDIR="$PWD/toolchain"

BINUTILS_TAR="binutils-$BINUTILS_VERSION.tar.xz"
GCC_TAR="gcc-$GCC_VERSION.tar.xz"

BINUTILS_URL="https://ftp.gnu.org/gnu/binutils/$BINUTILS_TAR"
GCC_URL="https://ftp.gnu.org/gnu/gcc/gcc-$GCC_VERSION/$GCC_TAR"

BINUTILS_SRC="$WORKDIR/binutils-$BINUTILS_VERSION"
BINUTILS_BUILD="$WORKDIR/binutils-build"

GCC_SRC="$WORKDIR/gcc-$GCC_VERSION"
GCC_BUILD="$WORKDIR/gcc-build"

# ----------------------------
# Host dependency checks
# ----------------------------

require() {
    command -v "$1" >/dev/null 2>&1 || {
        echo "Error: required tool '$1' not found in PATH"
        exit 1
    }
}

# Base build tools
require sh
require make
require gcc || require clang
require ld
require ar
require ranlib

# Archive / fetch tools
require tar
require xz
require wget

# Misc tools used by GCC/binutils
require sed
require awk
require grep
require bison
require flex
require python3 || require python

# ----------------------------
# Fetch
# ----------------------------

fetch() {
    mkdir -p "$WORKDIR"
    cd "$WORKDIR"

    [ -f "$BINUTILS_TAR" ] || wget "$BINUTILS_URL"
    [ -f "$GCC_TAR" ] || wget "$GCC_URL"
}

# ----------------------------
# Binutils
# ----------------------------

build_binutils() {
    tar -xf "$WORKDIR/$BINUTILS_TAR" -C "$WORKDIR"

    mkdir -p "$BINUTILS_BUILD"
    cd "$BINUTILS_BUILD"

    "$BINUTILS_SRC/configure" \
        --target="$ARCH" \
        --prefix="$PREFIX" \
        --with-sysroot="$SYSROOT" \
        --disable-nls \
        --disable-werror

    make -j"$JOBS"
    make install
}

# ----------------------------
# GCC (bootstrap)
# ----------------------------

build_gcc() {
    tar -xf "$WORKDIR/$GCC_TAR" -C "$WORKDIR"

    cd "$GCC_SRC"
    ./contrib/download_prerequisites

    mkdir -p "$GCC_BUILD"
    cd "$GCC_BUILD"

    "$GCC_SRC/configure" \
        --target="$ARCH" \
        --prefix="$PREFIX" \
        --with-sysroot="$SYSROOT" \
        --disable-nls \
        --disable-multilib \
        --enable-languages=c,c++ \
        --without-headers

    make -j"$JOBS" all-gcc all-target-libgcc
    make install-gcc install-target-libgcc
}

# ----------------------------
# Clean
# ----------------------------

clean() {
    rm -rf "$WORKDIR"
}

# ----------------------------
# Dispatch
# ----------------------------

case "$1" in
    fetch)
        fetch
        ;;
    binutils)
        fetch
        build_binutils
        ;;
    gcc)
        fetch
        build_gcc
        ;;
    toolchain)
        fetch
        build_binutils
        build_gcc
        ;;
    clean)
        clean
        ;;
    *)
        echo "Usage: $0 {fetch|binutils|gcc|toolchain|clean}"
        exit 1
        ;;
esac
