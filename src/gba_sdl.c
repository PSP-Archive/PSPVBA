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

#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <psptypes.h>

#include "global.h"
#include "psp_kbd.h"

#include "AutoBuild.h"

#include "SDL.h"
#include "GBA.h"
#include "Flash.h"
#include "Port.h"
#include "RTC.h"
#include "Sound.h"
#include "Text.h"
#include "unzip.h"
#include "Util.h"
#include "Gfx.h"

#include "global.h"
#include "psp_sdl.h"
#include "psp_gu.h"
#include "psp_kbd.h"

#ifndef WIN32
# include <unistd.h>
# define GETCWD getcwd
#else // WIN32
# include <direct.h>
# define GETCWD _getcwd
#endif // WIN32

# include "psp_sdl.h"

#ifdef MMX
extern bool cpu_mmx;
#endif
extern bool soundEcho;
extern bool soundLowPass;
extern bool soundReverse;

extern void SmartIB(u8*,u32,int,int);
extern void SmartIB32(u8*,u32,int,int);
extern void MotionBlurIB(u8*,u32,int,int);
extern void MotionBlurIB32(u8*,u32,int,int);

extern void CPUUpdateRenderBuffers(bool);

static void gba_sdl_render_normal();

 void (*gba_sdl_render_line_func)(void) = gba_sdl_render_normal;

int systemSpeed = 0;
int systemDebug = 0;
int systemVerbose = 0;
int systemSaveUpdateCounter = SYSTEM_SAVE_NOT_UPDATED;

int srcPitch = 0;
int srcWidth = 0;
int srcHeight = 0;
int destWidth = 0;
int destHeight = 0;

int sensorX = 2047;
int sensorY = 2047;

int sdlPrintUsage = 0;
int disableMMX = 0;

int cartridgeType = 0;
int captureFormat = 0;

int pauseWhenInactive = 0;
int active = 1;
int emulating = 0;
int RGB_LOW_BITS_MASK=0x821;
u16 systemGbPalette[24];
int ifbType = 0;
char ipsname[512];

static char *rewindMemory = NULL;
static int rewindPos = 0;
static int rewindTopPos = 0;
static int rewindCounter = 0;
static int rewindCount = 0;
static bool rewindSaveNeeded = false;
static int rewindTimer = 0;

#define REWIND_SIZE 400000

#define _stricmp strcasecmp

bool sdlButtons[4][12] = {
  { false, false, false, false, false, false, 
    false, false, false, false, false, false },
  { false, false, false, false, false, false,
    false, false, false, false, false, false },
  { false, false, false, false, false, false,
    false, false, false, false, false, false },
  { false, false, false, false, false, false,
    false, false, false, false, false, false }
};

bool sdlMotionButtons[4] = { false, false, false, false };

int sdlNumDevices = 0;
SDL_Joystick **sdlDevices = NULL;

bool wasPaused = false;

int frameskipadjust = 0;
int throttle = 0;
u32 throttleLastTime = 0;

int renderedFrames = 0;
int showRenderedFrames = 0;
# if 1 //LUDO: 
int showSpeed = 1;
# else
int showSpeed = 0;
# endif
int showSpeedTransparent = 1;

bool disableStatusMessages = false;
bool paused = false;
bool debugger = false;
int fullscreen = 1;
int yuvType = 0;
bool removeIntros = false;
int sdlFlashSize = 0;
int sdlAutoIPS = 1;
int sdlRtcEnable = 0;

int sdlDefaultJoypad = 0;

int  mouseCounter = 0;

bool screenMessage = false;
u32  screenMessageTime = 0;

int sdlSoundLen = 0;

char *arg0;

u16 joypad[4][12] = {
  { SDLK_LEFT,  SDLK_RIGHT,
    SDLK_UP,    SDLK_DOWN,
    SDLK_z,     SDLK_x,
    SDLK_RETURN,SDLK_BACKSPACE,
    SDLK_a,     SDLK_s,
    SDLK_SPACE, SDLK_F12
  },
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
};

