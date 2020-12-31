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

# ifndef _PSP_GU_H_
# define _PSP_GU_H_

# define GBA_SCREEN_WIDTH  240
# define GBA_SCREEN_HEIGHT 160

# define PSP_SCREEN_WIDTH  480
# define PSP_SCREEN_HEIGHT 272
# define PSP_LINE_SIZE     512
# define PSP_PIXEL_FORMAT    3

  extern void psp_gu_simple_blit(u16* src, bool sync);
  extern u16* psp_gu_get_vram_addr(void);

# endif


