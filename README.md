A minimal tool to perform mass `cryptsetup luksOpen` commands with the same passphrase.  Intended to be used as part of a Linux system that boots from LUKS encrypted volume(s).  An included [dracut](https://github.com/dracutdevs/dracut) module is the intended method.

# Usage

There there are no runtime options.  Simply run `mass-luks-open`.  This is what the dracut module does, at the right point during boot.

```
# mass-luks-open
Found 4 LUKS volumes:
  1            /dev/sdd2 => rpool_JETMMKKJ
  2            /dev/sdb2 => rpool_K82746KA
  3            /dev/sdc2 => rpool_WHU2I04G
  4            /dev/sda2 => rpool_Y9KWTIL7
Enter LUKS decryption passphrase:
Opening LUKS volume [/dev/sdd2] with label [rpool_JETMMKKJ]... success.
Opening LUKS volume [/dev/sdb2] with label [rpool_K82746KA]... success.
Opening LUKS volume [/dev/sdc2] with label [rpool_WHU2I04G]... success.
Opening LUKS volume [/dev/sda2] with label [rpool_Y9KWTIL7]... success.
```

LUKS volumes are detected with the `libblkid` library.  If running `blkid` on your system shows `TYPE="crypto_LUKS"` entries, they will be detected and used.  The same passphrase is automatically re-used across all detected volumes.  If you mistype the passphrase or use different passphrases on different volumes, you will be prompted to retry (or give up).

# Configuration

There is no configuration, per se.  However the label that `mass-luks-open` assigns to the unlocked volumes can be controlled.  The intended usage is to place LUKS volumes in GPT partitions, in which case the partition label will be used as the unlocked LUKS volume's label (as in the example above).  When partition labels are not available, a device label or UUID will be used.

# Build and Install

Short version: Just run `make` (as root).

Long version: This will compile the program and install the dracut module.  Compiling requires several libraries to be availble.  On Gentoo, these come from `sys-apps/util-linux` and `sys-fs/cryptsetup`.  Emerge these packages before proceeding.  Run `make` inside the `src` directory to only build the program, without install.
