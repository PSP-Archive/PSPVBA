// -*- C++ -*-
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

#ifndef VBA_GFX_H
#define VBA_GFX_H

#include <string.h>
#include "GBA.h"
#include "Gfx.h"
#include "Globals.h"

#include "Port.h"

static inline void gfxDrawTextScreen(u16, u16, u16, u32 *);
static inline void gfxDrawRotScreen_ref_ref(u16,
                      u16, u16,
                      u16, u16,
                      u16, u16,
                      u16, u16,
                      int*, int*,
                      int,
                      u32*);
static inline void gfxDrawRotScreen16Bit_ref_ref(u16,
                           u16, u16,
                           u16, u16,
                           u16, u16,
                           u16, u16,
                           int*, int*,
                           int,
                           u32*);
static inline void gfxDrawRotScreen256_ref_ref(u16,
                         u16, u16,
                         u16, u16,
                         u16, u16,
                         u16, u16,
                         int*, int*,
                         int,
                         u32*);
static inline void gfxDrawRotScreen16Bit160_ref_ref(u16,
                              u16, u16,
                              u16, u16,
                              u16, u16,
                              u16, u16,
                              int*, int*,
                              int,
                              u32*);
static inline void gfxDrawSprites();
# if 0 //LUDO: FOR_TEST
static inline void gfxIncreaseBrightness_pint_int(u32 *line, u8 coeff);
static inline void gfxDecreaseBrightness_pint_int(u32 *line, u8 coeff);
# endif
static inline void gfxAlphaBlend_pint_pint_int_int(u32 *ta, u32 *tb, u8 ca, u8 cb);

void mode0RenderLine();
void mode0RenderLineNoWindow();
void mode0RenderLineAll();

void mode1RenderLine();
void mode1RenderLineNoWindow();
void mode1RenderLineAll();

void mode2RenderLine();
void mode2RenderLineNoWindow();
void mode2RenderLineAll();

void mode3RenderLine();
void mode3RenderLineNoWindow();
void mode3RenderLineAll();

void mode4RenderLine();
void mode4RenderLineNoWindow();
void mode4RenderLineAll();

void mode5RenderLine();
void mode5RenderLineNoWindow();
void mode5RenderLineAll();

# if 0
# if 0 //LUDO: FOR_TEST
  static u16 last_val;
  static u16 last_col;
# endif

static inline unsigned short 
systemColorMap16(unsigned short c) 
{
# if 0 //LUDO: FOR_TEST
  if (c == last_col) return last_val;
  last_col = c;
  last_val = ( ((c  & 0x1f)) | (((c & 0x3e0) >> 5) << 6) | (((c & 0x7c00) >> 10) << 11) );
  return last_val;
# else
  return ( ((c  & 0x1f)) | (((c & 0x3e0) >> 5) << 6) | (((c & 0x7c00) >> 10) << 11) );
# endif
}
# else
# define systemColorMap16(c)  ( (((c)  & 0x1f)) | (((c) & 0x3e0)<< 1) | (((c) & 0x7c00) << 1) )
# endif

# define setlineMix(x,color) lineMix[x] = systemColorMap16((u16)color);

extern u8 coeff[32];
extern u32 line0[240];
extern u32 line1[240];
extern u32 line2[240];
extern u32 line3[240];
extern u32 lineOBJ[240];
extern bool __attribute__((aligned(32))) lineOBJWin[240];
extern volatile u16 __attribute__((aligned(64))) lineMixArray[160][240];
extern u16 *lineMix;
extern bool gfxInWin0[240];
extern bool gfxInWin1[240];

extern int gfxBG2Changed;
extern int gfxBG3Changed;

extern int gfxBG2X;
extern int gfxBG2Y;
extern int gfxBG2LastX;
extern int gfxBG2LastY;
extern int gfxBG3X;
extern int gfxBG3Y;
extern int gfxBG3LastX;
extern int gfxBG3LastY;
extern int gfxLastVCOUNT;

static inline void 
gfxClearArray(u32 *array)
{
  for(int i = 0; i < 240; i++) {
    array[i] = 0x80000000;
  }
}

static inline void 
gfxClearObjWinArray(u8* lineOBJWin)
{
  u32* p = (u32*)lineOBJWin;
  for(int i = 0; i < 60; i++) {
    p[i] = 0x00000000;
  }
}

static inline void 
gfxDrawTextScreen(u16 control, u16 hofs, u16 vofs, u32 *line)
{
  u16 *palette = (u16 *)paletteRAM;
  u8 *charBase = &vram[((control >> 2) & 0x03) << 14];
  u16 *screenBase = (u16 *)&vram[((control >> 8) & 0x1f) << 11];
  u32 prio = ((control & 3)<<25) + 0x1000000;
  int sizeX = 256;
  int sizeY = 256;
  unsigned char t = ((control >> 14) & 3);
  if (t & 1) sizeX = 512; 
  if (t & 2) sizeY = 512; 

  int maskX = sizeX-1;
  int maskY = sizeY-1;
  
  bool mosaicOn = (control & 0x40) ? true : false;

  int xxx = hofs & maskX;
  int yyy = (vofs + VCOUNT) & maskY;
  int mosaicX = (MOSAIC & 0x000F)+1;
  int mosaicY = ((MOSAIC & 0x00F0)>>4)+1;

  if(mosaicOn) {
    if((VCOUNT % mosaicY) != 0) {
      mosaicY = (VCOUNT / mosaicY) * mosaicY;
      yyy = (vofs + mosaicY) & maskY;
    }
  }

  if(yyy > 255 && sizeY > 256) {
    yyy &= 255;
    screenBase += 0x400;
    if(sizeX > 256)
      screenBase += 0x400;
  }
  
  int yshift = ((yyy>>3)<<5);
  if((control) & 0x80) {
    u16 *screenSource = screenBase + ((xxx>>8) << 10) + ((xxx & 255)>>3) + yshift;
    for(int x = 0; x < 240; x++) {
      u16 data = READ16LE(screenSource);
      
      int tile = data & 0x3FF;
      int tileX = (xxx & 7);
      int tileY = yyy & 7;
      
      if(data & 0x0400) tileX = 7 - tileX;
      if(data & 0x0800) tileY = 7 - tileY;
      
      u8 color = charBase[(tile << 6) + (tileY << 3) + tileX];
      line[x]  = color ? (READ16LE(&palette[color]) | prio): 0x80000000;
      
      if(data & 0x0400) {
        if(tileX == 0) screenSource++;
      } else 
      if(tileX == 7) screenSource++;

      xxx++;
      if(xxx == 256) {
        if(sizeX > 256)
          screenSource = screenBase + 0x400 + yshift;
        else {
          screenSource = screenBase + yshift;
          xxx = 0;
        }
      } else if(xxx >= sizeX) {
        xxx = 0;
        screenSource = screenBase + yshift;
      }
    }
  } else {
    u16 *screenSource = screenBase + ((xxx>>8) << 10) +((xxx&255)>>3) +
      yshift;
    for(int x = 0; x < 240; x++) {
      u16 data = READ16LE(screenSource);
        
      int tile = data & 0x3FF;
      int tileX = (xxx & 7);
      int tileY = yyy & 7;

      if(data & 0x0400) tileX = 7 - tileX;
      if(data & 0x0800) tileY = 7 - tileY;

      u8 color = charBase[(tile<<5) + (tileY<<2) + (tileX>>1)];

      if(tileX & 1) {
        color = (color >> 4);
      } else {
        color &= 0x0F;
      }
      
      int pal = (READ16LE(screenSource)>>8) & 0xF0;
      line[x] = color ? (READ16LE(&palette[pal + color])|prio): 0x80000000;

      if(data & 0x0400) {
        if(tileX == 0)
          screenSource++;
      } else if(tileX == 7)
        screenSource++;
      xxx++;
      if(xxx == 256) {
        if(sizeX > 256)
          screenSource = screenBase + 0x400 + yshift;
        else {
          screenSource = screenBase + yshift;
          xxx = 0;
        }
      } else if(xxx >= sizeX) {
        xxx = 0;
        screenSource = screenBase + yshift;
      }
    }
  }
  if(mosaicOn) {
    if(mosaicX > 1) {
      int m = 1;
      for(int i = 0; i < 239; i++) {
        line[i+1] = line[i];
        m++;
        if(m == mosaicX) {
          m = 1;
          i++;
        }
      }
    }
  }
}

