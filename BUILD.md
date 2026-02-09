# How to build BareOS

## Host requirements

You need a **Linux environment**.
Native Linux is recommended. WSL works, but is not officially supported.

No package manager is assumed.
Dependencies are checked, not auto-installed.

### Required host tools

The following tools must be available in `PATH`:

Build tools:

* `sh`
* `make`
* `gcc` and `g++` **or** `clang`
* `ld`, `ar`, `ranlib`
* `bison`
* `flex`
* `texinfo`
* `python3` **or** `python`

Archive / fetch tools:

* `tar`
* `xz`
* `wget`

Runtime / OS build tools:

* `nasm`
* `mtools`
* `qemu-system-*`

> Note
> You **do not** need `libgmp`, `libmpfr`, or `libmpc`.
> GCC dependencies are downloaded and built automatically by the toolchain script.

---

## Toolchain (i686-elf)

The project needs a **bare-metal cross compiler**.

### Build the toolchain

From the project root: (as administrator or root)

```sh
./toolchain.sh toolchain
```

This will:

* Download binutils and GCC
* Download GCC prerequisites (GMP, MPFR, MPC, ISL)
* Build and install the cross toolchain

### Install location

By default, the toolchain is installed to:

```
/opt/i686-elf-toolchain
```

You may need to run the script as root, or change the prefix inside `toolchain.sh`.

### Add toolchain to PATH

Add this to your shell configuration:

```sh
export PATH="/opt/i686-elf-toolchain/bin:$PATH"
```

Restart your shell or reload the config.

Verify:

```sh
i686-elf-gcc --version
```

---

## Building the OS

Once the toolchain is available:

```sh
make
```

---

## Running

To run the OS in QEMU:

```sh
./run
```
