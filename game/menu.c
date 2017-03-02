#include "game.h"
// http://www.picturetopeople.org/text_generator/others/3d/3d-perspective-text-effect-creator.html

extern void palette_menuInstall(void);
extern frame_buffer_t menuFrameBuffer;

#define MENU_NUM_ITEMS             6
#define MENU_TOP_COLOR             0x7ef
#define MENU_BOTTOM_COLOR          0x5cd
#define MENU_TOP_COLOR_SELECTED    0xbe0
#define MENU_BOTTOM_COLOR_SELECTED 0x9d4

typedef struct {
  uint16_t wait1[2];
  uint16_t color1[2];
  uint16_t wait2[2];
  uint16_t color2[2];
} menu_line_copper_t;

typedef struct {
  uint16_t bpl1[SCREEN_BIT_DEPTH*2*2];
  menu_line_copper_t lines[MENU_NUM_ITEMS];
  uint16_t end[2];
} menu_copper_t;

uint16_t menu_selected = 0;

#define MENU_COPPER_WAIT_TOP(x)     { 0x9fd1 + (0x800*(x*2)), 0xfffe}
#define MENU_COPPER_WAIT_BOTTOM(x)  { 0x9fd1 + 0x400 + (0x800*(x*2)), 0xfffe}
#define MENU_COPPER_LINE(c1, c2, x) [x] = {	\
    .wait1 = MENU_COPPER_WAIT_TOP(x),\
   .color1 = { COLOR07, c1},\
   .wait2 =  MENU_COPPER_WAIT_BOTTOM(x),\
   .color2 = { COLOR07, c2},\
   }

static uint16_t menuFadeIn[PALETTE_FADE_IN_SIZE] = {
#include "out/menu_fade_in.h"
};

static  __section(data_c)  menu_copper_t menu_copper  = {
  .bpl1 = {
    BPL1PTL,0x0000,
    BPL1PTH,0x0000,
    BPL2PTL,0x0000,
    BPL2PTH,0x0000,
    BPL3PTL,0x0000,
    BPL3PTH,0x0000,
    BPL4PTL,0x0000,
    BPL4PTH,0x0000,
    BPL5PTL,0x0000,
    BPL5PTH,0x0000,
  },
  .lines = {
    MENU_COPPER_LINE(MENU_TOP_COLOR_SELECTED, MENU_BOTTOM_COLOR_SELECTED, 0),
    MENU_COPPER_LINE(MENU_TOP_COLOR, MENU_BOTTOM_COLOR, 1),
    MENU_COPPER_LINE(MENU_TOP_COLOR, MENU_BOTTOM_COLOR, 2),
    MENU_COPPER_LINE(MENU_TOP_COLOR, MENU_BOTTOM_COLOR, 3),
    MENU_COPPER_LINE(MENU_TOP_COLOR, MENU_BOTTOM_COLOR, 4),
    MENU_COPPER_LINE(MENU_TOP_COLOR, MENU_BOTTOM_COLOR, 5),
  },
  .end = {0xFFFF, 0xFFFE}
};

typedef struct {
  char* text;
  menu_command_t command;
  int16_t done;
  void (*callback)(void);
} menu_item_t;

static void menu_music_toggle(void);

menu_item_t menu_items[MENU_NUM_ITEMS+1] = {
  {
    .text = "PLAY NOW!",
    .command = MENU_COMMAND_PLAY,
    .done = 1,
    .callback = 0
  },
  {
    .text = "RECORD GAME",
    .command = MENU_COMMAND_RECORD,
    .done = 1,
    .callback = 0
  },
  {
    .text = "PLAY RECORDING",
    .command = MENU_COMMAND_REPLAY,
    .done = 1,
    .callback = 0
  },
  {
    .text = "MUSIC - ON ",
    .command = MENU_COMMAND_PLAY,
    .done = 0,
    .callback = menu_music_toggle
  },
  {
    .text = "HI SCORES",
    .command = MENU_COMMAND_PLAY,
    .done = 0,
    .callback = 0
  },
  {
    .text = "CREDITS",
    .command = MENU_COMMAND_PLAY,
    .done = 0,
    .callback = 0
  }
};


static void
menu_vbl(void)
{
  hw_waitVerticalBlank();
  sound_vbl();
}


