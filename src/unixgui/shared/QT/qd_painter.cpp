//C-  -*- C++ -*-
//C-
//C-  Copyright © 1999-2001, LizardTech, Inc. All Rights Reserved.
//C-              Unauthorized use prohibited.
//C-
// 
// $Id: qd_painter.cpp,v 1.1 2001-05-29 22:05:31 bcr Exp $
// $Name:  $


#ifdef __GNUC__
#pragma implementation
#endif

#include "qd_painter.h"
#include "debug.h"
#include "qlib.h"
#include "cin_data.h"

#include <qpixmap.h>

void
QDPainter::drawDjVuWPaper(const GRect &rect, int x0, int y0)
{
   DEBUG_MSG("QDPainter::drawDjVuWPaper(): Displaying DejaVu wallpaper\n");
   DEBUG_MAKE_INDENT(3);

   if (!bmp || !bmp->rows()) bmp=GBitmap::create(*CINData::get("ppm_djvu_logo"));

   QPixmap qpix(bmp->columns(), bmp->rows());
   QePainter painter(&qpix);
   painter.drawBitmap(GRect(0, 0, qpix.width(), qpix.height()), bmp);
   painter.end();

   save();
   try
   {
      QPoint pnt=xForm(QPoint(0, 0));
      GRect ph_rect=rect; ph_rect.translate(pnt.x(), pnt.y());
      setClipRect(G2Q(ph_rect));
      setClipping(TRUE);
      fillRect(G2Q(rect), white);
      
	 // Display bitmap
      int  m = 40;
      int  xs = x0 + m - 1;
      for (int y=y0; y<rect.ymax; y+=qpix.height()+m)
      {
	 for (int x=xs; x<rect.xmax; x+=qpix.width()+m)
	    if (x+qpix.width()>rect.xmin && y+qpix.height()>=rect.ymin)
	       drawPixmap(x, y, qpix);
	 xs += qpix.width() * 3 / 5;
	 while (xs - m >= x0)
	    xs = xs - qpix.width() - m;
      }
   } catch(...)
   {
      restore();
      throw;
   }
   restore();
}
