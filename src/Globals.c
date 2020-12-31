// VisualBoyAdvance - Nintendo Gameboy/GameboyAdvance (TM) emulator.
// Copyright (C) 1999-2003 Forgotten
// Copyright (C) 2004 Forgotten and the VBA development team

// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2, or(at your option)
// any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software Foundation,
// Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

#include <malloc.h>
#include "global.h"
#include "GBA.h"

reg_pair reg[45];
memoryMap map[256];
bool ioReadable[0x400];
bool ALL_FLAGS[4];
bool armState = true;
bool armIrqEnable = true;
u32 armNextPC = 0x00000000;
int armMode = 0x1f;
u32 stop = 0x08000568;
int saveType = 0;
bool useBios = false;
bool skipBios = false;
bool synchronize = true;
bool cpuIsMultiBoot = false;
bool parseDebug = true;
int layerSettings = 0xff00;
int layerEnable = 0xff00;
bool speedHack = false;
int cpuSaveType = 0;
bool cpuEnhancedDetection = true;
bool cheatsEnabled = true;

//LUDO:
int gba_render_skip = 0;

# if 0
u8 bios[ 0x40000 ];
u8 rom[ GBA_ROM_SIZE ];
u8 internalRAM[ 0x8000 ];
u8 workRAM[ 0x40000 ];
u8 paletteRAM[ 0x400 ];
u8 vram[ 0x20000 ];
u8 oam[ 0x400 ];
u8 ioMem[ 0x400 ];
# else
u8 *bios;
u8 *rom;
u8 *internalRAM;
u8 *workRAM;
u8 *paletteRAM;
u8 *vram;
u8 *oam;
u8 *ioMem;
# endif

u16 DISPCNT  = 0x0080;
u16 DISPSTAT = 0x0000;
u16 VCOUNT   = 0x0000;
u16 BG0CNT   = 0x0000;
u16 BG1CNT   = 0x0000;
u16 BG2CNT   = 0x0000;
u16 BG3CNT   = 0x0000;
u16 BG0HOFS  = 0x0000;
u16 BG0VOFS  = 0x0000;
u16 BG1HOFS  = 0x0000;
u16 BG1VOFS  = 0x0000;
u16 BG2HOFS  = 0x0000;
u16 BG2VOFS  = 0x0000;
u16 BG3HOFS  = 0x0000;
u16 BG3VOFS  = 0x0000;
u16 BG2PA    = 0x0100;
u16 BG2PB    = 0x0000;
u16 BG2PC    = 0x0000;
u16 BG2PD    = 0x0100;
u16 BG2X_L   = 0x0000;
u16 BG2X_H   = 0x0000;
u16 BG2Y_L   = 0x0000;
u16 BG2Y_H   = 0x0000;
u16 BG3PA    = 0x0100;
u16 BG3PB    = 0x0000;
u16 BG3PC    = 0x0000;
u16 BG3PD    = 0x0100;
u16 BG3X_L   = 0x0000;
u16 BG3X_H   = 0x0000;
u16 BG3Y_L   = 0x0000;
u16 BG3Y_H   = 0x0000;
u16 WIN0H    = 0x0000;
u16 WIN1H    = 0x0000;
u16 WIN0V    = 0x0000;
u16 WIN1V    = 0x0000;
u16 WININ    = 0x0000;
u16 WINOUT   = 0x0000;
u16 MOSAIC   = 0x0000;
u16 BLDMOD   = 0x0000;
u16 COLEV    = 0x0000;
u16 COLY     = 0x0000;
u16 DM0SAD_L = 0x0000;
u16 DM0SAD_H = 0x0000;
u16 DM0DAD_L = 0x0000;
u16 DM0DAD_H = 0x0000;
u16 DM0CNT_L = 0x0000;
u16 DM0CNT_H = 0x0000;
u16 DM1SAD_L = 0x0000;
u16 DM1SAD_H = 0x0000;
u16 DM1DAD_L = 0x0000;
u16 DM1DAD_H = 0x0000;
u16 DM1CNT_L = 0x0000;
u16 DM1CNT_H = 0x0000;
u16 DM2SAD_L = 0x0000;
u16 DM2SAD_H = 0x0000;
u16 DM2DAD_L = 0x0000;
u16 DM2DAD_H = 0x0000;
u16 DM2CNT_L = 0x0000;
u16 DM2CNT_H = 0x0000;
u16 DM3SAD_L = 0x0000;
u16 DM3SAD_H = 0x0000;
u16 DM3DAD_L = 0x0000;
u16 DM3DAD_H = 0x0000;
u16 DM3CNT_L = 0x0000;
u16 DM3CNT_H = 0x0000;
u16 TM0D     = 0x0000;
u16 TM0CNT   = 0x0000;
u16 TM1D     = 0x0000;
u16 TM1CNT   = 0x0000;
u16 TM2D     = 0x0000;
u16 TM2CNT   = 0x0000;
u16 TM3D     = 0x0000;
u16 TM3CNT   = 0x0000;
u16 P1       = 0xFFFF;
u16 IE       = 0x0000;
u16 IF       = 0x0000;
u16 IME      = 0x0000;

void
global_init_memory(void)
{
  bios = (u8 *)malloc( 0x40000 );
  rom = (u8 *)malloc( GBA_ROM_SIZE );
  internalRAM = (u8 *)malloc( 0x8000 );
  workRAM = (u8 *)malloc( 0x40000 );
  paletteRAM = (u8 *)malloc( 0x400 );
  vram = (u8 *)malloc( 0x20000 );
  oam = (u8 *)malloc( 0x400 );
  ioMem = (u8 *)malloc( 0x400 );
}

