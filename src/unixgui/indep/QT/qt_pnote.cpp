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
// $Id: qt_pnote.cpp,v 1.4 2001-10-16 18:01:45 docbill Exp $
// $Name:  $


static char RCSVersion[]="@(#) $Id: qt_pnote.cpp,v 1.4 2001-10-16 18:01:45 docbill Exp $";

#ifdef __GNUC__
#pragma implementation
#endif

#include "qt_pnote.h"
#include "exc_misc.h"
#include "qt_imager.h"
#include "debug.h"
#include "GOS.h"

#include <qpushbutton.h>
#include <qbitmap.h>
#include <qpainter.h>
#include <qcolor.h>
#include <qpalette.h>
#include <qlayout.h>
#include <qapplication.h>

#include "qt_fix.h"

#define SIDE_MARGIN	10

#define BUBBLES		4
#define BUBBLES_SPEED	100

void QePopupNote::getBubbleCoords(int bubble, int * x, int * y, int * w, int * h)
{
   if (bubble<0) bubble=0;

   int a_x=0, a_y=0, a_w=contents->width(), a_h=contents->height();
   for(int i=0;i<=bubble;i++)
   {
      a_x=a_x+a_w/6;
      a_y=a_y+a_h+5;
      a_w=a_w/2;
      a_h=a_h/2;
   };
   if (x) *x=a_x;
   if (y) *y=a_y;
   if (w) *w=a_w;
   if (h) *h=a_h;
}

int QePopupNote::getBottomMargin(void)
{
   if (!shape_ext_supported) return 0;

   int y, h;
   getBubbleCoords(BUBBLES-1, 0, &y, 0, &h);
   
   return y+h+5;
}
      
void QePopupNote::setBubbleMask(int bubble)
{
   DEBUG_MSG("QePopupNote::setBubbleMask(): Setting mask for bubble=" << bubble << "\n");
   DEBUG_MAKE_INDENT(3);

   if (!shape_ext_supported) return;
   
   if (bubble>BUBBLES) bubble=BUBBLES;
   
      // Create the mask
   QBitmap bmp=QBitmap(width(), height(), TRUE);

   QPainter p(&bmp);
   p.setBrush(white);
      // Draw the mask
   if (bubble==BUBBLES)
   {
	 // Body with text
	 // TODO: Try drawRoundRect()
      p.drawPie(0, 0, 2*SIDE_MARGIN, 2*SIDE_MARGIN, 0, 360*16);
      p.drawPie(width()-2*SIDE_MARGIN, 0, 2*SIDE_MARGIN,
		2*SIDE_MARGIN, 0, 360*16);
      p.drawPie(width()-2*SIDE_MARGIN, contents->height()-2*SIDE_MARGIN,
		2*SIDE_MARGIN, 2*SIDE_MARGIN, 0, 360*16);
      p.drawPie(0, contents->height()-2*SIDE_MARGIN,
		2*SIDE_MARGIN, 2*SIDE_MARGIN, 0, 360*16);
      p.drawRect(SIDE_MARGIN, 0, width()-2*SIDE_MARGIN, contents->height());
      p.drawRect(0, SIDE_MARGIN, width(), contents->height()-2*SIDE_MARGIN);
   };
   for(int i=BUBBLES-1-bubble;i<BUBBLES;i++)
   {
      int x, y, w, h;
      getBubbleCoords(i, &x, &y, &w, &h);
      p.drawPie(x, y, w, h, 0, 360*16);
   };

   p.end();

   // Set the mask
   setMask(bmp);
}

void QePopupNote::getAnchorPos(int * x, int * y)
{
   int w=width(), h=height();
   *x=w/3; *y=h+20;
   if (shape_ext_supported)
   {
      int a_x, a_y, a_w, a_h;
      getBubbleCoords(BUBBLES, &a_x, &a_y, &a_w, &a_h);
      *x=a_x+a_w/2;
      *y=a_y+a_h/2;
   };
}

