#!/bin/bash

# called by dracut
check() {
  local fs

  # if cryptsetup is not installed, then we cannot support encrypted devices.
  require_any_binary "$systemdutildir"/systemd-cryptsetup cryptsetup || return 1

  [[ $hostonly ]] || [[ $mount_needs ]] && {
    for fs in "${host_fs_types[@]}"; do
      [[ $fs == "crypto_LUKS" ]] && return 0
    done
    return 255
  }

  return 0
}

# called by dracut
depends() {
  echo dm
}

# called by dracut
installkernel() {
  hostonly=""
  instmods drbg
  instmods dm_crypt

  # in case some of the crypto modules moved from compiled in
  # to module based, try to install those modules
  # best guess
  if [[ $hostonly ]] || [[ $mount_needs ]]; then
    # dmsetup returns s.th. like
    # cryptvol: 0 2064384 crypt aes-xts-plain64 :64:logon:cryptsetup:....
    dmsetup table | while read -r name _ _ is_crypt cipher _; do
      [[ $is_crypt == "crypt" ]] || continue
      # get the device name
      name=/dev/$(dmsetup info -c --noheadings -o blkdevname "${name%:}")
      # check if the device exists as a key in our host_fs_types (even with null string)
      # shellcheck disable=SC2030  # this is a shellcheck bug
      if [[ ${host_fs_types[$name]+_} ]]; then
        # split the cipher aes-xts-plain64 in pieces
        IFS='-:' read -ra mods <<< "$cipher"
        # try to load the cipher part with "crypto-" prepended
        # in non-hostonly mode
        hostonly='' instmods "${mods[@]/#/crypto-}" "crypto-$cipher"
      fi
    done
  else
    instmods "=crypto"
  fi
  return 0
}

# called by dracut
install() {
  inst_binary "$moddir/mass-luks-open" "/usr/bin/mass-luks-open"
  inst_hook pre-mount 97 "$moddir/mass-luks-open.sh"
  dracut_need_initqueue
}