u16 defaultJoypad[12] = {
  SDLK_LEFT,  SDLK_RIGHT,
  SDLK_UP,    SDLK_DOWN,
  SDLK_z,     SDLK_x,
  SDLK_RETURN,SDLK_BACKSPACE,
  SDLK_a,     SDLK_s,
  SDLK_SPACE, SDLK_F12
};

u16 motion[4] = {
  SDLK_KP4, SDLK_KP6, SDLK_KP8, SDLK_KP2
};

u16 defaultMotion[4] = {
  SDLK_KP4, SDLK_KP6, SDLK_KP8, SDLK_KP2
};

extern bool CPUIsGBAImage(char *);
extern bool gbIsGameboyRom(char *);

static inline void sdlStretch16x1(u8 *src, u8 *dest)
{
  u16 *s = (u16 *)src;
  u16 *d = (u16 *)dest;
  for(int i = 0; i < srcWidth; i++)
    *d++ = *s++;
}

#ifdef __MSC__
#define stat _stat
#define S_IFDIR _S_IFDIR
#endif

void sdlCheckDirectory(char *dir)
{
  struct stat buf;

  int len = strlen(dir);

  char *p = dir + len - 1;

  if(*p == '/' ||
     *p == '\\')
    *p = 0;
  
  if(stat(dir, &buf) == 0) {
    if(!(buf.st_mode & S_IFDIR)) {
      fprintf(stderr, "Error: %s is not a directory\n", dir);
      dir[0] = 0;
    }
  } else {
    fprintf(stderr, "Error: %s does not exist\n", dir);
    dir[0] = 0;
  }
}

char *sdlGetFilename(char *name)
{
  static char filebuffer[2048];

  int len = strlen(name);
  
  char *p = name + len - 1;
  
  while(true) {
    if(*p == '/' ||
       *p == '\\') {
      p++;
      break;
    }
    len--;
    p--;
    if(len == 0)
      break;
  }
  
  if(len == 0)
    strcpy(filebuffer, name);
  else
    strcpy(filebuffer, p);
  return filebuffer;
}

FILE *sdlFindFile(const char *name)
{
  char buffer[4096];
  char path[2048];

#ifdef WIN32
#define PATH_SEP ";"
#define FILE_SEP '\\'
#define EXE_NAME "VisualBoyAdvance-SDL.exe"
#else // ! WIN32
#define PATH_SEP ":"
#define FILE_SEP '/'
#define EXE_NAME "VisualBoyAdvance"
#endif // ! WIN32

  GETCWD(buffer, 2048);
  
  FILE *f = fopen(name, "r");
  if(f != NULL) {
    return f;
  }

  char *home = getenv("HOME");

  if(home != NULL) {
    sprintf(path, "%s%c%s", home, FILE_SEP, name);
    f = fopen(path, "r");
    if(f != NULL)
      return f;
  }

#ifdef WIN32
  home = getenv("USERPROFILE");
  if(home != NULL) {
    sprintf(path, "%s%c%s", home, FILE_SEP, name);
    f = fopen(path, "r");
    if(f != NULL)
      return f;
  }
#else // ! WIN32
    sprintf(path, "%s%c%s", SYSCONFDIR, FILE_SEP, name);
    f = fopen(path, "r");
    if(f != NULL)
      return f;
#endif // ! WIN32

  if(!strchr(arg0, '/') &&
     !strchr(arg0, '\\')) {
    char *path = getenv("PATH");

    if(path != NULL) {
      strncpy(buffer, path, 4096);
      buffer[4095] = 0;
      char *tok = strtok(buffer, PATH_SEP);
      
      while(tok) {
        sprintf(path, "%s%c%s", tok, FILE_SEP, EXE_NAME);
        f = fopen(path, "r");
        if(f != NULL) {
          char path2[2048];
          fclose(f);
          sprintf(path2, "%s%c%s", tok, FILE_SEP, name);
          f = fopen(path2, "r");
          if(f != NULL) {
            return f;
          }
        }
        tok = strtok(NULL, PATH_SEP);
      }
    }
  } else {
    // executable is relative to some directory
    strcpy(buffer, arg0);
    char *p = strrchr(buffer, FILE_SEP);
    if(p) {
      *p = 0;
      sprintf(path, "%s%c%s", buffer, FILE_SEP, name);
      f = fopen(path, "r");
      if(f != NULL)
        return f;
    }
  }
  return NULL;
}

