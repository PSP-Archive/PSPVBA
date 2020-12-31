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

#include <memory.h>
#include <malloc.h>

#include "global.h"
#include "GBA.h"
#include "Globals.h"
#include "Sound.h"
#include "Util.h"
#include "psp_sdl.h"
#include "psp_me.h"

#define USE_TICKS_AS  380

#define SOUND_MAGIC   0x60000000
#define SOUND_MAGIC_2 0x30000000
#define NOISE_MAGIC 4

extern bool stopState;

u8 soundWavePattern[4][32] = {
  {0x01,0x01,0x01,0x01,
   0xff,0xff,0xff,0xff,
   0xff,0xff,0xff,0xff,
   0xff,0xff,0xff,0xff,
   0xff,0xff,0xff,0xff,
   0xff,0xff,0xff,0xff,
   0xff,0xff,0xff,0xff,
   0xff,0xff,0xff,0xff},
  {0x01,0x01,0x01,0x01,
   0x01,0x01,0x01,0x01,
   0xff,0xff,0xff,0xff,
   0xff,0xff,0xff,0xff,
   0xff,0xff,0xff,0xff,
   0xff,0xff,0xff,0xff,
   0xff,0xff,0xff,0xff,
   0xff,0xff,0xff,0xff},
  {0x01,0x01,0x01,0x01,
   0x01,0x01,0x01,0x01,
   0x01,0x01,0x01,0x01,
   0x01,0x01,0x01,0x01,
   0xff,0xff,0xff,0xff,
   0xff,0xff,0xff,0xff,
   0xff,0xff,0xff,0xff,
   0xff,0xff,0xff,0xff},
  {0x01,0x01,0x01,0x01,
   0x01,0x01,0x01,0x01,
   0x01,0x01,0x01,0x01,
   0x01,0x01,0x01,0x01,
   0x01,0x01,0x01,0x01,
   0x01,0x01,0x01,0x01,
   0xff,0xff,0xff,0xff,
   0xff,0xff,0xff,0xff}
};

int soundFreqRatio[8] = {
  1048576, // 0
  524288,  // 1
  262144,  // 2
  174763,  // 3
  131072,  // 4
  104858,  // 5
  87381,   // 6
  74898    // 7
};

int soundShiftClock[16]= {
      2, // 0
      4, // 1
      8, // 2
     16, // 3
     32, // 4
     64, // 5
    128, // 6
    256, // 7
    512, // 8
   1024, // 9
   2048, // 10
   4096, // 11
   8192, // 12
  16384, // 13
  1,     // 14
  1      // 15
};

  volatile gba_sound_t *sd;

variable_desc *soundSaveStruct;
variable_desc *soundSaveStructV2;

void
sound_reset_memory()
{
  memset(sd, 0, sizeof(gba_sound_t));

  sd->soundQuality = GBA.psp_sound_quality;
  sd->soundTicks = GBA.psp_sound_quality * USE_TICKS_AS;
  sd->SOUND_CLOCK_TICKS = GBA.psp_sound_quality * USE_TICKS_AS;

  sd->soundVolume = 0;
  sd->soundBufferLen = 1024;
  sd->soundPlay = 0;
  sd->soundNextPosition = 0;
  sd->soundLevel1 = 0;
  sd->soundLevel2 = 0;
  sd->soundBalance = 0;
  sd->soundMasterOn = 0;
  sd->soundIndex = 0;
  sd->soundBufferIndex = 0;
  sd->soundDebug = 0;
  sd->sound1On = 0;
  sd->sound1ATL = 0;
  sd->sound1Skip = 0;
  sd->sound1Index = 0;
  sd->sound1Continue = 0;
  sd->sound1EnvelopeVolume =  0;
  sd->sound1EnvelopeATL = 0;
  sd->sound1EnvelopeUpDown = 0;
  sd->sound1EnvelopeATLReload = 0;
  sd->sound1SweepATL = 0;
  sd->sound1SweepATLReload = 0;
  sd->sound1SweepSteps = 0;
  sd->sound1SweepUpDown = 0;
  sd->sound1SweepStep = 0;
  sd->sound1Wave = (u8*)PSP_UNCACHE_PTR(soundWavePattern[2]);
  sd->sound2On = 0;
  sd->sound2ATL = 0;
  sd->sound2Skip = 0;
  sd->sound2Index = 0;
  sd->sound2Continue = 0;
  sd->sound2EnvelopeVolume =  0;
  sd->sound2EnvelopeATL = 0;
  sd->sound2EnvelopeUpDown = 0;
  sd->sound2EnvelopeATLReload = 0;
  sd->sound2Wave = (u8*)PSP_UNCACHE_PTR(soundWavePattern[2]);
  sd->sound3On = 0;
  sd->sound3ATL = 0;
  sd->sound3Skip = 0;
  sd->sound3Index = 0;
  sd->sound3Continue = 0;
  sd->sound3OutputLevel = 0;
  sd->sound3Last = 0;
  sd->sound3Bank = 0;
  sd->sound3DataSize = 0;
  sd->sound3ForcedOutput = 0;
  sd->sound4On = 0;
  sd->sound4Clock = 0;
  sd->sound4ATL = 0;
  sd->sound4Skip = 0;
  sd->sound4Index = 0;
  sd->sound4ShiftRight = 0x7f;
  sd->sound4ShiftSkip = 0;
  sd->sound4ShiftIndex = 0;
  sd->sound4NSteps = 0;
  sd->sound4CountDown = 0;
  sd->sound4Continue = 0;
  sd->sound4EnvelopeVolume =  0;
  sd->sound4EnvelopeATL = 0;
  sd->sound4EnvelopeUpDown = 0;
  sd->sound4EnvelopeATLReload = 0;
  sd->soundControl = 0;
  sd->soundDSFifoAIndex = 0;
  sd->soundDSFifoACount = 0;
  sd->soundDSFifoAWriteIndex = 0;
  sd->soundDSAEnabled = false;
  sd->soundDSATimer = 0;
  sd->soundDSAValue = 0;
  sd->soundDSFifoBIndex = 0;
  sd->soundDSFifoBCount = 0;
  sd->soundDSFifoBWriteIndex = 0;
  sd->soundDSBEnabled = false;
  sd->soundDSBTimer = 0;
  sd->soundDSBValue = 0;
  sd->soundEnableFlag = 0x3ff;
  sd->soundEchoIndex = 0;
  sd->soundEcho = false;
# if 1 //LUDO:
  sd->soundLowPass = false;
# else
  sd->soundLowPass = true;
# endif
  sd->soundReverse = false;
  sd->soundPaused = 1;
}

