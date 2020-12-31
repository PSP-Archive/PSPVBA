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

#include <pspctrl.h>
#include <pspkernel.h>
#include <pspdebug.h>

#include "global.h"
#include "psp_kbd.h"
#include "psp_sdl.h"
#include "psp_menu.h"
#include "psp_battery.h"

# define KBD_MIN_ANALOG_TIME      150000
# define KBD_MIN_START_TIME       800000
# define KBD_MAX_EVENT_TIME       500000
# define KBD_MIN_PENDING_TIME     300000
# define KBD_MIN_COMMAND_TIME     100000
# define KBD_MIN_BATTCHECK_TIME 90000000 

 static SceCtrlData    loc_button_data;
 static char           loc_button_press[ KBD_MAX_BUTTONS ]; 
 static char           loc_button_release[ KBD_MAX_BUTTONS ]; 
 static unsigned int   loc_button_mask[ KBD_MAX_BUTTONS ] =
 {
   PSP_CTRL_UP         , /*  KBD_UP         */
   PSP_CTRL_RIGHT      , /*  KBD_RIGHT      */
   PSP_CTRL_DOWN       , /*  KBD_DOWN       */
   PSP_CTRL_LEFT       , /*  KBD_LEFT       */
   PSP_CTRL_TRIANGLE   , /*  KBD_TRIANGLE   */
   PSP_CTRL_CIRCLE     , /*  KBD_CIRCLE     */
   PSP_CTRL_CROSS      , /*  KBD_CROSS      */
   PSP_CTRL_SQUARE     , /*  KBD_SQUARE     */
   PSP_CTRL_SELECT     , /*  KBD_SELECT     */
   PSP_CTRL_START      , /*  KBD_START      */
   PSP_CTRL_HOME       , /*  KBD_HOME       */
   PSP_CTRL_HOLD       , /*  KBD_HOLD       */
   PSP_CTRL_LTRIGGER   , /*  KBD_LTRIGGER   */
   PSP_CTRL_RTRIGGER   , /*  KBD_RTRIGGER   */
 };

  static char loc_button_name[ KBD_ALL_BUTTONS ][10] =
  {
    "UP",
    "RIGHT",
    "DOWN",
    "LEFT",
    "TRIANGLE",
    "CIRCLE",
    "CROSS",
    "SQUARE",
    "SELECT",
    "START",
    "HOME",
    "HOLD",
    "LTRIGGER",
    "RTRIGGER",
    "JOY_UP",
    "JOY_RIGHT",
    "JOY_DOWN",
    "JOY_LEFT"
  };

  struct gba_key_trans psp_gba_key_info[GBA_MAX_KEY]=
  {
    { KEY_LEFT,               "LEFT"    },
    { KEY_RIGHT,              "RIGHT"   },
    { KEY_UP,                 "UP"      },
    { KEY_DOWN,               "DOWN"    },
    { KEY_BUTTON_A,           "A"       },
    { KEY_BUTTON_B,           "B"       },
    { KEY_BUTTON_START,       "START"   },
    { KEY_BUTTON_SELECT,      "SELECT"  },
    { KEY_BUTTON_L,           "L"       },
    { KEY_BUTTON_R,           "R"       },
    { KEY_BUTTON_MENU,        "MENU"    }
  };

  static int loc_default_mapping[ KBD_ALL_BUTTONS ] = {
    KEY_UP              , /*  KBD_UP         */
    KEY_RIGHT           , /*  KBD_RIGHT      */
    KEY_DOWN            , /*  KBD_DOWN       */
    KEY_LEFT            , /*  KBD_LEFT       */
    KEY_BUTTON_START    , /*  KBD_TRIANGLE   */
    KEY_BUTTON_B        , /*  KBD_CIRCLE     */
    KEY_BUTTON_A        , /*  KBD_CROSS      */
    KEY_BUTTON_SELECT   , /*  KBD_SQUARE     */
    KEY_BUTTON_MENU     , /*  KBD_SELECT     */
    KEY_BUTTON_START    , /*  KBD_START      */
    KEY_BUTTON_MENU     , /*  KBD_HOME       */
    -1                  , /*  KBD_HOLD       */
    KEY_BUTTON_L        , /*  KBD_LTRIGGER   */
    KEY_BUTTON_R        , /*  KBD_RTRIGGER   */
    -1                  , /*  KBD_JOY_UP     */
    -1                  , /*  KBD_JOY_RIGHT  */
    -1                  , /*  KBD_JOY_DOWN   */
    -1                    /*  KBD_JOY_LEFT   */
  };

        int psp_kbd_mapping[ KBD_ALL_BUTTONS ];

 static int psp_kbd_mapping_initialized = 0;

 static int ret_key_code;