static inline void 
gfxDrawRotScreen_ref_ref(u16 control, 
                         u16 x_l, u16 x_h,
                         u16 y_l, u16 y_h,
                         u16 pa,  u16 pb,
                         u16 pc,  u16 pd,
                         int* currentX, int* currentY,
                         int changed,
                         u32 *line)
{
  u16 *palette = (u16 *)paletteRAM;
  u8 *charBase = &vram[((control >> 2) & 0x03) << 14];
  u8 *screenBase = (u8 *)&vram[((control >> 8) & 0x1f) << 11];
  int prio = ((control & 3) << 25) + 0x1000000;

  u8 t = ((control >> 14) & 3);
  int sizeX = 128 << t;
  int sizeY = 128 << t;

  int dx = pa & 0x7FFF;
  if(pa & 0x8000)
    dx |= 0xFFFF8000;
  int dmx = pb & 0x7FFF;
  if(pb & 0x8000)
    dmx |= 0xFFFF8000;
  int dy = pc & 0x7FFF;
  if(pc & 0x8000)
    dy |= 0xFFFF8000;
  int dmy = pd & 0x7FFFF;
  if(pd & 0x8000)
    dmy |= 0xFFFF8000;

  if(VCOUNT == 0) changed = 3;

  if(changed & 1) {
    *currentX = (x_l) | ((x_h & 0x07FF)<<16);
    if(x_h & 0x0800)
      *currentX |= 0xF8000000;
  } else {
    *currentX += dmx;
  }

  if(changed & 2) {
    *currentY = (y_l) | ((y_h & 0x07FF)<<16);
    if(y_h & 0x0800)
      *currentY |= 0xF8000000;
  } else {
    *currentY += dmy;
  }  

  if (gba_render_skip) {
    return;
  }
  
  int realX = *currentX;
  int realY = *currentY;

  if(control & 0x40) {
    int mosaicY = ((MOSAIC & 0xF0)>>4) + 1;
    int y = (VCOUNT % mosaicY);
    realX -= y*dmx;
    realY -= y*dmy;
  }
  
  int xxx = (realX >> 8);
  int yyy = (realY >> 8);
  
  if(control & 0x2000) {
    xxx &= sizeX-1;
    yyy &= sizeY-1;
    if(xxx < 0)
      xxx += sizeX;
    if(yyy < 0)
      yyy += sizeY;
  }
  
  if(control & 0x80) {
    for(int x = 0; x < 240; x++) {
      if(xxx < 0 ||
         yyy < 0 ||
         xxx >= sizeX ||
         yyy >= sizeY) {
        line[x] = 0x80000000;
      } else {
        int tile = screenBase[(xxx>>3) + (yyy>>3)*(sizeX>>3)];
        
        int tileX = (xxx & 7);
        int tileY = yyy & 7;
        
        u8 color = charBase[(tile<<6) + (tileY<<3) + tileX];
          
        line[x] = color ? (READ16LE(&palette[color])|prio): 0x80000000;
      }
      realX += dx;
      realY += dy;
      
      xxx = (realX >> 8);
      yyy = (realY >> 8);
      
      if(control & 0x2000) {
        xxx &= sizeX-1;
        yyy &= sizeY-1;
        if(xxx < 0)
          xxx += sizeX;
        if(yyy < 0)
          yyy += sizeY;
      }
    }
  } else {
    for(int x = 0; x < 240; x++) {
      if(xxx < 0 ||
         yyy < 0 ||
         xxx >= sizeX ||
         yyy >= sizeY) {
        line[x] = 0x80000000;
      } else {
        int tile = screenBase[(xxx>>3) + (yyy>>3)*(sizeX>>3)];
        
        int tileX = (xxx & 7);
        int tileY = yyy & 7;
        
        u8 color = charBase[(tile<<6) + (tileY<<3) + tileX];
          
        line[x] = color ? (READ16LE(&palette[color])|prio): 0x80000000;
      }
      realX += dx;
      realY += dy;
      
      xxx = (realX >> 8);
      yyy = (realY >> 8);
      
      if(control & 0x2000) {
        xxx &= sizeX-1;
        yyy &= sizeY-1;
        if(xxx < 0)
          xxx += sizeX;
        if(yyy < 0)
          yyy += sizeY;
      }
    }    
  }

  if(control & 0x40) {    
    int mosaicX = (MOSAIC & 0xF) + 1;
    if(mosaicX > 1) {
      int m = 1;
      for(int i = 0; i < 239; i++) {
        line[i+1] = line[i];
        m++;
        if(m == mosaicX) {
          m = 1;
          i++;
        }
      }
    }
  }  
}