void
sound_init_memory()
{
  sd = (volatile gba_sound_t *)malloc(sizeof(gba_sound_t));
  sd = (volatile gba_sound_t *)PSP_UNCACHE_PTR(sd);
  soundSaveStruct = (variable_desc *)malloc(sizeof(variable_desc) * 72 );

  sound_reset_memory();

  soundSaveStruct[0].address = &sd->soundPaused; soundSaveStruct[0].size = sizeof(int);
  soundSaveStruct[1].address = &sd->soundPlay; soundSaveStruct[1].size = sizeof(int);
  soundSaveStruct[2].address = &sd->soundTicks; soundSaveStruct[2].size = sizeof(int);
  soundSaveStruct[3].address = &sd->SOUND_CLOCK_TICKS; soundSaveStruct[3].size = sizeof(int);
  soundSaveStruct[4].address = &sd->soundLevel1; soundSaveStruct[4].size = sizeof(int);
  soundSaveStruct[5].address = &sd->soundLevel2; soundSaveStruct[5].size = sizeof(int);
  soundSaveStruct[6].address = &sd->soundBalance; soundSaveStruct[6].size = sizeof(int);
  soundSaveStruct[7].address = &sd->soundMasterOn; soundSaveStruct[7].size = sizeof(int);
  soundSaveStruct[8].address = &sd->soundIndex; soundSaveStruct[8].size = sizeof(int);
  soundSaveStruct[9].address = &sd->sound1On; soundSaveStruct[9].size = sizeof(int);
  soundSaveStruct[10].address = &sd->sound1ATL; soundSaveStruct[10].size = sizeof(int);
  soundSaveStruct[11].address = &sd->sound1Skip; soundSaveStruct[11].size = sizeof(int);
  soundSaveStruct[12].address = &sd->sound1Index; soundSaveStruct[12].size = sizeof(int);
  soundSaveStruct[13].address = &sd->sound1Continue; soundSaveStruct[13].size = sizeof(int);
  soundSaveStruct[14].address = &sd->sound1EnvelopeVolume; soundSaveStruct[14].size = sizeof(int);
  soundSaveStruct[15].address = &sd->sound1EnvelopeATL; soundSaveStruct[15].size = sizeof(int);
  soundSaveStruct[16].address = &sd->sound1EnvelopeATLReload; soundSaveStruct[16].size = sizeof(int);
  soundSaveStruct[17].address = &sd->sound1EnvelopeUpDown; soundSaveStruct[17].size = sizeof(int);
  soundSaveStruct[18].address = &sd->sound1SweepATL; soundSaveStruct[18].size = sizeof(int);
  soundSaveStruct[19].address = &sd->sound1SweepATLReload; soundSaveStruct[19].size = sizeof(int);
  soundSaveStruct[20].address = &sd->sound1SweepSteps; soundSaveStruct[20].size = sizeof(int);
  soundSaveStruct[21].address = &sd->sound1SweepUpDown; soundSaveStruct[21].size = sizeof(int);
  soundSaveStruct[22].address = &sd->sound1SweepStep; soundSaveStruct[22].size = sizeof(int);
  soundSaveStruct[23].address = &sd->sound2On; soundSaveStruct[23].size = sizeof(int);
  soundSaveStruct[24].address = &sd->sound2ATL; soundSaveStruct[24].size = sizeof(int);
  soundSaveStruct[25].address = &sd->sound2Skip; soundSaveStruct[25].size = sizeof(int);
  soundSaveStruct[26].address = &sd->sound2Index; soundSaveStruct[26].size = sizeof(int);
  soundSaveStruct[27].address = &sd->sound2Continue; soundSaveStruct[27].size = sizeof(int);
  soundSaveStruct[28].address = &sd->sound2EnvelopeVolume; soundSaveStruct[28].size = sizeof(int);
  soundSaveStruct[29].address = &sd->sound2EnvelopeATL; soundSaveStruct[29].size = sizeof(int);
  soundSaveStruct[30].address = &sd->sound2EnvelopeATLReload; soundSaveStruct[30].size = sizeof(int);
  soundSaveStruct[31].address = &sd->sound2EnvelopeUpDown; soundSaveStruct[31].size = sizeof(int);
  soundSaveStruct[32].address = &sd->sound3On; soundSaveStruct[32].size = sizeof(int);
  soundSaveStruct[33].address = &sd->sound3ATL; soundSaveStruct[33].size = sizeof(int);
  soundSaveStruct[34].address = &sd->sound3Skip; soundSaveStruct[34].size = sizeof(int);
  soundSaveStruct[35].address = &sd->sound3Index; soundSaveStruct[35].size = sizeof(int);
  soundSaveStruct[36].address = &sd->sound3Continue; soundSaveStruct[36].size = sizeof(int);
  soundSaveStruct[37].address = &sd->sound3OutputLevel; soundSaveStruct[37].size = sizeof(int);
  soundSaveStruct[38].address = &sd->sound4On; soundSaveStruct[38].size = sizeof(int);
  soundSaveStruct[39].address = &sd->sound4ATL; soundSaveStruct[39].size = sizeof(int);
  soundSaveStruct[40].address = &sd->sound4Skip; soundSaveStruct[40].size = sizeof(int);
  soundSaveStruct[41].address = &sd->sound4Index; soundSaveStruct[41].size = sizeof(int);
  soundSaveStruct[42].address = &sd->sound4Clock; soundSaveStruct[42].size = sizeof(int);
  soundSaveStruct[43].address = &sd->sound4ShiftRight; soundSaveStruct[43].size = sizeof(int);
  soundSaveStruct[44].address = &sd->sound4ShiftSkip; soundSaveStruct[44].size = sizeof(int);
  soundSaveStruct[45].address = &sd->sound4ShiftIndex; soundSaveStruct[45].size = sizeof(int);
  soundSaveStruct[46].address = &sd->sound4NSteps; soundSaveStruct[46].size = sizeof(int);
  soundSaveStruct[47].address = &sd->sound4CountDown; soundSaveStruct[47].size = sizeof(int);
  soundSaveStruct[48].address = &sd->sound4Continue; soundSaveStruct[48].size = sizeof(int);
  soundSaveStruct[49].address = &sd->sound4EnvelopeVolume; soundSaveStruct[49].size = sizeof(int);
  soundSaveStruct[50].address = &sd->sound4EnvelopeATL; soundSaveStruct[50].size = sizeof(int);
  soundSaveStruct[51].address = &sd->sound4EnvelopeATLReload; soundSaveStruct[51].size = sizeof(int);
  soundSaveStruct[52].address = &sd->sound4EnvelopeUpDown; soundSaveStruct[52].size = sizeof(int);
  soundSaveStruct[53].address = &sd->soundEnableFlag; soundSaveStruct[53].size = sizeof(int);
  soundSaveStruct[54].address = &sd->soundControl; soundSaveStruct[54].size = sizeof(int);
  soundSaveStruct[55].address = &sd->soundDSFifoAIndex; soundSaveStruct[55].size = sizeof(int);
  soundSaveStruct[56].address = &sd->soundDSFifoACount; soundSaveStruct[56].size = sizeof(int);
  soundSaveStruct[57].address = &sd->soundDSFifoAWriteIndex; soundSaveStruct[57].size = sizeof(int);
  soundSaveStruct[58].address = &sd->soundDSAEnabled; soundSaveStruct[58].size = sizeof(bool);
  soundSaveStruct[59].address = &sd->soundDSATimer; soundSaveStruct[59].size = sizeof(int);
  soundSaveStruct[60].address = &sd->soundDSFifoA[0]; soundSaveStruct[60].size = 32;
  soundSaveStruct[61].address = &sd->soundDSAValue; soundSaveStruct[61].size = sizeof(u8);
  soundSaveStruct[62].address = &sd->soundDSFifoBIndex; soundSaveStruct[62].size = sizeof(int);
  soundSaveStruct[63].address = &sd->soundDSFifoBCount; soundSaveStruct[63].size = sizeof(int);
  soundSaveStruct[64].address = &sd->soundDSFifoBWriteIndex; soundSaveStruct[64].size = sizeof(int);
  soundSaveStruct[65].address = &sd->soundDSBEnabled; soundSaveStruct[65].size = sizeof(int);
  soundSaveStruct[66].address = &sd->soundDSBTimer; soundSaveStruct[66].size = sizeof(int);
  soundSaveStruct[67].address = &sd->soundDSFifoB[0]; soundSaveStruct[67].size = 32;
  soundSaveStruct[68].address = &sd->soundDSBValue; soundSaveStruct[68].size = sizeof(int);
  soundSaveStruct[69].address = &sd->soundBuffer[0][0]; soundSaveStruct[69].size = 6*735;
  soundSaveStruct[70].address = &sd->soundFinalWave[0]; soundSaveStruct[70].size = 2*735;
  soundSaveStruct[71].address = NULL; soundSaveStruct[71].size = 0;

  soundSaveStructV2 = (variable_desc *)malloc(sizeof(variable_desc) * 5 );
  soundSaveStructV2[0].address = &sd->sound3WaveRam[0]; soundSaveStructV2[0].size = 0x20;
  soundSaveStructV2[1].address = &sd->sound3Bank      ; soundSaveStructV2[1].size = sizeof(int);
  soundSaveStructV2[2].address = &sd->sound3DataSize  ; soundSaveStructV2[2].size = sizeof(int);
  soundSaveStructV2[3].address = &sd->sound3ForcedOutput; soundSaveStructV2[3].size = sizeof(int);
  soundSaveStructV2[4].address = NULL                 ; soundSaveStructV2[4].size = 0;
}

