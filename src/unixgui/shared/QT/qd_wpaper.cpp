//C-  -*- C++ -*-
//C-
//C-  Copyright © 1999-2001, LizardTech, Inc. All Rights Reserved.
//C-              Unauthorized use prohibited.
//C-
// 
// $Id: qd_wpaper.cpp,v 1.1 2001-05-29 22:05:31 bcr Exp $
// $Name:  $


#ifdef __GNUC__
#pragma implementation
#endif

#include "qd_wpaper.h"

#include "qd_painter.h"
#include "qlib.h"

#include "qt_fix.h"

void QDWPaper::paintEvent(QPaintEvent * ev)
{
   try
   {
      QDPainter p(this);
      p.drawDjVuWPaper(Q2G(ev->rect()), 0, 0);
      if (text.length())
      {
	 p.setFont(QFont("Helvetica", 18));
	 p.drawText(0, 0, width(), height(), AlignCenter, text);
      }
      p.end();
   } catch(const GException & exc)
   {
      showError(this, "DjVu Error", exc);
   }
}

QDWPaper::QDWPaper(QWidget * parent, const char * name) :
      QWidget(parent, name)
{
   setBackgroundColor(white);
}