static inline void 
gfxDrawRotScreen16Bit_ref_ref(u16 control,
                                  u16 x_l, u16 x_h,
                                  u16 y_l, u16 y_h,
                                  u16 pa,  u16 pb,
                                  u16 pc,  u16 pd,
                                  int* currentX, int* currentY,
                                  int changed,
                                  u32 *line)
{
  u16 *screenBase = (u16 *)&vram[0];
  int prio = ((control & 3) << 25) + 0x1000000;
  int sizeX = 240;
  int sizeY = 160;
  
  int startX = (x_l) | ((x_h & 0x07FF)<<16);
  if(x_h & 0x0800)
    startX |= 0xF8000000;
  int startY = (y_l) | ((y_h & 0x07FF)<<16);
  if(y_h & 0x0800)
    startY |= 0xF8000000;

  int dx = pa & 0x7FFF;
  if(pa & 0x8000)
    dx |= 0xFFFF8000;
  int dmx = pb & 0x7FFF;
  if(pb & 0x8000)
    dmx |= 0xFFFF8000;
  int dy = pc & 0x7FFF;
  if(pc & 0x8000)
    dy |= 0xFFFF8000;
  int dmy = pd & 0x7FFFF;
  if(pd & 0x8000)
    dmy |= 0xFFFF8000;

  if(VCOUNT == 0)
    changed = 3;
  
  if(changed & 1) {
    *currentX = (x_l) | ((x_h & 0x07FF)<<16);
    if(x_h & 0x0800)
      *currentX |= 0xF8000000;
  } else
    *currentX += dmx;

  if(changed & 2) {
    *currentY = (y_l) | ((y_h & 0x07FF)<<16);
    if(y_h & 0x0800)
      *currentY |= 0xF8000000;
  } else {
    *currentY += dmy;
  }  

  if (gba_render_skip) {
    return;
  }
  
  int realX = *currentX;
  int realY = *currentY;

  if(control & 0x40) {
    int mosaicY = ((MOSAIC & 0xF0)>>4) + 1;
    int y = (VCOUNT % mosaicY);
    realX -= y*dmx;
    realY -= y*dmy;
  }
  
  int xxx = (realX >> 8);
  int yyy = (realY >> 8);
  
  for(int x = 0; x < 240; x++) {
    if(xxx < 0 ||
       yyy < 0 ||
       xxx >= sizeX ||
       yyy >= sizeY) {
      line[x] = 0x80000000;
    } else {
      line[x] = (READ16LE(&screenBase[yyy * sizeX + xxx]) | prio);
    }
    realX += dx;
    realY += dy;
    
    xxx = (realX >> 8);
    yyy = (realY >> 8);
  }

  if(control & 0x40) {    
    int mosaicX = (MOSAIC & 0xF) + 1;
    if(mosaicX > 1) {
      int m = 1;
      for(int i = 0; i < 239; i++) {
        line[i+1] = line[i];
        m++;
        if(m == mosaicX) {
          m = 1;
          i++;
        }
      }
    }
  }  
}

static inline void 
gfxDrawRotScreen256_ref_ref(u16 control, 
                                u16 x_l, u16 x_h,
                                u16 y_l, u16 y_h,
                                u16 pa,  u16 pb,
                                u16 pc,  u16 pd,
                                int *currentX, int* currentY,
                                int changed,
                                u32 *line)
{
  u16 *palette = (u16 *)paletteRAM;
  u8 *screenBase = (DISPCNT & 0x0010) ? &vram[0xA000] : &vram[0x0000];
  int prio = ((control & 3) << 25) + 0x1000000;
  int sizeX = 240;
  int sizeY = 160;
  
  int startX = (x_l) | ((x_h & 0x07FF)<<16);
  if(x_h & 0x0800)
    startX |= 0xF8000000;
  int startY = (y_l) | ((y_h & 0x07FF)<<16);
  if(y_h & 0x0800)
    startY |= 0xF8000000;

  int dx = pa & 0x7FFF;
  if(pa & 0x8000)
    dx |= 0xFFFF8000;
  int dmx = pb & 0x7FFF;
  if(pb & 0x8000)
    dmx |= 0xFFFF8000;
  int dy = pc & 0x7FFF;
  if(pc & 0x8000)
    dy |= 0xFFFF8000;
  int dmy = pd & 0x7FFFF;
  if(pd & 0x8000)
    dmy |= 0xFFFF8000;

  if(VCOUNT == 0)
    changed = 3;

  if(changed & 1) {
    *currentX = (x_l) | ((x_h & 0x07FF)<<16);
    if(x_h & 0x0800)
      *currentX |= 0xF8000000;
  } else {
    *currentX += dmx;
  }

  if(changed & 2) {
    *currentY = (y_l) | ((y_h & 0x07FF)<<16);
    if(y_h & 0x0800)
      *currentY |= 0xF8000000;
  } else {
    *currentY += dmy;
  }  
  
  if (gba_render_skip) {
    return;
  }

  int realX = *currentX;
  int realY = *currentY;

  if(control & 0x40) {
    int mosaicY = ((MOSAIC & 0xF0)>>4) + 1;
    int y = (VCOUNT / mosaicY) * mosaicY;
    realX = startX + y*dmx;
    realY = startY + y*dmy;
  }
  
  int xxx = (realX >> 8);
  int yyy = (realY >> 8);
  
  for(int x = 0; x < 240; x++) {
    if(xxx < 0 ||
         yyy < 0 ||
       xxx >= sizeX ||
       yyy >= sizeY) {
      line[x] = 0x80000000;
    } else {
      u8 color = screenBase[yyy * 240 + xxx];
      
      line[x] = color ? (READ16LE(&palette[color])|prio): 0x80000000;
    }
    realX += dx;
    realY += dy;
    
    xxx = (realX >> 8);
    yyy = (realY >> 8);
  }

  if(control & 0x40) {    
    int mosaicX = (MOSAIC & 0xF) + 1;
    if(mosaicX > 1) {
      int m = 1;
      for(int i = 0; i < 239; i++) {
        line[i+1] = line[i];
        m++;
        if(m == mosaicX) {
          m = 1;
          i++;
        }
      }
    }
  }    
}

