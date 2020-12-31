#include <stdio.h>
#include <zlib.h>
#include "SDL.h"

#include <pspkernel.h>
#include <pspdebug.h>
#include <psppower.h>
#include <pspsdk.h>
#include <pspctrl.h>
#include <pspthreadman.h>
#include <stdlib.h>
#include <stdio.h>

# ifdef PSP_ME
#include "psp_me.h"
# endif
#include "psp_sdl.h"

#define STDOUT_FILE  "stdout.txt"
#define STDERR_FILE  "stderr.txt"

/* If application's main() is redefined as SDL_main, and libSDLmain is
   linked, then this file will create the standard exit callback,
   define the PSP_* macros, add an exception handler, nullify device 
   checks and exit back to the browser when the program is finished. */

extern int SDL_main(int argc, char *argv[]);

static void cleanup_output(void);

PSP_MODULE_INFO("PSPVBA", 0x1000, 1, 1);
# ifdef PSP_ME
PSP_MAIN_THREAD_ATTR(THREAD_ATTR_VFPU);
PSP_MAIN_THREAD_STACK_SIZE_KB(256);
# else
PSP_MAIN_THREAD_ATTR(THREAD_ATTR_USER|THREAD_ATTR_VFPU);
# endif

int 
sdl_psp_exit_callback(int arg1, int arg2, void *common)
{
  psp_sdl_exit(0);
  return 0;
}

int
sdl_psp_power_callback(int arg1, int arg2, void *arg3)
{
  return 0;
}

int 
sdl_psp_callback_thread(SceSize args, void *argp)
{
  int cbid;
  cbid = sceKernelCreateCallback("Exit Callback", sdl_psp_exit_callback, NULL);
  sceKernelRegisterExitCallback(cbid);
  cbid = sceKernelCreateCallback("Power Callback", sdl_psp_power_callback, NULL);
  scePowerRegisterCallback(0,cbid);

  sceKernelSleepThreadCB();
  return 0;
}

int 
sdl_psp_setup_callbacks(void)
{
  int thid = 0;

  sceCtrlSetSamplingCycle(0);
  sceCtrlSetSamplingMode(PSP_CTRL_MODE_ANALOG);

  thid = sceKernelCreateThread("update_thread", sdl_psp_callback_thread, 0x11, 0xFA0, 0, 0);
  if(thid >= 0) sceKernelStartThread(thid, 0, 0);
  return thid;
}


/* Remove the output files if there was no output written */
static void cleanup_output(void)
{
#ifndef NO_STDIO_REDIRECT
  FILE *file;
  int empty;
#endif

  /* Flush the output in case anything is queued */
  fclose(stdout);
  fclose(stderr);

#ifndef NO_STDIO_REDIRECT
  /* See if the files have any output in them */
  file = fopen(STDOUT_FILE, "rb");
  if ( file ) {
    empty = (fgetc(file) == EOF) ? 1 : 0;
    fclose(file);
    if ( empty ) {
      remove(STDOUT_FILE);
    }
  }
  file = fopen(STDERR_FILE, "rb");
  if ( file ) {
    empty = (fgetc(file) == EOF) ? 1 : 0;
    fclose(file);
    if ( empty ) {
      remove(STDERR_FILE);
    }
  }
#endif
}

void _fini(void);

#ifdef main
#undef main
#endif

int 
main(int argc, char *argv[])
{
  pspDebugScreenInit();
# ifdef PSP_ME
  pspDebugScreenPrintf("PSPVBA v1.2.4-me: VBA ported by Zx-81\n");
# else
  pspDebugScreenPrintf("PSPVBA v1.2.4: VBA ported by Zx-81\n");
# endif
  pspDebugScreenPrintf("Please wait now loading !\n");

  sceKernelDelayThread(2500000); 

  sdl_psp_setup_callbacks();

#ifndef NO_STDIO_REDIRECT
  /* Redirect standard output and standard error. */
  /* TODO: Error checking. */
  freopen(STDOUT_FILE, "w", stdout);
  freopen(STDERR_FILE, "w", stderr);
  setvbuf(stdout, NULL, _IOLBF, BUFSIZ);  /* Line buffered */
  setbuf(stderr, NULL);          /* No buffering */
#endif /* NO_STDIO_REDIRECT */

  /* Functions registered with atexit() are called in reverse order, so make
     sure that we register sceKernelExitGame() first, so that it's called last. */
  atexit(sceKernelExitGame);
  /* Make sure that _fini() is called before returning to the OS. */
  atexit(_fini);
  atexit(cleanup_output);

# ifdef PSP_ME
  pspDebugScreenPrintf("Starting media engine ...\n");
  psp_me_init();
# endif
  pspDebugScreenPrintf("Starting main thread ...\n");
  (void)SDL_main(argc, argv);

  /* Delay 2.5 seconds before returning to the OS. */
  sceKernelDelayThread(2500000);

  return 0;
}
