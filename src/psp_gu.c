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
#include <SDL/SDL.h>
#include <SDL/SDL_mixer.h>
#include <SDL/SDL_image.h>
#include <SDL/SDL_ttf.h>
#include <pspctrl.h>

#include <pspkernel.h>
#include <pspdebug.h>
#include <pspdisplay.h>

#include <pspgu.h>
#include <psppower.h>
#include <psprtc.h>

#include "psp_sdl.h"
#include "global.h"
#include "psp_gu.h"

static unsigned int __attribute__((aligned(16))) list[262144];
static int gecbid = -1;
static short *ScreenVertex = (short *)0x441FC100;
static unsigned int *GEcmd = (unsigned int *)0x441FC000;
static u16* psp_gu_vram_base = (u16*) (0x44000000);

u16*
psp_gu_get_vram_addr(void)
{
  return psp_gu_vram_base;
}

static void
Ge_Finish_Callback(int id, void *arg)
{
}


int
psp_gu_init(void)
{
	sceDisplaySetMode(0, PSP_SCREEN_WIDTH, PSP_SCREEN_HEIGHT);

	sceDisplayWaitVblankStart();
	sceDisplaySetFrameBuf((void*)psp_gu_vram_base, PSP_LINE_SIZE, PSP_DISPLAY_PIXEL_FORMAT_565, PSP_DISPLAY_SETBUF_NEXTFRAME);

	sceGuInit();
	
	sceGuStart(GU_DIRECT, list);
	sceGuDrawBuffer(GU_PSM_5650 , (void*)0, PSP_LINE_SIZE);
	sceGuDispBuffer(PSP_SCREEN_WIDTH, PSP_SCREEN_HEIGHT, (void*)0, PSP_LINE_SIZE);
	sceGuClear(GU_COLOR_BUFFER_BIT | GU_DEPTH_BUFFER_BIT);

	sceGuDepthBuffer((void*) 0x110000, PSP_LINE_SIZE);
	sceGuOffset(2048 - (PSP_SCREEN_WIDTH / 2), 2048 - (PSP_SCREEN_HEIGHT / 2));
	sceGuViewport(2048, 2048, PSP_SCREEN_WIDTH, PSP_SCREEN_HEIGHT);
	sceGuDepthRange(0xc350, 0x2710);

	sceGuScissor(0, 0, PSP_SCREEN_WIDTH+1, PSP_SCREEN_HEIGHT+1);
	sceGuEnable(GU_SCISSOR_TEST);
	sceGuTexMode(GU_PSM_5650 , 0, 0, GU_FALSE);
	sceGuTexFunc(GU_TFX_REPLACE, GU_TCC_RGBA);
	sceGuTexFilter(GU_LINEAR, GU_LINEAR);
	
	sceGuAlphaFunc(GU_GREATER, 0, 0xff);
	sceGuEnable(GU_ALPHA_TEST);
	sceGuDepthFunc(GU_GEQUAL);
	sceGuEnable(GU_DEPTH_TEST);
	sceGuFrontFace(GU_CW);
	sceGuShadeModel(GU_SMOOTH);
	sceGuEnable(GU_CULL_FACE);
	sceGuEnable(GU_TEXTURE_2D);
	sceGuAmbientColor(0xffffffff);
	sceGuDisable(GU_BLEND);

	sceGuFinish();
	sceGuSync(0, 0);

	sceDisplayWaitVblankStart();
	sceGuDisplay(GU_TRUE);

	PspGeCallbackData gecb;
	gecb.signal_func = NULL;
	gecb.signal_arg = NULL;
	gecb.finish_func = Ge_Finish_Callback;
	gecb.finish_arg = NULL;
	gecbid = sceGeSetCallback(&gecb);

  return 1;
}

