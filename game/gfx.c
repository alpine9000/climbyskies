#include "game.h"

#define printf(...)
#define dprintf(...)

typedef struct {
  uint16 x;
  uint16 y;
} char_lookup_t;

static char_lookup_t charAtlas[127];
static uint16 dyOffsetsLUT[SCREEN_HEIGHT];
static __chip uint8 fontBuffer[SCREEN_WIDTH_BYTES*SCREEN_BIT_DEPTH*SCREEN_HEIGHT];

static void _gfx_renderCharRetro(frame_buffer_t fb, int16 x, int16 y, char c, uint16 color);

void 
gfx_init()
{
  for (uint16 y = 0; y < SCREEN_HEIGHT; y++) {
    dyOffsetsLUT[y] = (y * (SCREEN_WIDTH_BYTES*SCREEN_BIT_DEPTH));
  }

  hw_waitBlitter();

  int x = 0, y = 0;
  for (unsigned char c = 0; c < 127; c++) {
    if (x+16 > SCREEN_WIDTH) {
      x = 0;
      y += 8;
    }
    charAtlas[c].x = x;
    charAtlas[c].y = y;
    _gfx_renderCharRetro(fontBuffer, x, y, c, 1);
    x+=16;
  }
}


void
gfx_fillRect(frame_buffer_t fb, uint16 x, uint16 y, uint16 w, uint16 h, uint16 color)
{
  static volatile struct Custom* _custom = CUSTOM;
  static uint16 startBitPatterns[] = { 0xffff,
			       0x7fff, 0x3fff, 0x1fff, 0x0fff, 
			       0x07ff, 0x03ff, 0x01ff, 0x00ff,
			       0x007f, 0x003f, 0x001f, 0x000f,
			       0x0007, 0x0003, 0x0001, 0x0000 };

  static uint16 endBitPatterns[] = { 0xffff, 
				    0x8000, 0xc000, 0xe000, 0xf000,
				    0xf800, 0xfc00, 0xfe00, 0xff00,
				    0xff80, 0xffc0, 0xffe0, 0xfff0,
				    0xfff8, 0xfffc, 0xfffe, 0xffff};

  uint16 startMask = startBitPatterns[x & 0xf]; 
  uint16 endMask = endBitPatterns[(x+w) & 0xf]; 
  uint32 widthWords = (((x&0x0f)+w)+15)>>4;
  
  if (widthWords == 1) {
    startMask &= endMask;
  }
  
  fb += dyOffsetsLUT[y] + (x>>3);

  int colorInPlane;
  for (int plane = 0; plane < SCREEN_BIT_DEPTH; plane++) {
    colorInPlane = (1<<plane) & color;
    hw_waitBlitter();
    
    _custom->bltcon0 = (SRCC|DEST|0xca);
    _custom->bltcon1 = 0;
    _custom->bltafwm = 0xffff;
    _custom->bltalwm = 0xffff;
    _custom->bltdmod = (SCREEN_WIDTH_BYTES*(SCREEN_BIT_DEPTH-1))+(SCREEN_WIDTH_BYTES-2);
    _custom->bltcmod = (SCREEN_WIDTH_BYTES*(SCREEN_BIT_DEPTH-1))+(SCREEN_WIDTH_BYTES-2);
    _custom->bltbmod = 0;
    _custom->bltamod = 0;
    _custom->bltadat = startMask;
    _custom->bltbdat = colorInPlane ? 0xffff : 0x0;
    _custom->bltcpt = fb;
    _custom->bltdpt = fb;
    _custom->bltsize = h<<6 | 1;
    
    if (widthWords > 1) {
      hw_waitBlitter();    
      _custom->bltcon0 = (SRCC|DEST|0xca);
      _custom->bltadat = endMask;
      _custom->bltcpt = fb+((widthWords-1)<<1);
      _custom->bltdpt = fb+((widthWords-1)<<1);
      _custom->bltsize = h<<6 | 1;
    }
    
    if (widthWords > 2) {
      hw_waitBlitter();    
      _custom->bltcon0 = (DEST|(colorInPlane ? 0xff : 0x00));
      _custom->bltdmod = (SCREEN_WIDTH_BYTES*(SCREEN_BIT_DEPTH-1))+(SCREEN_WIDTH_BYTES-((widthWords-2)<<1));
      _custom->bltdpt = fb+2;
      _custom->bltsize = h<<6 | widthWords-2;
    }    

    fb += SCREEN_WIDTH_BYTES;
  }
}


