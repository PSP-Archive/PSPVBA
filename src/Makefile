#
# pspvba port on PSP 
#
# Copyright (C) 2006 Ludovic Jacomme (ludovic.jacomme@gmail.com)
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
#
TARGET = pspvba
PSPSDK = $(shell psp-config --pspsdk-path)
PSPBIN = $(shell psp-config --psp-prefix)/bin
SDL_CONFIG = $(PSPBIN)/sdl-config

PSP_ME=1

MORE_CFLAGS=$(shell $(SDL_CONFIG) --cflags)  -Wall -std=c99  -O3 -G0

DEFINES= \
-DPACKAGE_NAME=\"\" -DPACKAGE_TARNAME=\"\" -DPACKAGE_VERSION=\"\" \
-DPACKAGE_STRING=\"\" -DPACKAGE_BUGREPORT=\"\" -DPACKAGE=\"VisualBoyAdvance\" \
-DVERSION=\"1.7.2\" -DHAVE_LIBZ=1 -DHAVE_LIBPNG=1 -DSTDC_HEADERS=1 \
-DHAVE_SYS_TYPES_H=1 -DHAVE_SYS_STAT_H=1 -DHAVE_STDLIB_H=1 -DHAVE_STRING_H=1 \
-DHAVE_INTTYPES_H=1 -DHAVE_STDINT_H=1 -DHAVE_UNISTD_H=1 -DHAVE_MALLOC_H=1 \
-DHAVE_UNISTD_H=1 -DHAVE_ARPA_INET_H=1 -DHAVE_NETINET_IN_H=1  -I. \
-I../../src -DSYSCONFDIR=\".\"  -fno-exceptions \
-DC_CORE -DDEV_VERSION -DSDL

ifeq ($(PSP_ME),1)
  ME_OBJS =	me/mestub.o me/me.o me/mem64.o psp_me.o Sound_me.o
  DEFINES+= -DPSP_ME -I./me 
else
  ME_OBJS =	psp_sound.o Sound.o
endif

OBJS = $(ME_OBJS) \
Cheats.o EEprom.o Flash.o GBA.o Gfx.o Globals.o Mode0.o Mode1.o Mode2.o Mode3.o Mode4.o Mode5.o \
RTC.o Sram.o Text.o Util.o armdis.o elf.o \
memgzio.o pixel.o \
unzip.o gba_sdl.o \
psp_main.o \
psp_sdl.o  \
psp_font.o \
psp_fmgr.o \
psp_kbd.o \
psp_menu.o \
psp_menu_kbd.o \
psp_menu_set.o \
psp_battery.o \
psp_gu.o \
global.o 

#  -DWORDS_BIGENDIAN

CFLAGS = $(DEFINES) $(MORE_CFLAGS)
CXXFLAGS = $(DEFINES) $(MORE_CFLAGS) -fno-exceptions -fno-rtti


LIBS += -LUnzipLib -L/usr/local/pspdev/psp/lib -L/usr/local/pspdev/psp/sdk/lib \
-lSDL_image -lSDL -lpng -ljpeg -lGL -lGLU -lm -lc -lz \
-lpspaudiolib -lpspdebug -lpspgu -lpspctrl -lpspge -lpspaudio -lpspdisplay -lpsphprm \
-lpspsdk -lpspuser -lpsputility -lpspnet_inet -lpsppower \
-lpspvfpu -lpsprtc -lpspirkeyb -lpsppower

EXTRA_TARGETS = EBOOT.PBP
PSP_EBOOT_TITLE = PSPVBA
PSP_EBOOT_ICON= pspvba.png
PSP_EBOOT_PIC1 = pspvba-pic.png
PSP_FW_VERSION = 550
BUILD_PRX = 1

include $(PSPSDK)/lib/build.mak

ifeq ($(PSP_ME),1)
install: kxploit
	cp __SCE__pspvba/EBOOT.PBP /media/usbdisk/psp/game/__SCE__pspvba-v1.2.4/
	cp %__SCE__pspvba/EBOOT.PBP /media/usbdisk/psp/game/%__SCE__pspvba-v1.2.4/
else
install:
	cp EBOOT.PBP /media/usbdisk/psp/game/pspvba
endif

realclean:
	/bin/rm -f $(OBJS) EBOOT.BPB PARAM.SFO pspvba/EBOOT.PBP pspvba%/EBOOT.PBP pspvba.elf

