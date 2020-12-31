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

#include "global.h"
#include "GBA.h"
#include "Globals.h"
#include "Gfx.h"

void mode1RenderLine()
{
  u16 *palette = (u16 *)paletteRAM;

  if(DISPCNT & 0x80) {
    u32* p = (u32 *)lineMix;
    for (int x = 0; x < 120; x++) {
      *p++ = 0x7fff7fff;
    }
    gfxLastVCOUNT = VCOUNT;    
    return;
  }

  if(layerEnable & 0x0100) {
    gfxDrawTextScreen(BG0CNT, BG0HOFS, BG0VOFS, line0);
  }
  
  if(layerEnable & 0x0200) {
    gfxDrawTextScreen(BG1CNT, BG1HOFS, BG1VOFS, line1);
  }

  if(layerEnable & 0x0400) {
    int changed = gfxBG2Changed;
    if(gfxLastVCOUNT > VCOUNT)
      changed = 3;
    gfxDrawRotScreen_ref_ref(BG2CNT, BG2X_L, BG2X_H, BG2Y_L, BG2Y_H,
                     BG2PA, BG2PB, BG2PC, BG2PD,
                     &gfxBG2X, &gfxBG2Y, changed, line2);
  }

  if (gba_render_skip) {
    gfxBG2Changed = 0;
    gfxLastVCOUNT = VCOUNT;    
    return;
  }

  gfxDrawSprites();

  u32 backdrop = (READ16LE(&palette[0]) | 0x30000000);

  for(int x = 0; x < 240; x++) {
    u32 color = backdrop;
    u8 top = 0x20;

    if(line0[x] < color) {
      color = line0[x];
      top = 0x01;
    }

    if((u8)(line1[x]>>24) < (u8)(color >> 24)) {
      color = line1[x];
      top = 0x02;
    }

    if((u8)(line2[x]>>24) < (u8)(color >> 24)) {
      color = line2[x];
      top = 0x04;
    }

    if((u8)(lineOBJ[x]>>24) < (u8)(color >> 24)) {
      color = lineOBJ[x];
      top = 0x10;
    }

    if((top & 0x10) && (color & 0x00010000)) {
      // semi-transparent OBJ
      u32 back = backdrop;
      u8 top2 = 0x20;
      
      if((u8)(line0[x]>>24) < (u8)(back >> 24)) {
        back = line0[x];
        top2 = 0x01;
      }
      
      if((u8)(line1[x]>>24) < (u8)(back >> 24)) {
        back = line1[x];
        top2 = 0x02;
      }
      
      if((u8)(line2[x]>>24) < (u8)(back >> 24)) {
        back = line2[x];
        top2 = 0x04;
      }

      if(top2 & (BLDMOD>>8))
        color = gfxAlphaBlend_int_int_int_int(color, back,
                              coeff[COLEV & 0x1F],
                              coeff[(COLEV >> 8) & 0x1F]);
      else {
# if 0 //LUDO: FOR_TEST
        switch((BLDMOD >> 6) & 3) {
        case 2:
          if(BLDMOD & top)
            color = gfxIncreaseBrightness_int_int(color, coeff[COLY & 0x1F]);
          break;
        case 3:
          if(BLDMOD & top)
            color = gfxDecreaseBrightness_int_int(color, coeff[COLY & 0x1F]);
          break;
        }         
# endif
      } 
    }

    setlineMix(x, color);
  }
  gfxBG2Changed = 0;
  gfxLastVCOUNT = VCOUNT; 
}