bool 
soundInit()
{
  memset(sd->soundBuffer, 0, sizeof(sd->soundBuffer));
  memset(sd->soundFinalWave, 0, sizeof(sd->soundFinalWave));
  
  sd->soundPaused = true;
  return true;
}

void soundEvent_u8(u32 address, u8 data)
{
  int freq = 0;

  switch(address) {
  case NR10:
    data &= 0x7f;
    sd->sound1SweepATL = sd->sound1SweepATLReload = 344 * ((data >> 4) & 7);
    sd->sound1SweepSteps = data & 7;
    sd->sound1SweepUpDown = data & 0x08;
    sd->sound1SweepStep = 0;
    ioMem[address] = data;    
    break;
  case NR11:
    sd->sound1Wave = soundWavePattern[data >> 6];
    sd->sound1ATL  = 172 * (64 - (data & 0x3f));
    ioMem[address] = data;    
    break;
  case NR12:
    sd->sound1EnvelopeUpDown = data & 0x08;
    sd->sound1EnvelopeATLReload = 689 * (data & 7);
    if((data & 0xF8) == 0)
      sd->sound1EnvelopeVolume = 0;
    ioMem[address] = data;    
    break;
  case NR13:
    freq = (((int)(ioMem[NR14] & 7)) << 8) | data;
    sd->sound1ATL = 172 * (64 - (ioMem[NR11] & 0x3f));
    freq = 2048 - freq;
    if(freq) {
      sd->sound1Skip = SOUND_MAGIC / freq;
    } else
      sd->sound1Skip = 0;
    ioMem[address] = data;    
    break;
  case NR14:
    data &= 0xC7;
    freq = (((int)(data&7) << 8) | ioMem[NR13]);
    freq = 2048 - freq;
    sd->sound1ATL = 172 * (64 - (ioMem[NR11] & 0x3f));
    sd->sound1Continue = data & 0x40;
    if(freq) {
      sd->sound1Skip = SOUND_MAGIC / freq;
    } else
      sd->sound1Skip = 0;
    if(data & 0x80) {
      ioMem[NR52] |= 1;
      sd->sound1EnvelopeVolume = ioMem[NR12] >> 4;
      sd->sound1EnvelopeUpDown = ioMem[NR12] & 0x08;
      sd->sound1ATL = 172 * (64 - (ioMem[NR11] & 0x3f));
      sd->sound1EnvelopeATLReload = sd->sound1EnvelopeATL = 689 * (ioMem[NR12] & 7);
      sd->sound1SweepATL = sd->sound1SweepATLReload = 344 * ((ioMem[NR10] >> 4) & 7);
      sd->sound1SweepSteps = ioMem[NR10] & 7;
      sd->sound1SweepUpDown = ioMem[NR10] & 0x08;
      sd->sound1SweepStep = 0;
          
      sd->sound1Index = 0;
      sd->sound1On = 1;
    }
    ioMem[address] = data;    
    break;
  case NR21:
    sd->sound2Wave = soundWavePattern[data >> 6];
    sd->sound2ATL  = 172 * (64 - (data & 0x3f));
    ioMem[address] = data;    
    break;
  case NR22:
    sd->sound2EnvelopeUpDown = data & 0x08;
    sd->sound2EnvelopeATLReload = 689 * (data & 7);
    if((data & 0xF8) == 0)
      sd->sound2EnvelopeVolume = 0;
    ioMem[address] = data;    
    break;
  case NR23:
    freq = (((int)(ioMem[NR24] & 7)) << 8) | data;
    sd->sound2ATL = 172 * (64 - (ioMem[NR21] & 0x3f));
    freq = 2048 - freq;
    if(freq) {
      sd->sound2Skip = SOUND_MAGIC / freq;
    } else
      sd->sound2Skip = 0;
    ioMem[address] = data;    
    break;
  case NR24:
    data &= 0xC7;
    freq = (((int)(data&7) << 8) | ioMem[NR23]);
    freq = 2048 - freq;
    sd->sound2ATL = 172 * (64 - (ioMem[NR21] & 0x3f));
    sd->sound2Continue = data & 0x40;
    if(freq) {
      sd->sound2Skip = SOUND_MAGIC / freq;
    } else
      sd->sound2Skip = 0;
    if(data & 0x80) {
      ioMem[NR52] |= 2;
      sd->sound2EnvelopeVolume = ioMem[NR22] >> 4;
      sd->sound2EnvelopeUpDown = ioMem[NR22] & 0x08;
      sd->sound2ATL = 172 * (64 - (ioMem[NR21] & 0x3f));
      sd->sound2EnvelopeATLReload = sd->sound2EnvelopeATL = 689 * (ioMem[NR22] & 7);
      
      sd->sound2Index = 0;
      sd->sound2On = 1;
    }
    break;
    ioMem[address] = data;    
  case NR30:
    data &= 0xe0;
    if(!(data & 0x80)) {
      ioMem[NR52] &= 0xfb;
      sd->sound3On = 0;
    }
    if(((data >> 6) & 1) != sd->sound3Bank)
      memcpy(&ioMem[0x90], &sd->sound3WaveRam[(((data >> 6) & 1) * 0x10)^0x10],
             0x10);
    sd->sound3Bank = (data >> 6) & 1;
    sd->sound3DataSize = (data >> 5) & 1;
    ioMem[address] = data;    
    break;
  case NR31:
    sd->sound3ATL = 172 * (256-data);
    ioMem[address] = data;    
    break;
  case NR32:
    data &= 0xe0;
    sd->sound3OutputLevel = (data >> 5) & 3;
    sd->sound3ForcedOutput = (data >> 7) & 1;
    ioMem[address] = data;    
    break;
  case NR33:
    freq = 2048 - (((int)(ioMem[NR34]&7) << 8) | data);
    if(freq) {
      sd->sound3Skip = SOUND_MAGIC_2 / freq;
    } else
      sd->sound3Skip = 0;
    ioMem[address] = data;    
    break;
  case NR34:
    data &= 0xc7;
    freq = 2048 - (((data &7) << 8) | (int)ioMem[NR33]);
    if(freq) {
      sd->sound3Skip = SOUND_MAGIC_2 / freq;
    } else {
      sd->sound3Skip = 0;
    }
    sd->sound3Continue = data & 0x40;
    if((data & 0x80) && (ioMem[NR30] & 0x80)) {
      ioMem[NR52] |= 4;
      sd->sound3ATL = 172 * (256 - ioMem[NR31]);
      sd->sound3Index = 0;
      sd->sound3On = 1;
    }
    ioMem[address] = data;    
    break;
  case NR41:
    data &= 0x3f;
    sd->sound4ATL  = 172 * (64 - (data & 0x3f));
    ioMem[address] = data;    
    break;
  case NR42:
    sd->sound4EnvelopeUpDown = data & 0x08;
    sd->sound4EnvelopeATLReload = 689 * (data & 7);
    if((data & 0xF8) == 0)
      sd->sound4EnvelopeVolume = 0;
    ioMem[address] = data;    
    break;
  case NR43:
    freq = soundFreqRatio[data & 7];
    sd->sound4NSteps = data & 0x08;

    sd->sound4Skip = (freq << 8) / NOISE_MAGIC;
    
    sd->sound4Clock = data >> 4;

    freq = freq / soundShiftClock[sd->sound4Clock];

    sd->sound4ShiftSkip = (freq << 8) / NOISE_MAGIC;
    ioMem[address] = data;    
    break;
  case NR44:
    data &= 0xc0;
    sd->sound4Continue = data & 0x40;
    if(data & 0x80) {
      ioMem[NR52] |= 8;
      sd->sound4EnvelopeVolume = ioMem[NR42] >> 4;
      sd->sound4EnvelopeUpDown = ioMem[NR42] & 0x08;
      sd->sound4ATL = 172 * (64 - (ioMem[NR41] & 0x3f));
      sd->sound4EnvelopeATLReload = sd->sound4EnvelopeATL = 689 * (ioMem[NR42] & 7);

      sd->sound4On = 1;
      
      sd->sound4Index = 0;
      sd->sound4ShiftIndex = 0;
      
      freq = soundFreqRatio[ioMem[NR43] & 7];

      sd->sound4Skip = (freq << 8) / NOISE_MAGIC;
      
      sd->sound4NSteps = ioMem[NR43] & 0x08;
      
      freq = freq / soundShiftClock[ioMem[NR43] >> 4];

      sd->sound4ShiftSkip = (freq << 8) / NOISE_MAGIC;
      if(sd->sound4NSteps)
        sd->sound4ShiftRight = 0x7fff;
      else
        sd->sound4ShiftRight = 0x7f;      
    }
    ioMem[address] = data;    
    break;
  case NR50:
    data &= 0x77;
    sd->soundLevel1 = data & 7;
    sd->soundLevel2 = (data >> 4) & 7;
    ioMem[address] = data;    
    break;
  case NR51:
    sd->soundBalance = (data & sd->soundEnableFlag);
    ioMem[address] = data;    
    break;
  case NR52:
    data &= 0x80;
    data |= ioMem[NR52] & 15;
    sd->soundMasterOn = data & 0x80;
    if(!(data & 0x80)) {
      sd->sound1On = 0;
      sd->sound2On = 0;
      sd->sound3On = 0;
      sd->sound4On = 0;
    }
    ioMem[address] = data;    
    break;
  case 0x90:
  case 0x91:
  case 0x92:
  case 0x93:
  case 0x94:
  case 0x95:
  case 0x96:
  case 0x97:
  case 0x98:
  case 0x99:
  case 0x9a:
  case 0x9b:
  case 0x9c:
  case 0x9d:
  case 0x9e:
  case 0x9f:
    sd->sound3WaveRam[(sd->sound3Bank*0x10)^0x10+(address&15)] = data;
    break;
  }
}

