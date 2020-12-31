
    Welcome to PSPVBA - 1.2.4

Original Authors of VisualBoyAdvance

  VBA Development Team  (http://vba.ngemu.com/contact.shtml)

Authors of the PSP port versions

  psp298 made the initial port of VBA on PSP

  Ludovic.Jacomme also known as Zx-81 (zx81.zx81@gmail.com)

  *** Special Thanks ***

  -> to Win-Win for his beautiful icons and background !
  -> to YoyoFR to provide the source code of his Snes emulator 
     (usefull to understand Media Engine :) )

1. INTRODUCTION
   ------------

  VisualBoyAdvance is an emulator for GameBoy Advance console.
  See http://vba.ngemu.com/ for further informations.

  PSPVBA is a port on PSP of the VisualBoyAdvance v1.7.2.

  This software is under GPL Copyright, read COPYING file for more 
  information about it.

2. INSTALLATION
   ------------

  Unzip the zip file, and copy the content of the directory fw2.x or fw1.5
  (depending of the version of your firmware) on the psp/game directory.

  Put your rom image files on "roms" sub-directory.

  It has been developped on linux for Firmware 1.5, and 2.1+ using DevHook 
  and the great GTA eloader (0.97) from http://www.fanjita.org/.

  For any comments or questions on this version, please visit 
  http://zx81.zx81.free.fr or http://zx81.dcemu.co.uk

3. CONTROL
   ------------

  In the GBA emulator window

    PSP        GBA 
  
    Square     SELECT
    Triangle   START
    Cross      A
    Circle     B
    Up         Up
    Down       Down
    Left       Left
    Right      Right
    LTrigger   L
    RTrigger   R
    Start      START
    Select     MENU

    A-Pad      Disable (but you can toggle with D-pad)
  
    Press Start  + L + R   to exit and return to eloader.
    Press Select           to enter in emulator main menu.

  In the main menu

    L+R+Start  Exit the emulator
    R Trigger  Reset the GBA console

    Triangle   Go Up directory
    Cross      Valid
    Circle     Valid
    Square     Go Back to the emulator window

4. LOADING GBA ROM FILES
   ------------

  If you want to load rom image in your emulator, you have to put your rom file
  (with .zip, .rom, .gba, or .bin file extension) on your PSP memory stick in 
  the 'roms' directory.

  Then, while inside PSPVBA emulator, just press SELECT to enter in 
  the emulator main menu, and then using the file selector choose one 
  rom file to load in your emulator.

  Back to the emulator window, the rom should stard automatically.

5. LOADING KEY MAPPING FILES
   ------------

  For given games, the default keyboard mapping between PSP Keys and
  GBA keys, is not suitable, and the game can't be played on PSPVBA.

  To overcome the issue, you can write your own mapping file. Using notepad for
  example you can edit a file with the .kbd extension and put it in the kbd 
  directory.

  For the exact syntax of those mapping files, have a look on sample files
  already presents in the kbd directory (default.kbd etc ...).

  After writting such keyboard mapping file, you can load them using the main
  menu inside the emulator.

  If the keyboard filename is the same as the rom filename (.zip etc ...)
  then when you load this rom, the corresponding keyboard file is automatically 
  loaded !

  You can now use the Keyboard menu and edit, load and save your
  keyboard mapping files inside the emulator. The Save option save the .kbd
  file in the kbd directory using the "Game Name" as filename. The game name
  is displayed on the right corner in the emulator menu.

6. GBA BIOS File support
   ---------------------

  If you want to use a GBA bios dump file, then put the file GBA.BIOS in the PSPVBA
  directory (PSP/GAME/__SCE__PSPVBA if you are in 1.5FW, and PSP/GAME/PSPVBA
  in 2.1+FW). 

  Then enter in the emulator and select the settings menu. You can enable the
  bios file support, and the new settings will be saved. You will need to
  restart the emulator, and it will load the bios file on startup.

  (WARNING) Save slot/states done with a the bios file option set to on (at
  save time), can't be read when the bios file feature is reset to off ... 
  (same when the bios file option is set to off etc ...)


7. COMPILATION
   ------------

  It has been developped under Linux using gcc with PSPSDK. 
  To rebuild the homebrew run the Makefile in the src archive.
