#include "winSC.h"
#include <assert.h>
#include <SDL_ttf.h>

static int map_ttychar[ROWNO][COLNO]; // The characters and colors of the tty map   

static std::string map_fontfile("/usr/share/fonts/truetype/hack/Hack-Regular.ttf");

static TTF_Font * map_font = nullptr;

static int map_fontsize = 10 ; 

ScMapWindow::ScMapWindow(ScContext &ctxt) :
  ScWindow(ctxt,NHW_MAP,true)
{
  map_font = TTF_OpenFont(map_fontfile.c_str(), map_fontsize);
}

void
ScMapWindow::do_clear()
{
  // TODO
}

void
ScMapWindow::do_display(bool blocking UNUSED)
{
  // TODO
}

void
ScMapWindow::do_print_glyph(coordxy x,
                            coordxy y,
                            const glyph_info *glyphinfo UNUSED,
                            const glyph_info *bkglyphinfo UNUSED)
{
  // TODO:
  printf("print_glyph %d %d\n", x,y);


  if ( x<0 || x>=COLNO) {
    printf(" -> WARNING: x out of bound\n");
    return;
  }
  if ( y<0 || y>=ROWNO) {
    printf(" -> WARNING: y out of bound\n");
    return;
  }
  printf("   glyph=%d , ttychar=%d \n", glyphinfo->glyph, glyphinfo->ttychar);
  if (glyphinfo->glyph!=0) {
    map_ttychar[y][x] = glyphinfo->ttychar;
  }

}

