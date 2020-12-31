/*
 *  Copyright (C) 2006 Ludovic Jacomme (ludovic.jacomme@gmail.com)
 *
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
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/time.h>
#include <fcntl.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <SDL/SDL.h>
#include <SDL/SDL_mixer.h>
#include <SDL/SDL_ttf.h>

#include <pspctrl.h>
#include <pspkernel.h>
#include <pspdebug.h>
#include <pspiofilemgr.h>

#include "global.h"
#include "psp_sdl.h"
#include "psp_kbd.h"
#include "psp_menu.h"
#include "psp_fmgr.h"
#include "psp_menu_set.h"

extern SDL_Surface *back_surface;
static int psp_menu_dirty = 1;

# define MENU_SET_SOUND_QUALITY  0
# define MENU_SET_BIOS_FILE      1
# define MENU_SET_SOUND          2
# define MENU_SET_SKIP_FPS       3
# define MENU_SET_TURBO_MODE     4
# define MENU_SET_GPU_CACHE      5
# define MENU_SET_SHOW_FPS       6
# define MENU_SET_RENDER         7
# define MENU_SET_SCREEN_YMIN    8
# define MENU_SET_SCREEN_YMAX    9
# define MENU_SET_SFX           10
# define MENU_SET_ANALOG        11
# define MENU_SET_CLOCK         12
# define MENU_SET_TICK_ARM      13
# define MENU_SET_SAVE_BATTERY  14
                             
# define MENU_SET_LOAD          15
# define MENU_SET_SAVE          16
# define MENU_SET_RESET         17
                                 
# define MENU_SET_BACK          18

# define MAX_MENU_SET_ITEM (MENU_SET_BACK + 1)

  static menu_item_t menu_list[] =
  {
    { "Sound quality       :"},
    { "BIOS File           :"},

    { "Sound enable        :"},
    { "Skip frame          :"},
    { "Turbo mode          :"},
    { "Disable gpu cache   :"},
    { "Show fps            :"},
    { "Render mode         :"},
    { "Screen Ymin         :"},
    { "Screen Ymax         :"},
    { "Disable SFX         :"},
    { "Swap Analog/Cursor  :"},
    { "Clock frequency     :"},
    { "ARM tick average    :"},
    { "GBA Flash autosave  :"},

    { "Load settings"        },
    { "Save settings"        },
    { "Reset settings"       },
    { "Back to Menu"         }
  };

  static int cur_menu_id = MENU_SET_LOAD;

  static int gba_snd_enable        = 0;
  static int gba_render_mode       = 0;
  static int gba_disable_sfx       = 1;
  static int psp_reverse_analog    = 0;
  static int psp_show_fps          = 0;
  static int psp_cpu_clock         = 222;
  static int psp_sound_quality     = 2;
  static int psp_screen_ymin       = 0;
  static int psp_screen_ymax       = 160;
  static int arm_tick_average      = 0;
  static int gba_skip_fps          = 0;
  static int gba_turbo_mode        = 1;
  static int gba_save_battery      = 0;
  static int psp_disable_gpu_cache = 1;
  static int gba_bios_file_enable  = 0;

static void 
psp_display_screen_settings_menu(void)
{
  char buffer[64];
  int menu_id = 0;
  int slot_id = 0;
  int color   = 0;
  int x       = 0;
  int y       = 0;
  int y_step  = 0;

  if (psp_menu_dirty) {

    psp_sdl_blit_background();
    psp_menu_dirty = 0;
   
    psp_sdl_draw_rectangle(10,10,459,249,PSP_MENU_BORDER_COLOR,0);
    psp_sdl_draw_rectangle(11,11,457,247,PSP_MENU_BORDER_COLOR,0);

    psp_sdl_back2_print( 30, 6, " Start+L+R: EXIT ", PSP_MENU_WARNING_COLOR);

    psp_display_screen_menu_battery();

    psp_sdl_back2_print( 370, 6, " R: RESET ", PSP_MENU_NOTE_COLOR);

    psp_sdl_back2_print(30, 254, " []: Cancel  O/X: Valid  SELECT: Back ", 
                       PSP_MENU_BORDER_COLOR);

    psp_sdl_back2_print(370, 254, " By Zx-81 ",
                       PSP_MENU_AUTHOR_COLOR);
  }
  
  x      = 20;
  y      = 25;
  y_step = 10;
  
  for (menu_id = 0; menu_id < MAX_MENU_SET_ITEM; menu_id++) {
    color = PSP_MENU_TEXT_COLOR;
    if (cur_menu_id == menu_id) color = PSP_MENU_SEL_COLOR;
    else
    if ((menu_id == MENU_SET_SOUND_QUALITY) ||
        (menu_id == MENU_SET_BIOS_FILE    )) color = PSP_MENU_TEXT3_COLOR;

    psp_sdl_back2_print(x, y, menu_list[menu_id].title, color);

    if (menu_id == MENU_SET_SOUND) {
      if (gba_snd_enable) strcpy(buffer,"yes");
      else                strcpy(buffer,"no ");
      string_fill_with_space(buffer, 4);
      psp_sdl_back2_print(210, y, buffer, color);
    } else
    if (menu_id == MENU_SET_SFX) {
      if (gba_disable_sfx) strcpy(buffer,"yes");
      else                 strcpy(buffer,"no ");
      string_fill_with_space(buffer, 4);
      psp_sdl_back2_print(210, y, buffer, color);
    } else
    if (menu_id == MENU_SET_GPU_CACHE) {
      if (psp_disable_gpu_cache) strcpy(buffer,"yes");
      else                       strcpy(buffer,"no ");
      string_fill_with_space(buffer, 4);
      psp_sdl_back2_print(210, y, buffer, color);
    } else
    if (menu_id == MENU_SET_ANALOG) {
      if (psp_reverse_analog) strcpy(buffer,"yes");
      else                    strcpy(buffer,"no ");
      string_fill_with_space(buffer, 4);
      psp_sdl_back2_print(210, y, buffer, color);
    } else
    if (menu_id == MENU_SET_SKIP_FPS) {
      sprintf(buffer,"%d", gba_skip_fps);
      string_fill_with_space(buffer, 4);
      psp_sdl_back2_print(210, y, buffer, color);
    } else
    if (menu_id == MENU_SET_TURBO_MODE) {
      sprintf(buffer,"%d", gba_turbo_mode);
      string_fill_with_space(buffer, 4);
      psp_sdl_back2_print(210, y, buffer, color);
    } else
    if (menu_id == MENU_SET_SHOW_FPS) {
      if (psp_show_fps) strcpy(buffer,"yes");
      else              strcpy(buffer,"no ");
      string_fill_with_space(buffer, 4);
      psp_sdl_back2_print(210, y, buffer, color);
    } else
    if (menu_id == MENU_SET_RENDER) {

      if (gba_render_mode == GBA_RENDER_NORMAL    ) strcpy(buffer, "normal");
      else 
      if (gba_render_mode == GBA_RENDER_NORMAL_SCANLINE) strcpy(buffer, "scanline");
      else 
      if (gba_render_mode == GBA_RENDER_FIT_HEIGHT) strcpy(buffer, "fit");
      else 
      if (gba_render_mode == GBA_RENDER_MAX       ) strcpy(buffer, "max");
      else 
      if (gba_render_mode == GBA_RENDER_MAX_SCANLINE) strcpy(buffer, "max scanline");

      string_fill_with_space(buffer, 13);
      psp_sdl_back2_print(210, y, buffer, color);
    } else
    if (menu_id == MENU_SET_SCREEN_YMAX) {
      sprintf(buffer,"%d", psp_screen_ymax);
      string_fill_with_space(buffer, 4);
      psp_sdl_back2_print(210, y, buffer, color);
    } else
    if (menu_id == MENU_SET_SCREEN_YMIN) {
      sprintf(buffer,"%d", psp_screen_ymin);
      string_fill_with_space(buffer, 4);
      psp_sdl_back2_print(210, y, buffer, color);
    } else
    if (menu_id == MENU_SET_CLOCK) {
      sprintf(buffer,"%d", psp_cpu_clock);
      string_fill_with_space(buffer, 4);
      psp_sdl_back2_print(210, y, buffer, color);
    } else
    if (menu_id == MENU_SET_SOUND_QUALITY) {
      sprintf(buffer,"%d", psp_sound_quality);
      string_fill_with_space(buffer, 4);
      psp_sdl_back2_print(210, y, buffer, color);
    } else
    if (menu_id == MENU_SET_TICK_ARM) {
      sprintf(buffer,"%d", arm_tick_average);
      string_fill_with_space(buffer, 4);
      psp_sdl_back2_print(210, y, buffer, color);
    } else
    if (menu_id == MENU_SET_BIOS_FILE) {
      if (gba_bios_file_enable) strcpy(buffer,"yes");
      else                      strcpy(buffer,"no ");
      string_fill_with_space(buffer, 4);
      psp_sdl_back2_print(210, y, buffer, color);

      y += y_step;
    } else
    if (menu_id == MENU_SET_SAVE_BATTERY) {
      if (gba_save_battery) strcpy(buffer,"yes");
      else                  strcpy(buffer,"no ");
      string_fill_with_space(buffer, 4);
      psp_sdl_back2_print(210, y, buffer, color);

      y += y_step;
    }

    y += y_step;
  }

  psp_menu_display_save_name();
}

static void
psp_settings_menu_clock(int step)
{
  if (step > 0) {
    if (psp_cpu_clock == 222) {
      psp_cpu_clock = 300;
    } else if (psp_cpu_clock == 300) {
      psp_cpu_clock = 333;
    } else {
      psp_cpu_clock = 222;
    }
  } else {
    if (psp_cpu_clock == 222) {
      psp_cpu_clock = 333;
    } else if (psp_cpu_clock == 333) {
      psp_cpu_clock = 300;
    } else {
      psp_cpu_clock = 222;
    }
  }
}

static void
psp_settings_menu_sound_quality(int step)
{
  if (step > 0) {
    if (psp_sound_quality <   4) psp_sound_quality++;
    if (psp_sound_quality == 3) psp_sound_quality = 4;
  } else {
    if (psp_sound_quality >   1) psp_sound_quality--;
    if (psp_sound_quality == 3) psp_sound_quality = 2;
  }
}

static void
psp_settings_menu_screen_ymax(int step)
{
  if (step > 0) {
    if (psp_screen_ymax <  160) psp_screen_ymax += 5;
  } else {
    if (psp_screen_ymax >   80) psp_screen_ymax -= 5;
  }
}

static void
psp_settings_menu_screen_ymin(int step)
{
  if (step > 0) {
    if (psp_screen_ymin <   80) psp_screen_ymin += 5;
  } else {
    if (psp_screen_ymin >    0) psp_screen_ymin -= 5;
  }
}

static void
psp_settings_menu_skip_fps(int step)
{
  if (step > 0) {
    if (gba_skip_fps <  25) gba_skip_fps++;
  } else {
    if (gba_skip_fps >  -2) gba_skip_fps--;
  }
}

static void
psp_settings_menu_turbo_mode(int step)
{
  if (step > 0) {
    if (gba_turbo_mode <  4) gba_turbo_mode++;
  } else {
    if (gba_turbo_mode >  0) gba_turbo_mode--;
  }
}

static void
psp_settings_menu_arm_tick(int step)
{
  if (step > 0) {
    if (arm_tick_average < 20) arm_tick_average++;
  } else {
    if (arm_tick_average >  1) arm_tick_average--;
  }
}

static void
psp_settings_menu_render(int step)
{
  if (step > 0) {
    if (gba_render_mode < GBA_LAST_RENDER) gba_render_mode++;
    else                                   gba_render_mode = 0;
  } else {
    if (gba_render_mode > 0) gba_render_mode--;
    else                     gba_render_mode = GBA_LAST_RENDER;
  }
}

static void
psp_settings_menu_init(void)
{
  gba_snd_enable        = GBA.gba_snd_enable;
  gba_disable_sfx       = GBA.gba_disable_sfx;
  gba_render_mode       = GBA.gba_render_mode;
  gba_skip_fps          = GBA.psp_skip_max_frame;
  gba_turbo_mode        = GBA.psp_turbo_mode;
  psp_screen_ymin       = GBA.psp_screen_ymin;
  psp_screen_ymax       = GBA.psp_screen_ymax;
  psp_show_fps          = GBA.psp_show_fps;
  psp_cpu_clock         = GBA.psp_cpu_clock;
  gba_save_battery      = GBA.gba_save_battery;
  arm_tick_average      = GBA.arm_tick_average;
  psp_reverse_analog    = GBA.psp_reverse_analog;
  psp_disable_gpu_cache = GBA.psp_disable_gpu_cache;
  psp_sound_quality     = GBA.psp_sound_quality;
  gba_bios_file_enable  = GBA.gba_bios_file_enable;
}

static void
psp_settings_menu_load(int format)
{
  int ret;

  ret = psp_fmgr_menu(format);
  if (ret ==  1) /* load OK */
  {
    psp_display_screen_settings_menu();
    psp_sdl_back2_print(270,  80, "File loaded !", 
                       PSP_MENU_NOTE_COLOR);
    psp_menu_dirty = 1;
    psp_sdl_flip();
    sleep(1);
    psp_settings_menu_init();
  }
  else 
  if (ret == -1) /* Load Error */
  {
    psp_display_screen_settings_menu();
    psp_sdl_back2_print(270,  80, "Can't load file !", 
                       PSP_MENU_WARNING_COLOR);
    psp_menu_dirty = 1;
    psp_sdl_flip();
    sleep(1);
  }
}

