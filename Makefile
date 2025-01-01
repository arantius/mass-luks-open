.PHONY: install uninstall

MODS_DIR = /usr/lib/dracut/modules.d/
MOD_DIR = /usr/lib/dracut/modules.d/20mass-luks-open

install: | $(MODS_DIR)
	make -C src
	install --group=root --owner=root --mode=755 --directory $(MOD_DIR)
	install --group=root --owner=root --mode=644 module-setup.sh $(MOD_DIR)
	install --group=root --owner=root --mode=755 src/mass-luks-open.sh src/mass-luks-open $(MOD_DIR)

uninstall:
	rm -fr $(MOD_DIR)