void QePopupNote::moveNote(int x, int y)
      // (x, y) here are relative to parent widget
{
   DEBUG_MSG("QePopupNote::moveNote(): positioning the message\n");
   DEBUG_MAKE_INDENT(3);

   ref_pos=QPoint(x, y);
   ref_size=ref_widget->size();
   QPoint abs_pos=ref_widget->mapToGlobal(ref_pos);
   
   int ax, ay;
   getAnchorPos(&ax, &ay);
   abs_pos-=QPoint(ax, ay);
   if (abs_pos.x()<0) abs_pos.setX(0);
   if (abs_pos.y()<0) abs_pos.setY(0);

   move(abs_pos);
}

void QePopupNote::showNote(int x, int y)
      // The note is supposed to be positioned already with respect to
{
   DEBUG_MSG("QePopupNote::showNote(): showing the note\n");
   DEBUG_MAKE_INDENT(3);

   moveNote(x, y);

   if (shape_ext_supported)
   {
      setBubbleMask(0);
      show();
      
      for(int bubble=1;bubble<=BUBBLES;bubble++)
      {
	 GOS::sleep(BUBBLES_SPEED);
	 setBubbleMask(bubble);
	 qApp->processEvents(BUBBLES_SPEED/2);
      };
   } else show();
}

QePopupNote::QePopupNote(int _shape_ext_supported,
			 const GPopupNote & gnote, QWidget * parent,
			 const char * name) :
      QDialog(0, name, FALSE), GPopupNote(gnote), ref_widget(parent),
      shape_ext_supported(_shape_ext_supported)
{
   DEBUG_MSG("QePopupNote::QePopupNote(): initializing class\n");
   DEBUG_MAKE_INDENT(3);

   if (!parent) throw BAD_ARGUMENTS("QePopupNote::QePopupNote",
				    "Internal error: ZERO parent passed as input.");
   
   resize(0, 0);
   setCaption("DjVu Popup Note");
   
   QColor qback_color=qeImager->getColor(back_color);
   QColor qfore_color=qeImager->getColor(fore_color);
   QColor qmidlight_color=qeImager->getColor(qback_color.light());
   QColor qlight_color=qeImager->getColor(qmidlight_color.light());
   QColor qmid_color=qeImager->getColor(qback_color.dark());
   QColor qdark_color=qeImager->getColor(qmid_color.dark());
   QColorGroup col_grp(qfore_color, qback_color, qlight_color,
		       qdark_color, qmidlight_color, qfore_color,
		       qback_color);
   setPalettePropagation(AllChildren);
   setPalette(QPalette(col_grp, col_grp, col_grp));
   QPixmap back_pix=qeImager->getColorPixmap(20, 20, back_color);
   setBackgroundPixmap(back_pix);
   
   QVBoxLayout * vlay=new QVBoxLayout(this, 0, 0, "vlay");
   contents=new QWidget(this, "pnote_contents");
   contents->setBackgroundPixmap(back_pix);
   vlay->addWidget(contents);
   vlay->addStretch(1);
   QVBoxLayout * cont_vlay=new QVBoxLayout(contents, SIDE_MARGIN,
					   SIDE_MARGIN, "cont_vlay");
   text=new QeLabel(note, contents, "pnote_text");
   text->setBackgroundPixmap(back_pix);
   QFont font=text->font(); font.setFamily("courier"); text->setFont(font);
   cont_vlay->addWidget(text, 1);

   QHBoxLayout * butt_lay=new QHBoxLayout(10, "butt_lay");
   cont_vlay->addLayout(butt_lay);
   butt_lay->addStretch(1);
   QPushButton * close_butt=new QPushButton("&Close", contents, "ok_butt");
   close_butt->setBackgroundPixmap(back_pix);
   QSize close_size=close_butt->fontMetrics().size(SingleLine, "Close");
   close_butt->setMinimumSize(close_size.width()+6, close_size.height()+6);
   butt_lay->addWidget(close_butt);
   cont_vlay->activate();

   vlay->activate();

   if (shape_ext_supported)
      resize(contents->width(), contents->height()+getBottomMargin());

   parent->insertChild(this);

   connect(close_butt, SIGNAL(clicked(void)), this, SLOT(reject(void)));
}