static void sdlApplyPerImagePreferences()
{
  FILE *f = sdlFindFile("vba-over.ini");
  if(!f) {
    return;
  }

  char buffer[7];
  buffer[0] = '[';
  buffer[1] = rom[0xac];
  buffer[2] = rom[0xad];
  buffer[3] = rom[0xae];
  buffer[4] = rom[0xaf];
  buffer[5] = ']';
  buffer[6] = 0;

  char readBuffer[2048];

  bool found = false;
  
  while(1) {
    char *s = fgets(readBuffer, 2048, f);

    if(s == NULL)
      break;

    char *p  = strchr(s, ';');
    
    if(p)
      *p = 0;
    
    char *token = strtok(s, " \t\n\r=");

    if(!token)
      continue;
    if(strlen(token) == 0)
      continue;

    if(!strcmp(token, buffer)) {
      found = true;
      break;
    }
  }

  if(found) {
    while(1) {
      char *s = fgets(readBuffer, 2048, f);

      if(s == NULL)
        break;

      char *p = strchr(s, ';');
      if(p)
        *p = 0;

      char *token = strtok(s, " \t\n\r=");
      if(!token)
        continue;
      if(strlen(token) == 0)
        continue;

      if(token[0] == '[') // starting another image settings
        break;
      char *value = strtok(NULL, "\t\n\r=");
      if(value == NULL)
        continue;
      
      if(!strcmp(token, "rtcEnabled"))
        rtcEnable(atoi(value) == 0 ? false : true);
      else if(!strcmp(token, "flashSize")) {
        int size = atoi(value);
        if(size == 0x10000 || size == 0x20000)
          flashSetSize(size);
      } else if(!strcmp(token, "saveType")) {
        int save = atoi(value);
        if(save >= 0 && save <= 5)
          cpuSaveType = save;
      }
    }
  }
  fclose(f);
}

static int sdlCalculateMaskWidth(u32 mask)
{
  int m = 0;
  int mask2 = mask;

  while(mask2) {
    m++;
    mask2 >>= 1;
  }

  int m2 = 0;
  mask2 = mask;
  while(!(mask2 & 1)) {
    m2++;
    mask2 >>= 1;
  }

  return m - m2;
}

void 
sdlWriteBattery()
{
  if (GBA.gba_save_battery) {
    char  FileName[MAX_PATH+1];
    snprintf(FileName, MAX_PATH, "%s/save/bat_%s.sav", GBA.gba_home_dir, GBA.gba_save_name);
    CPUWriteBatteryFile(FileName);
  }
}

void 
sdlReadBattery()
{
  if (GBA.gba_save_battery) {
    char  FileName[MAX_PATH+1];
    snprintf(FileName, MAX_PATH, "%s/save/bat_%s.sav", GBA.gba_home_dir, GBA.gba_save_name);
    CPUReadBatteryFile(FileName);
  }
}

#define MOD_KEYS    (KMOD_CTRL|KMOD_SHIFT|KMOD_ALT|KMOD_META)
#define MOD_NOCTRL  (KMOD_SHIFT|KMOD_ALT|KMOD_META)
#define MOD_NOALT   (KMOD_CTRL|KMOD_SHIFT|KMOD_META)
#define MOD_NOSHIFT (KMOD_CTRL|KMOD_ALT|KMOD_META)