void soundEvent_u16(u32 address, u16 data)
{
  switch(address) {
  case SGCNT0_H:
    data &= 0xFF0F;
    sd->soundControl = data & 0x770F;;
    if(data & 0x0800) {
      sd->soundDSFifoAWriteIndex = 0;
      sd->soundDSFifoAIndex = 0;
      sd->soundDSFifoACount = 0;
      sd->soundDSAValue = 0;
      memset(sd->soundDSFifoA, 0, 32);
    }
    sd->soundDSAEnabled = (data & 0x0300) ? true : false;
    sd->soundDSATimer = (data & 0x0400) ? 1 : 0;    
    if(data & 0x8000) {
      sd->soundDSFifoBWriteIndex = 0;
      sd->soundDSFifoBIndex = 0;
      sd->soundDSFifoBCount = 0;
      sd->soundDSBValue = 0;
      memset(sd->soundDSFifoB, 0, 32);
    }
    sd->soundDSBEnabled = (data & 0x3000) ? true : false;
    sd->soundDSBTimer = (data & 0x4000) ? 1 : 0;
    *((u16 *)&ioMem[address]) = data;    
    break;
  case FIFOA_L:
  case FIFOA_H:
    sd->soundDSFifoA[sd->soundDSFifoAWriteIndex++] = data & 0xFF;
    sd->soundDSFifoA[sd->soundDSFifoAWriteIndex++] = data >> 8;
    sd->soundDSFifoACount += 2;
    sd->soundDSFifoAWriteIndex &= 31;
    *((u16 *)&ioMem[address]) = data;    
    break;
  case FIFOB_L:
  case FIFOB_H:
    sd->soundDSFifoB[sd->soundDSFifoBWriteIndex++] = data & 0xFF;
    sd->soundDSFifoB[sd->soundDSFifoBWriteIndex++] = data >> 8;
    sd->soundDSFifoBCount += 2;
    sd->soundDSFifoBWriteIndex &= 31;
    *((u16 *)&ioMem[address]) = data;    
    break;
  case 0x88:
    data &= 0xC3FF;
    *((u16 *)&ioMem[address]) = data;
    break;
  case 0x90:
  case 0x92:
  case 0x94:
  case 0x96:
  case 0x98:
  case 0x9a:
  case 0x9c:
  case 0x9e:
    *((u16 *)&sd->sound3WaveRam[(sd->sound3Bank*0x10)^0x10+(address&14)]) = data;
    *((u16 *)&ioMem[address]) = data;    
    break;    
  }
}