extern  int key_shift;
extern  int key_consol;
extern  int key_pressed;
extern  int joystick_states[2][6];

int
gba_key_event(int gba_idx, int button_press)
{
  gba_sdl_key_press(gba_idx, button_press);
  return 0;
}

int 
gba_kbd_reset()
{
  int index;
  for (index = 0; index < GBA_MAX_KEY; index++) {
    gba_key_event(index, 0);
  }
  return 0;
}

int
psp_kbd_reset_mapping(void)
{
  memcpy(psp_kbd_mapping, loc_default_mapping, sizeof(loc_default_mapping));
  return 0;
}

int
psp_kbd_load_mapping(char *kbd_filename)
{
  char     Buffer[512];
  FILE    *KbdFile;
  char    *Scan;
  int      tmp_mapping[KBD_ALL_BUTTONS];
  int      gba_key_id = 0;
  int      kbd_id = 0;
  int      error = 0;

  memcpy(tmp_mapping, loc_default_mapping, sizeof(loc_default_mapping));

  KbdFile = fopen(kbd_filename, "r");
  error   = 1;

  if (KbdFile != (FILE*)0) {

    while (fgets(Buffer,512,KbdFile) != (char *)0) {

      Scan = strchr(Buffer,'\n');
      if (Scan) *Scan = '\0';
      /* For this #@$% of windows ! */
      Scan = strchr(Buffer,'\r');
      if (Scan) *Scan = '\0';
      if (Buffer[0] == '#') continue;

      Scan = strchr(Buffer,'=');
      if (! Scan) continue;

      *Scan = '\0';
      gba_key_id = atoi(Scan + 1);

      for (kbd_id = 0; kbd_id < KBD_ALL_BUTTONS; kbd_id++) {
        if (!strcasecmp(Buffer,loc_button_name[kbd_id])) {
          tmp_mapping[kbd_id] = gba_key_id;
          break;
        }
      }
    }

    error = 0;
    fclose(KbdFile);
  }

  memcpy(psp_kbd_mapping, tmp_mapping, sizeof(psp_kbd_mapping));
  psp_kbd_mapping_initialized = 1;

  return error;
}

int
psp_kbd_save_mapping(char *kbd_filename)
{
  FILE    *KbdFile;
  int      kbd_id = 0;
  int      error = 0;

  KbdFile = fopen(kbd_filename, "w");
  error   = 1;

  if (KbdFile != (FILE*)0) {

    for (kbd_id = 0; kbd_id < KBD_ALL_BUTTONS; kbd_id++)
    {
      fprintf(KbdFile, "%s=%d\n", loc_button_name[kbd_id], psp_kbd_mapping[kbd_id]);
    }
    error = 0;
    fclose(KbdFile);
  }

  return error;
}

