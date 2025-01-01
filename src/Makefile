CFLAGS =
CFLAGS += -Wall
CFLAGS += -Werror
CFLAGS += -Wextra
CFLAGS += -Wformat
CFLAGS += -Wshadow
CFLAGS += -Wstrict-prototypes
CFLAGS += -Wno-unused-parameter
CFLAGS += -Wno-unused-variable

LIB =
LIB += -lblkid
LIB += -lbsd
LIB += -lcryptsetup

mass-luks-open: mass-luks-open.c
	gcc $(CFLAGS) -o $@ $^ $(LIB)
