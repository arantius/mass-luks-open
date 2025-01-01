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

#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <blkid/blkid.h>
#include <bsd/readpassphrase.h>
#include <libcryptsetup.h>


#define VOL_STATUS_UNKNOWN 1
#define VOL_STATUS_OPEN 2
#define VOL_STATUS_FAIL 3


struct LuksVolume {
  const char* device_path;
  const char* label;
  int status;

  struct LuksVolume* next;
};


struct LuksVolume* _new_luks_volume(const char* device_path, const char* label) {
  struct LuksVolume* v = malloc(sizeof(struct LuksVolume));
  v->device_path = strdup(device_path);
  v->label = strdup(label);
  v->status = VOL_STATUS_UNKNOWN;
  v->next = NULL;
  return v;
}


int gather_luks_volumes(struct LuksVolume** list) {
  blkid_probe pr;
  blkid_dev dev;
  const char *device_path;
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
    device_path = blkid_dev_devname(dev);
    if (!device_path) continue;

    type = blkid_get_tag_value(cache, "TYPE", device_path);
    if (!type || strcmp(type, "crypto_LUKS") != 0) continue;

    label = blkid_get_tag_value(cache, "PARTLABEL", device_path);
    if (!label) {
      label = blkid_get_tag_value(cache, "LABEL", device_path);
    }
    if (!label) {
      label = blkid_get_tag_value(cache, "UUID", device_path);
    }

    struct LuksVolume* volume = _new_luks_volume(device_path, label);
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


int open_luks_volumes(struct LuksVolume** volumes) {
  #define PASSPHRASE_MAX_LEN 512
  char* passphrase = crypt_safe_alloc(PASSPHRASE_MAX_LEN);
  size_t passphraseLen = 0;
  struct LuksVolume* v = *volumes;
  while (v != NULL) {
    if (v->status == VOL_STATUS_OPEN) continue;

    if (passphraseLen == 0) {
      if (readpassphrase(
          "Enter LUKS decryption passphrase: ", passphrase,
          PASSPHRASE_MAX_LEN, RPP_REQUIRE_TTY) == NULL) {
        fprintf(stderr, "readpassphrase() failed.\n");
        goto check_drives;
      }
      passphraseLen = strlen(passphrase);
    }

    printf(
        "Opening LUKS volume [%s] with label [%s]... ",
        v->device_path, v->label);

    struct crypt_device *cd;
    if (crypt_init(&cd, v->device_path)) {
      fprintf(stderr, "crypt_init() failed for %s.\n", v->device_path);
      return 1;
    }

    if (crypt_load(cd, CRYPT_LUKS, NULL)) {
      fprintf(stderr, "crypt_load() failed for %s.\n", v->device_path);
      goto next_drive_fail;
    }

    int r = crypt_activate_by_passphrase(
        cd, v->label, CRYPT_ANY_SLOT, passphrase, passphraseLen,
        CRYPT_ACTIVATE_ALLOW_DISCARDS);
    if (r < 0) {
      printf("Device %s activation failed.\n", v->device_path);
      goto next_drive_fail;
    }

    printf("success.\n");
    v->status = VOL_STATUS_OPEN;
    goto next_drive;

    next_drive_fail:
    v->status = VOL_STATUS_FAIL;

    next_drive:
    crypt_free(cd);
    v = v->next;
  }

  check_drives:
  crypt_safe_free(passphrase);

  int num_open = 0;
  int num_fail = 0;
  v = *volumes;
  while (v != NULL) {
    if (v->status == VOL_STATUS_OPEN) num_open++; else num_fail++;
    v = v->next;
  }
  if (num_fail > 0) {
    char input[2];
    printf(
        "Successfully opened %d, failed to open %d.  Retry? [Y/n] ",
        num_open, num_fail);
    fgets(input, sizeof(input), stdin);
    if (input[0] == '\n' || input[0] == 'Y' || input[0] == 'y') {
      return open_luks_volumes(volumes);
    }
  }

  return num_open > 0;
}


void show_luks_volumes(struct LuksVolume** volumes) {
  size_t num_volumes = 0;
  struct LuksVolume* v = *volumes;
  while (v != NULL) {
    num_volumes++;
    v = v->next;
  }
  printf("Found %ld LUKS volumes:\n", num_volumes);
  v = *volumes;
  int i = 1;
  while (v != NULL) {
    printf("%3d %20s => %s\n", i++, v->device_path, v->label);
    v = v->next;
  }
}


int main(int argc, char **argv) {
  if (geteuid()) {
    printf("mass-luks-open requires super user privileges.\n");
    return 1;
  }

  struct LuksVolume* volumes = NULL;
  if (gather_luks_volumes(&volumes)) return 1;
  if (volumes == NULL) {
    fprintf(stderr, "Found no LUKS volumes!\n");
    return 1;
  }
  show_luks_volumes(&volumes);
  if (open_luks_volumes(&volumes)) return 1;

  return 0;
}