void soundChannel1()
{
  int vol = sd->sound1EnvelopeVolume;

  int freq = 0;
  int value = 0;
  
  if(sd->sound1On && (sd->sound1ATL || !sd->sound1Continue)) {
    sd->sound1Index += sd->soundQuality*sd->sound1Skip;
    sd->sound1Index &= 0x1fffffff;

    value = ((s8)sd->sound1Wave[sd->sound1Index>>24]) * vol;
  }

  sd->soundBuffer[0][sd->soundIndex] = value;

  
  if(sd->sound1On) {
    if(sd->sound1ATL) {
      sd->sound1ATL-=sd->soundQuality;
      
      if(sd->sound1ATL <=0 && sd->sound1Continue) {
        ioMem[NR52] &= 0xfe;
        sd->sound1On = 0;
      }
    }
    
    if(sd->sound1EnvelopeATL) {
      sd->sound1EnvelopeATL-=sd->soundQuality;
      
      if(sd->sound1EnvelopeATL<=0) {
        if(sd->sound1EnvelopeUpDown) {
          if(sd->sound1EnvelopeVolume < 15)
            sd->sound1EnvelopeVolume++;
        } else {
          if(sd->sound1EnvelopeVolume)
            sd->sound1EnvelopeVolume--;
        }
        
        sd->sound1EnvelopeATL += sd->sound1EnvelopeATLReload;
      }
    }
    
    if(sd->sound1SweepATL) {
      sd->sound1SweepATL-=sd->soundQuality;
      
      if(sd->sound1SweepATL<=0) {
        freq = (((int)(ioMem[NR14]&7) << 8) | ioMem[NR13]);
          
        int updown = 1;
        
        if(sd->sound1SweepUpDown)
          updown = -1;
        
        int newfreq = 0;
        if(sd->sound1SweepSteps) {
          newfreq = freq + updown * freq / (1 << sd->sound1SweepSteps);
          if(newfreq == freq)
            newfreq = 0;
        } else
          newfreq = freq;
        
        if(newfreq < 0) {
          sd->sound1SweepATL += sd->sound1SweepATLReload;
        } else if(newfreq > 2047) {
          sd->sound1SweepATL = 0;
          sd->sound1On = 0;
          ioMem[NR52] &= 0xfe;
        } else {
          sd->sound1SweepATL += sd->sound1SweepATLReload;
          sd->sound1Skip = SOUND_MAGIC/(2048 - newfreq);
          
          ioMem[NR13] = newfreq & 0xff;
          ioMem[NR14] = (ioMem[NR14] & 0xf8) |((newfreq >> 8) & 7);
        }
      }
    }
  }
}

void soundChannel2()
{
  //  int freq = 0;
  int vol = sd->sound2EnvelopeVolume;

  int value = 0;
  
  if(sd->sound2On && (sd->sound2ATL || !sd->sound2Continue)) {
    sd->sound2Index += sd->soundQuality*sd->sound2Skip;
    sd->sound2Index &= 0x1fffffff;

    value = ((s8)sd->sound2Wave[sd->sound2Index>>24]) * vol;
  }
  
  sd->soundBuffer[1][sd->soundIndex] = value;
    
  if(sd->sound2On) {
    if(sd->sound2ATL) {
      sd->sound2ATL-=sd->soundQuality;
      
      if(sd->sound2ATL <= 0 && sd->sound2Continue) {
        ioMem[NR52] &= 0xfd;
        sd->sound2On = 0;
      }
    }
    
    if(sd->sound2EnvelopeATL) {
      sd->sound2EnvelopeATL-=sd->soundQuality;
      
      if(sd->sound2EnvelopeATL <= 0) {
        if(sd->sound2EnvelopeUpDown) {
          if(sd->sound2EnvelopeVolume < 15)
            sd->sound2EnvelopeVolume++;
        } else {
          if(sd->sound2EnvelopeVolume)
            sd->sound2EnvelopeVolume--;
        }
        sd->sound2EnvelopeATL += sd->sound2EnvelopeATLReload;
      }
    }
  }
}  

