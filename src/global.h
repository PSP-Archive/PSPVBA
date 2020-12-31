#ifndef __GLOBAL_H_
#define __GLOBAL_H_

#include <stdio.h>

# define true      1
# define false     0

#define PSP_UNCACHE_PTR(p) ( ((long)(p))|0x40000000 )
#define PSP_NORMAL_PTR(p)  ( ((long)(p))&0xBFFFFFFF )

extern int strcasecmp(const char *s1, const char *s2);
extern char *strdup(const char *s);
typedef unsigned char bool;

# define log printf

# define GBA_RENDER_NORMAL          0
# define GBA_RENDER_NORMAL_SCANLINE 1
# define GBA_RENDER_FIT_HEIGHT      2
# define GBA_RENDER_MAX             3
# define GBA_RENDER_MAX_SCANLINE    4
# define GBA_LAST_RENDER            4

# define MAX_PATH   256
# define GBA_MAX_SAVE_STATE 5

  typedef struct GBA_t {
 
    char gba_save_used[GBA_MAX_SAVE_STATE];
    char gba_save_name[MAX_PATH];
    char gba_bios_name[MAX_PATH];
    char gba_home_dir[MAX_PATH];
    int  gba_use_bios;
    int  psp_screenshot_id;
    int  psp_cpu_clock;
    int  psp_reverse_analog;
    int  gba_snd_enable;
    int  gba_render_mode;
    int  gba_disable_sfx;
    int  psp_skip_max_frame;
    int  psp_screen_ymin;
    int  psp_screen_ymax;
    int  psp_turbo_mode;
    int  psp_show_fps;
    int  arm_tick_average;
    int  gba_save_battery;
    int  psp_disable_gpu_cache;
    int  psp_sound_quality;
    int  gba_bios_file_enable;

  } GBA_t;

  extern GBA_t GBA;

  extern void (*gba_sdl_render_line_func)(void);

  extern void update_save_name(char *Name);
  extern void reset_save_name();
  extern void gba_kbd_load(void);
  extern int  gba_kbd_save(void);
  extern void emulator_reset(void);
  extern int gba_load_rom(char *FileName);
  extern int gba_load_state(char *FileName);

  extern int gba_load_settings();
  extern int gba_save_settings();

  extern int gba_load_pspvba_settings();
  extern int gba_save_pspvba_settings();

  extern int gba_snapshot_save_slot(int save_id);
  extern int gba_snapshot_load_slot(int load_id);
  extern int gba_snapshot_del_slot(int save_id);
  extern void gba_initialize_config(void);
  extern void gba_sdl_set_render_line(int render_mode);
  extern int gba_load_file_settings(char *FileName);

#endif
