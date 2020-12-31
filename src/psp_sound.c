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
#include "psp_sound.h"
#include "Sound.h"

static int  psp_sound_channel = -1;
static int loc_sound_exit = 0;
static int  loc_sound_thid = -1;
static u16 *psp_snd_buffer = NULL;
static int  psp_snd_buffer_length = 0;
static int  psp_snd_buffer_max_length = 8192;
static int  psp_snd_initialized = 0;

# define MAXVOLUME	0x8000

void
psp_sound_process_buffer(void)
{
  sceAudioOutputPannedBlocking( psp_sound_channel, MAXVOLUME, MAXVOLUME, psp_snd_buffer);
}

void 
psp_sound_thread(SceSize args, void *argp)
{
  do {
    psp_sound_process_buffer();

    while ((soundPaused) || (!GBA.gba_snd_enable)) {
      sceKernelDelayThread(1000000);
      if (loc_sound_exit) break;
    }

  } while (!loc_sound_exit);
}

void
psp_sound_start(void)
{
  loc_sound_exit = 0;
  loc_sound_thid = sceKernelCreateThread( "sound thread", (SceKernelThreadEntry)psp_sound_thread, 0x8, 256*1024, 0, 0 );
  if(loc_sound_thid >= 0) sceKernelStartThread(loc_sound_thid, 0, 0);
}

void
psp_sound_stop(void)
{
  if (loc_sound_thid != -1) {
    loc_sound_exit = 1;
    sceKernelWaitThreadEnd( loc_sound_thid, NULL );
    sceKernelDeleteThread( loc_sound_thid );        
    loc_sound_thid = -1;    
  }
  sceAudioChRelease( psp_sound_channel );
  sceKernelDelayThread(1000000); 
}

void
psp_sound_init()
{
  /* LUDO: */

  /* Quality 1 : sample_stereo_16bits: 1024 */
  /* Quality 2 : sample_stereo_16bits:  512 */
  /* Quality 4 : sample_stereo_16bits:  256 */
  int num_sample_stereo_16bits;

  if (soundQuality == 1) {
    num_sample_stereo_16bits = 1024;
  } else 
  if (soundQuality == 2) {
    num_sample_stereo_16bits =  512;
  } else {
    num_sample_stereo_16bits =  256;
  }

  soundBufferLen = num_sample_stereo_16bits * 2; /* num samples mono */

  if (psp_sound_channel == -1) {
# if 1 //LUDO: FOR_TEST
    psp_sound_channel = sceAudioChReserve( -1, num_sample_stereo_16bits * 2, 0 ); /* num samples stereo */
# else
    psp_sound_channel = sceAudioChReserve( -1, num_sample_stereo_16bits, 0 ); /* num samples stereo */
# endif
  } else {
    sceAudioSetChannelDataLen( psp_sound_channel, num_sample_stereo_16bits * 2 );
  }

  if (psp_snd_buffer == NULL) {
    psp_snd_buffer_max_length = 16384;
    psp_snd_buffer = (u16 *)malloc(psp_snd_buffer_max_length);
  }
  memset(psp_snd_buffer, 0, psp_snd_buffer_max_length);
  /* size in bytes of the ME buffer */
  psp_snd_buffer_length = num_sample_stereo_16bits * 8;
  psp_snd_initialized   = 1;
}

int
systemWriteDataToSoundBuffer()
{
  u16 *snd_buffer = psp_snd_buffer;
  if (soundPaused) return 0;

  if (snd_buffer) {
# if 1 //LUDO: FOR_TEST
    for (int index = 0; index < soundBufferLen; index += 2) {
      snd_buffer[(index << 1) + 0] = soundFinalWave[index + 0];
      snd_buffer[(index << 1) + 1] = soundFinalWave[index + 1];
      snd_buffer[(index << 1) + 2] = soundFinalWave[index + 0];
      snd_buffer[(index << 1) + 3] = soundFinalWave[index + 1];
    }
# else
    for (int index = 0; index < soundBufferLen; index ++) {
      snd_buffer[index] = soundFinalWave[index];
    }
# endif
    return 1;
  }
 
  return 0;
}

void 
systemSoundPause()
{
  if (psp_snd_initialized) {
    memset(psp_snd_buffer, 0, psp_snd_buffer_length);
  }

  soundPaused = 1;
}

void systemSoundResume()
{
  if (psp_snd_initialized) {
    memset(psp_snd_buffer, 0, psp_snd_buffer_length);
  }

  soundPaused = 0;
}

void systemSoundReset()
{
}

void systemSoundShutdown()
{
}