static inline void 
gfxDrawRotScreen16Bit160_ref_ref(u16 control,
                                     u16 x_l, u16 x_h,
                                     u16 y_l, u16 y_h,
                                     u16 pa,  u16 pb,
                                     u16 pc,  u16 pd,
                                     int* currentX, int* currentY,
                                     int changed,
                                     u32 *line)
{
  u16 *screenBase = (DISPCNT & 0x0010) ? (u16 *)&vram[0xa000] :
    (u16 *)&vram[0];
  int prio = ((control & 3) << 25) + 0x1000000;
  int sizeX = 160;
  int sizeY = 128;
  
  int startX = (x_l) | ((x_h & 0x07FF)<<16);
  if(x_h & 0x0800) startX |= 0xF8000000;
  int startY = (y_l) | ((y_h & 0x07FF)<<16);
  if(y_h & 0x0800) startY |= 0xF8000000;

  int dx = pa & 0x7FFF;
  if(pa & 0x8000) dx |= 0xFFFF8000;
  int dmx = pb & 0x7FFF;
  if(pb & 0x8000) dmx |= 0xFFFF8000;
  int dy = pc & 0x7FFF;
  if(pc & 0x8000) dy |= 0xFFFF8000;
  int dmy = pd & 0x7FFFF;
  if(pd & 0x8000) dmy |= 0xFFFF8000;

  if(VCOUNT == 0) changed = 3;

  if(changed & 1) {
    *currentX = (x_l) | ((x_h & 0x07FF)<<16);
    if(x_h & 0x0800)
      *currentX |= 0xF8000000;
  } else {
    *currentX += dmx;
  }

  if(changed & 2) {
    *currentY = (y_l) | ((y_h & 0x07FF)<<16);
    if(y_h & 0x0800)
      *currentY |= 0xF8000000;
  } else {
    *currentY += dmy;
  }  

  if (gba_render_skip) {
    return;
  }
  
  int realX = *currentX;
  int realY = *currentY;

  if(control & 0x40) {
    int mosaicY = ((MOSAIC & 0xF0)>>4) + 1;
    int y = (VCOUNT / mosaicY) * mosaicY;
    realX = startX + y*dmx;
    realY = startY + y*dmy;
  }
  
  int xxx = (realX >> 8);
  int yyy = (realY >> 8);
  
  for(int x = 0; x < 240; x++) {
    if(xxx < 0 ||
       yyy < 0 ||
       xxx >= sizeX ||
       yyy >= sizeY) {
      line[x] = 0x80000000;
    } else {
      line[x] = (READ16LE(&screenBase[yyy * sizeX + xxx]) | prio);
    }
    realX += dx;
    realY += dy;
    
    xxx = (realX >> 8);
    yyy = (realY >> 8);
  }

  if(control & 0x40) {    
    int mosaicX = (MOSAIC & 0xF) + 1;
    if(mosaicX > 1) {
      int m = 1;
      for(int i = 0; i < 239; i++) {
        line[i+1] = line[i];
        m++;
        if(m == mosaicX) {
          m = 1;
          i++;
        }
      }
    }
  }      
}

static u8 loc_size_x_array[] = { 8, 16, 32, 64, 16, 32, 32, 64,  8,  8, 16, 32 };
static u8 loc_size_y_array[] = { 8, 16, 32, 64,  8,  8, 16, 32, 16, 32, 32, 64 };

