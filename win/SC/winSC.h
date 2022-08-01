#ifndef WINSDLCAIRO_H
#define WINSDLCAIRO_H

#include <SDL2/SDL.h>
//#include <cairo/cairo.h>
#include <vector>
#include <string>

extern "C" {
#include "hack.h"
#include "color.h"
}

extern "C" {
void SC_init_nhwindows(int *, char **);
void SC_player_selection(void);
void SC_askname(void);
void SC_get_nh_event(void);
void SC_exit_nhwindows(const char *);
void SC_suspend_nhwindows(const char *);
void SC_resume_nhwindows(void);
winid SC_create_nhwindow(int);
void SC_clear_nhwindow(winid);
void SC_display_nhwindow(winid, boolean);
void SC_destroy_nhwindow(winid);
void SC_curs(winid, int, int);
void SC_putstr(winid, int, const char *);
void SC_putmixed(winid, int, const char *);
void SC_display_file(const char *, boolean);
void SC_start_menu(winid, unsigned long);
void SC_add_menu(winid, const glyph_info *, const ANY_P *,
                  char, char, int, int,
                  const char *, unsigned int);
void SC_end_menu(winid, const char *);
int SC_select_menu(winid, int, MENU_ITEM_P **);
char SC_message_menu(char, int, const char *);
void SC_mark_synch(void);
void SC_wait_synch(void);
#ifdef CLIPPING
void SC_cliparound(int, int);
#endif
#ifdef POSITIONBAR
void SC_update_positionbar(char *);
#endif
void SC_print_glyph(winid, coordxy, coordxy,
                     const glyph_info *, const glyph_info *);
void SC_raw_print(const char *);
void SC_raw_print_bold(const char *);
int SC_nhgetch(void);
int SC_nh_poskey(coordxy *, coordxy *, int *);
void SC_nhbell(void);
int SC_doprev_message(void);
char SC_yn_function(const char *, const char *, char);
void SC_getlin(const char *, char *);
int SC_get_ext_cmd(void);
void SC_number_pad(int);
void SC_delay_output(void);
#ifdef CHANGE_COLOR
void SC_change_color(int, long, int);
char *SC_get_color_string(void);
#endif
void SC_start_screen(void);
void SC_end_screen(void);
void SC_outrip(winid, int, time_t);
void SC_preference_update(const char *);
char *SC_getmsghistory(boolean);
void SC_putmsghistory(const char *, boolean);
void SC_status_init(void);
void SC_status_finish(void);
void SC_status_enablefield(int, const char *, const char *,
                            boolean);
void SC_status_update(int, genericptr_t, int, int, int,
                       unsigned long *);
boolean SC_can_suspend(void);
void SC_update_inventory(int);
win_request_info *SC_ctrl_nhwindow(winid, int, win_request_info *);
} // extern "C"


/// Use the shorter prefix Sc for the C++ stuff, 

#define TRACE do { fprintf(stderr,"TRACE: in %s at %s line %d\n", __func__, __FILE__, __LINE__); } while(0)

#define TODO do { fprintf(stderr,"TODO in %s at %s line %d\n", __func__, __FILE__, __LINE__); exit(1); } while(0)

#define WARN_TODO do { fprintf(stderr,"TODO in %s at %s line %d\n", __func__, __FILE__, __LINE__); } while(0)


class ScContext;


struct ScEvent
{
  enum Type {

    // A keypress recognized by Nethack
    //    nh_key = A non-zero key code in nethack format
    // See also nhgetch() and nh_poskey()
    EV_NH_KEY,

    // A mouse click on the map
    //    nh_key = 0
    //    nh_x   = x map position
    //    nh_y   = y map position
    //    nh_mod = CLICK_1 or CLICK_2
    // See also nh_poskey()
    EV_NH_MAP_CLICK,

    // Indicates that some windows were resized. 
    EV_RESIZED, 

    // Various special keys that are not recognized by Nethack
    EV_UP,
    EV_DOWN,
    EV_LEFT,
    EV_RIGHT,
    EV_SHIFT_UP,
    EV_SHIFT_DOWN,
    EV_SHIFT_LEFT,
    EV_SHIFT_RIGHT,
    
  };

  Type type; 
  int nh_keycode{0} ; // Set for EV_NH_KEY
  int nh_x{0} ;   // Set for EV_NH_CLICK  
  int nh_y{0} ;   // Set for EV_NH_CLICK 
  int nh_mod{0} ; // Set for EV_NH_CLICK 

private:

  ScEvent(Type t, int k=0, int x=0, int y=0, int mod=0) :
    type(t), nh_keycode(k), nh_x(x), nh_y(y), nh_mod(mod)
  {
  }
  
public:
  
  static ScEvent make_nh_key(int ascii) {
    return ScEvent(EV_NH_KEY, ascii); 
  }

