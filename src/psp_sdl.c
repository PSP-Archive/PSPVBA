/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <SDL/SDL.h>
#include <SDL/SDL_mixer.h>
#include <SDL/SDL_image.h>
#include <SDL/SDL_ttf.h>
#include <psptypes.h>
#include <pspctrl.h>

#include "global.h"
#include "psp_sdl.h"
#include "psp_gu.h"

  extern unsigned char psp_font[];

  SDL_Surface *back2_surface;

#define PSP_SDL_RGB(r,g,b) ((((b>>3) & 0x1F)<<11)|(((g>>3) & 0x1F)<<6)|(((r>>3) & 0x1F)<<0))


uint
psp_sdl_rgb(uchar R, uchar G, uchar B)
{
  return PSP_SDL_RGB(R, G, B);
}

ushort *
psp_sdl_get_vram_addr(uint x, uint y)
{
  u16* vram= psp_gu_get_vram_addr();
  return vram + x + (y*PSP_LINE_SIZE);
}

void
loc_psp_debug(char *file, int line, char *message)
{
  static int current_line = 0;
  static int current_col  = 10;

  char buffer[128];
  current_line += 10;
  if (current_line > 250)
  {
    if (current_col == 200) {
      psp_sdl_clear_screen(psp_sdl_rgb(0, 0, 0xff));
      current_col = 10;
    } else {
      current_col = 200;
    }
    
    current_line = 10;
  }
  sprintf(buffer,"%s:%d %s", file, line, message);
  psp_sdl_print(current_col, current_line, buffer, psp_sdl_rgb(0xff,0xff,0xff) );
}

void 
psp_sdl_print(int x,int y, char *str, int color)
{
  int index;
  int x0 = x;

  for (index = 0; str[index] != '\0'; index++) {
    psp_sdl_put_char(x, y, color, 0, (unsigned char)str[index], 1, 0);
    x += 8;
    if (x >= (PSP_SDL_SCREEN_WIDTH - 8)) {
      x = x0; y++;
    }
    if (y >= (PSP_SDL_SCREEN_HEIGHT - 8)) break;
  }
}

static struct timeval start;

void 
sdl_start_ticks(void)
{
  gettimeofday(&start, NULL);
}

u32 
sdl_get_ticks(void)
{
  struct timeval now;
  long long ticks;

  gettimeofday(&now, NULL);
  ticks=(now.tv_sec-start.tv_sec)*1000000 + now.tv_usec - start.tv_usec;
  ticks /= 1000;
  return((u32)ticks);
}

void
psp_sdl_clear_screen(int color)
{
  int x; int y;
  ushort *vram = psp_sdl_get_vram_addr(0,0);
  
  for (y = 0; y < PSP_SDL_SCREEN_HEIGHT; y++) {
    for (x = 0; x < PSP_SDL_SCREEN_WIDTH; x++) {
      vram[x + (y*PSP_LINE_SIZE)] = color;
    }
  }
  psp_sdl_flush();
}

static int
psp_sdl_get_back2_color(int x, int y)
{
  uchar *back2 = (uchar *)back2_surface->pixels;
  int bytes_per_pixels = 3;
  int pitch            = 1440;
  Uint8 r = back2[0 + (y * pitch) + (x * bytes_per_pixels)];
  Uint8 g = back2[1 + (y * pitch) + (x * bytes_per_pixels)];
  Uint8 b = back2[2 + (y * pitch) + (x * bytes_per_pixels)];
	int color = psp_sdl_rgb(r, g, b);

  return color;
}

void
psp_sdl_blit_surface2()
{
  int x; int y;
  ushort *vram = psp_sdl_get_vram_addr(0,0);
  
  for (y = 0; y < PSP_SDL_SCREEN_HEIGHT; y++) {
    for (x = 0; x < PSP_SDL_SCREEN_WIDTH; x++) {
      vram[x + (y*PSP_LINE_SIZE)] = psp_sdl_get_back2_color(x, y);
    }
  }
  psp_sdl_flush();
}


void 
psp_sdl_draw_rectangle(int x, int y, int w, int h, int border, int mode) 
{
  ushort *vram = (ushort *)psp_sdl_get_vram_addr(x, y);
  int xo, yo;
  if (mode == PSP_SDL_XOR) {
    for (xo = 0; xo < w; xo++) {
      vram[xo] ^=  border;
      vram[xo + h * PSP_LINE_SIZE] ^=  border;
    }
    for (yo = 0; yo < h; yo++) {
      vram[yo * PSP_LINE_SIZE] ^=  border;
      vram[w + yo * PSP_LINE_SIZE] ^=  border;
    }
  } else {
    for (xo = 0; xo < w; xo++) {
      vram[xo] =  border;
      vram[xo + h * PSP_LINE_SIZE] =  border;
    }
    for (yo = 0; yo < h; yo++) {
      vram[yo * PSP_LINE_SIZE] =  border;
      vram[w + yo * PSP_LINE_SIZE] =  border;
    }
  }
}