int
gba_load_rom_filename(char *filename)
{
  char szFile[MAX_PATH];
  char base_filename[MAX_PATH];

  strcpy(base_filename, filename);
  strcpy(szFile       , filename);

  utilGetBaseName(filename, base_filename);
  if(ipsname[0] == 0) {
    sprintf(ipsname, "%s.ips", base_filename);
  }
  
  bool failed = false;

  enum IMAGE_TYPE type = utilFindType(szFile);

  if(type == IMAGE_UNKNOWN) {
    systemMessage(0, "Unknown file type %s", szFile);
    exit(-1);
  }
  cartridgeType = (int)type;
  
  if(type == IMAGE_GBA) {
    int size = CPULoadRom(szFile);
    failed = (size == 0);
    if(!failed) {
      //        if(cpuEnhancedDetection && cpuSaveType == 0) {
      //          utilGBAFindSave(rom, size);
      //        }

      sdlApplyPerImagePreferences();
      
      cartridgeType = 0;

      CPUInit(GBA.gba_use_bios);

      CPUReset();
      if(sdlAutoIPS) {
        int size = 0x2000000;
        utilApplyIPS(ipsname, &rom, &size);
        if(size != 0x2000000) {
          CPUReset();
        }
      }
    }
  }
  
  if(failed) {
    systemMessage(0, "Failed to load file %s", szFile);
    return -1;
  }

  sdlReadBattery();

  srcWidth = 240;
  srcHeight = 160;
  
  destWidth  = srcWidth;
  destHeight = srcHeight;

  return 0;
}

int 
SDL_main(int argc, char **argv)
{
  arg0 = argv[0];
  
  ipsname[0] = 0;
  
  parseDebug = true;

  sdlPrintUsage = 0;

  global_init_memory();

  gba_initialize_config();

# ifdef PSP_ME
  sound_init_memory();

  psp_me_sound_init();
# endif
#ifdef MMX
  if(disableMMX) cpu_mmx = 0;
#endif

  if(rewindTimer) rewindMemory = (char *)malloc(8*REWIND_SIZE);

  if(sdlFlashSize == 0) flashSetSize(0x10000);
  else                  flashSetSize(0x20000);

  rtcEnable(sdlRtcEnable ? true : false);
  
  for(int i = 0; i < 24;) {
    systemGbPalette[i++] = (0x1f) | (0x1f << 5) | (0x1f << 10);
    systemGbPalette[i++] = (0x15) | (0x15 << 5) | (0x15 << 10);
    systemGbPalette[i++] = (0x0c) | (0x0c << 5) | (0x0c << 10);
    systemGbPalette[i++] = 0;
  }

  systemSaveUpdateCounter = SYSTEM_SAVE_NOT_UPDATED;

  if (gba_load_rom("default.gba")) {
    fprintf(stderr,"failed to load default rom\n");
    exit(-1);
  }

  sdl_start_ticks();

  psp_gu_init();

  psp_sdl_clear_screen(0x00);

# ifndef PSP_ME
  psp_sound_init();
# endif

  int Rmask=0x1f;
  int Gmask=0x7e0;
  int Bmask=0xf800;

  if(sdlCalculateMaskWidth(Gmask) == 6) {
    RGB_LOW_BITS_MASK = 0x821;
  } else {
    RGB_LOW_BITS_MASK = 0x421;      
  }
  
  
  srcPitch = srcWidth * 2+4;

  emulating = 1;
  renderedFrames = 0;

  soundInit();

# ifdef PSP_ME
  psp_me_sound_start();
# else
  psp_sound_start();
# endif

  int check_kbd_counter = 0;

  while (emulating) {

    if (++check_kbd_counter > 10) {
      psp_update_keys();
      check_kbd_counter = 0;
    }

    if(!paused && active) {
      CPULoop(5000);
      if(rewindSaveNeeded && rewindMemory && CPUWriteMemState) {
        rewindCount++;
        if(rewindCount > 8)
          rewindCount = 8;
        if(CPUWriteMemState &&
           CPUWriteMemState(&rewindMemory[rewindPos*REWIND_SIZE], 
                                     REWIND_SIZE)) {
          rewindPos = (rewindPos + 1) & 7;
          if(rewindCount == 8)
            rewindTopPos = (rewindTopPos + 1) & 7;
        }
      }

      rewindSaveNeeded = false;
      
    }
  }

# ifdef PSP_ME
  psp_me_sound_stop();
# else
  psp_sound_stop();
# endif
  
  emulating = 0;
  soundShutdown();

  if(rom != NULL) {
    sdlWriteBattery();
    systemSaveUpdateCounter = SYSTEM_SAVE_NOT_UPDATED;
    CPUCleanUp();
  }

  return 0;
}

