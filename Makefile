TARGET = go_audiofix
OBJS = main.o osd.o logic.o pro_imports.o exports.o

# Kernel PRX: sem CRT/newlib, EBOOT ou alocacao da particao de usuario.
USE_KERNEL_LIBS = 1
PSP_FW_VERSION = 660
INCDIR =
LIBDIR =
LIBS = -lpsprtc_driver
CFLAGS = -Os -std=gnu99 -Wall -Wextra -Werror -fno-pic -fno-common -G0
ASFLAGS = $(CFLAGS)
LDFLAGS =
EXTRA_CLEAN = exports.c

PSPSDK := $(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build_prx.mak

main.o: main.c config.h osd.h logic.h
osd.o: osd.c config.h osd.h logic.h
logic.o: logic.c logic.h config.h