void soundChannel3()
{
  int value = sd->sound3Last;
  
  if(sd->sound3On && (sd->sound3ATL || !sd->sound3Continue)) {
    sd->sound3Index += sd->soundQuality*sd->sound3Skip;
    if(sd->sound3DataSize) {
      sd->sound3Index &= 0x3fffffff;
      value = sd->sound3WaveRam[sd->sound3Index>>25];
    } else {
      sd->sound3Index &= 0x1fffffff;
      value = sd->sound3WaveRam[sd->sound3Bank*0x10 + (sd->sound3Index>>25)];
    }
    
    if( (sd->sound3Index & 0x01000000)) {
      value &= 0x0f;
    } else {
      value >>= 4;
    }

    value -= 8;
    value *= 2;
    
    if(sd->sound3ForcedOutput) {
      value = ((value >> 1) + value) >> 1;
    } else {
      switch(sd->sound3OutputLevel) {
      case 0:
        value = 0;
        break;
      case 1:
        break;
      case 2:
        value = (value >> 1);
        break;
      case 3:
        value = (value >> 2);
        break;
      }
    }
    sd->sound3Last = value;
  }
  
  sd->soundBuffer[2][sd->soundIndex] = value;
  
  if(sd->sound3On) {
    if(sd->sound3ATL) {
      sd->sound3ATL-=sd->soundQuality;
      
      if(sd->sound3ATL <= 0 && sd->sound3Continue) {
        ioMem[NR52] &= 0xfb;
        sd->sound3On = 0;
      }
    }
  }
}

void soundChannel4()
{
  int vol = sd->sound4EnvelopeVolume;

  int value = 0;

  if(sd->sound4Clock <= 0x0c) {
    if(sd->sound4On && (sd->sound4ATL || !sd->sound4Continue)) {
      sd->sound4Index += sd->soundQuality*sd->sound4Skip;
      sd->sound4ShiftIndex += sd->soundQuality*sd->sound4ShiftSkip;

      if(sd->sound4NSteps) {
        while(sd->sound4ShiftIndex > 0x1fffff) {
          sd->sound4ShiftRight = (((sd->sound4ShiftRight << 6) ^
                               (sd->sound4ShiftRight << 5)) & 0x40) |
            (sd->sound4ShiftRight >> 1);
          sd->sound4ShiftIndex -= 0x200000;
        }
      } else {
        while(sd->sound4ShiftIndex > 0x1fffff) {
          sd->sound4ShiftRight = (((sd->sound4ShiftRight << 14) ^
                              (sd->sound4ShiftRight << 13)) & 0x4000) |
            (sd->sound4ShiftRight >> 1);

          sd->sound4ShiftIndex -= 0x200000;   
        }
      }

      sd->sound4Index &= 0x1fffff;    
      sd->sound4ShiftIndex &= 0x1fffff;        
    
      value = ((sd->sound4ShiftRight & 1)*2-1) * vol;
    } else {
      value = 0;
    }
  }
  
  sd->soundBuffer[3][sd->soundIndex] = value;

  if(sd->sound4On) {
    if(sd->sound4ATL) {
      sd->sound4ATL-=sd->soundQuality;
      
      if(sd->sound4ATL <= 0 && sd->sound4Continue) {
        ioMem[NR52] &= 0xfd;
        sd->sound4On = 0;
      }
    }
    
    if(sd->sound4EnvelopeATL) {
      sd->sound4EnvelopeATL-=sd->soundQuality;
      
      if(sd->sound4EnvelopeATL <= 0) {
        if(sd->sound4EnvelopeUpDown) {
          if(sd->sound4EnvelopeVolume < 15)
            sd->sound4EnvelopeVolume++;
        } else {
          if(sd->sound4EnvelopeVolume)
            sd->sound4EnvelopeVolume--;
        }
        sd->sound4EnvelopeATL += sd->sound4EnvelopeATLReload;
      }
    }
  }
}

void soundDirectSoundA()
{
  sd->soundBuffer[4][sd->soundIndex] = sd->soundDSAValue;
}

void soundDirectSoundATimer()
{
  if(sd->soundDSAEnabled) {
    if(sd->soundDSFifoACount <= 16) {
      CPUCheckDMA(3, 2);
      if(sd->soundDSFifoACount <= 16) {
        soundEvent_u16(FIFOA_L, (u16)0);
        soundEvent_u16(FIFOA_H, (u16)0);
        soundEvent_u16(FIFOA_L, (u16)0);
        soundEvent_u16(FIFOA_H, (u16)0);
        soundEvent_u16(FIFOA_L, (u16)0);
        soundEvent_u16(FIFOA_H, (u16)0);
        soundEvent_u16(FIFOA_L, (u16)0);
        soundEvent_u16(FIFOA_H, (u16)0);
      }
    }
    
    sd->soundDSAValue = (sd->soundDSFifoA[sd->soundDSFifoAIndex]);
    sd->soundDSFifoAIndex = (sd->soundDSFifoAIndex + 1) & 31;
    sd->soundDSFifoACount--;
  } else
    sd->soundDSAValue = 0;
}

void soundDirectSoundB()
{
  sd->soundBuffer[5][sd->soundIndex] = sd->soundDSBValue;
}

void soundDirectSoundBTimer()
{
  if(sd->soundDSBEnabled) {
    if(sd->soundDSFifoBCount <= 16) {
      CPUCheckDMA(3, 4);
      if(sd->soundDSFifoBCount <= 16) {
        soundEvent_u16(FIFOB_L, (u16)0);
        soundEvent_u16(FIFOB_H, (u16)0);
        soundEvent_u16(FIFOB_L, (u16)0);
        soundEvent_u16(FIFOB_H, (u16)0);
        soundEvent_u16(FIFOB_L, (u16)0);
        soundEvent_u16(FIFOB_H, (u16)0);
        soundEvent_u16(FIFOB_L, (u16)0);
        soundEvent_u16(FIFOB_H, (u16)0);
      }
    }
    
    sd->soundDSBValue = (sd->soundDSFifoB[sd->soundDSFifoBIndex]);
    sd->soundDSFifoBIndex = (sd->soundDSFifoBIndex + 1) & 31;
    sd->soundDSFifoBCount--;
  } else {
    sd->soundDSBValue = 0;
  }
}

