FROM debian:stable-slim

RUN apt-get update && apt-get install -y \
    build-essential \
    bison flex \
    python3 \
    tar xz-utils wget \
    nasm mtools qemu-system-x86 \
    && rm -rf /var/lib/apt/lists/*