int
gba_decode_key(int psp_b, int button_pressed)
{
  int wake = 0;
  int reverse_analog = GBA.psp_reverse_analog;

  if (reverse_analog) {
    if ((psp_b >= KBD_JOY_UP  ) &&
        (psp_b <= KBD_JOY_LEFT)) {
      psp_b = psp_b - KBD_JOY_UP + KBD_UP;
    } else
    if ((psp_b >= KBD_UP  ) &&
        (psp_b <= KBD_LEFT)) {
      psp_b = psp_b - KBD_UP + KBD_JOY_UP;
    }
  }

  if (psp_kbd_mapping[psp_b] == KEY_BUTTON_MENU) {
    if (button_pressed) {
      psp_main_menu();
      psp_init_keyboard();
    }
  } else {
 
    if (psp_kbd_mapping[psp_b] != -1) {
      wake = 1;
      gba_key_event(psp_kbd_mapping[psp_b], button_pressed);
    }
  }
  return 0;
}

# define ANALOG_THRESHOLD 60

void 
kbd_get_analog_direction(int Analog_x, int Analog_y, int *x, int *y)
{
  int DeltaX = 255;
  int DeltaY = 255;
  int DirX   = 0;
  int DirY   = 0;

  *x = 0;
  *y = 0;

  if (Analog_x <=        ANALOG_THRESHOLD)  { DeltaX = Analog_x; DirX = -1; }
  else 
  if (Analog_x >= (255 - ANALOG_THRESHOLD)) { DeltaX = 255 - Analog_x; DirX = 1; }

  if (Analog_y <=        ANALOG_THRESHOLD)  { DeltaY = Analog_y; DirY = -1; }
  else 
  if (Analog_y >= (255 - ANALOG_THRESHOLD)) { DeltaY = 255 - Analog_y; DirY = 1; }

  *x = DirX;
  *y = DirY;
}

static long first_time_stamp = -1;

int
kbd_scan_keyboard(void)
{
  SceCtrlData c;
  long        delta_stamp;
  int         event;
  int         b;

  int new_Lx;
  int new_Ly;
  int old_Lx;
  int old_Ly;

  event = 0;
  sceCtrlPeekBufferPositive(&c, 1);
  c.Buttons &= PSP_ALL_BUTTON_MASK;

  if ((c.Buttons & (PSP_CTRL_LTRIGGER|PSP_CTRL_RTRIGGER|PSP_CTRL_START)) ==
      (PSP_CTRL_LTRIGGER|PSP_CTRL_RTRIGGER|PSP_CTRL_START)) {
    /* Exit ! */
    psp_sdl_exit(0);
  } else
  if ((c.Buttons & (PSP_CTRL_SELECT|PSP_CTRL_START)) == (PSP_CTRL_SELECT|PSP_CTRL_START)) {
    psp_main_menu();
    psp_init_keyboard();
    return 0;
  }

  delta_stamp = c.TimeStamp - first_time_stamp;
  if ((delta_stamp < 0) || (delta_stamp > KBD_MIN_BATTCHECK_TIME)) {
    first_time_stamp = c.TimeStamp;
    if (psp_is_low_battery()) {
      psp_main_menu();
      psp_init_keyboard();
      return 0;
    }
  }

  /* Check Analog Device */
  kbd_get_analog_direction(loc_button_data.Lx,loc_button_data.Ly,&old_Lx,&old_Ly);
  kbd_get_analog_direction( c.Lx, c.Ly, &new_Lx, &new_Ly);

  /* Analog device has moved */
  if (new_Lx > 0) {
    if (old_Lx  > 0) gba_decode_key(KBD_JOY_LEFT , 0);
    gba_decode_key(KBD_JOY_RIGHT, 1);

  } else 
  if (new_Lx < 0) {
    if (old_Lx  < 0) gba_decode_key(KBD_JOY_RIGHT, 0);
    gba_decode_key(KBD_JOY_LEFT , 1);

  } else {
    if (old_Lx  > 0) gba_decode_key(KBD_JOY_LEFT , 0);
    else
    if (old_Lx  < 0) gba_decode_key(KBD_JOY_RIGHT, 0);
    else {
      gba_decode_key(KBD_JOY_LEFT  , 0);
      gba_decode_key(KBD_JOY_RIGHT , 0);
    }
  }

  if (new_Ly < 0) {
    if (old_Ly  > 0) gba_decode_key(KBD_JOY_DOWN , 0);
    gba_decode_key(KBD_JOY_UP   , 1);

  } else 
  if (new_Ly > 0) {
    if (old_Ly  < 0) gba_decode_key(KBD_JOY_UP   , 0);
    gba_decode_key(KBD_JOY_DOWN , 1);

  } else {
    if (old_Ly  > 0) gba_decode_key(KBD_JOY_DOWN , 0);
    else
    if (old_Ly  < 0) gba_decode_key(KBD_JOY_UP   , 0);
    else {
      gba_decode_key(KBD_JOY_DOWN , 0);
      gba_decode_key(KBD_JOY_UP   , 0);
    }
  }

  for (b = 0; b < KBD_MAX_BUTTONS; b++) 
  {
    if (c.Buttons & loc_button_mask[b]) {
      if (!(loc_button_data.Buttons & loc_button_mask[b])) {
        loc_button_press[b] = 1;
        event = 1;
      }
    } else {
      if (loc_button_data.Buttons & loc_button_mask[b]) {
        loc_button_release[b] = 1;
        event = 1;
      }
    }
  }
  memcpy(&loc_button_data,&c,sizeof(SceCtrlData));

  return event;
}

