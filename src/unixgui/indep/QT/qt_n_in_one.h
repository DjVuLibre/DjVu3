//C-  -*- C++ -*-
//C-
//C-  Copyright © 1999-2001, LizardTech, Inc. All Rights Reserved.
//C-              Unauthorized use prohibited.
//C-
// 
// $Id: qt_n_in_one.h,v 1.1 2001-05-29 22:05:29 bcr Exp $
// $Name:  $


#ifndef HDR_QT_N_IN_ONE
#define HDR_QT_N_IN_ONE

#ifdef __GNUC__
#pragma interface
#endif

#include <qobjectlist.h>
#include <qwidget.h>

#include "qt_fix.h"

#include "debug.h"

class QeNInOne : public QWidget
{
   Q_OBJECT
private:
   QWidget	* activeWidget;
   int		resizable;	// If to change min/max size when new w is active

      // Returns the same (if valid) or the last widget from the children list
      // E.g. If w==0 then the result will ne non-null if there is at least
      // one child available.
   QWidget	* checkWidget(QWidget * w);

      // Recomputes min/max sizes. Used only when resizable is FALSE
   void		recomputeMinMax(void);
   
      // Checks and maybe changes the activeWidget
   void		checkActiveWidget(void)
   {
      QWidget * w=checkWidget(activeWidget);
      if (w!=activeWidget) setActiveWidget(w);
   };
protected:
   virtual void		resizeEvent(QResizeEvent * ev)
   {
      checkActiveWidget();
      if (activeWidget && ev->size().width() && ev->size().height())
	 activeWidget->resize(ev->size());
   };
   virtual bool		event(QEvent * ev);
public:
   virtual void	hide(void);
   virtual void	show(void);
	 
   QeNInOne(QWidget * parent=0, const char * name=0) :
	 QWidget(parent, name), activeWidget(0), resizable(0) {};

   void		setActiveWidget(QWidget * new_w);
   QWidget *	getActiveWidget(void) const { return activeWidget; };
   
   void		dontResize(bool dr)
   {
      resizable=!dr;
   };
};

#endif
