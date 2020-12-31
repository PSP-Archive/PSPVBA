// -*- C++ -*-
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

#ifndef VBA_SOUND_H
#define VBA_SOUND_H

#define NR10 0x60
#define NR11 0x62
#define NR12 0x63
#define NR13 0x64
#define NR14 0x65
#define NR21 0x68
#define NR22 0x69
#define NR23 0x6c
#define NR24 0x6d
#define NR30 0x70
#define NR31 0x72
#define NR32 0x73
#define NR33 0x74
#define NR34 0x75
#define NR41 0x78
#define NR42 0x79
#define NR43 0x7c
#define NR44 0x7d
#define NR50 0x80
#define NR51 0x81
#define NR52 0x84
#define SGCNT0_H 0x82
#define FIFOA_L 0xa0
#define FIFOA_H 0xa2
#define FIFOB_L 0xa4
#define FIFOB_H 0xa6

  typedef struct gba_sound_t {
  
    int  soundVolume;
    u8   soundBuffer[6][1024];
    u16  soundFinalWave[2048];
    int  soundBufferLen;
    int  soundQuality;
    int  soundPaused;
    int  soundPlay;
    int  soundTicks;
    int  SOUND_CLOCK_TICKS;
    u32  soundNextPosition;
    int  soundLevel1;
    int  soundLevel2;
    int  soundBalance;
    int  soundMasterOn;
    int  soundIndex;
    int  soundBufferIndex;
    int  soundDebug;
    int  sound1On;
    int  sound1ATL;
    int  sound1Skip;
    int  sound1Index;
    int  sound1Continue;
    int  sound1EnvelopeVolume;
    int  sound1EnvelopeATL;
    int  sound1EnvelopeUpDown;
    int  sound1EnvelopeATLReload;
    int  sound1SweepATL;
    int  sound1SweepATLReload;
    int  sound1SweepSteps;
    int  sound1SweepUpDown;
    int  sound1SweepStep;
    u8  *sound1Wave;
    int  sound2On;
    int  sound2ATL;
    int  sound2Skip;
    int  sound2Index;
    int  sound2Continue;
    int  sound2EnvelopeVolume;
    int  sound2EnvelopeATL;
    int  sound2EnvelopeUpDown;
    int  sound2EnvelopeATLReload;
    u8  *sound2Wave;
    int  sound3On;
    int  sound3ATL;
    int  sound3Skip;
    int  sound3Index;
    int  sound3Continue;
    int  sound3OutputLevel;
    int  sound3Last;
    u8   sound3WaveRam[0x20];
    int  sound3Bank;
    int  sound3DataSize;
    int  sound3ForcedOutput;
    int  sound4On;
    int  sound4Clock;
    int  sound4ATL;
    int  sound4Skip;
    int  sound4Index;
    int  sound4ShiftRight;
    int  sound4ShiftSkip;
    int  sound4ShiftIndex;
    int  sound4NSteps;
    int  sound4CountDown;
    int  sound4Continue;
    int  sound4EnvelopeVolume;
    int  sound4EnvelopeATL;
    int  sound4EnvelopeUpDown;
    int  sound4EnvelopeATLReload;
    int  soundControl;
    int  soundDSFifoAIndex;
    int  soundDSFifoACount;
    int  soundDSFifoAWriteIndex;
    bool soundDSAEnabled;
    int  soundDSATimer;
    u8   soundDSFifoA[32];
    u8   soundDSAValue;
    int  soundDSFifoBIndex;
    int  soundDSFifoBCount;
    int  soundDSFifoBWriteIndex;
    bool soundDSBEnabled;
    int  soundDSBTimer;
    u8   soundDSFifoB[32];
    u8   soundDSBValue;
    int  soundEnableFlag;
    s16  soundFilter[4000];
    s16  soundRight[5];
    s16  soundLeft[5];
    int  soundEchoIndex;
    bool soundEcho;
    bool soundLowPass;
    bool soundReverse;
  
  } gba_sound_t;

  extern volatile gba_sound_t *sd;

extern void soundTick();
extern void soundShutdown();
extern bool soundInit();
extern void soundPause();
extern void soundResume();
extern void soundEnable(int);
extern void soundDisable(int);
extern int  soundGetEnable();
extern void soundReset();
extern void soundSaveGame(gzFile);
extern void soundReadGame(gzFile, int);
extern void soundEvent_u8(u32, u8);
extern void soundEvent_u16(u32, u16);
extern void soundTimerOverflow(int);

#endif // VBA_SOUND_H