void soundTimerOverflow(int timer)
{
  if(sd->soundDSAEnabled && (sd->soundDSATimer == timer)) {
    soundDirectSoundATimer();
  }
  if(sd->soundDSBEnabled && (sd->soundDSBTimer == timer)) {
    soundDirectSoundBTimer();
  }
}

#ifndef max
#define max(a,b) (a)<(b)?(b):(a)
#endif

void 
soundMix()
{
  int res = 0;
  int cgbRes = 0;
  int ratio = ioMem[0x82] & 3;
  int dsaRatio = ioMem[0x82] & 4;
  int dsbRatio = ioMem[0x82] & 8;
  
  if(sd->soundBalance & 16) {
    cgbRes = ((s8)sd->soundBuffer[0][sd->soundIndex]);
  }
  if(sd->soundBalance & 32) {
    cgbRes += ((s8)sd->soundBuffer[1][sd->soundIndex]);
  }
  if(sd->soundBalance & 64) {
    cgbRes += ((s8)sd->soundBuffer[2][sd->soundIndex]);
  }
  if(sd->soundBalance & 128) {
    cgbRes += ((s8)sd->soundBuffer[3][sd->soundIndex]);
  }

  if((sd->soundControl & 0x0200) && (sd->soundEnableFlag & 0x100)){
    if(!dsaRatio)
      res = ((s8)sd->soundBuffer[4][sd->soundIndex])>>1;
    else
      res = ((s8)sd->soundBuffer[4][sd->soundIndex]);
  }
  
  if((sd->soundControl & 0x2000) && (sd->soundEnableFlag & 0x200)){
    if(!dsbRatio)
      res += ((s8)sd->soundBuffer[5][sd->soundIndex])>>1;
    else
      res += ((s8)sd->soundBuffer[5][sd->soundIndex]);
  }
  
  res = (res * 170);
  cgbRes = (cgbRes * 52 * sd->soundLevel1);

  switch(ratio) {
  case 0:
  case 3: // prohibited, but 25%    
    cgbRes >>= 2;
    break;
  case 1:
    cgbRes >>= 1;
    break;
  case 2:
    break;
  }

  res += cgbRes;

  if(sd->soundEcho) {
    res *= 2;
    res += sd->soundFilter[sd->soundEchoIndex];
    res /= 2;
    sd->soundFilter[sd->soundEchoIndex++] = res;
  }

  if(sd->soundLowPass) {
    sd->soundLeft[4] = sd->soundLeft[3];
    sd->soundLeft[3] = sd->soundLeft[2];
    sd->soundLeft[2] = sd->soundLeft[1];
    sd->soundLeft[1] = sd->soundLeft[0];
    sd->soundLeft[0] = res;
    res = (sd->soundLeft[4] + 2*sd->soundLeft[3] + 8*sd->soundLeft[2] + 2*sd->soundLeft[1] +
           sd->soundLeft[0])/14;
  }

  switch(sd->soundVolume) {
  case 0:
  case 1:
  case 2:
  case 3:
    res *= (sd->soundVolume+1);
    break;
  case 4:
    res >>= 2;
    break;
  case 5:
    res >>= 1;
    break;
  }
  
  if(res > 32767)
    res = 32767;
  if(res < -32768)
    res = -32768;

  if(sd->soundReverse) {
    sd->soundFinalWave[++sd->soundBufferIndex] = res;
  } else {
    sd->soundFinalWave[sd->soundBufferIndex++] = res;
  }
  
  res = 0;
  cgbRes = 0;

  if(sd->soundBalance & 1) {
    cgbRes = ((s8)sd->soundBuffer[0][sd->soundIndex]);
  }
  if(sd->soundBalance & 2) {
    cgbRes += ((s8)sd->soundBuffer[1][sd->soundIndex]);
  }
  if(sd->soundBalance & 4) {
    cgbRes += ((s8)sd->soundBuffer[2][sd->soundIndex]);
  }
  if(sd->soundBalance & 8) {
    cgbRes += ((s8)sd->soundBuffer[3][sd->soundIndex]);
  }

  if((sd->soundControl & 0x0100) && (sd->soundEnableFlag & 0x100)){
    if(!dsaRatio)
      res = ((s8)sd->soundBuffer[4][sd->soundIndex])>>1;
    else
      res = ((s8)sd->soundBuffer[4][sd->soundIndex]);
  }
  
  if((sd->soundControl & 0x1000) && (sd->soundEnableFlag & 0x200)){
    if(!dsbRatio)
      res += ((s8)sd->soundBuffer[5][sd->soundIndex])>>1;
    else
      res += ((s8)sd->soundBuffer[5][sd->soundIndex]);
  }

  res = (res * 170);
  cgbRes = (cgbRes * 52 * sd->soundLevel1);
  
  switch(ratio) {
  case 0:
  case 3: // prohibited, but 25%
    cgbRes >>= 2;
    break;
  case 1:
    cgbRes >>= 1;
    break;
  case 2:
    break;
  }

  res += cgbRes;
  
  if(sd->soundEcho) {
    res *= 2;
    res += sd->soundFilter[sd->soundEchoIndex];
    res /= 2;
    sd->soundFilter[sd->soundEchoIndex++] = res;

    if(sd->soundEchoIndex >= 4000)
      sd->soundEchoIndex = 0;
  }

  if(sd->soundLowPass) {
    sd->soundRight[4] = sd->soundRight[3];
    sd->soundRight[3] = sd->soundRight[2];
    sd->soundRight[2] = sd->soundRight[1];
    sd->soundRight[1] = sd->soundRight[0];
    sd->soundRight[0] = res;
    res = (sd->soundRight[4] + 2*sd->soundRight[3] + 8*sd->soundRight[2] + 2*sd->soundRight[1] +
           sd->soundRight[0])/14;
  }

  switch(sd->soundVolume) {
  case 0:
  case 1:
  case 2:
  case 3:
    res *= (sd->soundVolume+1);
    break;
  case 4:
    res >>= 2;
    break;
  case 5:
    res >>= 1;
    break;
  }
  
  if(res > 32767)
    res = 32767;
  if(res < -32768)
    res = -32768;

  if(sd->soundReverse)  {
    sd->soundFinalWave[-1+sd->soundBufferIndex++] = res;
  } else {
    sd->soundFinalWave[sd->soundBufferIndex++] = res;
  }
}

