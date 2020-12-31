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

# ifndef _PSP_ME_H_
# define _PSP_ME_H_

  typedef struct {
     int  sound_tick;
     int  sound_pause;
     int  num_tick;
     int  num_loop;
     int  exit;
     int  dummy;

  } me_sound_t;


  extern void psp_me_init(void);
  extern void psp_me_sound_stop(void);

  extern int  psp_me_get_sound_tick();
  extern int  psp_me_get_num_tick();
  extern int  psp_me_get_num_loop();

# endif