static void
psp_settings_menu_validate(void)
{
  /* Validate */
  GBA.gba_snd_enable      = gba_snd_enable;
  GBA.gba_render_mode     = gba_render_mode;
  GBA.gba_disable_sfx     = gba_disable_sfx;
  GBA.psp_screen_ymin     = psp_screen_ymin;
  GBA.psp_screen_ymax     = psp_screen_ymax;
  GBA.psp_cpu_clock       = psp_cpu_clock;
  GBA.psp_reverse_analog  = psp_reverse_analog;
  GBA.psp_show_fps        = psp_show_fps;
  GBA.psp_skip_max_frame  = gba_skip_fps;
  GBA.psp_turbo_mode      = gba_turbo_mode;
  GBA.arm_tick_average    = arm_tick_average;
  GBA.gba_save_battery    = gba_save_battery;
  GBA.psp_disable_gpu_cache = psp_disable_gpu_cache;

  if ((GBA.psp_sound_quality != psp_sound_quality) ||
      (GBA.gba_bios_file_enable != gba_bios_file_enable)) {
    GBA.psp_sound_quality = psp_sound_quality;
    GBA.gba_bios_file_enable = gba_bios_file_enable;
    gba_save_pspvba_settings();

    psp_menu_dirty = 1;
    psp_display_screen_settings_menu();
    psp_sdl_back2_print(270,  80, "PSPVBA settings saved !",
                       PSP_MENU_WARNING_COLOR);
    psp_sdl_back2_print(250,  90, "Will be taken into account",
                       PSP_MENU_WARNING_COLOR);
    psp_sdl_back2_print(290, 100, "on next startup !",
                       PSP_MENU_WARNING_COLOR);
    psp_menu_dirty = 1;
    psp_sdl_flip();
    sleep(4);
  }

  gba_sdl_set_render_line(GBA.gba_render_mode);

  scePowerSetClockFrequency(GBA.psp_cpu_clock, GBA.psp_cpu_clock, GBA.psp_cpu_clock/2);
}

