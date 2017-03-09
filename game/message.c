#include "game.h"

#define MESSAGE_BOX_WIDTH 160
#define MESSAGE_BOX_HEIGHT 16
#define MESSAGE_BOX_SAVE_WIDTH (MESSAGE_BOX_WIDTH+16)

extern frame_buffer_t message_frameBuffer;
static uint16_t message_on = 0;

typedef struct {
  uint8_t fb[((MESSAGE_BOX_SAVE_WIDTH)/8)*SCREEN_BIT_DEPTH*MESSAGE_BOX_HEIGHT];
} message_box_sprite_save_t;

typedef struct {
  sprite_t sprite;
  sprite_save_t saves[2];
  message_box_sprite_save_t saveBuffers[2];
  uint16_t visible;
  image_t image;
  int16_t screenY;
  int16_t targetY;
  void (*callback)(void);
} message_box_sprite_t;

static __section(bss_c) message_box_sprite_t message_boxSprite = {
  .visible = 0
};

#if TRACKLOADER==1
typedef struct {
  uint16_t bpl1[SCREEN_BIT_DEPTH*2*2];
  uint16_t end[2];
} message_copper_t;

static  __section(data_c)  message_copper_t message_copper  = {
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
  .end = {0xFFFF, 0xFFFE}
};

static void 
message_pokeCopperList(frame_buffer_t frameBuffer)
{
  uint16_t volatile* copperPtr = (uint16_t*)&message_copper;
  /* poke bitplane pointers into copper list */
  uint32_t bitplanesPtr = (uint32_t)frameBuffer;

  for (int16_t i = 0; i < SCREEN_BIT_DEPTH; i++) {
    copperPtr[1] = (uint16_t)bitplanesPtr;
    copperPtr[3] = (uint16_t)(((uint32_t)bitplanesPtr)>>16);
    bitplanesPtr = bitplanesPtr + (SCREEN_WIDTH_BYTES);
    copperPtr = copperPtr + 4;
  }
}
#endif


void
message_screenOn(char* message)
{
  if (message_on) {
    return;
  }

  hw_waitVerticalBlank();
  palette_black();

#if TRACKLOADER==1
  volatile uint16_t scratch;

  custom->dmacon = DMAF_RASTER|DMAF_SPRITE;
  custom->dmacon = (DMAF_BLITTER|DMAF_SETCLR|DMAF_MASTER);

  uint16_t volatile* copperPtr = (uint16_t*)&message_copper;

  gfx_fillRectSmallScreen(game_offScreenBuffer, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0);
  text_drawMaskedText8Blitter(game_offScreenBuffer, message, (SCREEN_WIDTH/2)-(strlen(message)*4), (SCREEN_HEIGHT/2)+4);
  hw_waitBlitter();
  custom->bltafwm = 0xffff;

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

  message_pokeCopperList(game_offScreenBuffer);

  custom->dmacon = (DMAF_BLITTER|DMAF_SETCLR|DMAF_COPPER|DMAF_RASTER|DMAF_MASTER);

  hw_waitVerticalBlank();
  custom->color[1] = 0xfff;
#else
  USE(message);
#endif

  message_on = 1;
}

void
message_screenOff(void)
{
#if TRACKLOADER==1
  hw_waitVerticalBlank();
  palette_black();

  gfx_fillRectSmallScreen(game_offScreenBuffer, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0);
  hw_waitBlitter();

  hw_waitVerticalBlank();
  screen_setup((uint16_t*)&copper);
#endif
  message_on = 0;
}