static inline void 
gfxDrawSprites()
{
  int m=0;

  gfxClearArray(lineOBJ);

  if(layerEnable & 0x1000) {
    u16 *sprites = (u16 *)oam;
    u16 *spritePalette = &((u16 *)paletteRAM)[256];
    int mosaicY = ((MOSAIC & 0xF000)>>12) + 1;
    int mosaicX = ((MOSAIC & 0xF00)>>8) + 1;    
    for(int x = 0; x < 128 ; x++) {
      u16 a0 = READ16LE(sprites++);
      // ignore OBJ-WIN
      if((a0 & 0x0c00) == 0x0800) {
         sprites += 3;
         continue;
      }

      u16 a1 = READ16LE(sprites++);
      u16 a2 = READ16LE(sprites++);
      sprites++;

      u8 icoef = (((a0 >>12) & 0x0c)|(a1>>14));

      int sizeY;
      int sizeX;

      if (icoef < 12) {
        sizeX = loc_size_x_array[icoef];
        sizeY = loc_size_y_array[icoef];
      } else continue;

      int sy = (a0 & 255);

      if(sy > 160) sy -= 256;
      
      if(a0 & 0x0100) {
        int fieldX = sizeX;
        int fieldY = sizeY;
        if(a0 & 0x0200) {
          fieldX <<= 1;
          fieldY <<= 1;
        }
        
        int t = VCOUNT - sy;
        if((t >= 0) && (t < fieldY)) {
          int sx = (a1 & 0x1FF);
          if((sx < 240) || (((sx + fieldX) & 511) < 240)) {
            // int t2 = t - (fieldY >> 1);
            int rot = (a1 >> 9) & 0x1F;
            u16 *OAM = (u16 *)oam;
            int dx = READ16LE(&OAM[3 + (rot << 4)]);
            if(dx & 0x8000) dx |= 0xFFFF8000;
            int dmx = READ16LE(&OAM[7 + (rot << 4)]);
            if(dmx & 0x8000) dmx |= 0xFFFF8000;
            int dy = READ16LE(&OAM[11 + (rot << 4)]);
            if(dy & 0x8000) dy |= 0xFFFF8000;
            int dmy = READ16LE(&OAM[15 + (rot << 4)]);
            if(dmy & 0x8000) dmy |= 0xFFFF8000;
            
            if(a0 & 0x1000) {
              t -= (t % mosaicY);
            }

            int realX = ((sizeX) << 7) - (fieldX >> 1)*dx - (fieldY>>1)*dmx
              + t * dmx;
            int realY = ((sizeY) << 7) - (fieldX >> 1)*dy - (fieldY>>1)*dmy
              + t * dmy;

            u32 prio = (((a2 >> 10) & 3) << 25) | ((a0 & 0x0c00)<<6);
            
            if(a0 & 0x2000) {
              int c = (a2 & 0x3FF);
              if((DISPCNT & 7) > 2 && (c < 512))
                continue;
              int inc = 32;
              if(DISPCNT & 0x40)
                inc = sizeX >> 2;
              else
                c &= 0x3FE;
              for(int x = 0; x < fieldX; x++) {
                int xxx = realX >> 8;
                int yyy = realY >> 8;
                
                if(xxx < 0 || xxx >= sizeX ||
                   yyy < 0 || yyy >= sizeY ||
                   sx >= 240);
                else {
                  u32 color = vram[0x10000 + ((((c + (yyy>>3) * inc)<<5)
                  + ((yyy & 7)<<3) + ((xxx >> 3)<<6) +
                                    (xxx & 7))&0x7FFF)];
                  if ((color==0) && (((prio >> 25)&3) < 
                                     ((lineOBJ[sx]>>25)&3))) {
                    lineOBJ[sx] = (lineOBJ[sx] & 0xF9FFFFFF) | prio;
                    if((a0 & 0x1000) && m)
                      lineOBJ[sx]=(lineOBJ[sx-1] & 0xF9FFFFFF) | prio;
                  } else if((color) && (prio < (lineOBJ[sx]&0xFF000000))) {
                    lineOBJ[sx] = READ16LE(&spritePalette[color]) | prio;
                    if((a0 & 0x1000) && m)
                      lineOBJ[sx]=(lineOBJ[sx-1] & 0xF9FFFFFF) | prio;
                  }

                  if (a0 & 0x1000) {
                    m++;
                    if (m==mosaicX)
                      m=0;
                  }
                }
                sx = (sx+1)&511;;
                realX += dx;
                realY += dy;
              }
            } else {
              int c = (a2 & 0x3FF);
              if((DISPCNT & 7) > 2 && (c < 512))
                continue;
              
              int inc = 32;
              if(DISPCNT & 0x40)
                inc = sizeX >> 3;
              int palette = (a2 >> 8) & 0xF0;                 
              for(int x = 0; x < fieldX; x++) {
                int xxx = realX >> 8;
                int yyy = realY >> 8;
                if(xxx < 0 || xxx >= sizeX ||
                   yyy < 0 || yyy >= sizeY ||
                   sx >= 240);
                else {
                  u32 color = vram[0x10000 + ((((c + (yyy>>3) * inc)<<5)
                                                + ((yyy & 7)<<2) + ((xxx >> 3)<<5) +
                                               ((xxx & 7)>>1))&0x7FFF)];
                  if(xxx & 1)
                    color >>= 4;
                  else
                    color &= 0x0F;
                  
                  if ((color==0) && (((prio >> 25)&3) < 
                                     ((lineOBJ[sx]>>25)&3))) {
                    lineOBJ[sx] = (lineOBJ[sx] & 0xF9FFFFFF) | prio;
                    if((a0 & 0x1000) && m)
                      lineOBJ[sx]=(lineOBJ[sx-1] & 0xF9FFFFFF) | prio;
                  } else if((color) && (prio < (lineOBJ[sx]&0xFF000000))) {
                    lineOBJ[sx] = READ16LE(&spritePalette[palette+color]) | prio;
                    if((a0 & 0x1000) && m)
                      lineOBJ[sx]=(lineOBJ[sx-1] & 0xF9FFFFFF) | prio;
                  }
                }
                if((a0 & 0x1000) && m) {
                  m++;
                  if (m==mosaicX)
                    m=0;
                }
                sx = (sx+1)&511;;
                realX += dx;
                realY += dy;
                
              }       
            }
          }
        }
      } else {
        int t = VCOUNT - sy;
        if((t >= 0) && (t < sizeY)) {
          int sx = (a1 & 0x1FF);
          if(((sx < 240)||(((sx+sizeX)&511)<240)) && !(a0 & 0x0200)) {
            if(a0 & 0x2000) {
              if(a1 & 0x2000)
                t = sizeY - t - 1;
              int c = (a2 & 0x3FF);
              if((DISPCNT & 7) > 2 && (c < 512))
                continue;
              
              int inc = 32;
              if(DISPCNT & 0x40) {
                inc = sizeX >> 2;
              } else {
                c &= 0x3FE;
              }
              int xxx = 0;
              if(a1 & 0x1000)
                xxx = sizeX-1;

              if(a0 & 0x1000) {
                t -= (t % mosaicY);
              }

              int address = 0x10000 + ((((c+ (t>>3) * inc) << 5)
                + ((t & 7) << 3) + ((xxx>>3)<<6) + (xxx & 7)) & 0x7FFF);
                
              if(a1 & 0x1000)
                xxx = 7;
              u32 prio = (((a2 >> 10) & 3) << 25) | ((a0 & 0x0c00)<<6);
                
              for(int xx = 0; xx < sizeX; xx++) {
                if(sx < 240) {
                  u8 color = vram[address];
                  if ((color==0) && (((prio >> 25)&3) < 
                                     ((lineOBJ[sx]>>25)&3))) {
                    lineOBJ[sx] = (lineOBJ[sx] & 0xF9FFFFFF) | prio;
                    if((a0 & 0x1000) && m)
                      lineOBJ[sx]=(lineOBJ[sx-1] & 0xF9FFFFFF) | prio;
                  } else if((color) && (prio < (lineOBJ[sx]&0xFF000000))) {
                    lineOBJ[sx] = READ16LE(&spritePalette[color]) | prio;
                    if((a0 & 0x1000) && m)
                      lineOBJ[sx]=(lineOBJ[sx-1] & 0xF9FFFFFF) | prio;
                  }

                  if (a0 & 0x1000) {
                    m++;
                    if (m==mosaicX)
                      m=0;
                  }
                }
                  
                sx = (sx+1) & 511;
                if(a1 & 0x1000) {
                  xxx--;
                  address--;
                  if(xxx == -1) {
                    address -= 56;
                    xxx = 7;
                  }
                  if(address < 0x10000)
                    address += 0x8000;
                } else {
                  xxx++;
                  address++;
                  if(xxx == 8) {
                    address += 56;
                    xxx = 0;
                  }
                  if(address > 0x17fff)
                    address -= 0x8000;
                }
              }
            } else {
              if(a1 & 0x2000)
                t = sizeY - t - 1;
              int c = (a2 & 0x3FF);
              if((DISPCNT & 7) > 2 && (c < 512))
                continue;
              
              int inc = 32;
              if(DISPCNT & 0x40) {
                inc = sizeX >> 3;
              }
              int xxx = 0;
              if(a1 & 0x1000)
                xxx = sizeX - 1;
                  
                if(a0 & 0x1000) {
                  t -= (t % mosaicY);
                }

              int address = 0x10000 + ((((c + (t>>3) * inc)<<5)
                + ((t & 7)<<2) + ((xxx>>3)<<5) + ((xxx & 7) >> 1))&0x7FFF);
              u32 prio = (((a2 >> 10) & 3) << 25) | ((a0 & 0x0c00)<<6);
              int palette = (a2 >> 8) & 0xF0;         
              if(a1 & 0x1000) {
                xxx = 7;
                for(int xx = sizeX - 1; xx >= 0; xx--) {
                  if(sx < 240) {
                    u8 color = vram[address];
                    if(xx & 1) {
                      color = (color >> 4);
                    } else
                      color &= 0x0F;
                    
                    if ((color==0) && (((prio >> 25)&3) < 
                                       ((lineOBJ[sx]>>25)&3))) {
                      lineOBJ[sx] = (lineOBJ[sx] & 0xF9FFFFFF) | prio;
                      if((a0 & 0x1000) && m)
                        lineOBJ[sx]=(lineOBJ[sx-1] & 0xF9FFFFFF) | prio;
                    } else if((color) && (prio < (lineOBJ[sx]&0xFF000000))) {
                      lineOBJ[sx] = READ16LE(&spritePalette[palette + color]) | prio;
                      if((a0 & 0x1000) && m)
                        lineOBJ[sx]=(lineOBJ[sx-1] & 0xF9FFFFFF) | prio;
                    }
                  }
                  if (a0 & 0x1000) {
                    m++;
                    if (m==mosaicX)
                      m=0;
                  }
                  sx = (sx+1) & 511;
                  xxx--;
                  if(!(xx & 1))
                    address--;
                  if(xxx == -1) {
                    xxx = 7;
                    address -= 28;
                  }
                  if(address < 0x10000)
                    address += 0x8000;
                }           
              } else {        
                for(int xx = 0; xx < sizeX; xx++) {
                  if(sx < 240) {
                    u8 color = vram[address];
                    if(xx & 1) {
                      color = (color >> 4);
                    } else
                      color &= 0x0F;
                    
                    if ((color==0) && (((prio >> 25)&3) < 
                                       ((lineOBJ[sx]>>25)&3))) {
                      lineOBJ[sx] = (lineOBJ[sx] & 0xF9FFFFFF) | prio;
                      if((a0 & 0x1000) && m)
                        lineOBJ[sx]=(lineOBJ[sx-1] & 0xF9FFFFFF) | prio;
                    } else if((color) && (prio < (lineOBJ[sx]&0xFF000000))) {
                      lineOBJ[sx] = READ16LE(&spritePalette[palette + color]) | prio;
                      if((a0 & 0x1000) && m)
                        lineOBJ[sx]=(lineOBJ[sx-1] & 0xF9FFFFFF) | prio;

                    }
                  }
                  if (a0 & 0x1000) {
                    m++;
                    if (m==mosaicX)
                      m=0;
                  }
                  sx = (sx+1) & 511;
                  xxx++;
                  if(xx & 1)
                    address++;
                  if(xxx == 8) {
                    address += 28;
                    xxx = 0;
                  }
                  if(address > 0x17fff)
                    address -= 0x8000;
                }           
              }
            }
          }
        }
      }
    }
  }
}