static void
psp_settings_menu_save_config()
{
  int error;

  psp_settings_menu_validate();

  error = gba_save_settings();

  if (! error) /* save OK */
  {
    psp_display_screen_settings_menu();
    psp_sdl_back2_print(270,  80, "File saved !", 
                       PSP_MENU_NOTE_COLOR);
    psp_menu_dirty = 1;
    psp_sdl_flip();
    sleep(1);
  }
  else 
  {
    psp_display_screen_settings_menu();
    psp_sdl_back2_print(270,  80, "Can't save file !", 
                       PSP_MENU_WARNING_COLOR);
    psp_menu_dirty = 1;
    psp_sdl_flip();
    sleep(1);
  }
}

int
psp_settings_menu_exit(void)
{
  SceCtrlData c;

  psp_display_screen_settings_menu();
  psp_sdl_back2_print(270,  80, "press X to confirm !", 
                     PSP_MENU_WARNING_COLOR);
  psp_menu_dirty = 1;
  psp_sdl_flip();

  psp_kbd_wait_no_button();

  do
  {
    sceCtrlReadBufferPositive(&c, 1);
    c.Buttons &= PSP_ALL_BUTTON_MASK;

    if (c.Buttons & PSP_CTRL_CROSS) psp_sdl_exit(0);

  } while (c.Buttons == 0);

  psp_kbd_wait_no_button();

  return 0;
}

