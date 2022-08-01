#include "winSC.h"
#include <assert.h>

ScMessageWindow::ScMessageWindow(ScContext &ctxt) :
  ScWindow(ctxt,NHW_MESSAGE,true)
{
}


void
ScMessageWindow::do_clear()
{
}

void
ScMessageWindow::do_display(bool blocking UNUSED)
{
}

