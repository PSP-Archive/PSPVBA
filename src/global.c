#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>

#include <zlib.h>
#include <psppower.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <SDL.h>
#include "global.h"
#include "psp_fmgr.h"
#include "psp_kbd.h"
#include "System.h"

GBA_t GBA;
int   psp_screenshot_mode = 0;

static int
loc_gba_save_settings(char *chFileName)
{
  FILE* FileDesc;
  int   error = 0;

  FileDesc = fopen(chFileName, "w");
  if (FileDesc != (FILE *)0 ) {

    fprintf(FileDesc, "psp_cpu_clock=%d\n"        , GBA.psp_cpu_clock);
    fprintf(FileDesc, "psp_reverse_analog=%d\n"   , GBA.psp_reverse_analog);
    fprintf(FileDesc, "gba_snd_enable=%d\n"       , GBA.gba_snd_enable);
    fprintf(FileDesc, "gba_disable_sfx=%d\n"      , GBA.gba_disable_sfx);
    fprintf(FileDesc, "gba_render_mode=%d\n"      , GBA.gba_render_mode);
    fprintf(FileDesc, "psp_skip_max_frame=%d\n"   , GBA.psp_skip_max_frame);
    fprintf(FileDesc, "psp_turbo_mode=%d\n"       , GBA.psp_turbo_mode);
    fprintf(FileDesc, "psp_show_fps=%d\n"         , GBA.psp_show_fps);
    fprintf(FileDesc, "psp_screen_ymin=%d\n"      , GBA.psp_screen_ymin);
    fprintf(FileDesc, "psp_screen_ymax=%d\n"      , GBA.psp_screen_ymax);
    fprintf(FileDesc, "arm_tick_average=%d\n"     , GBA.arm_tick_average);
    fprintf(FileDesc, "gba_save_battery=%d\n"     , GBA.gba_save_battery);
    fprintf(FileDesc, "psp_disable_gpu_cache=%d\n", GBA.psp_disable_gpu_cache);

    fclose(FileDesc);

  } else {
    error = 1;
  }

  return error;
}

static int
loc_gba_save_pspvba_settings(char *chFileName)
{
  FILE* FileDesc;
  int   error = 0;

  FileDesc = fopen(chFileName, "w");
  if (FileDesc != (FILE *)0 ) {

    fprintf(FileDesc, "psp_sound_quality=%d\n"   , GBA.psp_sound_quality);
    fprintf(FileDesc, "gba_bios_file_enable=%d\n", GBA.gba_bios_file_enable);

    fclose(FileDesc);

  } else {
    error = 1;
  }

  return error;
}

static int
loc_gba_load_settings(char *chFileName)
{
  char  Buffer[512];
  char *Scan;
  unsigned int Value;
  FILE* FileDesc;

  FileDesc = fopen(chFileName, "r");
  if (FileDesc == (FILE *)0 ) return 0;

  while (fgets(Buffer,512, FileDesc) != (char *)0) {

    Scan = strchr(Buffer,'\n');
    if (Scan) *Scan = '\0';
    /* For this #@$% of windows ! */
    Scan = strchr(Buffer,'\r');
    if (Scan) *Scan = '\0';
    if (Buffer[0] == '#') continue;

    Scan = strchr(Buffer,'=');
    if (! Scan) continue;

    *Scan = '\0';
    Value = atoi(Scan+1);

    if (!strcasecmp(Buffer,"psp_cpu_clock"))      GBA.psp_cpu_clock = Value;
    else
    if (!strcasecmp(Buffer,"psp_reverse_analog")) GBA.psp_reverse_analog = Value;
    else
    if (!strcasecmp(Buffer,"gba_snd_enable"))     GBA.gba_snd_enable = Value;
    else
    if (!strcasecmp(Buffer,"gba_render_mode"))    GBA.gba_render_mode = Value;
    else
    if (!strcasecmp(Buffer,"gba_disable_sfx"))    GBA.gba_disable_sfx = Value;
    else
    if (!strcasecmp(Buffer,"psp_skip_max_frame")) GBA.psp_skip_max_frame = Value;
    else
    if (!strcasecmp(Buffer,"psp_turbo_mode"))     GBA.psp_turbo_mode = Value;
    else
    if (!strcasecmp(Buffer,"psp_show_fps"))       GBA.psp_show_fps = Value;
    else
    if (!strcasecmp(Buffer,"psp_screen_ymin"))    GBA.psp_screen_ymin = Value;
    else
    if (!strcasecmp(Buffer,"psp_screen_ymax"))    GBA.psp_screen_ymax = Value;
    else
    if (!strcasecmp(Buffer,"arm_tick_average"))   GBA.arm_tick_average = Value;
    else
    if (!strcasecmp(Buffer,"gba_save_battery"))   GBA.gba_save_battery = Value;
    else
    if (!strcasecmp(Buffer,"psp_disable_gpu_cache")) GBA.psp_disable_gpu_cache = Value;
  }

  fclose(FileDesc);

  return 0;
}

