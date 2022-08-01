#include "winSC.h"
#include <assert.h>

#include <SDL2/SDL_ttf.h>

static ScContext *ctxt = nullptr;

static ScMenuWindow *
require_menu(winid wid, const char *from)
{
  ScMenuWindow *menu = ctxt->find_menu_window(wid);
  if (!menu) {
    impossible("%s: called on a non-menu window",from);
    return nullptr; 
  }
  return menu;
}


//
// Similar to ScContext::find_menu_window(wid) but 
// make sure that the window can handle putstr.
//
static ScWindow *
require_putstr_window(winid wid, const char *from)
{
  ScWindow *window = ctxt->find_window(wid);
  if (!window) {
    impossible("%s: called on a non existing window",from);
    return nullptr; 
  }

  // Using putstr on a non-started menu makes it become a text window.
  //
  // This is really a strange behavior!
  //
  if (window->m_type == NHW_MENU) {
    printf("MENU CAST 1 %p \n",(void*)window);
    ScMenuWindow *menu = dynamic_cast<ScMenuWindow*>(window);
    printf("MENU CAST 2 %p \n",(void*)menu);    
    if ( menu->m_state != ScMenuWindow::MS_EMPTY ) {
      impossible( "%s: called on an already started menu",from);
      return nullptr;
    } else {
      window = new ScTextWindow(*ctxt);
      printf("%p %d %p\n", (void*) window, window->m_winid , (void*)ctxt->find_window(window->m_winid) );
      ctxt->swap_windows(window, menu); 
      delete menu;
    }
  }
  return window;
}


void
SC_init_nhwindows(int *argcp UNUSED,
                        char **argv UNUSED)
{   
 
  ctxt = new ScContext();
  ctxt->render();
}

void
SC_player_selection(void)
{
  // Select the character role, gender, ...
  //
  // Remark: This is called even when WC_PLAYER_SELECTION is not set.
  //         There does not seem to be any generic implementation. 
  //
  // The goal here is to set flags.initrole, flags.initrace, flags.initgend and flags.initalign.
  //
  // See curses_choose_character() for a sample implementation.
  // 
  // For now, let's simply pick them randomly.
  //

  auto & role = flags.initrole ;
  auto & race = flags.initrace ;
  auto & gend = flags.initgend ;
  auto & alig = flags.initalign ;

  try_again:
  
  if (!validrole(role) ) {  
    int sel = pick_role(race, gend, alig, PICK_RANDOM);
    if (sel < 0)
      sel = randrole(FALSE);
    role = sel;
  }
  
  if (!validrace(role,race)) {  
    int sel = pick_race(role, gend, alig, PICK_RANDOM);
    if (sel < 0)
      sel = randrace(role);
    race = sel;
  }
  
  if (!validgend(role,race,gend)) {  
    int sel = pick_gend(role, race, alig, PICK_RANDOM);
    if (sel < 0)
      sel = randgend(role, race);
    gend = sel;
  }
  
  if (!validalign(role,race,alig)) {
    int sel = pick_align(role, race, gend, PICK_RANDOM);
    if (sel < 0)
      sel = randalign(role, race);
    alig = sel;
  }
  
  // The algorithm above should always give a valid configuration
  // but let's be a bit paranoid.
    
  if ( !validrole(role) ||
       !validrace(role,race) ||
       !validgend(role,race,gend) ||
       !validalign(role,race,alig) )
  {
    role = ROLE_RANDOM;
    race = ROLE_RANDOM;
    gend = ROLE_RANDOM;
    alig = ROLE_RANDOM;
    goto try_again;
  }
  
}

void
SC_askname(void)
{
  TODO;
}

void
SC_get_nh_event(void)
{
  TODO;
}

void
SC_exit_nhwindows(const char *msg)
{
  // TODO: Cleanup everything
  if (msg)
    fprintf(stderr,"%s\n",msg);  
}

void
SC_suspend_nhwindows(const char * str UNUSED)
{
  TODO;
}

void
SC_resume_nhwindows(void)
{
  TODO;
}

winid
SC_create_nhwindow(int type UNUSED)
{
  const char *tname = "?";
#define TYPE_CASE(x) if(type==NHW_##x) tname=#x 
  TYPE_CASE(MESSAGE);
  TYPE_CASE(MAP);
  TYPE_CASE(STATUS);
  TYPE_CASE(MENU);
  TYPE_CASE(PERMINVENT);
  TYPE_CASE(TEXT);
  printf("SC_create_nhwindow type=%s\n",tname);
  
  ScWindow * win=nullptr;
  
  if (type==NHW_MENU) {
    ScMenuWindow * menu = new ScMenuWindow(*ctxt) ;
    win = dynamic_cast<ScWindow*>(menu);
    printf("MENU: %p %p\n",(void*)menu, (void*)win);
  }
  else if (type==NHW_TEXT)
    win = new ScTextWindow(*ctxt) ;
  else if (type==NHW_STATUS)
    win = ctxt->m_status_window;
  else if (type==NHW_MESSAGE)
    win = ctxt->m_message_window;
  else if (type==NHW_MAP)
    win = ctxt->m_map_window;
  else if (type==NHW_PERMINVENT)
    win = ctxt->m_perminvent_window;

  winid id = WIN_ERR;
  if (win) {
    if (win->do_create()) {
      id = win->m_winid;
    }
  }
  
  printf("   -> %d\n", id);
  return id;
}