void 
systemMessage(int num, const char *msg, ...)
{
  char buffer[1024];
  va_list valist;
  
  va_start(valist, msg);
  vsprintf(buffer, msg, valist);
  
  fprintf(stderr, "%s\n", buffer);
  va_end(valist);
}

typedef unsigned char byte;
typedef unsigned short word;
typedef unsigned int dword;

/* 

  LUDO: 16-bit HiColor (565 format) 
  see http://www.compuphase.com/graphic/scale3.htm

 */
static inline word loc_coloraverage(word a, word b)
{
  return (word)(((a ^ b) & 0xf7deU) >> 1) + (a & b);
}

static inline void 
render16bpp_X15_pixel(word *dist, word *src)
{
  dist[0] = src[0];
  dist[1] = loc_coloraverage(src[0], src[1]);
  dist[2] = src[1];
  dist[3] = src[2];
  dist[4] = loc_coloraverage(src[2], src[3]);
  dist[5] = src[3];
}


void 
systemShowSpeed(int speed)
{
  systemSpeed = speed;

  showRenderedFrames = renderedFrames;
  renderedFrames = 0;  
}

extern void (*renderLine)();

static void 
gba_sdl_render_normal()
{
  psp_gu_normal_blit((u16 *)lineMixArray, 0);
}

static void 
gba_sdl_render_normal_scanline()
{
  for (int src_y = 1; src_y < 160; src_y++)
  {
    if (src_y & 1) continue;
    u16 *dest = psp_gu_get_vram_addr();
    u16 *loc_lineMix = lineMixArray[src_y - 1];
    int destPitch = 512;
    int delta_y = src_y + ((272 - srcHeight) / 2);
    int delta_x = ((480 - srcWidth) / 2);
    dest += (delta_y * destPitch) + delta_x;

    for(int x = 0; x < 240; x++) {
      *dest++ = loc_lineMix[x];
    }
  }
}

static void 
gba_sdl_render_fit_height()
{
  psp_gu_fit_height_blit((u16 *)lineMixArray, 0);
}

static void 
gba_sdl_render_max()
{
  psp_gu_max_blit((u16 *)lineMixArray, 0);
}

static void 
gba_sdl_render_max_scanline()
{
  for (int src_y = 1; src_y < 160; src_y++)
  {
    u16 *dest = psp_gu_get_vram_addr();
    u16 *loc_lineMix = lineMixArray[src_y - 1];
    int destPitch = 512;
    int delta_y = (src_y * 272) / srcHeight;
    if (delta_y & 1) delta_y--;
    dest += (delta_y * destPitch);
    short color0;
    short color1 = loc_lineMix[0];
    for(int x = 1; x < 240; x++) {
      color0 = color1;
      color1 = loc_lineMix[x];
      *dest++ = color0;
      *dest++ = loc_coloraverage(color0, color1);
    }
  }
}

bool systemReadJoypads()
{
  return true;
}

void
gba_sdl_key_press(int key_id, int key_press)
{
  sdlButtons[0][key_id] = (key_press != 0);
}