static inline void 
gfxDrawOBJWin()
{
  gfxClearObjWinArray(lineOBJWin);
  if(layerEnable & 0x8000) {
    u16 *sprites = (u16 *)oam;
    // u16 *spritePalette = &((u16 *)paletteRAM)[256];
    for(int x = 0; x < 128 ; x++) {
      u16 a0 = READ16LE(sprites++);
      // ignore non OBJ-WIN
      if((a0 & 0x0c00) != 0x0800) {
         sprites += 3;
         continue;
      }
      u16 a1 = READ16LE(sprites++);
      
      u8 icoef = ((a0 >>12) & 0x0c)|(a1>>14);

      int sizeY;
      int sizeX;

      if (icoef < 12) {
        sizeX = loc_size_x_array[icoef];
        sizeY = loc_size_y_array[icoef];
      } else {
        sprites += 2;
        continue;
      }

      u16 a2 = READ16LE(sprites++);
      sprites++;

      int sy = (a0 & 255);
      if(sy > 160) sy -= 256;
      
      if(a0 & 0x0100) {
        int fieldX = sizeX;
        int fieldY = sizeY;
        if(a0 & 0x0200) {
          fieldX <<= 1;
          fieldY <<= 1;
        }
        
        int t = VCOUNT - sy;
        if((t >= 0) && (t < fieldY)) {
          int sx = (a1 & 0x1FF);
          if((sx < 240) || (((sx + fieldX) & 511) < 240)) {
            if(a0 & 0x2000) {
              int c = (a2 & 0x3FF);
              if((DISPCNT & 7) > 2 && (c < 512)) continue;

              int inc = 32;
              if(DISPCNT & 0x40) inc = sizeX >> 2;
              else c &= 0x3FE;

            int rot = (a1 >> 9) & 0x1F;
            u16 *OAM = (u16 *)oam;
            int dx = READ16LE(&OAM[3 + (rot << 4)]);
            if(dx & 0x8000) dx |= 0xFFFF8000;
            int dmx = READ16LE(&OAM[7 + (rot << 4)]);
            if(dmx & 0x8000) dmx |= 0xFFFF8000;
            int dy = READ16LE(&OAM[11 + (rot << 4)]);
            if(dy & 0x8000) dy |= 0xFFFF8000;
            int dmy = READ16LE(&OAM[15 + (rot << 4)]);
            if(dmy & 0x8000) dmy |= 0xFFFF8000;
            
            int realX = ((sizeX) << 7) - (fieldX >> 1)*dx - (fieldY>>1)*dmx + t * dmx;
            int realY = ((sizeY) << 7) - (fieldX >> 1)*dy - (fieldY>>1)*dmy + t * dmy;

              for(int x = 0; x < fieldX; x++) {
                int xxx = realX >> 8;
                int yyy = realY >> 8;
                
                if(xxx < 0 || xxx >= sizeX ||
                   yyy < 0 || yyy >= sizeY) {
                } else {
                  u32 color = vram[0x10000 + ((((c + (yyy>>3) * inc)<<5)
                                    + ((yyy & 7)<<3) + ((xxx >> 3)<<6) +
                                   (xxx & 7))&0x7fff)];
                  if(color) {
                    lineOBJWin[sx] = 1;
                  }
                }
                sx = (sx+1)&511;;
                realX += dx;
                realY += dy;
              }
            } else {
              int c = (a2 & 0x3FF);
              if((DISPCNT & 7) > 2 && (c < 512)) continue;
              
              int inc = 32;
              if(DISPCNT & 0x40) inc = sizeX >> 3;


            int rot = (a1 >> 9) & 0x1F;
            u16 *OAM = (u16 *)oam;
            int dx = READ16LE(&OAM[3 + (rot << 4)]);
            if(dx & 0x8000) dx |= 0xFFFF8000;
            int dmx = READ16LE(&OAM[7 + (rot << 4)]);
            if(dmx & 0x8000) dmx |= 0xFFFF8000;
            int dy = READ16LE(&OAM[11 + (rot << 4)]);
            if(dy & 0x8000) dy |= 0xFFFF8000;
            int dmy = READ16LE(&OAM[15 + (rot << 4)]);
            if(dmy & 0x8000) dmy |= 0xFFFF8000;
            
            int realX = ((sizeX) << 7) - (fieldX >> 1)*dx - (fieldY>>1)*dmx + t * dmx;
            int realY = ((sizeY) << 7) - (fieldX >> 1)*dy - (fieldY>>1)*dmy + t * dmy;


              // int palette = (a2 >> 8) & 0xF0;                      
              for(int x = 0; x < fieldX; x++) {
                int xxx = realX >> 8;
                int yyy = realY >> 8;

                  if(xxx < 0 || xxx >= sizeX ||
                     yyy < 0 || yyy >= sizeY){
                  } else {
                    u32 color = vram[0x10000 + ((((c + (yyy>>3) * inc)<<5)
                                     + ((yyy & 7)<<2) + ((xxx >> 3)<<5) +
                                     ((xxx & 7)>>1))&0x7fff)];
                    if(xxx & 1)
                      color >>= 4;
                    else
                      color &= 0x0F;
                    
                    if(color) {
                      lineOBJWin[sx] = 1;
                    }
                  }
                sx = (sx+1)&511;;
                realX += dx;
                realY += dy;
              }       
            }
          }
        }
      } else {
        int t = VCOUNT - sy;
        if((t >= 0) && (t < sizeY)) {
          int sx = (a1 & 0x1FF);
          if(((sx < 240)||(((sx+sizeX)&511)<240)) && !(a0 & 0x0200)) {
            if(a0 & 0x2000) {
              if(a1 & 0x2000)
                t = sizeY - t - 1;
              int c = (a2 & 0x3FF);
              if((DISPCNT & 7) > 2 && (c < 512))
                continue;
              
              int inc = 32;
              if(DISPCNT & 0x40) {
                inc = sizeX >> 2;
              } else {
                c &= 0x3FE;
              }
              int xxx = 0;
              if(a1 & 0x1000)
                xxx = sizeX-1;
              int address = 0x10000 + ((((c+ (t>>3) * inc) << 5)
                + ((t & 7) << 3) + ((xxx>>3)<<6) + (xxx & 7))&0x7fff);
              if(a1 & 0x1000)
                xxx = 7;
              // u32 prio = (((a2 >> 10) & 3) << 25) | ((a0 & 0x0c00)<<6);
              for(int xx = 0; xx < sizeX; xx++) {
                if(sx < 240) {
                  u8 color = vram[address];
                  if(color) {
                    lineOBJWin[sx] = 1;
                  }
                }
                  
                sx = (sx+1) & 511;
                if(a1 & 0x1000) {
                  xxx--;
                  address--;
                  if(xxx == -1) {
                    address -= 56;
                    xxx = 7;
                  }
                  if(address < 0x10000)
                    address += 0x8000;
                } else {
                  xxx++;
                  address++;
                  if(xxx == 8) {
                    address += 56;
                    xxx = 0;
                  }
                  if(address > 0x17fff)
                    address -= 0x8000;
                }
              }
            } else {
              if(a1 & 0x2000)
                t = sizeY - t - 1;
              int c = (a2 & 0x3FF);
              if((DISPCNT & 7) > 2 && (c < 512))
                continue;
              
              int inc = 32;
              if(DISPCNT & 0x40) {
                inc = sizeX >> 3;
              }
              int xxx = 0;
              if(a1 & 0x1000)
                xxx = sizeX - 1;
              int address = 0x10000 + ((((c + (t>>3) * inc)<<5)
                + ((t & 7)<<2) + ((xxx>>3)<<5) + ((xxx & 7) >> 1))&0x7fff);
              // u32 prio = (((a2 >> 10) & 3) << 25) | ((a0 & 0x0c00)<<6);
              // int palette = (a2 >> 8) & 0xF0;              
              if(a1 & 0x1000) {
                xxx = 7;
                for(int xx = sizeX - 1; xx >= 0; xx--) {
                  if(sx < 240) {
                    u8 color = vram[address];
                    if(xx & 1) {
                      color = (color >> 4);
                    } else
                      color &= 0x0F;
                    
                    if(color) {
                      lineOBJWin[sx] = 1;
                    }
                  }
                  sx = (sx+1) & 511;
                  xxx--;
                  if(!(xx & 1))
                    address--;
                  if(xxx == -1) {
                    xxx = 7;
                    address -= 28;
                  }
                  if(address < 0x10000)
                    address += 0x8000;
                }           
              } else {        
                for(int xx = 0; xx < sizeX; xx++) {
                  if(sx < 240) {
                    u8 color = vram[address];
                    if(xx & 1) {
                      color = (color >> 4);
                    } else
                      color &= 0x0F;
                    
                    if(color) {
                      lineOBJWin[sx] = 1;
                    }
                  }
                  sx = (sx+1) & 511;
                  xxx++;
                  if(xx & 1)
                    address++;
                  if(xxx == 8) {
                    address += 28;
                    xxx = 0;
                  }
                  if(address > 0x17fff)
                    address -= 0x8000;
                }           
              }
            }
          }
        }
      }
    }
  }
}

