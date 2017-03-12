#include "game.h" 

#if TRACKLOADER==0

__EXTERNAL void
hiscore_save(void)
{ 
  //extern BPTR startupDirLock;

  dos_init();

  BPTR file = Open("PROGDIR:hello2", MODE_NEWFILE);
  if (file) {
    Write(file, (void*)"Hello World!\n", strlen("Hello World!\n"));
    Close(file);
  }
}

#else

void
hiscore_save(void)
{

}

#endif
