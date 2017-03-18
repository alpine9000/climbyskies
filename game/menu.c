#include "game.h"
// http://www.picturetopeople.org/text_generator/others/3d/3d-perspective-text-effect-creator.html

extern void palette_menuInstall(void);
extern frame_buffer_t menu_frameBuffer;

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
  uint16_t wrap[2];
  menu_line_copper_t overflowLine;
  uint16_t end[2];
} menu_copper_t;

uint16_t menu_selected = 0;

#define MENU_TEXT_START (0x9fd1+((RASTER_Y_START-0x1d)*0x100))
#define MENU_COPPER_WAIT_TOP(x)     { MENU_TEXT_START + (0x800*(x*2)), 0xfffe}
#define MENU_COPPER_WAIT_BOTTOM(x)  { MENU_TEXT_START + 0x400 + (0x800*(x*2)), 0xfffe}
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
  .wrap = {0xffdf,0xfffe},
  .overflowLine = {
    .wait1 =  {0x0001, 0xfffe},
    .color1 = { COLOR07, MENU_TOP_COLOR},
    .wait2 =  {0x03df, 0xfffe},
   .color2 = { COLOR07, MENU_BOTTOM_COLOR}, 
  },
  .end = {0xFFFF, 0xFFFE}
};

typedef struct {
  char* text;
  menu_command_t command;
  int16_t done;
  void (*callback)(void);
} menu_item_t;


static void
menu_render(void);
static void
menu_toggleMusic(void);
static void
menu_showHiScores(void);
static void
menu_select(uint16_t i);

static menu_mode_t menu_mode;
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
    .callback = menu_toggleMusic
  },
  {
    .text = "HI SCORES",
    .command = MENU_COMMAND_PLAY,
    .done = 0,
    .callback = menu_showHiScores
  },
  {
    .text = "CREDITS",
    .command = MENU_COMMAND_PLAY,
    .done = 0,
    .callback = 0
  },
  {
    .text = "",
    .command = MENU_COMMAND_PLAY,
    .done = 0,
    .callback = 0
  }
};


hiscore_t* menu_hiscores;  

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

#if TRACKLOADER==1
#ifdef DEBUG
static inline void
debug_hiscoreStress(void)
{
  for (int16_t i = 0; i < 100; i++) {
    hiscore_saveData(1);
    hw_waitVerticalBlank();
    message_loading("Loading hiscore...");
    if (hiscore_load(1)) {
      message_loading("Loading failed...");
      hw_waitScanLines(200);
    }
    hw_waitVerticalBlank();
  }
  
  message_screenOff();
}
#endif
#endif

