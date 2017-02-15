#ifndef __GAME_H
#define __GAME_H

#include <hardware/custom.h>
#include <hardware/dmabits.h>
#include <hardware/intbits.h>


#define NEW_TILE_COLLISION        1
#define FIX_TILE_INVALIDATE_BUG   1
#define ENABLE_PLAYER_HEADSMASH   1
#define INLINE_EVERYTHING         1
//#define SHOW_SPEED              1
//#define CLOUD_FULLCOLOR         1
#define CLOUD_ENABLE_SETUPRENDERPARTIALTILE 1
#define GAME_PAUSE_DISABLES_COLLISION 1


#ifdef ENABLE_PLAYER_HEADSMASH
#define PLAYER_HEADSMASH 1
#else
#define PLAYER_HEADSMASH 0
#endif


#ifdef INLINE_EVERYTHING
#define INLINE static inline
#define STATIC_INLINE static inline
#else
#define INLINE
#define STATIC_INLINE static
#endif

#define abs(a) (a >= 0 ? a : -a)  

#define MAP_TILE_WIDTH      16
#define MAP_TILE_HEIGHT     202
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
#define SCROLL_PIXELS       4

#define PHYSICS_VELOCITY_G        1
#define PHYSICS_TERMINAL_VELOCITY (SCROLL_PIXELS*2)
#define PHYSICS_VELOCITY_RUN      2
#define PHYSICS_VELOCITY_JUMP     -16
#define PHYSICS_VELOCITY_KILL     -10


typedef UBYTE uint8_t;
typedef SHORT int16_t;
typedef USHORT uint16_t;
typedef LONG int32_t;
typedef ULONG uint32_t;
typedef ULONG size_t;

#define __EXTERNAL __attribute__((externally_visible))

extern void* memcpy(void* destination, void* source, size_t num);

#if defined(__GNUC__)
extern void* memset(void *dst, int c, size_t n);
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
#define __attribute__(x)
#endif

typedef volatile uint8_t * frame_buffer_t;
typedef volatile struct Custom* custom_t;

extern custom_t custom; 
extern int game_cameraY;
extern int game_screenScrollY;
extern int game_scrollCount;
extern int game_scroll;
extern int game_paused;
extern uint32_t game_frameCount;
extern int game_levelComplete;
extern frame_buffer_t game_saveBuffer;
extern frame_buffer_t game_offScreenBuffer;
extern frame_buffer_t game_onScreenBuffer;
extern frame_buffer_t game_scoreBoardFrameBuffer;
extern uint32_t game_score;

#include "registers.h"
#include "hw.h"
#include "init.h"
#include "screen.h"
#include "gfx.h"
#include "mouse.h"
#include "palette.h"
#include "sprite.h"
#include "item.h"
#include "level.h"
#include "enemy.h"
#include "tile.h"
#include "copper.h"
#include "music.h"
#include "actor.h"
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
void
game_loseLife();

#endif /* __GAME_H */