void
SC_clear_nhwindow(winid window UNUSED)
{
  ScWindow *w = ctxt->find_window(window);
  w->do_clear();
}

void
SC_display_nhwindow(winid wid,
                          boolean blocking)
{
  printf("--> display_nhwindow %d blocking%d\n",wid, blocking);
  ScWindow *w = ctxt->find_window(wid);
  w->do_display(blocking);
}

void
SC_destroy_nhwindow(winid wid UNUSED)
{
  ScWindow *window = ctxt->find_window(wid);
  if (!window) {
    impossible("destroy_nhwindow called on a non-existing window");
    return;
  }
  
  window->do_destroy();  
}

void
SC_curs(winid wid,
              int x,
              int y)
{
  printf("SC_curs win=%d x=%d y=%d\n",wid,x,y) ;
  
  ScWindow *window = ctxt->find_window(wid);
  if (!window) {
    impossible("curs called on a non-existing window");
    return;
  }
  
  window->m_curs_x = x ;
  window->m_curs_y = y ;
}

void
SC_putstr(winid wid,
                int attr,
                const char *str)
{
  ScWindow * window = require_putstr_window(wid, "putstr");
  if (!window)
    return;

  window->do_putstr(attr,str);

  
  //  printf("SC_putstr win=%d attr=%d '%s'\n",wid,attr,str) ;
  //  TODO;
  
}

/* This differs from putstr() because the str parameter can
 * contain a sequence of characters representing:
 *        \GXXXXNNNN    a glyph value, encoded by encglyph().
 */
void
SC_putmixed(winid wid, int attr, const char *str)
{
  // TODO: For now, use the generic version that get
  //       rids of the glyphs and call 'putstr'
  // REMINDER: If we decide to      
  genl_putmixed(wid,attr,str);
}

void
SC_display_file(const char *str UNUSED,
                 boolean complain UNUSED)
{
  TODO;
}

void
SC_start_menu(winid wid,
                    unsigned long mbehavior)
{
  TRACE;
  ScMenuWindow *menu = require_menu(wid,"start_menu");
  if (menu) 
    menu->do_start_menu(mbehavior);    
}

void
SC_add_menu(winid wid,
                  const glyph_info *glyphinfo,
                  const ANY_P * identifier,
                  char ch,
                  char gch,
                  int att,
                  int clr,
                  const char *str,
                  unsigned int itemflags)
{
  TRACE;
  ScMenuWindow *menu = require_menu(wid,"add_menu");
  if (menu) {
    menu->do_add_menu(glyphinfo,identifier,ch,gch,att,clr,str,itemflags);
  }
}

void
SC_end_menu(winid wid,
                  const char *query)
{
  TRACE;
  ScMenuWindow *menu = require_menu(wid,"end_menu");
  if (menu) {
    menu->do_end_menu(query);
  }
}

int
SC_select_menu(winid wid,
                     int how,
                     MENU_ITEM_P **menu_list)
{
  TRACE;  
  ScMenuWindow *menu = require_menu(wid,"select_menu");
  if (!menu) {
    return -1; 
  }
  int answer = menu->do_select_menu(how,menu_list);
  return answer;
}

char
SC_message_menu(char let,
                      int how,
                      const char *mesg)
{
  printf("SC_message_menu \n");
  // USe generic fallback. Probably good enough for now
  return genl_message_menu( let, how, mesg);
}

void
SC_mark_synch(void)
{
  // Don't care?
}

void
SC_wait_synch(void)
{
  // Don't care?
}

#ifdef CLIPPING
void
SC_cliparound(int x UNUSED,
                    int y UNUSED)
{
  printf("in SC_cliparound x=%d y=%d\n", x,y);
//  TODO;
}
#endif

void
SC_print_glyph(winid window,
                     coordxy x,
                     coordxy y,
                     const glyph_info *glyphinfo,
                     const glyph_info *bkglyphinfo)
{

  ScMapWindow *map_win = ctxt->find_map_window(window);
  if (!map_win) {    
    impossible("SC_print_glyph: map window expected");
    return;
  }  
  map_win->do_print_glyph(x,y,glyphinfo,bkglyphinfo);
}

void
SC_raw_print(const char *str UNUSED)
{
  puts(str);
}

void
SC_raw_print_bold(const char *str UNUSED)
{
  puts(str);
}

int
SC_nhgetch(void)
{
  TODO;
}