static int16_t
menu_processKeyboard(void)
{
  uint32_t code = keyboard_getKey();
  
  switch (code) {
#if TRACKLOADER==1
#ifdef DEBUG
  case 'Y':
    debug_hiscoreStress();
    return MENU_COMMAND_LEVEL;
    break;
#endif
#endif
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
menu_renderText(frame_buffer_t fb, char* text, uint16_t y)
{
  uint16_t len = strlen(text);
  text_drawMaskedText8Blitter(fb, text, (SCREEN_WIDTH/2)-(len<<2)+1, y+1);
  text_drawMaskedText8Blitter(fb, text, (SCREEN_WIDTH/2)-(len<<2), y);
  fb -= SCREEN_WIDTH_BYTES;
  text_drawMaskedText8Blitter(fb, text, (SCREEN_WIDTH/2)-(len<<2), y);
  fb += SCREEN_WIDTH_BYTES;
}



static void
menu_redraw(uint16_t i)
{
  frame_buffer_t fb = game_onScreenBuffer;
  fb += 2*SCREEN_WIDTH_BYTES;

  int16_t y = 130 + (i*16);

  int16_t len = max(strlen(menu_items[i].text), strlen(menu_hiscores[i-1].text));

  hw_waitVerticalBlank();
  gfx_fillRectSmallScreen(game_onScreenBuffer, (SCREEN_WIDTH/2)-(len<<2), y, (len<<3)+2, 9, 1);

  if (menu_mode == MENU_MODE_MENU) {        
    menu_renderText(fb, menu_items[i].text, y);
  } else {
    if (i == 0) {
      menu_renderText(fb, "HI SCORES", y);
    } else {
      menu_renderText(fb, menu_hiscores[i-1].text, y);
    }
  }
}

static void menu_refresh(void)
{
  for (uint16_t i = 0; i < MENU_NUM_ITEMS+1; i++) {
    menu_redraw(i);
  }    
}

static void
menu_showHiScores(void)
{
  menu_mode = MENU_MODE_HISCORES;
  menu_select(0);
  menu_refresh();
}

static void
menu_update_music_menu(void)
{
  for (uint16_t i = 0; i < MENU_NUM_ITEMS; i++) {
    if (menu_items[i].callback == menu_toggleMusic) {
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

  switch (menu_mode) {
  case MENU_MODE_MENU:
    for (uint16_t i = 0; i < MENU_NUM_ITEMS; i++) {
      menu_renderText(fb, menu_items[i].text, y);
      y+= 16;
    }
    break;
  case MENU_MODE_HISCORES:
    menu_renderText(fb, "HI SCORES", y);
    y+= 16;
    for (uint16_t i = 0; i < HISCORE_NUM_SCORES; i++) {
      menu_renderText(fb, menu_hiscores[i].text, y);
      y+= 16;
    }
    break;
  }
}


static void
menu_toggleMusic(void)
{
  music_toggle();
  menu_update_music_menu();
  menu_redraw(menu_selected);
}


static void
menu_select(uint16_t i)
{
  hw_waitVerticalBlank();
  menu_copper.lines[menu_selected].color1[1] = MENU_TOP_COLOR;
  menu_copper.lines[menu_selected].color2[1] = MENU_BOTTOM_COLOR;
  menu_selected = i;
  menu_copper.lines[menu_selected].color1[1] = MENU_TOP_COLOR_SELECTED;
  menu_copper.lines[menu_selected].color2[1] = MENU_BOTTOM_COLOR_SELECTED;    
}

static void
menu_up(void)
{
  if (menu_mode == MENU_MODE_MENU && menu_selected > 0) {
    sound_playSound(SOUND_MENU);
    hw_waitVerticalBlank();
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
  if (menu_mode == MENU_MODE_MENU && menu_selected < MENU_NUM_ITEMS-1) {
    sound_playSound(SOUND_MENU);    
    hw_waitVerticalBlank();
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
menu_loop(menu_mode_t mode)
{
  menu_command_t command;
  uint16_t done;
  volatile uint16_t scratch;

  menu_mode = mode;

  menu_hiscores = hiscore_render();

  sound_init();

  message_loading("Loading...");

  disk_loadData((void*)game_onScreenBuffer, (void*)menu_frameBuffer, SCREEN_WIDTH_BYTES*SCREEN_HEIGHT*SCREEN_BIT_DEPTH);
  message_screenOff();

  hw_waitVerticalBlank();
  custom->dmacon = DMAF_RASTER|DMAF_SPRITE;

  palette_black();

  uint16_t volatile* copperPtr = (uint16_t*)&menu_copper;

  custom->dmacon = (DMAF_BLITTER|DMAF_SETCLR|DMAF_MASTER);

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

  hw_waitVerticalBlank();

  custom->dmacon = (DMAF_BLITTER|DMAF_SETCLR|DMAF_COPPER|DMAF_RASTER|DMAF_MASTER);

  palette_fadeIn(menuFadeIn);
  
  custom->color[5] = 0x08d;  

  for (uint16_t i = 0; i != MENU_NUM_ITEMS; i++) {
    menu_copper.lines[i].color1[1] = 0x08d;
    menu_copper.lines[i].color2[1] = 0x08d;
  }

  menu_render();

  hw_waitVerticalBlank();
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
  done = 0;
  
  while (!done) {
    hw_readJoystick();
    if (JOYSTICK_BUTTON_DOWN) {
      if (menu_mode == MENU_MODE_HISCORES) {
	menu_mode = MENU_MODE_MENU;
	menu_refresh();
      } else {
	command = menu_items[menu_selected].command;
	done = menu_items[menu_selected].done;
	if (menu_items[menu_selected].callback != 0) {
	  menu_items[menu_selected].callback();
	}
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
    hw_waitVerticalBlank();
  }

  hw_waitVerticalBlank();
  custom->dmacon = DMAF_RASTER;
  palette_black();

  return command;
}
