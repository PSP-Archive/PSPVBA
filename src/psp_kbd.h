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

# ifndef _KBD_H_
# define _KBD_H_

# ifdef __cplusplus
  extern "C" {
# endif

# define PSP_ALL_BUTTON_MASK 0xFFFF

 enum gba_keys_emum {

    KEY_LEFT, 
    KEY_RIGHT,
    KEY_UP, 
    KEY_DOWN,
    KEY_BUTTON_A,
    KEY_BUTTON_B,
    KEY_BUTTON_START,
    KEY_BUTTON_SELECT,
    KEY_BUTTON_L, 
    KEY_BUTTON_R,
    KEY_BUTTON_MENU,
    GBA_MAX_KEY      
  };

# define KBD_UP           0
# define KBD_RIGHT        1
# define KBD_DOWN         2
# define KBD_LEFT         3
# define KBD_TRIANGLE     4
# define KBD_CIRCLE       5
# define KBD_CROSS        6
# define KBD_SQUARE       7
# define KBD_SELECT       8
# define KBD_START        9
# define KBD_HOME        10
# define KBD_HOLD        11
# define KBD_LTRIGGER    12
# define KBD_RTRIGGER    13

# define KBD_MAX_BUTTONS 14

# define KBD_JOY_UP      14
# define KBD_JOY_RIGHT   15
# define KBD_JOY_DOWN    16
# define KBD_JOY_LEFT    17

# define KBD_ALL_BUTTONS 18

 struct gba_key_trans {
   int  key;
   char name[10];
 };
  
  extern int psp_kbd_mode_ui;

  extern int    psp_screenshot_mode;
  extern int    psp_kbd_mapping[ KBD_ALL_BUTTONS ];
  extern struct gba_key_trans psp_gba_key_info[GBA_MAX_KEY];

  extern int    psp_update_keys(void);
  extern void   kbd_wait_start(void);
  extern void   psp_init_keyboard(void);
  extern void   psp_kbd_wait_no_button(void);
  extern int    psp_kbd_is_danzeff_mode(void);
  extern int    psp_kbd_load_mapping(char *kbd_filename);

# ifdef __cplusplus
  }
# endif
# endif
