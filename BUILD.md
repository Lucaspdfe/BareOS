# How to compile

## Dependencies

You'll need to use Linux, preferably any debian-based distro, or you can use WSL(Windows Subsystem for Linux) for Windows.

List of dependencies:
    `make`
    `nasm`
    `mtools`
    `qemu-system`
    `build-essential`
    `bison`
    `flex`
    `libgmp-dev`
    `libmpfr-dev`
    `libmpc-dev`
    `texinfo`
    `wget`
    `xz-utils`

### Toolchain (i686-elf)

To compile the toolchain, run these commands:

```bash
./toolchain.sh fetch_binutils
./toolchain.sh fetch_gcc
./toolchain.sh toolchain
echo 'export PATH="$PATH:/opt/i686-elf-toolchain/i686-elf/bin"' >> ~/.bashrc
   ```

(If this command didn't work, try modifying $BINUTILS_VERSION and $GCC_VERSION on toolchain.sh)

## Compiling

To compile, run `make` and if you want to run the OS, run `./run`.