uint8
gfx_getPixel(frame_buffer_t fb, int16 x, int16 y) 
{
  fb += (y*SCREEN_WIDTH_BYTES) + (x >> 3);
  return *fb & (0x80 >> (x & 0x7));
}


void
gfx_drawPixel(frame_buffer_t fb, int16 x, int16 y, uint16 color) 
{
  fb += (y*SCREEN_WIDTH_BYTES) + (x >> 3);
  if (color) {
    *fb |= (0x80 >> (x & 0x7));
  } else {
    *fb &= ~(0x80 >> (x & 0x7));
  }
}


static void
_gfx_renderCharRetro(frame_buffer_t fb, int16 x, int16 y, char c, uint16 color) 
{
  for (unsigned char i =0; i<gfx_retroFontWidth; i++ ) {
    unsigned char line = font[(c*gfx_retroFontWidth)+i];
    for (unsigned char j = 0; j<gfx_retroFontHeight; j++) {
      if (line & 0x1) {
	gfx_drawPixel(fb, x+i, y+j, color);
      }
      line >>= 1;
    }
  }
}


void
gfx_drawCharRetro(frame_buffer_t fb, int16 x, int16 y, char c, uint16 color) 
{
  gfx_bitBlt(fb, charAtlas[c].x, charAtlas[c].y, x, y, 8, 8, fontBuffer);
}


void 
gfx_drawStringRetro(frame_buffer_t fb, int16 x, int16 y, char *c, uint16 color, int spaceSize)
{
  while (c[0] != 0) {
    gfx_drawCharRetro(fb, x, y, c[0], color);
    x += (gfx_retroFontWidth+spaceSize);
    c++;
  }
}


// bresenham's algorithm - thx wikpedia
#define swap(a, b) { int t = a; a = b; b = t; }
#define abs(x) (x > 0 ? x : -x)
void 
gfx_drawLine(frame_buffer_t fb, int16 x0, int16 y0, int16 x1, int16 y1, uint16 color) {
  uint16 steep = abs(y1 - y0) > abs(x1 - x0);

  if (steep) {
    swap(x0, y0);
    swap(x1, y1);
  }

  if (x0 > x1) {
    swap(x0, x1);
    swap(y0, y1);
  }

  uint32 dx, dy;
  dx = x1 - x0;
  dy = abs(y1 - y0);

  int32 err = dx / 2;
  int32 ystep;

  if (y0 < y1) {
    ystep = 1;
  } else {
    ystep = -1;
  }

  for (; x0<=x1; x0++) {
    if (steep) {
      gfx_drawPixel(fb, y0, x0, color);
    } else {
      gfx_drawPixel(fb, x0, y0, color);
    }
    err -= dy;
    if (err < 0) {
      y0 += ystep;
      err += dx;
    }
  }
}


void
gfx_bitBlt(frame_buffer_t dest, int16 sx, int16 sy, int16 dx, int16 dy, int16 w, int16 h, frame_buffer_t source)
{
  static volatile struct Custom* _custom = CUSTOM;

  uint32 widthWords =  ((w+15)>>4)+1;
  int shift = (dx&0xf);
  
  dest += dyOffsetsLUT[dy] + (dx>>3);
  source += dyOffsetsLUT[sy] + (sx>>3);

  hw_waitBlitter();

  _custom->bltcon0 = (SRCA|SRCB|SRCC|DEST|0xca|shift<<ASHIFTSHIFT);
  _custom->bltcon1 = shift<<BSHIFTSHIFT;
  _custom->bltafwm = 0xffff;
  _custom->bltalwm = 0x0000;
  _custom->bltamod = SCREEN_WIDTH_BYTES-(widthWords<<1);
  _custom->bltbmod = SCREEN_WIDTH_BYTES-(widthWords<<1);
  _custom->bltcmod = SCREEN_WIDTH_BYTES-(widthWords<<1);
  _custom->bltdmod = SCREEN_WIDTH_BYTES-(widthWords<<1);
  _custom->bltapt = source;
  _custom->bltbpt = source;
  _custom->bltcpt = dest;
  _custom->bltdpt = dest;
  _custom->bltsize = h<<6 | widthWords;
}