u32 systemReadJoypad(int which)
{
  if(which < 0 || which > 3)
    which = sdlDefaultJoypad;
  
  u32 res = 0;
  
  if(sdlButtons[which][KEY_BUTTON_A])
    res |= 1;
  if(sdlButtons[which][KEY_BUTTON_B])
    res |= 2;
  if(sdlButtons[which][KEY_BUTTON_SELECT])
    res |= 4;
  if(sdlButtons[which][KEY_BUTTON_START])
    res |= 8;
  if(sdlButtons[which][KEY_RIGHT])
    res |= 16;
  if(sdlButtons[which][KEY_LEFT])
    res |= 32;
  if(sdlButtons[which][KEY_UP])
    res |= 64;
  if(sdlButtons[which][KEY_DOWN])
    res |= 128;
  if(sdlButtons[which][KEY_BUTTON_R])
    res |= 256;
  if(sdlButtons[which][KEY_BUTTON_L])
    res |= 512;

  // disallow L+R or U+D of being pressed at the same time
  if((res & 48) == 48)
    res &= ~16;
  if((res & 192) == 192)
    res &= ~128;

  return res;
}

void system10Frames()
{
  if(rewindMemory) {
    if(++rewindCounter >= rewindTimer) {
      rewindSaveNeeded = true;
      rewindCounter = 0;
    }
  }

  if (systemSaveUpdateCounter) {
    if(--systemSaveUpdateCounter <= SYSTEM_SAVE_NOT_UPDATED) {
      sdlWriteBattery();
      systemSaveUpdateCounter = SYSTEM_SAVE_NOT_UPDATED;
    }
  }

  wasPaused = false;
}

u32 systemGetClock()
{
  return sdl_get_ticks();
}

void systemUpdateMotionSensor()
{
  if(sdlMotionButtons[KEY_LEFT]) {
    sensorX += 3;
    if(sensorX > 2197)
      sensorX = 2197;
    if(sensorX < 2047)
      sensorX = 2057;
  } else if(sdlMotionButtons[KEY_RIGHT]) {
    sensorX -= 3;
    if(sensorX < 1897)
      sensorX = 1897;
    if(sensorX > 2047)
      sensorX = 2037;
  } else if(sensorX > 2047) {
    sensorX -= 2;
    if(sensorX < 2047)
      sensorX = 2047;
  } else {
    sensorX += 2;
    if(sensorX > 2047)
      sensorX = 2047;
  }

  if(sdlMotionButtons[KEY_UP]) {
    sensorY += 3;
    if(sensorY > 2197)
      sensorY = 2197;
    if(sensorY < 2047)
      sensorY = 2057;
  } else if(sdlMotionButtons[KEY_DOWN]) {
    sensorY -= 3;
    if(sensorY < 1897)
      sensorY = 1897;
    if(sensorY > 2047)
      sensorY = 2037;
  } else if(sensorY > 2047) {
    sensorY -= 2;
    if(sensorY < 2047)
      sensorY = 2047;
  } else {
    sensorY += 2;
    if(sensorY > 2047)
      sensorY = 2047;
  }    
}

int systemGetSensorX()
{
  return sensorX;
}

int systemGetSensorY()
{
  return sensorY;
}

void systemGbPrint(u8 *data,int pages,int feed,int palette, int contrast)
{
}

// Code donated by Niels Wagenaar (BoycottAdvance)

void emulator_reset(void) {
  CPUReset();
}

void
gba_sdl_set_render_line(int render_mode)
{
  gba_sdl_render_line_func = gba_sdl_render_normal;

  if (GBA.gba_render_mode == GBA_RENDER_NORMAL         ) gba_sdl_render_line_func = gba_sdl_render_normal;
  else
  if (GBA.gba_render_mode == GBA_RENDER_NORMAL_SCANLINE) gba_sdl_render_line_func = gba_sdl_render_normal_scanline;
  else
  if (GBA.gba_render_mode == GBA_RENDER_FIT_HEIGHT     ) gba_sdl_render_line_func = gba_sdl_render_fit_height;
  else
  if (GBA.gba_render_mode == GBA_RENDER_MAX            ) gba_sdl_render_line_func = gba_sdl_render_max;
  else
  if (GBA.gba_render_mode == GBA_RENDER_MAX_SCANLINE   ) gba_sdl_render_line_func = gba_sdl_render_max_scanline;
}

