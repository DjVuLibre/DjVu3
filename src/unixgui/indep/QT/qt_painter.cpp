//C-  -*- C++ -*-
//C-
//C-  Copyright © 1999-2001, LizardTech, Inc. All Rights Reserved.
//C-              Unauthorized use prohibited.
//C-
// 
// $Id: qt_painter.cpp,v 1.1 2001-05-29 22:05:29 bcr Exp $
// $Name:  $


#ifdef __GNUC__
#pragma implementation
#endif

#include "qt_painter.h"
#include "exc_msg.h"

#ifdef UNIX
#include "qx_imager.h"
#else
// "You got to include smth like qw_imager.h here"
#endif

void
QePainter::drawPixmap(const GRect & irect, GPixmap *pm,
		      int use_shm_extension)
{
   GRect ph_rect=irect;
   QPoint pnt=xForm(QPoint(0, 0));
   ph_rect.translate(pnt.x(), pnt.y());

#ifdef UNIX
   if (!qxImager)
      throw ERROR_MESSAGE("QePainter::drawPixmap",
			  "Can't draw pixmap since QXImager has not been initialized yet.");

   qxImager->displayPixmap((u_long) hd, gc,
			   ph_rect, pm, use_shm_extension);
#else
   throw ERROR_MESSAGE("QePainter::drawPixmap",
                       "Implement QePainter for windows\n");
#endif
}

void
QePainter::drawPixmap(const GRect &irect, int pm_x0, int pm_y0,
		      GPixmap *pm, int use_shm_extension)
{
   GRect ph_rect=irect;
   QPoint pnt=xForm(QPoint(0, 0));
   ph_rect.translate(pnt.x(), pnt.y());
   
#ifdef UNIX
   if (!qxImager)
      throw ERROR_MESSAGE("QePainter::drawPixmap",
			  "Can't draw pixmap since QXImager has not been initialized yet.");
   
   qxImager->displayPixmap((u_long) hd, gc,
			   ph_rect, pm_x0, pm_y0, pm, use_shm_extension);
#else
   throw ERROR_MESSAGE("QePainter::drawPixmap",
                       "Implement QePainter for windows\n");
#endif
}

void
QePainter::drawBitmap(const GRect &irect, GBitmap *bm,
		      int use_shm_extension)
{
   GRect ph_rect=irect;
   QPoint pnt=xForm(QPoint(0, 0));
   ph_rect.translate(pnt.x(), pnt.y());
   
#ifdef UNIX
   if (!qxImager)
      throw ERROR_MESSAGE("QePainter::drawBitmap",
			  "Can't draw bitmap since QXImager has not been initialized yet.");
   
   qxImager->displayBitmap((u_long) hd, gc,
			   ph_rect, bm, use_shm_extension);
#else
   throw ERROR_MESSAGE("QePainter::drawBitmap",
                       "Implement QePainter for windows\n");
#endif
}

void
QePainter::drawBitmap(const GRect &irect, int bm_x0, int bm_y0,
		      GBitmap *bm, int use_shm_extension)
{
   GRect ph_rect=irect;
   QPoint pnt=xForm(QPoint(0, 0));
   ph_rect.translate(pnt.x(), pnt.y());
   
#ifdef UNIX
   if (!qxImager)
      throw ERROR_MESSAGE("QePainter::drawBitmap",
			  "Can't draw bitmap since QXImager has not been initialized yet.");
   
   qxImager->displayBitmap((u_long) hd, gc,
			   ph_rect, bm_x0, bm_y0, bm, use_shm_extension);
#else
   throw ERROR_MESSAGE("QePainter::drawBitmap",
                       "Implement QePainter for windows\n");
#endif
}

void
QePainter::drawPatchedBitmap(const GRect & bm_rect, GBitmap * bm,
			     const GRect & pm_rect, GPixmap * pm,
			     int use_shm_extension)
{
   GPList<PatchRect> list;
   list.append(new PatchRect(pm_rect, pm));
   drawPatchedBitmaps(bm_rect, bm, list, use_shm_extension);
}

void
QePainter::drawPatchedBitmaps(const GRect & bm_rect, GBitmap * bm,
			      const GPList<PatchRect> & pm_list,
			      int use_shm_extension)
{
#ifdef UNIX
   if (!qxImager)
      throw ERROR_MESSAGE("QePainter::drawPatchedBitmap",
			  "Can't draw bitmap since QXImager has not been initialized yet.");

   QPoint pnt=xForm(QPoint(0, 0));

   GRect bmh_rect=bm_rect;
   bmh_rect.translate(pnt.x(), pnt.y());
   
   GPList<QXImager::PatchRect> list;
   for(GPosition pos=pm_list;pos;++pos)
   {
      PatchRect & p=*pm_list[pos];
      GP<QXImager::PatchRect> prect=new QXImager::PatchRect(p.rect, p.pixmap);
      prect->rect.translate(pnt.x(), pnt.y());
      list.append(prect);
   }
   
   qxImager->displayPatchedBitmaps((u_long) hd, gc, bmh_rect, bm,
				   list, use_shm_extension);
#else
   throw ERROR_MESSAGE("QePainter::drawPatchedBitmap",
                       "Implement QePainter for windows\n");
#endif
}

