//C-  -*- C++ -*-
//C-
//C-  Copyright © 1999-2001, LizardTech, Inc. All Rights Reserved.
//C-              Unauthorized use prohibited.
//C-
// 
// $Id: qxlib.cpp,v 1.1 2001-05-29 22:05:29 bcr Exp $
// $Name:  $


#ifdef __GNUC__
#pragma implementation
#endif

#include "qxlib.h"
#include "debug.h"
#include "exc_misc.h"

#include <X11/Xlib.h>

void x11Redraw(QWidget * w, const QRect * rect=0)
{
   QRect qrect;
   if (rect) qrect=*rect;
   else qrect=w->geometry();
   
   XEvent event;
   memset(&event, 0, sizeof(event));
   event.type=Expose;
   event.xexpose.send_event=True;
   event.xexpose.display=w->x11Display();
   event.xexpose.window=(Window) w->handle();
   event.xexpose.x=qrect.x();
   event.xexpose.y=qrect.y();
   event.xexpose.width=qrect.width();
   event.xexpose.height=qrect.height();
   XSendEvent(w->x11Display(), (Window) w->handle(), False, ExposureMask, &event);
}

u_long x11GetTopLevelWindow(void * _displ, u_long _start)
{
   DEBUG_MSG("x11GetTopLevelWindow(): traversing the tree up to reach the shell...\n");
   DEBUG_MAKE_INDENT(3);

   Display * displ=(Display *) _displ;
   Window ref_window=(Window) _start;
   
   if (!displ || !ref_window)
      throw BAD_ARGUMENTS("x11GetTopLevelWindow",
			  "Internal error: ZERO display or window passed as input.");
   
   Atom atom_wm=XInternAtom(displ, "WM_STATE", True);
   if (!atom_wm) return 0;

   Window shell_win=0;
   Window cur_win=ref_window;
   while(1)
   {
      DEBUG_MSG("looking at window=" << cur_win << "\n");
      
      int props;
      Atom * prop=XListProperties(displ, cur_win, &props);
      if (prop && props)
      {
	 int i;
	 for(i=0;i<props;i++)
	    if (prop[i]==atom_wm) break;
	 XFree(prop);
	 if (i<props)
	 {
	    shell_win=cur_win;
	    break;
	 }
      }
      
      Window root, parent, * child;
      u_int childs;
      if (XQueryTree(displ, cur_win, &root, &parent, &child, &childs))
      {
	 if (child) XFree(child);
	 if (parent==root)
	 {
	    shell_win=cur_win;
	    break;
	 }
	 
	 cur_win=parent;
      }
   }
   DEBUG_MSG("got window=" << shell_win << "\n");
   return shell_win;
}