  static ScEvent make_nh_map_click(int x,int y, int button) {
    return ScEvent(EV_NH_MAP_CLICK, 0, x, y, (button==1?CLICK_1:CLICK_2) ) ;
  }

  static ScEvent make_other(ScEvent::Type type) {
    return ScEvent(type) ;
  }
      

};

struct ScWindow
{
  ScWindow() =delete ;
  ScWindow(ScContext &ctxt, int type, bool unique=false);
  virtual ~ScWindow();

  ScContext &m_ctxt;
  int   m_type{-1}; // one of NHW_MESSAGE, NHW_STATUS, ...
  winid m_winid{WIN_ERR};

  bool  m_is_unique{false};
  bool  m_is_created{false};

  // Cursor position
  int  m_curs_x{0}; // column 
  int  m_curs_y{0}; // line 

  bool do_create(); 
  bool do_destroy();

  virtual void do_putstr(int attr, const char *str);
                
  virtual void do_clear()=0;
  virtual void do_display(bool blocking)=0;
    
};

struct ScMapWindow : public ScWindow
{
  ScMapWindow(ScContext &ctxt);
  void do_clear() override;
  void do_display(bool blocking) override;
  
  void do_print_glyph(coordxy x,
                      coordxy y,
                      const glyph_info *glyphinfo,
                      const glyph_info *bkglyphinfo);
  
  SDL_Surface * m_surface = nullptr;
};

struct ScMessageWindow : public ScWindow
{
  ScMessageWindow(ScContext &ctxt);
  void do_clear() override;
  void do_display(bool blocking) override;
};

struct ScStatusWindow: public ScWindow
{
  ScStatusWindow(ScContext &ctxt);
  void do_clear() override;
  void do_display(bool blocking) override;
};




struct ScMenuWindow : public ScWindow
{
  ScMenuWindow(ScContext &ctxt) ;

  void do_start_menu(unsigned long behavior) ;
  void do_add_menu(const glyph_info *glyphinfo,
                   const ANY_P * identifier,
                   char accel,
                   char group_accel,
                   int attribute,
                   int color,
                   const char *str,
                   unsigned int itemflags);
  void do_end_menu(const char *query);
  int do_select_menu(int how, MENU_ITEM_P **menu_list);


  void do_clear() override ;
  void do_display(bool blocking) override;

  enum State {
    MS_EMPTY,   // before 'start_menu'
    MS_STARTED, // between 'start_menu' and 'end_menu'
    MS_ENDED,   // after 'end_menu'
    MS_SELECTING, // during 'select_ qqmenu'      
    MS_DONE,    // after 'select_ qqmenu'      
  };

  struct Item {
    /* glyph_info gqlyphinfo; */
    anything identifier;
    char accel;
    char group_accel;
    int attribute;
    /* int color; */
    std::string str;
    unsigned int itemflags;

    bool selectable() { return identifier.a_void != 0 ; }
    
  };

  int m_accel_count = 0;
  std::vector<Item> m_items;  
  State m_state = MS_EMPTY;

private:

  char get_next_accel(); 
};

struct ScTextWindow : public ScWindow
{
  ScTextWindow(ScContext &ctxt);

  void do_clear() override ;
  void do_display(bool blocking) override;
};

struct ScPermInventWindow : public ScWindow
{
  ScPermInventWindow(ScContext &ctxt);

  void do_clear() override ;
  void do_display(bool blocking) override;
};


  
// Hold the whole context for the SC port. 
// There is only one global instance so the whole class could
// potentially be replaced by multiple global variables and functions. 
class ScContext
{
public:
  static ScContext *instance;
public:
  
  int           m_sdl_width{0};
  int           m_sdl_height{0};
  uint32_t      m_sdl_window_flags{0};  
  SDL_Window*   m_sdl_window{0};
  SDL_Renderer* m_sdl_renderer{0};

  // Contains all opened ScWindows:
  //   - The index in the vector corresponds to the m_winid.
  //   - The insertion & removal from that vector are handled by
  //     the constructor and destructor of ScWindow.
  //   - Newly created windows are always given a greater winid
  //     than existing windows.
  //   - Null elements may happen when the windows are not
  //     destroyed in reverse order of they creation.
  std::vector<ScWindow*> m_windows;  

  // Shortcuts for the windows with a unique type. 
  ScMapWindow     * m_map_window{0};
  ScStatusWindow  * m_status_window{0};
  ScMessageWindow * m_message_window{0};
  ScPermInventWindow * m_perminvent_window{0};
  
public:

  ScContext();
  
  void render();
  
  ScWindow *      find_window(winid id);  
  ScMenuWindow *  find_menu_window(winid id);
  ScMapWindow *   find_map_window(winid id);

  void assert_windows(ScWindow *window);

  void swap_windows(ScWindow *win1, ScWindow *win2);
  
  ScEvent wait_for_event() ;
};



#endif