# if 0 //LUDO: FOR_TEST
static inline u32 
gfxIncreaseBrightness_int_int(u32 color, u8 coeff)
{
  int r = (color & 0x1F);
  int g = ((color >> 5) & 0x1F);
  int b = ((color >> 10) & 0x1F);
  
  r = r + (((31 - r) * coeff) >> 4);
  g = g + (((31 - g) * coeff) >> 4);
  b = b + (((31 - b) * coeff) >> 4);
  if(r > 31) r = 31;
  if(g > 31) g = 31;
  if(b > 31) b = 31;
  color = (color & 0xFFFF0000) | (b << 10) | (g << 5) | r;
  return color;
}
# endif

# if 0 //LUDO: FOR_TEST
static inline void 
gfxIncreaseBrightness_pint_int(u32 *line, u8 coeff)
{
  for(int x = 0; x < 240; x++) {
    u32 color = *line;
    int r = (color & 0x1F);
    int g = ((color >> 5) & 0x1F);
    int b = ((color >> 10) & 0x1F);
    
    r = r + (((31 - r) * coeff) >> 4);
    g = g + (((31 - g) * coeff) >> 4);
    b = b + (((31 - b) * coeff) >> 4);
    if(r > 31)
      r = 31;
    if(g > 31)
      g = 31;
    if(b > 31)
      b = 31;
    *line++ = (color & 0xFFFF0000) | (b << 10) | (g << 5) | r;
  }
}
# endif