static int
loc_soundTick(void)
{
  int ret;

  if(sd->soundMasterOn && !stopState) {
    soundChannel1();
    soundChannel2();
    soundChannel3();
    soundChannel4();
    soundDirectSoundA();
    soundDirectSoundB();
    soundMix();
  } else {
    sd->soundFinalWave[sd->soundBufferIndex++] = 0;
    sd->soundFinalWave[sd->soundBufferIndex++] = 0;
  }
  
  sd->soundIndex++;
  
  if(sd->soundBufferIndex >= sd->soundBufferLen) {
    if(sd->soundPaused) {
      soundResume();
    }      
    
    ret = systemWriteDataToSoundBuffer();
    
    sd->soundIndex = 0;
    sd->soundBufferIndex = 0;

    return ret;
  }
  return 0;
}

void 
soundTick()
{
  if (! GBA.gba_snd_enable)  return;
  psp_me_set_sound_tick();
}


long me_timer;

void
me_soundTick(volatile me_sound_t* p)
{
  while (1) {
    if (p->exit) break;
    p->num_loop++;
    if (p->sound_tick) {
      loc_soundTick();
      p->sound_tick = 0;
    } 
  }
  p->exit = 2;
}

void 
soundShutdown()
{
  systemSoundShutdown();
}

void soundPause()
{
  sd->soundPaused = 1;
  systemSoundPause();
}

void soundResume()
{
  systemSoundResume();
  sd->soundPaused = 0;
}

void soundEnable(int channels)
{
  int c = channels & 0x0f;
  
  sd->soundEnableFlag |= ((channels & 0x30f) |c | (c << 4));
  if(ioMem)
    sd->soundBalance = (ioMem[NR51] & sd->soundEnableFlag);
}

void soundDisable(int channels)
{
  int c = channels & 0x0f;
  
  sd->soundEnableFlag &= (~((channels & 0x30f)|c|(c<<4)));
  if(ioMem)
    sd->soundBalance = (ioMem[NR51] & sd->soundEnableFlag);
}

int soundGetEnable()
{
  return (sd->soundEnableFlag & 0x30f);
}

void soundReset()
{
  systemSoundReset();

  sd->soundPaused = 1;
  sd->soundPlay = 0;
  sd->SOUND_CLOCK_TICKS = sd->soundQuality * USE_TICKS_AS;  
  sd->soundTicks = sd->SOUND_CLOCK_TICKS;
  sd->soundNextPosition = 0;
  sd->soundMasterOn = 1;
  sd->soundIndex = 0;
  sd->soundBufferIndex = 0;
  sd->soundLevel1 = 7;
  sd->soundLevel2 = 7;
  sd->sound1On = 0;
  sd->sound1ATL = 0;
  sd->sound1Skip = 0;
  sd->sound1Index = 0;
  sd->sound1Continue = 0;
  sd->sound1EnvelopeVolume =  0;
  sd->sound1EnvelopeATL = 0;
  sd->sound1EnvelopeUpDown = 0;
  sd->sound1EnvelopeATLReload = 0;
  sd->sound1SweepATL = 0;
  sd->sound1SweepATLReload = 0;
  sd->sound1SweepSteps = 0;
  sd->sound1SweepUpDown = 0;
  sd->sound1SweepStep = 0;
  sd->sound1Wave = (u8*)PSP_UNCACHE_PTR(soundWavePattern[2]);
  sd->sound2On = 0;
  sd->sound2ATL = 0;
  sd->sound2Skip = 0;
  sd->sound2Index = 0;
  sd->sound2Continue = 0;
  sd->sound2EnvelopeVolume =  0;
  sd->sound2EnvelopeATL = 0;
  sd->sound2EnvelopeUpDown = 0;
  sd->sound2EnvelopeATLReload = 0;
  sd->sound2Wave = (u8*)PSP_UNCACHE_PTR(soundWavePattern[2]);
  sd->sound3On = 0;
  sd->sound3ATL = 0;
  sd->sound3Skip = 0;
  sd->sound3Index = 0;
  sd->sound3Continue = 0;
  sd->sound3OutputLevel = 0;
  sd->sound3Last = 0;
  sd->sound3Bank = 0;
  sd->sound3DataSize = 0;
  sd->sound3ForcedOutput = 0;
  sd->sound4On = 0;
  sd->sound4Clock = 0;
  sd->sound4ATL = 0;
  sd->sound4Skip = 0;
  sd->sound4Index = 0;
  sd->sound4ShiftRight = 0x7f;
  sd->sound4NSteps = 0;
  sd->sound4CountDown = 0;
  sd->sound4Continue = 0;
  sd->sound4EnvelopeVolume =  0;
  sd->sound4EnvelopeATL = 0;
  sd->sound4EnvelopeUpDown = 0;
  sd->sound4EnvelopeATLReload = 0;
  sd->sound1On = 0;
  sd->sound2On = 0;
  sd->sound3On = 0;
  sd->sound4On = 0;
  
  int addr = 0x90;

  while(addr < 0xA0) {
    ioMem[addr++] = 0x00;
    ioMem[addr++] = 0xff;
  }

  addr = 0;
  while(addr < 0x20) {
    sd->sound3WaveRam[addr++] = 0x00;
    sd->sound3WaveRam[addr++] = 0xff;
  }

  memset(sd->soundFinalWave, 0, sizeof(sd->soundFinalWave));
  memset(sd->soundFilter, 0, sizeof(sd->soundFilter));
  sd->soundEchoIndex = 0;
}

void soundSaveGame(gzFile gzFile)
{
  utilWriteData(gzFile, soundSaveStruct);
  utilWriteData(gzFile, soundSaveStructV2);
  
  utilGzWrite(gzFile, &sd->soundQuality, sizeof(int));
}

void soundReadGame(gzFile gzFile, int version)
{
  utilReadData(gzFile, soundSaveStruct);
  if(version >= SAVE_GAME_VERSION_3) {
    utilReadData(gzFile, soundSaveStructV2);
  } else {
    sd->sound3Bank = (ioMem[NR30] >> 6) & 1;
    sd->sound3DataSize = (ioMem[NR30] >> 5) & 1;
    sd->sound3ForcedOutput = (ioMem[NR32] >> 7) & 1;
    // nothing better to do here...
    memcpy(&sd->sound3WaveRam[0x00], &ioMem[0x90], 0x10);    
    memcpy(&sd->sound3WaveRam[0x10], &ioMem[0x90], 0x10);
  }
  sd->soundBufferIndex = 0;
  sd->soundIndex  = 0;
  
  int quality = 1;
  utilGzRead(gzFile, &quality, sizeof(int));

  sd->sound1Wave = soundWavePattern[(ioMem[NR11] >> 6) & 0x3];
  sd->sound2Wave = soundWavePattern[(ioMem[NR21] >> 6) & 0x3];
}