void 
psp_sdl_fill_rectangle(int x, int y, int w, int h, int color, int mode)
{
  ushort *vram  = (ushort *)psp_sdl_get_vram_addr(x, y);
  int xo, yo;
  if (mode == PSP_SDL_XOR) {
    for (xo = 0; xo <= w; xo++) {
      for (yo = 0; yo <= h; yo++) {
        if ( ((xo == 0) && ((yo == 0) || (yo == h))) ||
             ((xo == w) && ((yo == 0) || (yo == h))) ) {
          /* Skip corner */
        } else {
          vram[xo + yo * PSP_LINE_SIZE] ^=  color;
        }
      }
    }
  } else {
    for (xo = 0; xo <= w; xo++) {
      for (yo = 0; yo <= h; yo++) {
        vram[xo + yo * PSP_LINE_SIZE] =  color;
      }
    }
  }
}


void 
psp_sdl_back2_rectangle(int x, int y, int w, int h)
{
  if (! back2_surface) {
    psp_sdl_fill_rectangle(x, y, w, h, 0x0, 0);
    return;
  }
  ushort *vram  = (ushort *)psp_sdl_get_vram_addr(x, y);
  int xo, yo;
  for (xo = 0; xo <= w; xo++) {
    for (yo = 0; yo <= h; yo++) {
      vram[xo + yo * PSP_LINE_SIZE] = psp_sdl_get_back2_color(x + xo, y + yo);
    }
  }
}

void 
psp_sdl_put_char(int x, int y, int color, int bgcolor, unsigned char c, int drawfg, int drawbg)
{
  int cx;
  int cy;
  int b;
  int index;

  ushort *vram = (ushort *)psp_sdl_get_vram_addr(x, y);
  index = ((ushort)c) * 8;

  for (cy=0; cy<8; cy++) {
    b=0x80;
    for (cx=0; cx<8; cx++) {
      if (psp_font[index] & b) {
        if (drawfg) vram[cx + cy * PSP_LINE_SIZE] = color;
      } else {
        if (drawbg) vram[cx + cy * PSP_LINE_SIZE] = bgcolor;
      }
      b = b >> 1;
    }
    index++;
  }
}

void 
psp_sdl_back2_put_char(int x, int y, int color, unsigned char c)
{
  int cx;
  int cy;
  int bmask;
  int index;

  if (! back2_surface) {
    psp_sdl_put_char(x, y, color, 0x0, c, 1, 1);
    return;
  }
  ushort *vram  = (ushort *)psp_sdl_get_vram_addr(x, y);
  index = ((ushort)c) * 8;

  for (cy=0; cy<8; cy++) {
    bmask=0x80;
    for (cx=0; cx<8; cx++) {
      if (psp_font[index] & bmask) {
        vram[cx + cy * PSP_LINE_SIZE] = color;
      } else {
        vram[cx + cy * PSP_LINE_SIZE] = psp_sdl_get_back2_color(x + cx, y + cy);
      }
      bmask = bmask >> 1;
    }
    index++;
  }
}

void 
psp_sdl_fill_print(int x,int y,const char *str, int color, int bgcolor)
{
  int index;
  int x0 = x;

  for (index = 0; str[index] != '\0'; index++) {
    psp_sdl_put_char(x, y, color, bgcolor, (unsigned char)str[index], 1, 1);
    x += 8;
    if (x >= (PSP_SDL_SCREEN_WIDTH - 8)) {
      x = x0; y++;
    }
    if (y >= (PSP_SDL_SCREEN_HEIGHT - 8)) break;
  }
}

void
psp_sdl_back2_print(int x,int y,const char *str, int color)
{
  int index;
  int x0 = x;

  for (index = 0; str[index] != '\0'; index++) {
    psp_sdl_back2_put_char(x, y, color, str[index]);
    x += 8;
    if (x >= (PSP_SDL_SCREEN_WIDTH - 8)) {
      x = x0; y++;
    }
    if (y >= (PSP_SDL_SCREEN_HEIGHT - 8)) break;
  }
}

void
psp_sdl_wait_vn(uint count)
{
  for (; count>0; --count) {
    sceDisplayWaitVblankStart();
  }
}

void
psp_sdl_wait_vblank(void)
{
  sceDisplayWaitVblankStart();
}

void
psp_sdl_flush(void)
{
  sceKernelDcacheWritebackAll();
}

void
psp_sdl_lock(void)
{
}

void
psp_sdl_load_background()
{
  back2_surface = IMG_Load("./background.jpg");
}

void
psp_sdl_blit_background()
{
  static int first = 1;

  if (first && (back2_surface == NULL)) {
    psp_sdl_load_background();
    first = 0;
  }

  if (back2_surface != NULL) {
	  psp_sdl_blit_surface2();
  } else {
    psp_sdl_clear_screen(psp_sdl_rgb(0x00, 0x00, 0x00));
  }
}

void
psp_sdl_unlock(void)
{
}

void
psp_sdl_flip(void)
{
  psp_sdl_flush();
}

void
psp_sdl_save_screenshot(void)
{
  char TmpFileName[MAX_PATH];

  sprintf(TmpFileName,"%s/scr/screenshot_%d.png", GBA.gba_home_dir, GBA.psp_screenshot_id++);
  if (GBA.psp_screenshot_id >= 10) GBA.psp_screenshot_id = 0;
  CPUWritePNGFile(TmpFileName);
}

void
psp_sdl_exit(int status)
{
# ifdef PSP_ME
  psp_me_sound_stop();
# else
  psp_sound_stop();
# endif
  exit(status);
}