void
kbd_wait_start(void)
{
  while (1)
  {
    SceCtrlData c;

    sceCtrlReadBufferPositive(&c, 1);
    c.Buttons &= PSP_ALL_BUTTON_MASK;

    if (c.Buttons & PSP_CTRL_START) break;
  }
}

void
psp_init_keyboard(void)
{
  gba_kbd_reset();
}

void
psp_kbd_wait_no_button(void)
{
  SceCtrlData c;

  do {

   sceCtrlPeekBufferPositive(&c, 1);
   c.Buttons &= PSP_ALL_BUTTON_MASK;

  } while (c.Buttons != 0);
} 

int
psp_update_keys(void)
{
  int         b;

  static char first_time = 1;
  static int release_pending = 0;

  if (first_time) {

    if (! psp_kbd_mapping_initialized) {
      memcpy(psp_kbd_mapping, loc_default_mapping, sizeof(loc_default_mapping));
      psp_kbd_mapping_initialized = 1;
    }

    SceCtrlData c;
    sceCtrlPeekBufferPositive(&c, 1);
    c.Buttons &= PSP_ALL_BUTTON_MASK;

    if (first_time_stamp == -1) first_time_stamp = c.TimeStamp;
    first_time      = 0;
    release_pending = 0;

    for (b = 0; b < KBD_MAX_BUTTONS; b++) {
      loc_button_release[b] = 0;
      loc_button_press[b] = 0;
    }
    sceCtrlPeekBufferPositive(&loc_button_data, 1);
    loc_button_data.Buttons &= PSP_ALL_BUTTON_MASK;

    psp_main_menu();
    psp_init_keyboard();

    return 0;
  }

  if (release_pending)
  {
    release_pending = 0;
    for (b = 0; b < KBD_MAX_BUTTONS; b++) {
      if (loc_button_release[b]) {
        loc_button_release[b] = 0;
        loc_button_press[b] = 0;
        gba_decode_key(b, 0);
      }
    }
  }

  kbd_scan_keyboard();

  /* check release event */
  for (b = 0; b < KBD_MAX_BUTTONS; b++) {
    if (loc_button_release[b]) {
      release_pending = 1;
      break;
    } 
  }
  /* check press event */
  for (b = 0; b < KBD_MAX_BUTTONS; b++) {
    if (loc_button_press[b]) {
      loc_button_press[b] = 0;
      release_pending     = 0;
      gba_decode_key(b, 1);
    }
  }

  return ret_key_code;
}
