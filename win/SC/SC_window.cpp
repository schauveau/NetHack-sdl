#include "winSC.h"
#include <assert.h>

// ===========================================

ScWindow::ScWindow(ScContext &ctxt, int type, bool unique)
: m_ctxt(ctxt), m_type(type), m_is_unique(unique)
{  
  
  // Find the number of null entries at the end of ctxt.m_windows
  size_t unused_count=0;
  for (auto it = ctxt.m_windows.rbegin() ; it!=ctxt.m_windows.rend() ; ++it) {
    if (*it == nullptr)
      unused_count++;
    else
      break;
  }

  // And place the newly created window at the end.
  if (unused_count==0) {
    ctxt.m_windows.push_back(this);
    assert(ctxt.m_windows.size() < 10000); // let's be reasonnable.
    m_winid = int(ctxt.m_windows.size())-1; 
  } else {
    assert(ctxt.m_windows.size() >= unused_count) ;
    m_winid = int(ctxt.m_windows.size()-unused_count);
    // TODO: Should we shink the vector when a lot of null elements are detected at the end?
    //       Is is possible to get a large number of windows? maybe with recursive menus.
  }
  ctxt.m_windows[m_winid] = this ;
  
}

ScWindow::~ScWindow()
{ 
  if ( m_ctxt.find_window(m_winid) == this ) 
    m_ctxt.m_windows[m_winid] = nullptr;
} 

bool
ScWindow::do_create()
{
  if (m_is_created) {
    return false;
  } else {
    m_is_created=true;
    return true;
  }
}
  
bool
ScWindow::do_destroy()
{
  if (m_is_created) {
    m_is_created = false;
    if (!m_is_unique) {
      delete this;
    }
    return true;
  } else {
    return false;
  }
}

void
ScWindow::do_putstr(int attr UNUSED, const char *str)
{
  // The default implementation does nothing.
  // -> should produce warning
  fprintf(stderr, "WARNING: default implementation of putstr called\n");
  fprintf(stderr, " -> '%s'\n",str);
  
}