void mode1RenderLineNoWindow()
{
  u16 *palette = (u16 *)paletteRAM;

  if(DISPCNT & 0x80) {
    u32* p = (u32 *)lineMix;
    for (int x = 0; x < 120; x++) {
      *p++ = 0x7fff7fff;
    }
    gfxLastVCOUNT = VCOUNT;    
    return;
  }

  if(layerEnable & 0x0100) {
    gfxDrawTextScreen(BG0CNT, BG0HOFS, BG0VOFS, line0);
  }

  if(layerEnable & 0x0200) {
    gfxDrawTextScreen(BG1CNT, BG1HOFS, BG1VOFS, line1);
  }

  if(layerEnable & 0x0400) {
    int changed = gfxBG2Changed;
    if(gfxLastVCOUNT > VCOUNT)
      changed = 3;
    gfxDrawRotScreen_ref_ref(BG2CNT, BG2X_L, BG2X_H, BG2Y_L, BG2Y_H,
                     BG2PA, BG2PB, BG2PC, BG2PD,
                     &gfxBG2X, &gfxBG2Y, changed, line2);
  }

  if (gba_render_skip) {
    gfxBG2Changed = 0;
    gfxLastVCOUNT = VCOUNT;    
    return;
  }

  gfxDrawSprites();

  u32 backdrop = (READ16LE(&palette[0]) | 0x30000000);

  for(int x = 0; x < 240; x++) {
    u32 color = backdrop;
    u8 top = 0x20;

    if(line0[x] < color) {
      color = line0[x];
      top = 0x01;
    }

    if((u8)(line1[x]>>24) < (u8)(color >> 24)) {
      color = line1[x];
      top = 0x02;
    }

    if((u8)(line2[x]>>24) < (u8)(color >> 24)) {
      color = line2[x];
      top = 0x04;
    }

    if((u8)(lineOBJ[x]>>24) < (u8)(color >> 24)) {
      color = lineOBJ[x];
      top = 0x10;
    }

    if(!(color & 0x00010000)) {
      switch((BLDMOD >> 6) & 3) {
      case 1:
        {
          if(top & BLDMOD) {
            u32 back = backdrop;
            u8 top2 = 0x20;
            if((u8)(line0[x]>>24) < (u8)(back >> 24)) {
              if(top != 0x01) {
                back = line0[x];
                top2 = 0x01;
              }
            }
            
            if((u8)(line1[x]>>24) < (u8)(back >> 24)) {
              if(top != 0x02) {
                back = line1[x];
                top2 = 0x02;
              }
            }
            
            if((u8)(line2[x]>>24) < (u8)(back >> 24)) {
              if(top != 0x04) {
                back = line2[x];
                top2 = 0x04;
              }
            }
            
            if((u8)(lineOBJ[x]>>24) < (u8)(back >> 24)) {
              if(top != 0x10) {
                back = lineOBJ[x];
                top2 = 0x10;
              }
            }
            
            if(top2 & (BLDMOD>>8))
              color = gfxAlphaBlend_int_int_int_int(color, back,
                                    coeff[COLEV & 0x1F],
                                    coeff[(COLEV >> 8) & 0x1F]);
          }
        }
        break;
# if 0 //LUDO: FOR_TEST
      case 0:
        break;
      case 2:
        if(BLDMOD & top)
          color = gfxIncreaseBrightness_int_int(color, coeff[COLY & 0x1F]);
        break;
      case 3:
        if(BLDMOD & top)
          color = gfxDecreaseBrightness_int_int(color, coeff[COLY & 0x1F]);
        break;
# endif
      }
    } else {
      // semi-transparent OBJ
      u32 back = backdrop;
      u8 top2 = 0x20;
      
      if((u8)(line0[x]>>24) < (u8)(back >> 24)) {
        back = line0[x];
        top2 = 0x01;
      }
      
      if((u8)(line1[x]>>24) < (u8)(back >> 24)) {
        back = line1[x];
        top2 = 0x02;
      }
      
      if((u8)(line2[x]>>24) < (u8)(back >> 24)) {
        back = line2[x];
        top2 = 0x04;
      }

      if(top2 & (BLDMOD>>8))
        color = gfxAlphaBlend_int_int_int_int(color, back,
                              coeff[COLEV & 0x1F],
                              coeff[(COLEV >> 8) & 0x1F]);
      else {
# if 0 //LUDO: FOR_TEST
        switch((BLDMOD >> 6) & 3) {
        case 2:
          if(BLDMOD & top)
            color = gfxIncreaseBrightness_int_int(color, coeff[COLY & 0x1F]);
          break;
        case 3:
          if(BLDMOD & top)
            color = gfxDecreaseBrightness_int_int(color, coeff[COLY & 0x1F]);
          break;
        }         
# endif
      }      
    }
    
    setlineMix(x, color);
  }
  gfxBG2Changed = 0;
  gfxLastVCOUNT = VCOUNT;  
}