void 
psp_gu_max_blit(u16* src, bool sync)
{
  //src = (u16 *)PSP_UNCACHE_PTR(src);
	u16* dest = psp_gu_vram_base;
	static int qid = -1;
	ScreenVertex[0] = 0;
	ScreenVertex[1] = 0;
	ScreenVertex[2] = 0;
	ScreenVertex[3] = 0;
	ScreenVertex[4] = 0;
	ScreenVertex[5] = GBA_SCREEN_WIDTH;
	ScreenVertex[6] = GBA_SCREEN_HEIGHT;
	ScreenVertex[7] = PSP_SCREEN_WIDTH;
	ScreenVertex[8] = PSP_SCREEN_HEIGHT;
	ScreenVertex[9] = 0;

	// Set Draw Buffer
	GEcmd[ 0] = 0x9C000000UL | ((u32)(unsigned char *)dest & 0x00FFFFFF);
	GEcmd[ 1] = 0x9D000000UL | (((u32)(unsigned char *)dest & 0xFF000000) >> 8) | 512;
	// Set Tex Buffer
	GEcmd[ 2] = 0xA0000000UL | ((u32)(unsigned char *)src & 0x00FFFFFF);
	GEcmd[ 3] = 0xA8000000UL | (((u32)(unsigned char *)src & 0xFF000000) >> 8) | GBA_SCREEN_WIDTH;
	// Tex size
	GEcmd[ 4] = 0xB8000000UL | (8 << 8) | 8;
	// Tex Flush
	GEcmd[ 5] = 0xCB000000UL;
	// Set Vertex
	GEcmd[ 6] = 0x12000000UL | (1 << 23) | (0 << 11) | (0 << 9) | (2 << 7) | (0 << 5) | (0 << 2) | 2;
	GEcmd[ 7] = 0x10000000UL;
	GEcmd[ 8] = 0x02000000UL;
	GEcmd[ 9] = 0x10000000UL | (((u32)(void *)ScreenVertex & 0xFF000000) >> 8);
	GEcmd[10] = 0x01000000UL | ((u32)(void *)ScreenVertex & 0x00FFFFFF);
	// Draw Vertex
	GEcmd[11] = 0x04000000UL | (6 << 16) | 2;
	// List End
	GEcmd[12] = 0x0F000000UL;
	GEcmd[13] = 0x0C000000UL;
	GEcmd[14] = 0;
	GEcmd[15] = 0;

  if (!GBA.psp_disable_gpu_cache) {
	  sceKernelDcacheWritebackAll();
  }
	qid = sceGeListEnQueue(&GEcmd[0], &GEcmd[14], gecbid, NULL);
	if (sync && qid >= 0) sceGeListSync(qid, 0);
}


void
psp_gu_fit_height_blit(u16* src, bool sync)
{
  //src = (u16 *)PSP_UNCACHE_PTR(src);
	u16* dest = psp_gu_vram_base;
	static int qid = -1;
	ScreenVertex[0] = 0;
	ScreenVertex[1] = 0;
	ScreenVertex[2] = 36;
	ScreenVertex[3] = 0;
	ScreenVertex[4] = 0;
	ScreenVertex[5] = GBA_SCREEN_WIDTH;
	ScreenVertex[6] = GBA_SCREEN_HEIGHT;
	ScreenVertex[7] = 408+36;
	ScreenVertex[8] = PSP_SCREEN_HEIGHT;
	ScreenVertex[9] = 0;

	// Set Draw Buffer
	GEcmd[ 0] = 0x9C000000UL | ((u32)(unsigned char *)dest & 0x00FFFFFF);
	GEcmd[ 1] = 0x9D000000UL | (((u32)(unsigned char *)dest & 0xFF000000) >> 8) | 512;
	// Set Tex Buffer
	GEcmd[ 2] = 0xA0000000UL | ((u32)(unsigned char *)src & 0x00FFFFFF);
	GEcmd[ 3] = 0xA8000000UL | (((u32)(unsigned char *)src & 0xFF000000) >> 8) | GBA_SCREEN_WIDTH;
	// Tex size
	GEcmd[ 4] = 0xB8000000UL | (8 << 8) | 8;
	// Tex Flush
	GEcmd[ 5] = 0xCB000000UL;
	// Set Vertex
	GEcmd[ 6] = 0x12000000UL | (1 << 23) | (0 << 11) | (0 << 9) | (2 << 7) | (0 << 5) | (0 << 2) | 2;
	GEcmd[ 7] = 0x10000000UL;
	GEcmd[ 8] = 0x02000000UL;
	GEcmd[ 9] = 0x10000000UL | (((u32)(void *)ScreenVertex & 0xFF000000) >> 8);
	GEcmd[10] = 0x01000000UL | ((u32)(void *)ScreenVertex & 0x00FFFFFF);
	// Draw Vertex
	GEcmd[11] = 0x04000000UL | (6 << 16) | 2;
	// List End
	GEcmd[12] = 0x0F000000UL;
	GEcmd[13] = 0x0C000000UL;
	GEcmd[14] = 0;
	GEcmd[15] = 0;

  if (!GBA.psp_disable_gpu_cache) {
	  sceKernelDcacheWritebackAll();
  }
	qid = sceGeListEnQueue(&GEcmd[0], &GEcmd[14], gecbid, NULL);
	if (sync && qid >= 0) sceGeListSync(qid, 0);
}