static void 
menu_pokeCopperList(frame_buffer_t frameBuffer)
{
  uint16_t volatile* copperPtr = (uint16_t*)&menu_copper;
  /* poke bitplane pointers into copper list */
  uint32_t bitplanesPtr = (uint32_t)frameBuffer;

  for (int16_t i = 0; i < SCREEN_BIT_DEPTH; i++) {
    copperPtr[1] = (uint16_t)bitplanesPtr;
    copperPtr[3] = (uint16_t)(((uint32_t)bitplanesPtr)>>16);
    bitplanesPtr = bitplanesPtr + (SCREEN_WIDTH_BYTES);
    copperPtr = copperPtr + 4;
  }
}


static int16_t
menu_processKeyboard(void)
{
  uint32_t code = keyboard_getKey();
  
  switch (code) {
  case 'P':
    return MENU_COMMAND_REPLAY;
    break;
  case 'R':
    return MENU_COMMAND_RECORD;
    break;
  case '1':
    return MENU_COMMAND_LEVEL;
    break;
  case '2':
    return MENU_COMMAND_LEVEL+1;
    break;
  case '3':
    return MENU_COMMAND_LEVEL+2;
    break;
#if TRACKLOADER==0
  case 'Q':
    return MENU_COMMAND_EXIT;
    break;
#endif
  default:
    return -1;
  }
}


static void
menu_redraw(uint16_t i)
{
  frame_buffer_t fb = game_onScreenBuffer;
  fb += 2*SCREEN_WIDTH_BYTES;

  int16_t y = 130 + (menu_selected*16);
  uint16_t len = strlen(menu_items[i].text);

  menu_vbl();

  gfx_fillRectSmallScreen(game_onScreenBuffer, (SCREEN_WIDTH/2)-(len<<2), y, (len<<3), 9, 1);

  text_drawMaskedText8Blitter(fb, menu_items[i].text, (SCREEN_WIDTH/2)-(len<<2)+1, y+1);
  text_drawMaskedText8Blitter(fb, menu_items[i].text, (SCREEN_WIDTH/2)-(len<<2), y);

  fb -= SCREEN_WIDTH_BYTES;
  text_drawMaskedText8Blitter(fb, menu_items[i].text, (SCREEN_WIDTH/2)-(strlen(menu_items[i].text)<<2), y);
}


static void
menu_update_music_menu(void)
{
  for (uint16_t i = 0; i < MENU_NUM_ITEMS; i++) {
    if (menu_items[i].callback == menu_music_toggle) {
      if (music_enabled()) {
	menu_items[i].text = "MUSIC - ON ";
      } else {
	menu_items[i].text = "MUSIC - OFF";
      }
      return;
    }
  }
}


static void
menu_render(void)
{
  menu_update_music_menu();

  frame_buffer_t fb = game_onScreenBuffer;
  fb += 2*SCREEN_WIDTH_BYTES;
  uint16_t y = 130;

  for (uint16_t i = 0; i < MENU_NUM_ITEMS; i++) {
    uint16_t len = strlen(menu_items[i].text);
    text_drawMaskedText8Blitter(fb, menu_items[i].text, (SCREEN_WIDTH/2)-(len<<2)+1, y+1);
    text_drawMaskedText8Blitter(fb, menu_items[i].text, (SCREEN_WIDTH/2)-(len<<2), y);
    fb -= SCREEN_WIDTH_BYTES;
    text_drawMaskedText8Blitter(fb, menu_items[i].text, (SCREEN_WIDTH/2)-(strlen(menu_items[i].text)<<2), y);
    fb += SCREEN_WIDTH_BYTES;
    y+= 16;
  }
}

static void
menu_music_toggle(void)
{
  music_toggle_music();
  menu_update_music_menu();
  menu_redraw(menu_selected);
}


static void
menu_up(void)
{
  if (menu_selected > 0) {
    sound_queueSound(SOUND_MENU);
    menu_vbl();
    menu_copper.lines[menu_selected].color1[1] = MENU_TOP_COLOR;
    menu_copper.lines[menu_selected].color2[1] = MENU_BOTTOM_COLOR;
    menu_selected--;
    menu_copper.lines[menu_selected].color1[1] = MENU_TOP_COLOR_SELECTED;
    menu_copper.lines[menu_selected].color2[1] = MENU_BOTTOM_COLOR_SELECTED;    
    do {
      hw_readJoystick();
    } while (JOYSTICK_UP());
  }
}

