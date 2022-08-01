#include "winSC.h"
#include <assert.h>


ScMenuWindow::ScMenuWindow(ScContext &ctxt)
  : ScWindow(ctxt,NHW_MENU,false)
{
} 

void
ScMenuWindow::do_start_menu(unsigned long behavior UNUSED)
{
  if (m_state != MS_EMPTY) {
    // Each menu should only be started once ... I think.
    impossible( "start_menu: called on an already started menu");
    return;
  } 
  m_state = MS_STARTED;
  m_accel_count = 0;  
}

void
ScMenuWindow::do_add_menu(const glyph_info *glyphinfo UNUSED,
                          const ANY_P *identifier,
                          char accel,
                          char group_accel,
                          int attr,
                          int color UNUSED,
                          const char *str,
                          unsigned int itemflags)
{
  Item item ;

  if (m_state != MS_STARTED) {
    // Each menu should only be started once ... I think.
    impossible( "add_menu: unexpected call");
    return;
  }

  if (identifier->a_void) {
    // A selectable entry
    if (accel==0) 
      accel = this->get_next_accel(); 
  } else {
    // A non-selectable entry such as a title or a helper text.
    // NOTE: Some menus may provide entries with a multi-lines helper text
    //       Only the first line is selectable while the other ones are not.
    //       Is there a clean way to group them together?
    //       See handler_menustyle() for an example.
    // 
    accel=0;
    group_accel=0;
  }
  
  // TODO item.glyphinfo  = gqlyphinfo;
  item.identifier = *identifier;
  item.accel = accel;
  item.group_accel = group_accel;
  item.attribute = attr;
  // TODO item.color = color;
  item.str = str;
  item.itemflags = itemflags;
  
  m_items.push_back(item);
}

void
ScMenuWindow::do_end_menu(const char *query UNUSED)
{
  if (m_state != MS_STARTED) {
    // Each menu should only be started once ... I think.
    impossible( "end_menu: unexpected call");
    return;
  }

  m_state = MS_ENDED;
}

int
ScMenuWindow::do_select_menu(int how  UNUSED,
                             MENU_ITEM_P **menu_list UNUSED)
{
  int nsel ; 
  if (m_state != MS_ENDED) {
    // Each menu should only be started once ... I think.
    impossible( "select_menu: unexpected call");
    return -1;
  }
  m_state = MS_SELECTING;

  fprintf(stderr, "in ScMenuWindow::do_select_menu\n");
  nsel = 0;

  if (true) {
    return -1;
  }
  
  m_state = MS_DONE;

  return nsel;
}

void
ScMenuWindow::do_clear()
{
}

void
ScMenuWindow::do_display(bool blocking UNUSED)
{
}

char
ScMenuWindow::get_next_accel()
{ 
  if ( m_accel_count == 2*26 )
    return 0 ; // no more  
  int n = m_accel_count++; 
  if (n<26)
    return 'a'+n ;
  else
    return 'A'+n ;
}
