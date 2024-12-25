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

#include <search.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#include <blkid/blkid.h>
//#include <libcryptsetup.h>


struct LuksVolume {
  const char* device_name;
  const char* label;

  struct LuksVolume* next;
};


struct LuksVolume* new_luks_volume(const char* device_name, const char* label) {
  struct LuksVolume* v = malloc(sizeof(struct LuksVolume));
  v->device_name = strdup(device_name);
  v->label = strdup(label);
  v->next = NULL;
  return v;
}


int gather_luks_volumes(struct LuksVolume** list) {
  blkid_probe pr;
  blkid_dev dev;
  const char *device_name;
  const char *label;
  const char *type;

  // https://cdn.kernel.org/pub/linux/utils/util-linux/v2.32/libblkid-docs/libblkid-Cache.html#blkid-probe-all ?

  blkid_cache cache;
  if (blkid_get_cache(&cache, NULL) != 0) {
    perror("blkid_get_cache");
    return 1;
  }

  blkid_dev_iterate iter = blkid_dev_iterate_begin(cache);
  if (iter == NULL) {
    fprintf(stderr, "Error initializing device iteration\n");
    blkid_put_cache(cache);
    return 1;
  }

  *list = NULL;
  struct LuksVolume* tail = NULL;

  while (!blkid_dev_next(iter, &dev)) {
    device_name = blkid_dev_devname(dev);
    if (!device_name) continue;

    type = blkid_get_tag_value(cache, "TYPE", device_name);
    if (!type || strcmp(type, "crypto_LUKS") != 0) continue;

    label = blkid_get_tag_value(cache, "PARTLABEL", device_name);
    if (!label) {
      label = blkid_get_tag_value(cache, "LABEL", device_name);
    }

    struct LuksVolume* volume = new_luks_volume(device_name, label);
    if (*list == NULL) {
      *list = volume;
      tail = volume;
    } else {
      tail->next = volume;
      tail = volume;
    }
  }

  // Cleanup iteration and cache
  blkid_dev_iterate_end(iter);
  blkid_put_cache(cache);

  return 0;
}


int main(int argc, char **argv) {
  if (geteuid()) {
    printf("mass-luks-open requires super user privileges.\n");
    return 1;
  }

  struct LuksVolume* volumes = NULL;
  gather_luks_volumes(&volumes);
  
  struct LuksVolume* v = volumes;
  while (v != NULL) {
    printf("LUKS volume [%s] has label [%s].\n", v->device_name, v->label);
    v = v->next;
  }

  return 0;
}