static void
menu_down(void)
{
  if (menu_selected < MENU_NUM_ITEMS-1) {
    sound_queueSound(SOUND_MENU);    
    menu_vbl();
    menu_copper.lines[menu_selected].color1[1] = MENU_TOP_COLOR;
    menu_copper.lines[menu_selected].color2[1] = MENU_BOTTOM_COLOR;
    menu_selected++;
    menu_copper.lines[menu_selected].color1[1] = MENU_TOP_COLOR_SELECTED;
    menu_copper.lines[menu_selected].color2[1] = MENU_BOTTOM_COLOR_SELECTED;    

    do {
      hw_readJoystick();
    } while (JOYSTICK_DOWN());
  }
}


__EXTERNAL menu_command_t
menu_loop(void)
{
  menu_command_t command;
  uint16_t done;
  volatile uint16_t scratch;

  menu_vbl();
  custom->dmacon = DMAF_RASTER|DMAF_SPRITE;

  palette_black();

  disk_loadData((void*)game_onScreenBuffer, (void*)menuFrameBuffer, SCREEN_WIDTH_BYTES*SCREEN_HEIGHT*SCREEN_BIT_DEPTH);

  uint16_t volatile* copperPtr = (uint16_t*)&menu_copper;

  custom->dmacon = (DMAF_BLITTER|DMAF_SETCLR);

  /* set up playfield */
  
  custom->diwstrt = (RASTER_Y_START<<8)|RASTER_X_START;
  custom->diwstop = ((MENU_RASTER_Y_STOP-256)<<8)|(RASTER_X_STOP-256);

  custom->ddfstrt = (RASTER_X_START/2-SCREEN_RES);
  custom->ddfstop = (RASTER_X_START/2-SCREEN_RES)+(8*((SCREEN_WIDTH/16)-1));
  custom->bplcon0 = (SCREEN_BIT_DEPTH<<12)|0x200;
  custom->bpl1mod = (SCREEN_WIDTH_BYTES*SCREEN_BIT_DEPTH)-SCREEN_WIDTH_BYTES;
  custom->bpl2mod = (SCREEN_WIDTH_BYTES*SCREEN_BIT_DEPTH)-SCREEN_WIDTH_BYTES;

  /* install copper list, then enable dma and selected interrupts */
  custom->cop1lc = (uint32_t)copperPtr;
  scratch = custom->copjmp1;

  USE(scratch);

  menu_pokeCopperList(game_onScreenBuffer);

  menu_vbl();

  custom->dmacon = (DMAF_BLITTER|DMAF_SETCLR|DMAF_COPPER|DMAF_RASTER|DMAF_MASTER);

#ifdef DEBUG_SCROLL
  if (0) {
#endif
    palette_fadeIn(menuFadeIn);
#ifdef DEBUG_SCROLL
  }
#endif
  
  custom->color[5] = 0x08d;  

  for (uint16_t i = 0; i != MENU_NUM_ITEMS; i++) {
    menu_copper.lines[i].color1[1] = 0x08d;
    menu_copper.lines[i].color2[1] = 0x08d;
  }

  menu_render();

  menu_vbl();
  custom->color[5] = 0x544;
  
  for (uint16_t i = 0; i != MENU_NUM_ITEMS; i++) {
    if (i == menu_selected) {
      menu_copper.lines[i].color1[1] = MENU_TOP_COLOR_SELECTED;
      menu_copper.lines[i].color2[1] = MENU_BOTTOM_COLOR_SELECTED;
    } else {
      menu_copper.lines[i].color1[1] = MENU_TOP_COLOR;
      menu_copper.lines[i].color2[1] = MENU_BOTTOM_COLOR;
    }
  }

  command = MENU_COMMAND_PLAY;
#ifdef DEBUG_SCROLL
  done = 1;
#else
  done = 0;
#endif

  
  while (!done) {
    hw_readJoystick();
    if (JOYSTICK_BUTTON_DOWN) {
      command = menu_items[menu_selected].command;
      done = menu_items[menu_selected].done;
      if (menu_items[menu_selected].callback != 0) {
	menu_items[menu_selected].callback();
      }
      do {
	hw_readJoystick();
      } while (JOYSTICK_BUTTON_DOWN);
    }
    if (JOYSTICK_DOWN()) {
      menu_down();
    } else if (JOYSTICK_UP()) {
      menu_up();
    }
    int16_t kbCommand;
    if ((kbCommand = menu_processKeyboard()) != -1) {
      command = kbCommand;
      done = 1;
    }
    menu_vbl();
  }

  menu_vbl();
  custom->dmacon = DMAF_RASTER;
  palette_black();

  return command;
}