int
SC_nh_poskey(coordxy *x UNUSED,
              coordxy *y UNUSED,
              int *mod UNUSED)
{
  TODO;
}

void
SC_nhbell(void)
{
  TODO;
}

int
SC_doprev_message(void)
{
  TODO;
}

char
SC_yn_function(const char *question UNUSED,
                const char *choices UNUSED,
                char def UNUSED)
{
  TODO;
}

void
SC_getlin(const char *question UNUSED,
           char *input UNUSED)
{
  TODO;
}

int
SC_get_ext_cmd(void)
{
  TODO;
}

void
SC_number_pad(int state UNUSED)
{
  TODO;
}

void
SC_delay_output(void)
{
  TODO;
}

void
SC_start_screen(void)
{
  TODO;
}

void
SC_end_screen(void)
{
  TODO;
}

void
SC_outrip(winid window UNUSED,
           int how UNUSED,
           time_t when UNUSED)
{
  TODO;
}

void
SC_preference_update(const char *pref UNUSED)
{
  TODO;
}

char *SC_getmsghistory(boolean init)
{
  //TODO;
  return genl_getmsghistory(init);
}

void
SC_putmsghistory(const char *msg,
                       boolean is_restoring)
{
  //TODO;
  return genl_putmsghistory(msg,is_restoring);
}

void
SC_status_init(void)
{
  WARN_TODO;
  genl_status_init();
}

void
SC_status_finish(void)
{
  TODO;
  genl_status_finish();
}

void
SC_status_enablefield(int fieldidx,
                            const char *nm,
                            const char *fmt,
                            boolean enable)
{
  printf("SC_status_enablefield %d '%s' '%s' = %d\n", fieldidx, nm, fmt, int(enable));
  // TODO;
  // Call the generic version required by genl_status_update
  genl_status_enablefield(fieldidx,nm,fmt,enable);
}

void
SC_status_update(int fld,
                       genericptr_t ptr UNUSED,
                       int chg UNUSED,
                       int percent UNUSED,
                       int color UNUSED,
                       unsigned long *colormasks UNUSED)
{
  printf("SC_status_update %d %p\n", fld, ptr);
  genl_status_update(fld,ptr,chg,percent,color,colormasks);
}

void
SC_update_inventory(int arg UNUSED)
{
  TODO;
}

win_request_info *
SC_ctrl_nhwindow(winid window UNUSED,
                  int request UNUSED,
                  win_request_info *wri UNUSED)
{
  TODO;
}


struct window_procs SC_procs = {
  WPID(SC),
  (
    WC_COLOR
    | WC_HILITE_PET
    | WC_ASCII_MAP
    | WC_TILED_MAP
    /* | WC_PLAYER_SELECTION  --> obsolete? */
    /* | WC_PERM_INVENT */
    /* | WC_MOUSE_SUPPORT */
    ),
  (
    WC2_FLUSH_STATUS | WC2_SELECTSAVED
#ifdef STATUS_HILITES
    | WC2_RESET_STATUS | WC2_HILITE_STATUS
#endif
    | WC2_MENU_SHIFT 
    ),
  {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}, /* color availability */
  SC_init_nhwindows,
  SC_player_selection,
  SC_askname,
  SC_get_nh_event,
  SC_exit_nhwindows,
  SC_suspend_nhwindows,
  SC_resume_nhwindows,
  SC_create_nhwindow,
  SC_clear_nhwindow,
  SC_display_nhwindow,
  SC_destroy_nhwindow,
  SC_curs,
  SC_putstr,
  SC_putmixed,
  SC_display_file,
  SC_start_menu,
  SC_add_menu,
  SC_end_menu,
  SC_select_menu,
  genl_message_menu, /* no need for X-specific handling */
  SC_mark_synch,
  SC_wait_synch,
#ifdef CLIPPING
  SC_cliparound,
#endif
#ifdef POSITIONBAR
  donull,
#endif
  SC_print_glyph,
  SC_raw_print,
  SC_raw_print_bold,
  SC_nhgetch,
  SC_nh_poskey,
  SC_nhbell,
  SC_doprev_message,
  SC_yn_function,
  SC_getlin,
  SC_get_ext_cmd,
  SC_number_pad,
  SC_delay_output,
#ifdef CHANGE_COLOR /* only a Mac option currently */
  donull, donull,
#endif
  /* other defs that really should go away (they're tty specific) */
  SC_start_screen,
  SC_end_screen,
#ifdef GRAPHIC_TOMBSTONE
  SC_outrip,
#else
  genl_outrip,
#endif
  SC_preference_update,
  SC_getmsghistory,
  SC_putmsghistory,
#if 0
  SC_status_init,
  SC_status_finish,
  SC_status_enablefield,
  SC_status_update,
#else
  genl_status_init,
  genl_status_finish,
  genl_status_enablefield,
  genl_status_update,
#endif
  genl_can_suspend_no, /* XXX may not always be correct */
  SC_update_inventory,
  SC_ctrl_nhwindow,
};