void mode1RenderLineAll()
{
  u16 *palette = (u16 *)paletteRAM;

  if(DISPCNT & 0x80) {
    u32* p = (u32 *)lineMix;
    for (int x = 0; x < 120; x++) {
      *p++ = 0x7fff7fff;
    }
    gfxLastVCOUNT = VCOUNT;    
    return;
  }

  bool inWindow0 = false;
  bool inWindow1 = false;

  if(layerEnable & 0x2000) {
    u8 v0 = WIN0V >> 8;
    u8 v1 = WIN0V & 255;
    inWindow0 = ((v0 == v1) && (v0 >= 0xe8));
    if(v1 >= v0)
      inWindow0 |= (VCOUNT >= v0 && VCOUNT < v1);
    else
      inWindow0 |= (VCOUNT >= v0 || VCOUNT < v1);
  }
  if(layerEnable & 0x4000) {
    u8 v0 = WIN1V >> 8;
    u8 v1 = WIN1V & 255;
    inWindow1 = ((v0 == v1) && (v0 >= 0xe8));    
    if(v1 >= v0)
      inWindow1 |= (VCOUNT >= v0 && VCOUNT < v1);
    else
      inWindow1 |= (VCOUNT >= v0 || VCOUNT < v1);
  }
  
  if(layerEnable & 0x0100) {
    gfxDrawTextScreen(BG0CNT, BG0HOFS, BG0VOFS, line0);
  }
  
  if(layerEnable & 0x0200) {
    gfxDrawTextScreen(BG1CNT, BG1HOFS, BG1VOFS, line1);
  }

  if(layerEnable & 0x0400) {
    int changed = gfxBG2Changed;
    if(gfxLastVCOUNT > VCOUNT)
      changed = 3;
    gfxDrawRotScreen_ref_ref(BG2CNT, BG2X_L, BG2X_H, BG2Y_L, BG2Y_H,
                     BG2PA, BG2PB, BG2PC, BG2PD,
                     &gfxBG2X, &gfxBG2Y, changed, line2);
  }

  if (gba_render_skip) {
    gfxBG2Changed = 0;
    gfxLastVCOUNT = VCOUNT;    
    return;
  }

  gfxDrawSprites();
  gfxDrawOBJWin();
  
  u32 backdrop = (READ16LE(&palette[0]) | 0x30000000);

  u8 inWin0Mask = WININ & 0xFF;
  u8 inWin1Mask = WININ >> 8;
  u8 outMask = WINOUT & 0xFF;
  
  for(int x = 0; x < 240; x++) {
    u32 color = backdrop;
    u8 top = 0x20;
    u8 mask = outMask;

    if(lineOBJWin[x]) {
      mask = WINOUT >> 8;
    }

    if(inWindow1) {
      if(gfxInWin1[x])
        mask = inWin1Mask;
    }

    if(inWindow0) {
      if(gfxInWin0[x]) {
        mask = inWin0Mask;
      }
    }
    
    if(line0[x] < color && (mask & 1)) {
      color = line0[x];
      top = 0x01;
    }

    if((u8)(line1[x]>>24) < (u8)(color >> 24) && (mask & 2)) {
      color = line1[x];
      top = 0x02;
    }

    if((u8)(line2[x]>>24) < (u8)(color >> 24) && (mask & 4)) {
      color = line2[x];
      top = 0x04;
    }

    if((u8)(lineOBJ[x]>>24) < (u8)(color >> 24) && (mask & 16)) {
      color = lineOBJ[x];
      top = 0x10;
    }

    // special FX on the window
    if(mask & 32) {
      if(!(color & 0x00010000)) {
        switch((BLDMOD >> 6) & 3) {
        case 1:
          {
            if(top & BLDMOD) {
              u32 back = backdrop;
              u8 top2 = 0x20;
              if((mask & 1) && (u8)(line0[x]>>24) < (u8)(back >> 24)) {
                if(top != 0x01) {
                  back = line0[x];
                  top2 = 0x01;
                }
              }
              
              if((mask & 2) && (u8)(line1[x]>>24) < (u8)(back >> 24)) {
                if(top != 0x02) {
                  back = line1[x];
                  top2 = 0x02;
                }
              }
              
              if((mask & 4) && (u8)(line2[x]>>24) < (u8)(back >> 24)) {
                if(top != 0x04) {
                  back = line2[x];
                  top2 = 0x04;
                }
              }
              
              if((mask & 16) && (u8)(lineOBJ[x]>>24) < (u8)(back >> 24)) {
                if(top != 0x10) {
                  back = lineOBJ[x];
                  top2 = 0x10;
                }
              }
              
              if(top2 & (BLDMOD>>8))
                color = gfxAlphaBlend_int_int_int_int(color, back,
                                      coeff[COLEV & 0x1F],
                                      coeff[(COLEV >> 8) & 0x1F]);
            }
          }
          break;
# if 0 //LUDO: FOR_TEST
        case 0:
          break;
        case 2:
          if(BLDMOD & top)
            color = gfxIncreaseBrightness_int_int(color, coeff[COLY & 0x1F]);
          break;
        case 3:
          if(BLDMOD & top)
            color = gfxDecreaseBrightness_int_int(color, coeff[COLY & 0x1F]);
          break;
# endif
        }
      } else {
        // semi-transparent OBJ
        u32 back = backdrop;
        u8 top2 = 0x20;
        
        if((mask & 1) && (u8)(line0[x]>>24) < (u8)(back >> 24)) {
          back = line0[x];
          top2 = 0x01;
        }
        
        if((mask & 2) && (u8)(line1[x]>>24) < (u8)(back >> 24)) {
          back = line1[x];
          top2 = 0x02;
        }
        
        if((mask & 4) && (u8)(line2[x]>>24) < (u8)(back >> 24)) {
          back = line2[x];
          top2 = 0x04;
        }
        
        if(top2 & (BLDMOD>>8))
          color = gfxAlphaBlend_int_int_int_int(color, back,
                                coeff[COLEV & 0x1F],
                                coeff[(COLEV >> 8) & 0x1F]);
        else {
# if 0 //LUDO: FOR_TEST
          switch((BLDMOD >> 6) & 3) {
          case 2:
            if(BLDMOD & top)
              color = gfxIncreaseBrightness_int_int(color, coeff[COLY & 0x1F]);
            break;
          case 3:
            if(BLDMOD & top)
              color = gfxDecreaseBrightness_int_int(color, coeff[COLY & 0x1F]);
            break;
          }       
# endif
        }
      }
    } else if(color & 0x00010000) {
      // semi-transparent OBJ
      u32 back = backdrop;
      u8 top2 = 0x20;
      
      if((mask & 1) && (u8)(line0[x]>>24) < (u8)(back >> 24)) {
        back = line0[x];
        top2 = 0x01;
      }
      
      if((mask & 2) && (u8)(line1[x]>>24) < (u8)(back >> 24)) {
        back = line1[x];
        top2 = 0x02;
      }
      
      if((mask & 4) && (u8)(line2[x]>>24) < (u8)(back >> 24)) {
        back = line2[x];
        top2 = 0x04;
      }
      
      if(top2 & (BLDMOD>>8))
        color = gfxAlphaBlend_int_int_int_int(color, back,
                              coeff[COLEV & 0x1F],
                              coeff[(COLEV >> 8) & 0x1F]);
      else {
# if 0 //LUDO: FOR_TEST
        switch((BLDMOD >> 6) & 3) {
        case 2:
          if(BLDMOD & top)
            color = gfxIncreaseBrightness_int_int(color, coeff[COLY & 0x1F]);
          break;
        case 3:
          if(BLDMOD & top)
            color = gfxDecreaseBrightness_int_int(color, coeff[COLY & 0x1F]);
          break;
        }         
# endif
      } 
    }
    
    setlineMix(x, color);
  }
  gfxBG2Changed = 0;
  gfxLastVCOUNT = VCOUNT;  
}
