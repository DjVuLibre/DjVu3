//C-  -*- C++ -*-
//C-
//C-  Copyright � 1999-2001, LizardTech, Inc. All Rights Reserved.
//C-              Unauthorized use prohibited.
//C-
// 
// $Id: qd_pane.cpp,v 1.2 2001-06-06 14:53:58 mchen Exp $
// $Name:  $


#ifdef __GNUC__
#pragma implementation
#endif

#include "qd_pane.h"
#include "debug.h"
#include "qlib.h"
#include "GContainer.h"

#include <qapplication.h>

#include <X11/Xlib.h>

#ifndef QT1
#include <q1xcompatibility.h>
#endif


#ifdef UNIX
bool
QDPane::x11Event(XEvent * ev)
{
      // We look at Expose-related events in this function trying to do
      // simple events compression (when one rectangle is completely inside
      // another). Another thing is to keep track of NoExpose and GraphicsExpose
      // events generated as a result of scrolling. We don't trust QT :)
   try
   {
      if (ev->type==MotionNotify &&
	  (ev->xmotion.state==Button1Mask ||
	   ev->xmotion.state==0))
      {
	    // We're scrolling the window contents. Smarty QT does some
	    // event compression in this case: it looks ahead in the
	    // event queue for other MotionNotify events. Unfortunately it
	    // skips ButtonRelease event, which results in the fact, that
	    // I stop scrolling too late. This is very annoying if you
	    // scroll fast

	    // Do event compression ourselves

	 QPoint point(ev->xmotion.x, ev->xmotion.y);
	 int ev_state=ev->xmotion.state;
	 int ev_mask=ev_state ? Button1MotionMask : PointerMotionMask;
	 while(XCheckWindowEvent(ev->xmotion.display, ev->xmotion.window,
				 ev_mask | ButtonPressMask | ButtonReleaseMask, ev))
	 {
	    if (ev->type==ButtonPress || ev->type==ButtonRelease)
	    {
	       XPutBackEvent(ev->xbutton.display, ev);
	       point=QPoint(ev->xbutton.x, ev->xbutton.y);
	       break;
	    } else point=QPoint(ev->xmotion.x, ev->xmotion.y);
	 }
	 
	 int qstate=0;
	 if (ev_state & Button1Mask) qstate|=LeftButton;
	 if (ev_state & Button2Mask) qstate|=MidButton;
	 if (ev_state & Button3Mask) qstate|=RightButton;
	 if (ev_state & ShiftMask) qstate|=ShiftButton;
	 if (ev_state & ControlMask) qstate|=ControlButton;
	 if (ev_state & Mod1Mask) qstate|=AltButton;
	 QMouseEvent qev(Event_MouseMove, point, NoButton, qstate);
	 QApplication::sendEvent(this, &qev);
	 return TRUE;
	 
      }
      if (ev->type==Expose || ev->type==GraphicsExpose || ev->type==NoExpose)
      {
	    // Process Expose, GraphicsExpose and NoExpose by modifying
	    // the 'shift_expose_...' stuff and updating the invalid_region
	    // and invalid_rects.

	    // We cope both with invalid_region and invalid_rects because
	    // in some situations the Region 'optimizes' the area so nice,
	    // that the number of rectangles actually doubles, in which
	    // case we just want to draw what we received originally from X11
	 
	 GList<void *> event_list;
	 XEvent event=*ev;

	 bool in_scroll=shift_expose_cnt>0;
	 
	 do
	 {
	    if (event.type==Expose)
	    {
	       int x, y, width, height;
	       x=event.xexpose.x+shift_expose_by_dx[shift_expose_cnt];
	       y=event.xexpose.y+shift_expose_by_dy[shift_expose_cnt];
	       width=event.xexpose.width;
	       height=event.xexpose.height;

	       invalid_rects.append(GRect(x, y, width, height));
	       invalid_region=invalid_region.unite(QRegion(x, y, width, height));
	       if (!timer.isActive()) timer.start(0, TRUE);
	    } else if (event.type==GraphicsExpose)
	    {
	       int x, y, width, height;
	       x=event.xgraphicsexpose.x+
		 shift_expose_by_dx[shift_expose_cnt>0 ? shift_expose_cnt-1 : 0];
	       y=event.xgraphicsexpose.y+
		 shift_expose_by_dy[shift_expose_cnt>0 ? shift_expose_cnt-1 : 0];
	       width=event.xgraphicsexpose.width;
	       height=event.xgraphicsexpose.height;

	       if (!event.xgraphicsexpose.count)
		  if (shift_expose_cnt>0) shift_expose_cnt--;

	       invalid_rects.append(GRect(x, y, width, height));
	       invalid_region=invalid_region.unite(QRegion(x, y, width, height));
	       if (!timer.isActive()) timer.start(0, TRUE);
	    } else if (event.type==NoExpose)
	    {
	       if (shift_expose_cnt>0) shift_expose_cnt--;
	    } else
	    {
	       XEvent * tmp_event=new XEvent; *tmp_event=event;
	       event_list.append(tmp_event);
	       continue;
	    }

	       // If we are in scroll the limit the rectangle compression
	       // because otherwise the results will be ugly though fast.
	    if (in_scroll)
	       if (invalid_rects.size()>=2) repaintRegion();
	 } while(XCheckWindowEvent(x11Display(), winId(), ExposureMask, &event));

	    // WARNING: Reversed order is important.
	 for(GPosition pos=event_list.lastpos();pos;--pos)
	 {
	    XPutBackEvent(x11Display(), (XEvent *) event_list[pos]);
	    delete (XEvent *) event_list[pos];
	 }

	 return TRUE;	// Stop event propagation
      }
   } catch(const GException & exc)
   {
      showError(this, tr("DjVu Error"), exc);
   }
   return FALSE;
}