static int
loc_gba_load_pspvba_settings(char *chFileName)
{
  char  Buffer[512];
  char *Scan;
  unsigned int Value;
  FILE* FileDesc;

  FileDesc = fopen(chFileName, "r");
  if (FileDesc == (FILE *)0 ) return 0;

  while (fgets(Buffer,512, FileDesc) != (char *)0) {

    Scan = strchr(Buffer,'\n');
    if (Scan) *Scan = '\0';
    /* For this #@$% of windows ! */
    Scan = strchr(Buffer,'\r');
    if (Scan) *Scan = '\0';
    if (Buffer[0] == '#') continue;

    Scan = strchr(Buffer,'=');
    if (! Scan) continue;

    *Scan = '\0';
    Value = atoi(Scan+1);

    if (!strcasecmp(Buffer,"psp_sound_quality")) GBA.psp_sound_quality = Value;
    else
    if (!strcasecmp(Buffer,"gba_bios_file_enable")) GBA.gba_bios_file_enable = Value;
  }

  fclose(FileDesc);

  return 0;
}

int
gba_load_settings()
{
  char  FileName[MAX_PATH+1];
  int   error;

  error = 1;

  snprintf(FileName, MAX_PATH, "%s/set/%s.set", GBA.gba_home_dir, GBA.gba_save_name);
  error = loc_gba_load_settings(FileName);

  return error;
}

int
gba_load_file_settings(char *FileName)
{
  return loc_gba_load_settings(FileName);
}

int
gba_load_pspvba_settings()
{
  char  FileName[MAX_PATH+1];
  int   error;

  error = 1;

  snprintf(FileName, MAX_PATH, "%s/pspvba.cfg", GBA.gba_home_dir);
  error = loc_gba_load_pspvba_settings(FileName);

  return error;
}

int
gba_save_settings()
{
  char  FileName[MAX_PATH+1];
  int   error;

  error = 1;

  snprintf(FileName, MAX_PATH, "%s/set/%s.set", GBA.gba_home_dir, GBA.gba_save_name);
  error = loc_gba_save_settings(FileName);

  return error;
}

int
gba_save_pspvba_settings()
{
  char  FileName[MAX_PATH+1];
  int   error;

  error = 1;

  snprintf(FileName, MAX_PATH, "%s/pspvba.cfg", GBA.gba_home_dir);
  error = loc_gba_save_pspvba_settings(FileName);

  return error;
}

extern int emulating;

static int 
loc_load_rom(char *TmpName)
{
  int error = gba_load_rom_filename(TmpName);
  if (!error) emulating = 1;
  return error;
}

void
update_save_name(char *Name)
{
  char        TmpFileName[MAX_PATH];
  struct stat aStat;
  int         index;
  char       *SaveName;
  char       *Scan1;
  char       *Scan2;

  SaveName = strrchr(Name,'/');
  if (SaveName != (char *)0) SaveName++;
  else                       SaveName = Name;

  if (!strncasecmp(SaveName, "sav_", 4)) {
    Scan1 = SaveName + 4;
    Scan2 = strrchr(Scan1, '_');
    if (Scan2 && (Scan2[1] >= '0') && (Scan2[1] <= '5')) {
      strncpy(GBA.gba_save_name, Scan1, MAX_PATH);
      GBA.gba_save_name[Scan2 - Scan1] = '\0';
    } else {
      strncpy(GBA.gba_save_name, SaveName, MAX_PATH);
    }
  } else {
    strncpy(GBA.gba_save_name, SaveName, MAX_PATH);
  }

  memset(GBA.gba_save_used,0,sizeof(GBA.gba_save_used));

  if (GBA.gba_save_name[0] == '\0') {
    strcpy(GBA.gba_save_name,"default");
  }

  for (index = 0; index < GBA_MAX_SAVE_STATE; index++) {
    snprintf(TmpFileName, MAX_PATH, "%s/save/sav_%s_%d.sgm", GBA.gba_home_dir, GBA.gba_save_name, index);
    if (! stat(TmpFileName, &aStat)) {
      GBA.gba_save_used[index] = 1;
    }
  }
}

void
reset_save_name()
{
  update_save_name("");
}

void
gba_kbd_load(void)
{
  char        TmpFileName[MAX_PATH + 1];
  struct stat aStat;

  snprintf(TmpFileName, MAX_PATH, "%s/kbd/%s.kbd", GBA.gba_home_dir, GBA.gba_save_name );
  if (! stat(TmpFileName, &aStat)) {
    psp_kbd_load_mapping(TmpFileName);
  }
}

int
gba_kbd_save(void)
{
  char TmpFileName[MAX_PATH + 1];
  snprintf(TmpFileName, MAX_PATH, "%s/kbd/%s.kbd", GBA.gba_home_dir, GBA.gba_save_name );
  return( psp_kbd_save_mapping(TmpFileName) );
}


int
gba_load_rom(char *FileName)
{
  char *scan;
  char  SaveName[MAX_PATH+1];
  int   error;

  error = 1;

  strncpy(SaveName,FileName,MAX_PATH);
  scan = strrchr(SaveName,'.');
  if (scan) *scan = '\0';
  update_save_name(SaveName);
  error = loc_load_rom(FileName);

  if (! error ) {
    gba_kbd_load();
    gba_load_settings();
  }

  return error;
}

