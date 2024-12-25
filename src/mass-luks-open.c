/*
mass-luks-open: A tool to open many LUKS volumes with the same passphrase.
Copyright (C) 2024  Anthony Lieuallen

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with this program; if not, see <https://www.gnu.org/licenses/>.
*/

#include <stdio.h>
//#include <stdlib.h>
#include <string.h>
#include <unistd.h>
//#include <inttypes.h>
#include <sys/types.h>

#include <blkid/blkid.h>
//#include <libcryptsetup.h>

int main(int argc, char **argv) {
  if (geteuid()) {
    printf("Using of libcryptsetup requires super user privileges.\n");
    return 1;
  }

  blkid_probe pr;
  blkid_dev dev;
  const char *device;
  const char *label;
  const char *type;

  // https://cdn.kernel.org/pub/linux/utils/util-linux/v2.32/libblkid-docs/libblkid-Cache.html#blkid-probe-all ?

  // Open the blkid cache to access device information
  blkid_cache cache;
  if (blkid_get_cache(&cache, NULL) != 0) {
    perror("blkid_get_cache");
    return 1;
  }

  // Enumerate all devices in the blkid cache
  blkid_dev_iterate iter = blkid_dev_iterate_begin(cache);
  if (iter == NULL) {
    fprintf(stderr, "Error initializing device iteration\n");
    blkid_put_cache(cache);
    return 1;
  }

  // Iterate through each device and check for LUKS signature
  while (!blkid_dev_next(iter, &dev)) {
    device = blkid_dev_devname(dev);
    if (!device) continue;

    type = blkid_get_tag_value(cache, "TYPE", device);
    if (!type || strcmp(type, "crypto_LUKS") != 0) continue;

    label = blkid_get_tag_value(cache, "PARTLABEL", device);
    if (!label) {
      label = blkid_get_tag_value(cache, "LABEL", device);
    }

    printf("Device %s has LUKS signature and label %s\n", device, label);
  }

  // Cleanup iteration and cache
  blkid_dev_iterate_end(iter);
  blkid_put_cache(cache);

  return 0;
}
