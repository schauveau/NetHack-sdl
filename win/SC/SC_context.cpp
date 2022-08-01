#include "winSC.h"
#include <assert.h>
#include <SDL2/SDL_ttf.h>

// =======================================================

ScContext::ScContext(void)
{
  if ( SDL_Init(SDL_INIT_VIDEO) != 0 ) 
  {
    SDL_Log("Unable to initialize SDL: %s", SDL_GetError());
    exit(1);
  }
  
  if (TTF_Init() < 0) {
    SDL_Log("Failed to initialize SDL-TTF\n");
    exit(1);
  }  
  
  m_sdl_width = 800;
  m_sdl_height = 600;
  m_sdl_window_flags = SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE;
  m_sdl_window = SDL_CreateWindow("Nethack",
                                  SDL_WINDOWPOS_UNDEFINED,
                                  SDL_WINDOWPOS_UNDEFINED,
                                  m_sdl_width,
                                  m_sdl_height,
                                  m_sdl_window_flags);
  
  assert(m_sdl_window);
  m_sdl_renderer = SDL_CreateRenderer(m_sdl_window,
                                      -1,
                                      SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
  assert(m_sdl_renderer);

  
  
  m_map_window        = new ScMapWindow(*this);
  m_status_window     = new ScStatusWindow(*this);
  m_message_window    = new ScMessageWindow(*this);
  m_perminvent_window = new ScPermInventWindow(*this);  
}

// Swap the ids of two windows.
void
ScContext::swap_windows(ScWindow *win1, ScWindow *win2)
{

  printf("==> %p %d %p\n", (void*)win1, win1->m_winid, (void*)this->find_window(win1->m_winid)) ;
  assert( win1 && this->find_window(win1->m_winid) == win1 ) ;
  assert( win2 && this->find_window(win2->m_winid) == win2 ) ;

  std::swap( m_windows[win1->m_winid], m_windows[win2->m_winid] );
  std::swap( win1->m_winid, win2->m_winid );
}


ScWindow *
ScContext::find_window(winid id)
{
  if (id<0 || id >= int(m_windows.size()))
    return nullptr;

  ScWindow *window = m_windows[id];

  // Unique windows should not be used before 'create_nhwindow'.
  if (window && window->m_is_unique && !window->m_is_created) {
    window = nullptr; 
  }   

  return window;
}

ScMenuWindow *
ScContext::find_menu_window(winid id)
{
  auto * menu = dynamic_cast<ScMenuWindow*>( this->find_window(id) ) ;
  if (!menu) {
    fprintf(stderr,"Error in SdlCairo_start_menu: window %d not found or is not a menu\n", id);
    exit(1);
  }
  return menu;
}

ScMapWindow *
ScContext::find_map_window(winid id)
{
  return dynamic_cast<ScMapWindow*>( this->find_window(id) ) ;  
}

void
ScContext::render(void)
{
  int width;
  int height;
  
  SDL_GetRendererOutputSize(m_sdl_renderer, &width, &height);
    
  // Reminder:
  //   The only 32bit pixel format without alpha supported by Cairo is CAIRO_FORMAT_RGB24.
  //   From https://developer.gnome.org/cairo/stable/cairo-Image-Surfaces.html#cairo-format-t
  //     CAIRO_FORMAT_RGB24
  //       each pixel is a 32-bit quantity, with the upper 8 bits unused. Red, Green,
  //         and Blue are stored in the remaining 24 bits in that order.
  //
  //   In practice, that means that in the SDL texture  the R, G and B masks must
  //   be set to 0x00FF000, 0x0000F00 and 0x00000FF.
  //
    
  SDL_Surface *sdl_surface = SDL_CreateRGBSurface(0,
                                                  width,
                                                  height,
                                                  32,
                                                  0x00ff0000,
                                                  0x0000ff00,
                                                  0x000000ff,
                                                  0);

#if 0
  cairo_surface_t *cr_surface;
  cr_surface = cairo_image_surface_create_for_data((unsigned char *)sdl_surface->pixels,
                                                   CAIRO_FORMAT_RGB24,
                                                   sdl_surface->w,
                                                   sdl_surface->h,
                                                   sdl_surface->pitch);
#endif
  
  SDL_SetRenderDrawColor(m_sdl_renderer, 0, 0, 0, 0);
  SDL_RenderClear(m_sdl_renderer);

  // Draw here on sdl_surface
  {
    
  }
  
  SDL_Texture *texture = SDL_CreateTextureFromSurface(m_sdl_renderer, sdl_surface);
  SDL_FreeSurface(sdl_surface);
  
  SDL_RenderCopy(m_sdl_renderer, texture, NULL, NULL);
  SDL_RenderPresent(m_sdl_renderer);

#if 0  
  cairo_surface_destroy(cr_surface);
#endif
  
  SDL_DestroyTexture(texture);    
}

static bool alt_is_enabled = false ; // TODO: move in ctxt

ScEvent 
ScContext::wait_for_event()
{
  const int timeout_ms = 500;

  // ### Notes about keyboard processing in SDL2 ###
  // 
  // SDL2 generates two main kinds of keyboard events: 
  // - Events of type SDL_KEYDOWN or SDL_KEYUP to represent individual
  //   key press or release. That event provides a physical key code
  //   (scancode), a virtual keycode (sym/keycode) and a bimask
  //   of modifiers (e.g. KMOD_LCTRL, ...).
  //
  // - Events of type SDL_TEXTINPUT provide a UTF-8 text without any
  //   modifier information. This is not well documented but we can
  //   probably assume that SINGLE UNICODE character potentially composed
  //   of multiple CODEPOINTS (because of combining characters).
  //
  // - Support for external IME (Input Method Editor) is also
  //   provided via SDL_TEXTEDITING & SDL_TEXTEDITING_EXT events
  //   but this is probably not relevant (at least for now).
  //
  // In SDL 1, a 'unicode' field containing a unicode 'character'
  // was provided with each SDL_KEYDOWN or SDL_KEYUP. That field
  // does not exist anymore.
  //
  // Generally speaking, we can assume that the 'keycode' indicates
  // the character that is produced when the key is pressed without
  // any modifier. There is however no safe way to manually figure
  // out which characters are emited with SHIFT or other modifiers
  // such as the 3rd level chooser (e.g. mapped by default to RALT
  // on non-US layouts such as French). For that, we have to rely
  // on the SDL_TEXTINPUT event.
  //
  //
  // For nethack, we need to indentify 5 kinds of keyboard events
  //  (case 1) Keys that produce visible 7bit ascii characters in range 32-126
  //  (case 2) Special keys that produce a standard control character such
  //      as RETURN (10), TAB (8), ESC (27).
  //  (case 3) CTRL + [A-Z@].
  //      This is encoded by the macro C(x)  
  //  (case 4) ALT + any of the visible characters.
  //      This is encoded by the macro M(x)   
  //  (case 5) Other special keys that could be interpreted by the window
  //      (e.g. PAGEUP, PAGEDOWN, Fn, ... ) potentially with modifiers.  
  //
  // Remark about the SDL events (and keyboard events in general):
  //   - SDL_TEXTINPUT does not occur while the CTRL modifier is active.
  //   - SDL_TEXTINPUT will occur while the ALT modifiers is active. More generally,
  //     the ALT modifier does not appear to have any effect on the emited text.
  //   - Because of keyboard layout options, we should not assume that
  //     a modifier is active because it key is currently pressed.
  //     For example, on a typical french keyboard, a SDL_KEYDOWN is produced
  //     for the 'RIGHT ALT' key but its associated modifier is 'level 3'.
  //     We should only rely on the 'modifiers' mask provided by SDL_KEYDOWN
  //     and SDL_KEYUP.
  //   - SDL_TEXTINPUT is typically preceded by the SDL_KEYDOWN that ended
  //     the sequence of keypresses recognized a valid by the keyboard layout
  //     (that sequence may be arbitrary long.).  
  //
  //  Consequently, the strategy for handling keyboard events is as follow:
  //   - CTRL + A-Z or '@' is detected and processed in SDL_KEYDOWN using the
  //     provided 'keycode' and 'modifiers'. 
  //   - The state of the ALT modifier is memorized in all SDL_KEYDOWN
  //     and SDL_KEYUP events.
  //   - In SDL_TEXTINPUT,
  //       - texts of length 1 byte are processed as (case 1) or (case 4) depending
  //         of the memorize ALT state. 
  //       - texts longer that 1 byte are discarded by default.
  //         Some of them could also be processed as (case 5) if needed. 
  //  
  // Known issues and potential problems:
  //  - Keyboard layout changes are not handled properly by all versions of SDL2.
  //    For example, with SDL 2.20 on Wayland, the keycode in SDL_KEYDOWN are 
  //    incorrectly using the old layout while the text in SDL_TEXTINPUT is correctly
  //    using the new layout. So after switching from a US 'qwerty' to a French
  //    'azerty' layout, pressing CTRL+A will be processed as CTRL+Q.
  //      --> that seems to be solved in git so SDL 2.21 should be
  //
  //  - A significant limitation of SDL is that the actual keyboard layout
  //    is not exposed to the user. That makes is almost impossible to
  //    interpret correctly the keys when CTRL is combined with SHIFT or
  //    RALT/AltGr is combined. CTRL+SHIFT+'-' could mean CTRL+'_' on
  //    a US keyboard but something totally different on other layouts.  
  //    That could be a problem for the command "retravel" which is
  //    mapped to C('_'). 
 
  // 
  
  while (true) {

    SDL_Event ev;

    int ok = SDL_WaitEventTimeout(&ev,timeout_ms) ;
    if (!ok) {
      // TODO: process if error?
      continue ;
    }

    switch(ev.type)
    {
      case SDL_KEYDOWN:
      {        
        int keycode  = ev.key.keysym.sym;
        int mod      = ev.key.keysym.mod;
        //bool repeat  = (ev.key.repeat != 0);
        
        bool alt   =  (mod & KMOD_ALT) != 0;
        bool ctrl  =  (mod & KMOD_CTRL) != 0;
        bool shift =  (mod & KMOD_SHIFT) != 0;
        
        
        /* Reminder: For printable characters, the Alt modifier is
         *           usually handled in SDL_TEXTINPUT.
         */
        alt_is_enabled = alt;
        
        if (SDLK_a <= keycode && keycode <= SDLK_z) {
          return ScEvent::make_nh_key( C('A'+(keycode-SDLK_a)) ) ;    
        }
                
        switch(keycode) {
          case SDLK_RETURN: return ScEvent::make_nh_key('\n');
          case SDLK_ESCAPE: return ScEvent::make_nh_key('\033');
          case SDLK_TAB:    return ScEvent::make_nh_key('\t');                      
        }
        
        if (shift && !ctrl && !alt) {
          switch(keycode) {
            case SDLK_UP:    return ScEvent::make_other( ScEvent::EV_SHIFT_UP) ;
            case SDLK_DOWN:  return ScEvent::make_other( ScEvent::EV_SHIFT_DOWN) ;
            case SDLK_LEFT:  return ScEvent::make_other( ScEvent::EV_SHIFT_LEFT) ;
            case SDLK_RIGHT: return ScEvent::make_other( ScEvent::EV_SHIFT_RIGHT) ;              
          }
        }
        
        if (!shift && !ctrl && !alt) {
          switch(keycode) {
            case SDLK_UP:    return ScEvent::make_other( ScEvent::EV_UP) ;
            case SDLK_DOWN:  return ScEvent::make_other( ScEvent::EV_DOWN) ;
            case SDLK_LEFT:  return ScEvent::make_other( ScEvent::EV_LEFT) ;
            case SDLK_RIGHT: return ScEvent::make_other( ScEvent::EV_RIGHT) ;              
          }
        }
        printf("Unhandled SDL_KEYDOWN keycode='%s' (%d) mod=%08x'\n", SDL_GetKeyName(keycode), keycode, mod);
      }
      break;

      case SDL_KEYUP:
      {
        // int keycode  = ev.key.keysym.sym;
        int mod      = ev.key.keysym.mod;
        alt_is_enabled = (mod & KMOD_ALT) != 0;
      }
      break;
      
      case SDL_TEXTINPUT:
      {
        printf("TEXTINPUT '%s'\n", ev.text.text);
        const char *utf8_text = ev.text.text;
        int len = strlen(utf8_text);
        if (len!=1)
        {
          // Nethack cannot handle multi-bytes UTF-8 so ignore that input.
          // An alternative could be to send ESC (\033) or a custom event.
          break; 
        }
        else
        {
          int c = utf8_text[0];
          if (c<' ' || c>=127) {
            // A non-printable ascii character? This is not supposed
            // to happen but ignore anyway.
            break ;
          }
          // Remark: For Alt, we could also check SDL_GetModState() 
          //   but that would probably give the current state
          //   which may be different when SDL TEXTINPUT was emited.
          if (alt_is_enabled) {
            c = M(c);
          } 
          return ScEvent::make_nh_key(c); 
        }
      }
      break;
        
    }
      
  }
}
