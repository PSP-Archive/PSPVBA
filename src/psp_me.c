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
#include <zlib.h>
#include <string.h>
#include "SDL.h"

#include <pspkernel.h>
#include <pspdebug.h>
#include <psppower.h>
#include <pspsdk.h>
#include <pspctrl.h>
#include <pspaudio.h>
#include <pspthreadman.h>
#include <stdlib.h>
#include <stdio.h>

#include "global.h"
#include "psp_me.h"
#include "me/me.h"
#include "Sound.h"

static int  psp_me_sound_channel = -1;
static volatile int loc_me_sound_exit = 0;
static int  loc_me_sound_thid = -1;
static volatile u16 *psp_me_snd_buffer = NULL;
static int  psp_me_snd_buffer_length = 0;
static int  psp_me_snd_buffer_max_length = 8192;
static int  psp_me_snd_initialized = 0;

extern void me_soundTick(me_sound_t* p);

volatile me_sound_t __attribute__((aligned(64))) me_sound_data;
volatile struct me_struct                       *me_data;

void
psp_me_init(void)
{
  me_data = me_struct_init();
  me_startproc((u32)me_function, (u32)me_data);
}

void 
psp_me_stop(void)
{
  me_stopproc();
}

# define MAXVOLUME	0x8000

void
psp_me_process_buffer(void)
{
  sceAudioOutputPannedBlocking( psp_me_sound_channel, MAXVOLUME, MAXVOLUME, psp_me_snd_buffer);
}

void 
psp_me_sound_thread(SceSize args, void *argp)
{
  volatile me_sound_t *p=(me_sound_t *)PSP_UNCACHE_PTR(&me_sound_data);
  
  me_sceKernelDcacheWritebackInvalidateAll();
  sceKernelDcacheWritebackInvalidateAll();

  p->exit = 0;

  me_start(me_data, (int)(&me_soundTick), (int)p);    

  do {
    psp_me_process_buffer();

    while ((sd->soundPaused) || (!GBA.gba_snd_enable)) {
      sceKernelDelayThread(1000000);
      if (loc_me_sound_exit) break;
    }

  } while (!loc_me_sound_exit);

  p->exit=1;
  me_wait(me_data);  
}

void
psp_me_set_sound_tick()
{
  volatile me_sound_t *p=(me_sound_t *)PSP_UNCACHE_PTR(&me_sound_data);
  p->sound_tick = 1;
}

void 
psp_me_sound_pause(void)
{
  volatile me_sound_t *p=(me_sound_t *)PSP_UNCACHE_PTR(&me_sound_data);
  p->sound_pause = 1;
}

void 
psp_me_sound_resume(void)
{
  volatile me_sound_t *p=(me_sound_t *)PSP_UNCACHE_PTR(&me_sound_data);
  p->sound_pause = 0;
}


int
psp_me_get_sound_tick()
{
  volatile me_sound_t *p=(me_sound_t *)PSP_UNCACHE_PTR(&me_sound_data);
  return p->sound_tick;
}

int
psp_me_get_num_tick()
{
  volatile me_sound_t *p=(me_sound_t *)PSP_UNCACHE_PTR(&me_sound_data);
  return p->num_tick;
}

int
psp_me_get_num_loop()
{
  volatile me_sound_t *p=(me_sound_t *)PSP_UNCACHE_PTR(&me_sound_data);
  return p->num_loop;
}

void
psp_me_sound_start(void)
{
  loc_me_sound_exit = 0;
  loc_me_sound_thid = sceKernelCreateThread( "sound thread", (SceKernelThreadEntry)psp_me_sound_thread, 0x8, 256*1024, 0, 0 );
  if(loc_me_sound_thid >= 0) sceKernelStartThread(loc_me_sound_thid, 0, 0);
}

void
psp_me_sound_stop(void)
{
  if (loc_me_sound_thid != -1) {
    loc_me_sound_exit = 1;
    sceKernelWaitThreadEnd( loc_me_sound_thid, NULL );
    sceKernelDeleteThread( loc_me_sound_thid );        
    loc_me_sound_thid = -1;    
  }
  sceAudioChRelease( psp_me_sound_channel );
  sceKernelDelayThread(1000000); 
}

void
psp_me_sound_init()
{
  /* LUDO: */

  /* Quality 1 : sample_stereo_16bits: 1024 */
  /* Quality 2 : sample_stereo_16bits:  512 */
  /* Quality 4 : sample_stereo_16bits:  256 */
  int num_sample_stereo_16bits;

  if (sd->soundQuality == 1) {
    num_sample_stereo_16bits = 1024;
  } else 
  if (sd->soundQuality == 2) {
    num_sample_stereo_16bits =  512;
  } else {
    num_sample_stereo_16bits =  256;
  }

  sd->soundBufferLen = num_sample_stereo_16bits * 2; /* num samples mono */

  if (psp_me_sound_channel == -1) {
# if 1 //LUDO: FOR_TEST
    psp_me_sound_channel = sceAudioChReserve( -1, num_sample_stereo_16bits * 2, 0 ); /* num samples stereo */
# else
    psp_me_sound_channel = sceAudioChReserve( -1, num_sample_stereo_16bits, 0 ); /* num samples stereo */
# endif
  } else {
    sceAudioSetChannelDataLen( psp_me_sound_channel, num_sample_stereo_16bits * 2 );
  }

  if (psp_me_snd_buffer == NULL) {
    psp_me_snd_buffer_max_length = 16384;
    psp_me_snd_buffer = (u16 *)malloc(psp_me_snd_buffer_max_length);
  }
  memset(psp_me_snd_buffer, 0, psp_me_snd_buffer_max_length);
  /* size in bytes of the ME buffer */
  psp_me_snd_buffer_length = num_sample_stereo_16bits * 8;
  psp_me_snd_initialized   = 1;
}

int
systemWriteDataToSoundBuffer()
{
  u16 *snd_buffer = (u16 *)PSP_UNCACHE_PTR(psp_me_snd_buffer);
  if (sd->soundPaused) return 0;

  if (snd_buffer) {
# if 1 //LUDO: FOR_TEST
    for (int index = 0; index < sd->soundBufferLen; index += 2) {
      snd_buffer[(index << 1) + 0] = sd->soundFinalWave[index + 0];
      snd_buffer[(index << 1) + 1] = sd->soundFinalWave[index + 1];
      snd_buffer[(index << 1) + 2] = sd->soundFinalWave[index + 0];
      snd_buffer[(index << 1) + 3] = sd->soundFinalWave[index + 1];
    }
# else
    for (int index = 0; index < sd->soundBufferLen; index ++) {
      snd_buffer[index] = sd->soundFinalWave[index];
    }
# endif
    return 1;
  }
 
  return 0;
}

void 
systemSoundPause()
{
  if (psp_me_snd_initialized) {
    memset(psp_me_snd_buffer, 0, psp_me_snd_buffer_length);
  }

  psp_me_sound_pause();
}

void systemSoundResume()
{
  if (psp_me_snd_initialized) {
    memset(psp_me_snd_buffer, 0, psp_me_snd_buffer_length);
  }

  psp_me_sound_resume();
}

void systemSoundReset()
{
}

void systemSoundShutdown()
{
}