void
message_boxRender(frame_buffer_t fb)
{
  if (message_boxSprite.visible) {
    if (message_boxSprite.screenY == -MESSAGE_BOX_HEIGHT && message_boxSprite.targetY == -MESSAGE_BOX_HEIGHT) {
      if (message_boxSprite.saves[0].blit[0].size == 0 &&
	  message_boxSprite.saves[1].blit[0].size == 0) {
	message_boxSprite.visible = 0;
	if (message_boxSprite.callback != 0) {
	  (*message_boxSprite.callback)();
	}
      }
    } else {
      gfx_splitBlitNoMask(fb, message_frameBuffer, message_boxSprite.sprite.x, message_boxSprite.sprite.y-game_cameraY, 0, 0, MESSAGE_BOX_WIDTH, MESSAGE_BOX_HEIGHT);
    }
  }
}


void
message_boxSaveBackground(frame_buffer_t fb)
{
  if (message_boxSprite.visible) {
    message_boxSprite.sprite.y = game_cameraY+message_boxSprite.screenY;

    if (message_boxSprite.targetY > 0) {
      if (message_boxSprite.screenY < message_boxSprite.targetY) {
	message_boxSprite.screenY+=8;
      }
    } else if (message_boxSprite.targetY < 0) {
      if (message_boxSprite.screenY > message_boxSprite.targetY) {
	message_boxSprite.screenY-=8;
      }
    }

    if (message_boxSprite.callback && JOYSTICK_BUTTON_DOWN) {
      message_boxSprite.targetY = -MESSAGE_BOX_HEIGHT;
    }

    sprite_save(fb, &message_boxSprite.sprite);
    message_boxSprite.sprite.save = message_boxSprite.sprite.save == &message_boxSprite.saves[0] ? &message_boxSprite.saves[1] : &message_boxSprite.saves[0];
    message_boxSprite.sprite.saveBuffer = message_boxSprite.sprite.saveBuffer == message_boxSprite.saveBuffers[0].fb ? message_boxSprite.saveBuffers[1].fb : message_boxSprite.saveBuffers[0].fb;
  }
}


void
message_boxRestoreBackground(void)
{
  if (message_boxSprite.visible) {
    sprite_restore(message_boxSprite.sprite.save);
  }
}


void
message_box(char* message, void(*callback)(void))
{
  if (message_boxSprite.visible) {
    return;
  }
  message_boxSprite.callback = callback;
  message_boxSprite.screenY = -MESSAGE_BOX_HEIGHT;
  message_boxSprite.targetY = 48;
  message_boxSprite.sprite.x = (SCREEN_WIDTH/2)-(MESSAGE_BOX_WIDTH/2);
  message_boxSprite.sprite.save = &message_boxSprite.saves[0];
  message_boxSprite.sprite.saveBuffer = message_boxSprite.saveBuffers[0].fb;
  message_boxSprite.sprite.saveBufferHeightOffset = ((MESSAGE_BOX_SAVE_WIDTH/8)*SCREEN_BIT_DEPTH);
  message_boxSprite.saves[0].blit[0].size = 0;
  message_boxSprite.saves[0].blit[1].size = 0;
  message_boxSprite.saves[1].blit[0].size = 0;
  message_boxSprite.saves[1].blit[1].size = 0;
  message_boxSprite.sprite.image = &message_boxSprite.image;
  message_boxSprite.image.h = MESSAGE_BOX_HEIGHT;
  message_boxSprite.image.w = MESSAGE_BOX_WIDTH;

  message_boxSprite.visible = 1;

  gfx_fillRect(message_frameBuffer, 4, 4, MESSAGE_BOX_WIDTH-8, 8, 0);

  frame_buffer_t ptr = message_frameBuffer;
  ptr += FRAME_BUFFER_WIDTH_BYTES;
  text_drawText8(ptr, message, (MESSAGE_BOX_WIDTH/2)-((strlen(message)/2)*8), 4);
  ptr += 3*FRAME_BUFFER_WIDTH_BYTES;
  text_drawText8(ptr, message, (MESSAGE_BOX_WIDTH/2)-((strlen(message)/2)*8), 4);
}


void
message_boxOff(void)
{
  message_boxSprite.visible = 0;
}


void
message_boxDismiss(void)
{
  message_boxSprite.targetY = -MESSAGE_BOX_HEIGHT;
}
