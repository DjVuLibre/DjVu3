//C-  -*- C++ -*-
//C-
//C-  Copyright © 1999-2001, LizardTech, Inc. All Rights Reserved.
//C-              Unauthorized use prohibited.
//C-
// 
// $Id: qd_pane.h,v 1.1 2001-05-29 22:05:31 bcr Exp $
// $Name:  $


#ifndef HDR_QD_PANE
#define HDR_QD_PANE

#ifdef __GNUC__
#pragma interface
#endif

#include "GRect.h"
#include "GContainer.h"

#include <qwidget.h>
#include <qregion.h>
#include <qtimer.h>

#include "qt_fix.h"

// The purpose of this class it to do the following things:
//    1. Override some virtual functions, which cause repaint when I don't
//       need it at all (like when I change backgroundPixmap)
//    2. Provide scroll(dx, dy) for UNIX/X11, which will work better than QT

class QDPane : public QWidget, public GPEnabled
{
   Q_OBJECT
private:
#ifdef UNIX
   GC		gc;
   // Stuff needed to enable smooth scrolling (w/o excessive XSync())
#define SHIFT_EXPOSE_QUEUE_SIZE	512
   
   int		shift_expose_by_dx[SHIFT_EXPOSE_QUEUE_SIZE];
   int		shift_expose_by_dy[SHIFT_EXPOSE_QUEUE_SIZE];
   int		shift_expose_cnt;
#endif
   QRegion	invalid_region;
   GList<GRect>	invalid_rects;
   QTimer	timer;

   void		repaintRegion(void);
private slots:
   void		slotTimeout(void);
protected:
      // These two result in the window update after scrolling when I reset
      // the pane's mode from NoBackground to FixedPixmap
   virtual void	backgroundColorChange(const QColor &) {};
   virtual void	backgroundPixmapChange(const QPixmap &) {};

      // These two repaint the pane when mouse just enters or leaves the window
      // How sweet...
   virtual void	focusInEvent(QFocusEvent *) {};
   virtual void	focusOutEvent(QFocusEvent *) {};

#ifdef UNIX
   virtual bool	x11Event(XEvent * ev);
#endif
public:
#ifdef UNIX
   void		scroll(int dx, int dy);
#endif

   bool		canScrollMore(void) const { return shift_expose_cnt<=1; }
   
   QDPane(QWidget * parent=0, const char * name=0);
   ~QDPane(void);
};

#endif