static void
psp_settings_menu_save()
{
  int error;

  psp_settings_menu_validate();
  error = gba_save_settings();

  if (! error) /* save OK */
  {
    psp_display_screen_settings_menu();
    psp_sdl_back2_print(270,  80, "File saved !", 
                       PSP_MENU_NOTE_COLOR);
    psp_menu_dirty = 1;
    psp_sdl_flip();
    sleep(1);
  }
  else 
  {
    psp_display_screen_settings_menu();
    psp_sdl_back2_print(270,  80, "Can't save file !", 
                       PSP_MENU_WARNING_COLOR);
    psp_menu_dirty = 1;
    psp_sdl_flip();
    sleep(1);
  }
}

static void
psp_settings_menu_reset(void)
{
  psp_display_screen_settings_menu();
  psp_sdl_back2_print(270, 80, "Reset Settings !", 
                     PSP_MENU_WARNING_COLOR);
  psp_menu_dirty = 1;
  psp_sdl_flip();
  gba_default_settings();
  psp_settings_menu_init();
  sleep(1);
}

int 
psp_settings_menu(void)
{
  SceCtrlData c;
  long        new_pad;
  long        old_pad;
  int         last_time;
  int         end_menu;

  psp_kbd_wait_no_button();

  old_pad   = 0;
  last_time = 0;
  end_menu  = 0;

  psp_settings_menu_init();

  psp_menu_dirty = 1;

  while (! end_menu)
  {
    psp_display_screen_settings_menu();
    psp_sdl_flip();

    while (1)
    {
      sceCtrlReadBufferPositive(&c, 1);
      c.Buttons &= PSP_ALL_BUTTON_MASK;

      if (c.Buttons) break;
    }

    new_pad = c.Buttons;

    if ((old_pad != new_pad) || ((c.TimeStamp - last_time) > PSP_MENU_MIN_TIME)) {
      last_time = c.TimeStamp;
      old_pad = new_pad;

    } else continue;

    if ((c.Buttons & (PSP_CTRL_LTRIGGER|PSP_CTRL_RTRIGGER|PSP_CTRL_START)) ==
        (PSP_CTRL_LTRIGGER|PSP_CTRL_RTRIGGER|PSP_CTRL_START)) {
      /* Exit ! */
      psp_sdl_exit(0);
    } else
    if ((c.Buttons & PSP_CTRL_RTRIGGER) == PSP_CTRL_RTRIGGER) {
      psp_settings_menu_reset();
      end_menu = 1;
    } else
    if ((new_pad & PSP_CTRL_CROSS ) || 
        (new_pad & PSP_CTRL_CIRCLE) || 
        (new_pad & PSP_CTRL_RIGHT ) ||
        (new_pad & PSP_CTRL_LEFT  )) 
    {
      int step;

      if (new_pad & PSP_CTRL_LEFT)  step = -1;
      else 
      if (new_pad & PSP_CTRL_RIGHT) step =  1;
      else                          step =  0;

      switch (cur_menu_id ) 
      {
        case MENU_SET_SOUND        : gba_snd_enable = ! gba_snd_enable;
        break;                     
        case MENU_SET_SOUND_QUALITY : psp_settings_menu_sound_quality( step );
        break;               
        case MENU_SET_SFX          : gba_disable_sfx = ! gba_disable_sfx;
        break;                     
        case MENU_SET_GPU_CACHE    : psp_disable_gpu_cache = ! psp_disable_gpu_cache;
        break;                     
        case MENU_SET_SKIP_FPS     : psp_settings_menu_skip_fps( step );
        break;                     
        case MENU_SET_TURBO_MODE   : psp_settings_menu_turbo_mode( step );
        break;               
        case MENU_SET_SCREEN_YMIN  : psp_settings_menu_screen_ymin( step );
        break;                     
        case MENU_SET_SCREEN_YMAX  : psp_settings_menu_screen_ymax( step );
        break;               
        case MENU_SET_SHOW_FPS     : psp_show_fps = ! psp_show_fps;
        break;                     
        case MENU_SET_ANALOG       : psp_reverse_analog = ! psp_reverse_analog;
        break;                     
        case MENU_SET_RENDER       : psp_settings_menu_render( step );
        break;                     
        case MENU_SET_CLOCK        : psp_settings_menu_clock( step );
        break;
        case MENU_SET_TICK_ARM     : psp_settings_menu_arm_tick( step );
        break;              
        case MENU_SET_SAVE_BATTERY : gba_save_battery = ! gba_save_battery;
        break;              
        case MENU_SET_BIOS_FILE    : gba_bios_file_enable = ! gba_bios_file_enable;
        break;              
        case MENU_SET_LOAD         : psp_settings_menu_load(FMGR_FORMAT_SET);
                                     psp_menu_dirty = 1;
                                     old_pad = new_pad = 0;
        break;              
        case MENU_SET_SAVE         : psp_settings_menu_save();
                                     psp_menu_dirty = 1;
                                     old_pad = new_pad = 0;
        break;                     
        case MENU_SET_RESET        : psp_settings_menu_reset();
        break;                     
                                   
        case MENU_SET_BACK         : end_menu = 1;
        break;                     
      }

    } else
    if(new_pad & PSP_CTRL_UP) {

      if (cur_menu_id > 0) cur_menu_id--;
      else                 cur_menu_id = MAX_MENU_SET_ITEM-1;

    } else
    if(new_pad & PSP_CTRL_DOWN) {

      if (cur_menu_id < (MAX_MENU_SET_ITEM-1)) cur_menu_id++;
      else                                     cur_menu_id = 0;

    } else  
    if(new_pad & PSP_CTRL_SQUARE) {
      /* Cancel */
      end_menu = -1;
    } else 
    if(new_pad & PSP_CTRL_SELECT) {
      /* Back to GBA */
      end_menu = 1;
    }
  }
 
  if (end_menu > 0) {
    psp_settings_menu_validate();
  }

  psp_kbd_wait_no_button();

  return 1;
}

