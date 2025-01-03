A minimal tool to perform mass `cryptsetup luksOpen` commands with the same passphrase.  Intended to be used as part of a Linux system, booting from LUKS encrypted volume(s).  An included [dracut](https://github.com/dracutdevs/dracut) module is the intended method.

# Build and Install

Short version: Just run `make`.

Long version: This will compile the program and install the dracut module.  Compiling requires several libraries to be availble.  On Gentoo, these come from `sys-apps/util-linux` and `sys-fs/cryptsetup`.  Emerge these packages before proceeding.