# if 0 //LUDO: FOR_TEST
static inline u32 
gfxDecreaseBrightness_int_int(u32 color, u8 coeff)
{
  int r = (color & 0x1F);
  int g = ((color >> 5) & 0x1F);
  int b = ((color >> 10) & 0x1F);
  
  r = r - ((r * coeff) >> 4);
  g = g - ((g * coeff) >> 4);
  b = b - ((b * coeff) >> 4);
  if(r < 0)
    r = 0;
  if(g < 0)
    g = 0;
  if(b < 0)
    b = 0;
  color = (color & 0xFFFF0000) | (b << 10) | (g << 5) | r;
  return color;
}
# endif

# if 0 //LUDO: FOR_TEST
static inline void 
gfxDecreaseBrightness_pint_int(u32 *line, u8 coeff)
{
  for(int x = 0; x < 240; x++) {
    u32 color = *line;
    int r = (color & 0x1F);
    int g = ((color >> 5) & 0x1F);
    int b = ((color >> 10) & 0x1F);
    
    r = r - ((r * coeff) >> 4);
    g = g - ((g * coeff) >> 4);
    b = b - ((b * coeff) >> 4);
    if(r < 0)
      r = 0;
    if(g < 0)
      g = 0;
    if(b < 0)
      b = 0;
    *line++ = (color & 0xFFFF0000) | (b << 10) | (g << 5) | r;
  }
}
# endif

static inline u32 gfxAlphaBlend_int_int_int_int(u32 color, u32 color2, u8 ca, u8 cb)
{
  if(color < 0x80000000) {
    u16 r = (color & 0x1F);
    u16 g = ((color >> 5) & 0x1F);
    u16 b = ((color >> 10) & 0x1F);
    u16 r0 = (color2 & 0x1F);
    u16 g0 = ((color2 >> 5) & 0x1F);
    u16 b0 = ((color2 >> 10) & 0x1F);

    if (cb == 16) {
    } else
    if (cb == 8) {
      r0 >>= 1;
      g0 >>= 1;
      b0 >>= 1;
    } else
    if (cb == 4) {
      r0 >>= 2;
      g0 >>= 2;
      b0 >>= 2;
    } else 
    if (cb == 2) {
      r0 >>= 3;
      g0 >>= 3;
      b0 >>= 3;
    } else {
      r0 = ((r0 * cb) >> 4);
      g0 = ((g0 * cb) >> 4);
      b0 = ((b0 * cb) >> 4);
    }

    if (ca == 16) {
    } else
    if (ca == 8) {
      r >>= 1;
      g >>= 1;
      b >>= 1;
    } else
    if (ca == 4) {
      r >>= 2;
      g >>= 2;
      b >>= 2;
    } else 
    if (ca == 2) {
      r >>= 3;
      g >>= 3;
      b >>= 3;
    } else {
      r = ((r * ca) >> 4);
      g = ((g * ca) >> 4);
      b = ((b * ca) >> 4);
    }
    
    r += r0;
    g += g0;
    b += b0;
    
    if(r > 31) r = 31;
    if(g > 31) g = 31;
    if(b > 31) b = 31;

    return (color & 0xFFFF0000) | (b << 10) | (g << 5) | r;
  }
  return color;
}

static inline void 
gfxAlphaBlend_pint_pint_int_int(u32 *ta, u32 *tb, u8 ca, u8 cb)
{
  u32 prev_color = 0;
  u32 prev_color2 = 0;
  u32 prev_res = 0;
  
  for(int x = 0; x < 240; x++) {
    u32 color = *ta;
    if (color < 0x80000000) {
      u32 color2 = (*tb++);
      if ((color != prev_color) || (color2 != prev_color2)) {
        u16 r = (color & 0x1F);
        u16 g = ((color >> 5) & 0x1F);
        u16 b = ((color >> 10) & 0x1F);
        u16 r0 = (color2 & 0x1F);
        u16 g0 = ((color2 >> 5) & 0x1F);
        u16 b0 = ((color2 >> 10) & 0x1F);

        if (cb == 16) {
        } else
        if (cb == 8) {
          r0 >>= 1;
          g0 >>= 1;
          b0 >>= 1;
        } else
        if (cb == 4) {
          r0 >>= 2;
          g0 >>= 2;
          b0 >>= 2;
        } else 
        if (cb == 2) {
          r0 >>= 3;
          g0 >>= 3;
          b0 >>= 3;
        } else {
          r0 = ((r0 * cb) >> 4);
          g0 = ((g0 * cb) >> 4);
          b0 = ((b0 * cb) >> 4);
        }
       
        if (ca == 16) {
        } else
        if (ca == 8) {
          r >>= 1;
          g >>= 1;
          b >>= 1;
        } else
        if (ca == 4) {
          r >>= 2;
          g >>= 2;
          b >>= 2;
        } else 
        if (ca == 2) {
          r >>= 3;
          g >>= 3;
          b >>= 3;
        } else {
          r = ((r * ca) >> 4);
          g = ((g * ca) >> 4);
          b = ((b * ca) >> 4);
        }
        
        r += r0;
        g += g0;
        b += b0;
        
        if(r > 31) r = 31;
        if(g > 31) g = 31;
        if(b > 31) b = 31;
       
        prev_res = (color & 0xFFFF0000) | (b << 10) | (g << 5) | r;
      }
      *ta++ = prev_res;
    } else {
      ta++;
      tb++;
    }
  }
}

#endif // VBA_GFX_H