extern int gba_frame_parity;

void
gba_sdl_render(int loc_vcount)
{
  int draw = 0;

  if (((160 - loc_vcount) <= GBA.psp_screen_ymin) || 
      ((160 - loc_vcount) >= GBA.psp_screen_ymax)) {
    return;
  }
 
  if (! GBA.psp_turbo_mode) {
    draw = 1;
  } else
  if (gba_frame_parity) {
    if (GBA.psp_turbo_mode & 1) {
      if ((loc_vcount > 40) && (loc_vcount < 120)) {
        draw = 1;
      }
    } else {
      if ((loc_vcount > 50) && (loc_vcount < 110)) {
        draw = 1;
      }
    }
  } else {
    if (GBA.psp_turbo_mode & 1) {
      draw = 1;
    } else {
      if ((loc_vcount <= 60) || (loc_vcount >= 100)) {
        draw = 1;
      }
    }
  }

  if ((GBA.psp_turbo_mode <= 2) || draw) {
    gba_render_skip = 0;
  } else {
    gba_render_skip = 1;
  }
  if ((GBA.psp_turbo_mode <= 4) || draw) {
# if 0
    lineMix = PSP_UNCACHE_PTR(lineMixArray[VCOUNT]);
# else
    lineMix = lineMixArray[VCOUNT];
# endif
    (*renderLine)();
  }
}

void
gba_reset_mix_array(void)
{
  memset(lineMixArray, 0, sizeof(lineMixArray));
}

extern unsigned char psp_font[];

void 
gba_mix_print_char(int x, int y, int color, int bgcolor, char c, int drawfg, int drawbg)
{
  int cx;
  int cy;
  int b;
  int index;

  ushort *vram = (ushort *)lineMixArray + (y * GBA_SCREEN_WIDTH) + x;
  index = ((ushort)c) * 8;

  for (cy=0; cy<8; cy++) {
    b=0x80;
    for (cx=0; cx<8; cx++) {
      if (psp_font[index] & b) {
        if (drawfg) vram[cx + cy * GBA_SCREEN_WIDTH] = color;
      } else {
        if (drawbg) vram[cx + cy * GBA_SCREEN_WIDTH] = bgcolor;
      }
      b = b >> 1;
    }
    index++;
  }
}

void 
gba_mix_print(int x,int y,const char *str, int color, int bgcolor)
{
  int index;
  int x0 = x;

  for (index = 0; str[index] != '\0'; index++) {
    gba_mix_print_char(x, y, color, bgcolor, str[index], 1, 1);
    x += 8;
    if (x >= (GBA_SCREEN_WIDTH - 8)) {
      x = x0; y++;
    }
    if (y >= (GBA_SCREEN_HEIGHT - 8)) break;
  }
}


void
systemDisplayFps(void)
{
# if 1 //LUDO: FOR_TEST
  char buffer[64];
  sprintf(buffer, "%3d%%(%2d fps)", systemSpeed, showRenderedFrames);
  gba_mix_print(0, 150, buffer, psp_sdl_rgb(0xff, 0xff, 0xff), 0x0);
# else
# ifdef PSP_ME
  char buffer[64];
  sprintf(buffer, "%d %d", psp_me_get_num_loop(), psp_me_get_num_tick());
  gba_mix_print(0, 150, buffer, psp_sdl_rgb(0xff, 0xff, 0xff), 0x0);
# endif
# endif
}

void 
systemDrawScreen(void)
{
  renderedFrames++;

  if (GBA.psp_show_fps) systemDisplayFps();

  (*gba_sdl_render_line_func)();

  if (psp_screenshot_mode) {
    psp_screenshot_mode--;
    if (psp_screenshot_mode <= 0) {
      systemSoundPause();
      psp_sdl_save_screenshot();
      psp_screenshot_mode = 0;
      systemSoundResume();
    }
  }
}

