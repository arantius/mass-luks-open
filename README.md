A minimal tool to perform mass `cryptsetup luksOpen` commands with the same passphrase.  Intended to be used as part of a Linux system, booting from LUKS encrypted volume(s).  An included [dracut](https://github.com/dracutdevs/dracut) module is the intended method.

# Building

We need `libcryptsetup.a` and `libblkid.a`.  To get these on Gentoo, I:

```
i# To build, we want these.
sys-fs/cryptsetup static-libs
sys-apps/util-linux static-libs
# And transitively those require these.
dev-libs/json-c static-libs
dev-libs/popt static-libs
app-crypt/argon2 static-libs
dev-libs/openssl static-libs
sys-fs/lvm2 static static-libs -udev
```
