//C-  -*- C++ -*-
//C-
//C-  Copyright © 1999-2001, LizardTech, Inc. All Rights Reserved.
//C-              Unauthorized use prohibited.
//C-
// 
// $Id: qd_painter.h,v 1.1 2001-05-29 22:05:31 bcr Exp $
// $Name:  $


#ifndef HDR_QD_PAINTER
#define HDR_QD_PAINTER

#ifdef __GNUC__
#pragma interface
#endif

#include "qt_painter.h"

class QDPainter : public QePainter
{
private:
   GP<GBitmap>	bmp;
public:
   void		drawDjVuWPaper(const GRect &rect, int x0, int y0);
   
   QDPainter(void) {}
   QDPainter(const QPaintDevice * dev) : QePainter(dev) {}
   QDPainter(const QPaintDevice * dev, const QWidget * w) :
	 QePainter(dev, w) {}
   ~QDPainter(void) {}
};

#endif
