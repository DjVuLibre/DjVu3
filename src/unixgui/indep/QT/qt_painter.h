//C-  -*- C++ -*-
//C-
//C-  Copyright © 1999-2001, LizardTech, Inc. All Rights Reserved.
//C-              Unauthorized use prohibited.
//C-
// 
// $Id: qt_painter.h,v 1.1 2001-05-29 22:05:29 bcr Exp $
// $Name:  $


#ifndef HDR_QT_PAINTER
#define HDR_QT_PAINTER

#ifdef __GNUC__
#pragma interface
#endif

#include "GPixmap.h"
#include "GBitmap.h"
#include "GContainer.h"
#include "GRect.h"

#include <qpainter.h>

class QePainter : public QPainter
{
public:
   class PatchRect : public GPEnabled
   {
   public:
      GRect		rect;
      GP<GPixmap>	pixmap;

      PatchRect(const GRect & _rect, const GP<GPixmap> & _pixmap) :
	    rect(_rect), pixmap(_pixmap) {}
   };
   
   void		drawPixmap(const GRect &rect, GPixmap *pm,
			   int use_shm_extension=0);
   void		drawPixmap(const GRect &rect, int pm_x0, int pm_y0,
			   GPixmap *pm, int use_shm_extension=0);
   void		drawBitmap(const GRect &rect, GBitmap *bm,
			   int use_shm_extension=0);
   void		drawBitmap(const GRect &rect, int bm_x0, int bm_y0,
			   GBitmap *bm, int use_shm_extension=0);
   void		drawPatchedBitmap(const GRect & bm_rect, GBitmap * bm,
				  const GRect & pm_rect, GPixmap * pm,
				  int use_shm_extension=0);
   void		drawPatchedBitmaps(const GRect & bm_rect, GBitmap * bm,
				   const GPList<PatchRect> & pm_list,
				   int use_shm_extension=0);

      // Provide wrappers for standard drawPixmap()
   void drawPixmap(int x, int y, const QPixmap & pix,
		   int sx=0, int sy=0, int sw=-1, int sh=-1)
   {
      QPainter::drawPixmap(x, y, pix, sx, sy, sw, sh);
   }
   void drawPixmap(const QPoint & pnt, const QPixmap & pix, const QRect & sr)
   {
      QPainter::drawPixmap(pnt, pix, sr);
   }
   void drawPixmap(const QPoint & pnt, const QPixmap & pix)
   {
      QPainter::drawPixmap(pnt, pix);
   }
   
   QePainter(void) {}
   QePainter(const QPaintDevice * dev) : QPainter(dev) {}
   QePainter(const QPaintDevice * dev, const QWidget * w) :
	 QPainter(dev, w) {}
   ~QePainter(void) {}
};

#endif
