//C-  -*- C++ -*-
//C-
//C- DjVu® Unix Viewer (v. 3.5)
//C- 
//C- Copyright © 1999-2001 LizardTech, Inc. All Rights Reserved.
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
// $Id: qx_pnote.cpp,v 1.5 2001-10-17 19:03:18 docbill Exp $
// $Name:  $


#include "qx_pnote.h"
#include "exc_msg.h"
#include "debug.h"

#include <qapplication.h>

#include "qt_fix.h"

#include <X11/Xlib.h>
#ifdef USE_XSHAPE
#include <X11/extensions/shape.h>
#endif

int QxPopupNote::isShapeExtSupported(void)
{
#ifdef USE_XSHAPE
   int shape_event_base, shape_error_base;
   return XShapeQueryExtension(x11Display(), &shape_event_base, &shape_error_base);
#else
   return 0;
#endif
}

void QxPopupNote::gotX11Event(XEvent * event)
      // This slot-function is called whenever QApplication received an X11
      // event. We're interested in events coming to the decor_win (window
      // with decorations created by WM for the nearest-to-ref-widget-shell) and
      // to the common_pwin (common parent of this widget and of the decor_win -
      // actually a kind of RootWindow()). What we need from common_pwin is
      // the ConfigureNotify event selected by SubstructureNotifyMask
      // where we fix the stacking order.
{
   DEBUG_MSG("QxPopupNote::gotX11Event(): got smth interesting...\n");
   DEBUG_MAKE_INDENT(3);

   switch(event->type)
   {
      case UnmapNotify:
	 DEBUG_MSG("UnmapNotify\n");
	 if (event->xunmap.window==decor_win) hide();
	 break;

      case MapNotify:
	 DEBUG_MSG("MapNotify\n");
	 if (event->xmap.window==decor_win) show();
	 break;

      case ConfigureNotify:
	 DEBUG_MSG("ConfigureNotify\n");
	 if (event->xconfigure.window==decor_win)
	 {
	    moveNote(ref_pos.x()*ref_widget->width()/ref_size.width(),
		     ref_pos.y()*ref_widget->height()/ref_size.height());

	       // Change sibling
	    XWindowChanges attr;
	    attr.sibling=decor_win;
	    attr.stack_mode=Above;
	    XConfigureWindow(x11Display(), winId(), CWSibling | CWStackMode, &attr);
	 };

	 if (event->xconfigure.event==common_pwin)
	 {
	       // For some reason QT hates when it receives ConfigureEvents
	       // generated as a result of selection of SubstructureNotifyMask
	       // on the root window. All I can do here is to block them from
	       // further propagation.
	    ((QeApplication *) qApp)->x11EventResult=TRUE;
	    
	    if (event->xconfigure.window!=winId() &&
		event->xconfigure.window!=decor_win)
	    {
		  // Change sibling
	       XWindowChanges attr;
	       attr.sibling=decor_win;
	       attr.stack_mode=Above;
	       XConfigureWindow(x11Display(), winId(), CWSibling | CWStackMode, &attr);
	    };
	 };
	 break;
   };
}

QxPopupNote::QxPopupNote(const GPopupNote & note, QWidget * parent, const char * name) :
      QePopupNote(isShapeExtSupported(), note, parent, name),
      decor_win(0), common_pwin(0)
{
   DEBUG_MSG("QxPopupNote::QxPopupNote(): doing the relevant X11 stuff\n");
   DEBUG_MAKE_INDENT(3);

      // Changing the window to OverrideRedirect
   XSetWindowAttributes attr;
   attr.override_redirect=True;
   XChangeWindowAttributes(x11Display(), winId(), CWOverrideRedirect, &attr);

      // Now determine the decoration window (created by WM) for shell_win
   Window root, pwin, * child;
   u_int childs;
   if (XQueryTree(x11Display(), winId(), &root, &pwin, &child, &childs))
   {
      common_pwin=pwin;
      if (child) XFree(child);

	 // Now going up starting from parent's window to find a window with
	 // the same parent as the popup note's one
      pwin=parent->winId();
      Window win;
      do
      {
         win=pwin;
         if (!XQueryTree(x11Display(), win, &root, &pwin, &child, &childs)) break;
         if (child) XFree(child);
      } while(pwin!=common_pwin);
      if (pwin==common_pwin) decor_win=win;
   };

      // Make sure we know when the reference window is moved or resized
   XSelectInput(x11Display(), decor_win, StructureNotifyMask);

      // Honestly, common_pwin is very likely to be ROOT window, but I'm note
      // sure and don't want to check it out.
   
      // Select SubstructureNotify to catch changes in stacking order
   XSelectInput(x11Display(), common_pwin, SubstructureNotifyMask);

   if (!qApp->inherits("QeApplication"))
      throw ERROR_MESSAGE("QxPopupNote::QxPopupNote",
			  "Internal error: QeApplication should be used instead of QApplication.");
   connect(qApp, SIGNAL(gotX11Event(XEvent *)), this, SLOT(gotX11Event(XEvent *)));
}

QxPopupNote::~QxPopupNote(void)
{
      // It's important to stop XServer from sending us ConfigureEvents
      // designated for the ROOT window (common_pwin). Otherwise our filter
      // stops working (the object is being destroyed after all)
      // and QT gets really upset about unexpected events
   
   DEBUG_MSG("QxPopupNote::~QxPopupNote(): removing SubstructureNotifyMask\n");
   DEBUG_MAKE_INDENT(3);

   XWindowAttributes get_attr;
   XGetWindowAttributes(x11Display(), common_pwin, &get_attr);
   DEBUG_MSG("got root window attributes\n");

   XSetWindowAttributes set_attr;
   set_attr.event_mask=get_attr.your_event_mask & ~SubstructureNotifyMask;
   XChangeWindowAttributes(x11Display(), common_pwin, CWEventMask, &set_attr);
   DEBUG_MSG("changed root window attributes\n");
}