extern struct EmulatedSystem emulator;

static int
loc_load_state(char *filename)
{
  int error = 1;
  error = ! CPUReadState_pchar(filename);
  return error;
}

int
gba_load_state(char *FileName)
{
  char *scan;
  char  SaveName[MAX_PATH+1];
  int   error;

  error = 1;

  strncpy(SaveName,FileName,MAX_PATH);
  scan = strrchr(SaveName,'.');
  if (scan) *scan = '\0';
  update_save_name(SaveName);
  error = loc_load_state(FileName);

  if (! error ) {
    gba_kbd_load();
    gba_load_settings();
  }

  return error;
}

static int
gba_save_state(char *filename)
{
  int error = 1;
  error = ! CPUWriteState_pchar(filename);
  return error;
}

int
gba_snapshot_save_slot(int save_id)
{
  char  FileName[MAX_PATH+1];
  int   error;

  error = 1;

  if (save_id < GBA_MAX_SAVE_STATE) {
    snprintf(FileName, MAX_PATH, "%s/save/sav_%s_%d.sgm", GBA.gba_home_dir, GBA.gba_save_name, save_id);
    error = gba_save_state(FileName);
    if (! error) GBA.gba_save_used[save_id] = 1;
  }

  return error;
}

int
gba_snapshot_load_slot(int load_id)
{
  char  FileName[MAX_PATH+1];
  int   error;

  error = 1;

  if (load_id < GBA_MAX_SAVE_STATE) {
    snprintf(FileName, MAX_PATH, "%s/save/sav_%s_%d.sgm", GBA.gba_home_dir, GBA.gba_save_name, load_id);
    error = loc_load_state(FileName);
  }
  return error;
}

int
gba_snapshot_del_slot(int save_id)
{
  char  FileName[MAX_PATH+1];
  int   error;

  error = 1;

  if (save_id < GBA_MAX_SAVE_STATE) {
    snprintf(FileName, MAX_PATH, "%s/save/sav_%s_%d.sgm", GBA.gba_home_dir, GBA.gba_save_name, save_id);
    error = remove(FileName);
    if (! error) GBA.gba_save_used[save_id] = 0;
  }

  return error;
}

void
gba_default_settings(void)
{
  GBA.gba_snd_enable      = 1;
  GBA.gba_render_mode     = GBA_RENDER_FIT_HEIGHT;

  GBA.psp_skip_max_frame  = 1;
  GBA.psp_turbo_mode      = 1;
  GBA.gba_disable_sfx     = 1;
  GBA.psp_reverse_analog  = 0;
  GBA.psp_cpu_clock       = 333;
  GBA.gba_save_battery    = 0;
  GBA.psp_screenshot_id   = 0;
  GBA.psp_show_fps        = 0;
  GBA.psp_screen_ymin     = 0;
  GBA.psp_screen_ymax     = 160;
  GBA.arm_tick_average    = 5;
  GBA.psp_sound_quality     = 2;
  GBA.psp_disable_gpu_cache = 1;
  GBA.gba_bios_file_enable  = 0;

  gba_sdl_set_render_line(GBA.gba_render_mode);

  scePowerSetClockFrequency(GBA.psp_cpu_clock, GBA.psp_cpu_clock, GBA.psp_cpu_clock/2);
}

void
gba_bios_file_enable()
{
  char        TmpFileName[MAX_PATH + 1];
  struct stat aStat;

  sprintf(GBA.gba_bios_name, "%s/gba.bios", GBA.gba_home_dir);

  if (! stat(GBA.gba_bios_name, &aStat)) GBA.gba_use_bios = 1;
  else                                   GBA.gba_use_bios = 0;
}

void
gba_initialize_config(void)
{
  memset(&GBA, 0, sizeof(GBA_t));
  getcwd(GBA.gba_home_dir,MAX_PATH);

  //LUDO:
  GBA.gba_snd_enable        = 1;
  GBA.gba_render_mode       = GBA_RENDER_FIT_HEIGHT;
                            
  GBA.psp_skip_max_frame    = 1;
  GBA.psp_turbo_mode        = 1;
  GBA.gba_disable_sfx       = 1;
  GBA.psp_reverse_analog    = 0;
  GBA.psp_cpu_clock         = 333;
  GBA.gba_save_battery      = 0;
  GBA.psp_screenshot_id     = 0;
  GBA.psp_show_fps          = 0;
  GBA.psp_screen_ymin       = 0;
  GBA.psp_screen_ymax       = 160;
  GBA.arm_tick_average      = 5;
  GBA.psp_sound_quality     = 2;
  GBA.psp_disable_gpu_cache = 1;
  GBA.gba_bios_file_enable  = 1;

  update_save_name("");

  gba_load_pspvba_settings();

  gba_load_settings();

  if (GBA.gba_bios_file_enable) {
    gba_bios_file_enable();
  }

  gba_sdl_set_render_line(GBA.gba_render_mode);

  scePowerSetClockFrequency(GBA.psp_cpu_clock, GBA.psp_cpu_clock, GBA.psp_cpu_clock/2);
}
