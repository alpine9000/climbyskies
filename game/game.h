#ifndef __GAME_H
#define __GAME_H

#ifdef __cplusplus
extern "C" {
#endif

#include <hardware/custom.h>
#include <hardware/dmabits.h>
#include <hardware/intbits.h>

#define abs(a) (a >= 0 ? a : -a)  

#define MAP_TILE_WIDTH      16
#define MAP_TILE_HEIGHT     201
#define TILE_WIDTH          16
#define TILE_HEIGHT         16


#define CUSTOM ((struct Custom*)0xdff000)
#define SCREEN_WIDTH        MAP_TILE_WIDTH*TILE_WIDTH
#define SCREEN_HEIGHT       256
#define FRAME_BUFFER_OFFSCREEN_HEIGHT TILE_HEIGHT*4
#define FRAME_BUFFER_HEIGHT (SCREEN_HEIGHT+FRAME_BUFFER_OFFSCREEN_HEIGHT)
#define FRAME_BUFFER_WIDTH  (SCREEN_WIDTH+64)

#define SCREEN_HEIGHT_WORDS SCREEN_HEIGHT/16
#define SCREEN_WIDTH_BYTES  (SCREEN_WIDTH/8)
#define FRAME_BUFFER_WIDTH_BYTES  (FRAME_BUFFER_WIDTH/8)
#define SCREEN_WIDTH_WORDS  (SCREEN_WIDTH/16)
#define SCREEN_BIT_DEPTH    5
#define SCREEN_RES	    8 /* 8=lo resolution, 4=hi resolution */
#define RASTER_X_START	    (0x81+((320-SCREEN_WIDTH)/2)) /* hard coded coordinates from hardware manual */
#define RASTER_Y_START	    0x1d /* vertical overscan */
#define RASTER_X_STOP	    RASTER_X_START+SCREEN_WIDTH
#define RASTER_Y_STOP	    RASTER_Y_START+SCREEN_HEIGHT
#define SCOREBOARD_HEIGHT   16


#define WORLD_HEIGHT        (MAP_TILE_HEIGHT*TILE_HEIGHT)

#define SCROLL_PIXELS 4


#if defined(__GNUC__)
#if defined(GCC_CHECK)
#define __section(x)
#define __REG(reg, arg) arg
#else
#undef __chip
#define __section(x) __attribute__ ((section (#x))) 
#define __REG(reg, arg) register arg asm(reg)
#endif
#define USE(x) do { x = x; } while(0);
#else
#define USE(x)
#define __REG(reg, arg) __reg(reg) arg
#endif


typedef UBYTE uint8_t;
typedef SHORT int16_t;
typedef USHORT uint16_t;
typedef LONG int32_t;
typedef ULONG uint32_t;

typedef volatile uint8_t * frame_buffer_t;

typedef volatile struct Custom* custom_t;
extern custom_t custom; 
extern int cameraY;
extern int screenScrollY;
extern int scrollCount;
extern int scroll;
extern uint32_t frameCount;
extern frame_buffer_t saveBuffer;
extern frame_buffer_t offScreenBuffer;
extern frame_buffer_t onScreenBuffer;

#include "registers.h"
#include "hw.h"
#include "init.h"
#include "screen.h"
#include "gfx.h"
#include "mouse.h"
#include "palette.h"
#include "tile.h"
#include "copper.h"
#include "music.h"
#include "actor.h"
#include "sprite.h"
#include "player.h"
#include "cloud.h"
#include "text.h"
#include "disk.h"

void
game_init(void);
void 
game_loop(void);
void
game_setBackgroundScroll(int s);
void
game_shakeScreen(void);


#ifdef __cplusplus
} // extern "C"
#endif
#endif /* __GAME_H */
