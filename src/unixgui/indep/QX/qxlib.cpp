//C-  -*- C++ -*-
//C-
//C- DjVu� Unix Viewer (v. 3.5)
//C- 
//C- Copyright � 1999-2001 LizardTech, Inc. All Rights Reserved.
//C- The DjVu Reference Library is protected by U.S. Pat. No.
//C- 6,058,214 and patents pending.
//C- 
//C- This software is subject to, and may be distributed under, the
//C- GNU General Public License, Version 2. The license should have
//C- accompanied the software or you may obtain a copy of the license
//C- from the Free Software Foundation at http://www.fsf.org .
//C- 
//C- The computer code originally released by LizardTech under this
//C- license and unmodified by other parties is deemed the "LizardTech
//C- Original Code."
//C- 
//C- With respect to the LizardTech Original Code ONLY, and subject
//C- to any third party intellectual property claims, LizardTech
//C- grants recipient a worldwide, royalty-free, non-exclusive license
//C- under patent claims now or hereafter owned or controlled by
//C- LizardTech that are infringed by making, using, or selling
//C- LizardTech Original Code, but solely to the extent that any such
//C- patent(s) is/are reasonably necessary to enable you to make, have
//C- made, practice, sell, or otherwise dispose of LizardTech Original
//C- Code (or portions thereof) and not to any greater extent that may
//C- be necessary to utilize further modifications or combinations.
//C- 
//C- The LizardTech Original Code is provided "AS IS" WITHOUT WARRANTY
//C- OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED
//C- TO ANY WARRANTY OF NON-INFRINGEMENT, OR ANY IMPLIED WARRANTY OF
//C- MERCHANTIBILITY OR FITNESS FOR A PARTICULAR PURPOSE.
//C-
// 
// $Id: qxlib.cpp,v 1.3 2001-10-12 17:58:31 leonb Exp $
// $Name:  $

#ifdef HAVE_CONFIG_H
#include "config.h"
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
