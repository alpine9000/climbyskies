#ifndef __GAME_H
#define __GAME_H

#include <hardware/custom.h>
#include <hardware/dmabits.h>
#include <hardware/intbits.h>

#define CUSTOM ((struct Custom*)0xdff000)
#define SCREEN_WIDTH        224
#define SCREEN_HEIGHT       256
#define FRAME_BUFFER_OFFSCREEN_HEIGHT 32
#define FRAME_BUFFER_HEIGHT (SCREEN_HEIGHT+FRAME_BUFFER_OFFSCREEN_HEIGHT)
#define FRAME_BUFFER_WIDTH  (SCREEN_WIDTH+64)
#define TILE_WIDTH          16
#define TILE_HEIGHT         16
#define SCREEN_HEIGHT_WORDS SCREEN_HEIGHT/16
#define SCREEN_WIDTH_BYTES  (SCREEN_WIDTH/8)
#define FRAME_BUFFER_WIDTH_BYTES  (FRAME_BUFFER_WIDTH/8)
#define SCREEN_WIDTH_WORDS  (SCREEN_WIDTH/16)
#define SCREEN_BIT_DEPTH    5
#define SCREEN_RES	    8 /* 8=lo resolution, 4=hi resolution */
#define RASTER_X_START	    0x81 /* hard coded coordinates from hardware manual */
#define RASTER_Y_START	    0x1d /* vertical overscan */
#define RASTER_X_STOP	    RASTER_X_START+SCREEN_WIDTH
#define RASTER_Y_STOP	    RASTER_Y_START+SCREEN_HEIGHT

#define MAP_TILE_HEIGHT     201
#define MAP_TILE_WIDTH      14
#define WORLD_HEIGHT        (MAP_TILE_HEIGHT*TILE_HEIGHT)

#define SCROLL_PIXELS 4

#if defined(__GNUC__)
#define __reg(x)
#define __chip
#define USE(x) do { x = x; } while(0);
#else
#define USE(x)
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
extern volatile uint8_t* saveBuffer;

#include "registers.h"
#include "hw.h"
#include "init.h"
#include "screen.h"
#include "gfx.h"
#include "mouse.h"
#include "palette.h"
#include "tile.h"
#include "sprite.h"
#include "copper.h"
#include "music.h"
#include "bob.h"
#include "actor.h"
#include "player.h"

extern void game_init(void);
extern void game_loop(void);

#endif /* __GAME_H */