void
QDPane::scroll(int dx, int dy)
{
   DEBUG_MSG("QDPane::scroll(): moving by (" << dx << ", " << dy << ")\n");
   DEBUG_MAKE_INDENT(3);
   
   if (dx || dy)
   {
      int x0=dx>0 ? 0 : -dx;
      int y0=dy>0 ? 0 : -dy;
      int width=QWidget::width()-abs(dx);
      int height=QWidget::height()-abs(dy);
      XCopyArea(x11Display(), winId(), winId(), gc, x0, y0, width, height, x0+dx, y0+dy);
      if (shift_expose_cnt+1<SHIFT_EXPOSE_QUEUE_SIZE)
      {
	 for(int i=shift_expose_cnt+1;i>1;i--)
	 {
	    shift_expose_by_dx[i]=shift_expose_by_dx[i-1]+dx;
	    shift_expose_by_dy[i]=shift_expose_by_dy[i-1]+dy;
	 }
	 shift_expose_by_dx[1]=dx;
	 shift_expose_by_dy[1]=dy;
	 shift_expose_cnt++;
      }
      invalid_region.translate(dx, dy);
      for(GPosition pos=invalid_rects;pos;++pos)
	 invalid_rects[pos].translate(dx, dy);
      
	 // Perform manual update
      GRect grectBand=Q2G(geometry());
      if (dx>0) grectBand.xmax=grectBand.xmin+dx;
      else grectBand.xmin=grectBand.xmax+dx;
      repaint(G2Q(grectBand), FALSE);
      invalid_region=invalid_region.subtract(G2Q(grectBand));
      
      grectBand=Q2G(geometry());
      if (dy>0) grectBand.ymax=grectBand.ymin+dy;
      else grectBand.ymin=grectBand.ymax+dy;
      repaint(G2Q(grectBand), FALSE);
      invalid_region=invalid_region.subtract(G2Q(grectBand));
   }
}

void
QDPane::repaintRegion(void)
{
   timer.stop();
   if (!invalid_region.isEmpty())
   {
      QArray<QRect> rects=invalid_region.rects();

	 // See if we can merge some by inflating
      QRegion reg;
      for(u_int i=0;i<rects.size();i++)
      {
	 GRect rect=Q2G(rects[i]);
	 rect.inflate(16, 16);
	 reg=reg.unite(G2Q(rect));
      }
      reg=reg.intersect(invalid_region.boundingRect());
      QArray<QRect> rects1=reg.rects();
      if (rects1.size()<rects.size()) rects=rects1;

      bool done=false;
      if (rects.size()>1)
      {
	    // See if we can replace all of them by the bounding rectangle
	 int total_area=0;
	 for(u_int i=0;i<rects.size();i++)
	 {
	    QRect r=rects[i];
	    total_area+=r.width()*r.height();
	 }
	 QRect brect=reg.boundingRect();
	 if (total_area>=brect.width()*brect.height()*3/4)
	 {
	    repaint(brect, FALSE);
	    done=true;
	 }
      }
      if (!done)
      {
	 if (invalid_rects.size()<(int) rects.size())
	 {
	       // In some cases QRegion does BAD job in optimizing the area
	    for(GPosition pos=invalid_rects;pos;++pos)
	       repaint(G2Q(invalid_rects[pos]), FALSE);
	 } else
	 {
	    for(u_int i=0;i<rects.size();i++)
	       repaint(rects[i], FALSE);
	 }
      }
   }
   invalid_region=QRegion();
   invalid_rects.empty();
}

void
QDPane::slotTimeout(void)
{
   repaintRegion();
}

#endif

QDPane::QDPane(QWidget * parent, const char * name) :
      QWidget(parent, name)
{
   DEBUG_MSG("QDPane::QDPane(): Initializing class\n");
   DEBUG_MAKE_INDENT(3);

#ifdef UNIX
   shift_expose_cnt=0;
   shift_expose_by_dx[0]=0;
   shift_expose_by_dy[0]=0;

   XGCValues values;
   values.graphics_exposures=True;
   gc=XCreateGC(x11Display(), winId(), GCGraphicsExposures, &values);

   XSetWindowAttributes attr;
   attr.backing_store=WhenMapped;
   XChangeWindowAttributes(x11Display(), winId(), CWBackingStore, &attr);

   setWFlags(WPaintClever);	// Prevent QT from "optimization"
   setWFlags(WResizeNoErase);	// New QT tries to be smart and repaint()s
   				// the window itself, which is NOT what I want
   				// I want event compression.

   connect(&timer, SIGNAL(timeout(void)), this, SLOT(slotTimeout(void)));
#endif
}

QDPane::~QDPane(void)
{
   DEBUG_MSG("QDPane::~QDPane(): destroying class...\n");
#ifdef UNIX
   XFreeGC(x11Display(), gc);
#endif
}
