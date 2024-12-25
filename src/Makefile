CFLAGS =
CFLAGS += -Wall
CFLAGS += -Werror
CFLAGS += -Wextra
CFLAGS += -Wformat
CFLAGS += -Wshadow
CFLAGS += -Wstrict-prototypes
CFLAGS += -Wmissing-prototypes
CFLAGS += -Wno-unused-parameter
CFLAGS += -Wno-unused-variable

INC =
#INC += -Icryptsetup/lib/
#INC += -Iutil-linux/libblkid/src/
#INC += -Iutil-linux/include/

LIB =
LIB += -L/usr/lib64
LIB += -l:libcryptsetup.a
LIB += -l:libblkid.a

mass-luks-open: mass-luks-open.c
	gcc -static $(CFLAGS) $(INC) $^ $(LIB) -o $@
