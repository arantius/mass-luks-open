#!/bin/sh
# https://github.com/dracutdevs/dracut/blob/master/docs/HACKING.md#writing-modules
# "Hooks must have a .sh extension."
# So this is the (.sh) hook, which only calls the binary.
/usr/bin/mass-luks-open