void
psp_gu_normal_blit(u16* src, bool sync)
{
  //src = (u16 *)PSP_UNCACHE_PTR(src);
	u16* dest = psp_gu_vram_base;
	static int qid = -1;
	ScreenVertex[0] = 0;
	ScreenVertex[1] = 0;
	ScreenVertex[2] = 120;
	ScreenVertex[3] = 80;
	ScreenVertex[4] = 0;
	ScreenVertex[5] = GBA_SCREEN_WIDTH;
	ScreenVertex[6] = GBA_SCREEN_HEIGHT;
	ScreenVertex[7] = GBA_SCREEN_WIDTH+120;
	ScreenVertex[8] = GBA_SCREEN_HEIGHT+80;
	ScreenVertex[9] = 0;

	// Set Draw Buffer
	GEcmd[ 0] = 0x9C000000UL | ((u32)(unsigned char *)dest & 0x00FFFFFF);
	GEcmd[ 1] = 0x9D000000UL | (((u32)(unsigned char *)dest & 0xFF000000) >> 8) | 512;
	// Set Tex Buffer
	GEcmd[ 2] = 0xA0000000UL | ((u32)(unsigned char *)src & 0x00FFFFFF);
	GEcmd[ 3] = 0xA8000000UL | (((u32)(unsigned char *)src & 0xFF000000) >> 8) | GBA_SCREEN_WIDTH;
	// Tex size
	GEcmd[ 4] = 0xB8000000UL | (8 << 8) | 8;
	// Tex Flush
	GEcmd[ 5] = 0xCB000000UL;
	// Set Vertex
	GEcmd[ 6] = 0x12000000UL | (1 << 23) | (0 << 11) | (0 << 9) | (2 << 7) | (0 << 5) | (0 << 2) | 2;
	GEcmd[ 7] = 0x10000000UL;
	GEcmd[ 8] = 0x02000000UL;
	GEcmd[ 9] = 0x10000000UL | (((u32)(void *)ScreenVertex & 0xFF000000) >> 8);
	GEcmd[10] = 0x01000000UL | ((u32)(void *)ScreenVertex & 0x00FFFFFF);
	// Draw Vertex
	GEcmd[11] = 0x04000000UL | (6 << 16) | 2;
	// List End
	GEcmd[12] = 0x0F000000UL;
	GEcmd[13] = 0x0C000000UL;
	GEcmd[14] = 0;
	GEcmd[15] = 0;

  if (!GBA.psp_disable_gpu_cache) {
	  sceKernelDcacheWritebackAll();
  }
	qid = sceGeListEnQueue(&GEcmd[0], &GEcmd[14], gecbid, NULL);
	if (sync && qid >= 0) sceGeListSync(qid, 0);
}

