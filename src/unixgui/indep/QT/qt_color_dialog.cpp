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
// $Id: qt_color_dialog.cpp,v 1.5 2001-10-17 19:02:05 docbill Exp $
// $Name:  $


#include "qt_color_dialog.h"

#include "qt_imager.h"
#include "qt_painter.h"
#include "col_db.h"
#include "qlib.h"

#include <qlayout.h>
#include <qgroupbox.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qapplication.h>
#include <math.h>

#include "qt_fix.h"

#ifndef QT1
#include <q1xcompatibility.h>
#endif

#define Pi      3.14159265359
#define COS30   0.8660254038
#define SIN60   0.8660254038

#define IWIDTH          15
#define MARGIN          5
#define CROSS_SIZE      5

static inline int max(int x, int y) { return x>y ? x : y; }
static inline int min(int x, int y) { return x>y ? y : x; }

//***************************************************************************
//******************************** Math *************************************
//***************************************************************************

static void RGBtoHSI(u_char r, u_char g, u_char b,
                     float * h, float * s, float * i)
{
   // Golzalez and Woods convertion
   // (http://www.wmin.ac.uk/ITRG/docs/ftp/coloureq.txt)

   u_char min_comp=min(r, min(g, b));
   u_char max_comp=max(r, max(g, b));

   *i=max_comp/255.0;//(r+g+b)/255.0/3;

   if (max_comp) *s=1-3.0*min_comp/(r+g+b);
   else *s=0;

   if (*s!=0) *h=acos(0.5*(r-g+r-b)/sqrt((r-g)*(r-g)+(r-b)*(g-b)))/2/Pi;
   else *h=0;
   if (b>g) *h=1-*h;
}

static void HSItoRGB(float h, float s, float i,
                     u_char * r, u_char * g, u_char * b)
{
   float rf, gf, bf;
   if (h<1.0/3)
   {
      h*=2*Pi;
      bf=(1-s)/3;
      rf=(1+s*cos(h)/cos(Pi/3-h))/3;
      gf=1-bf-rf;
   } else
      if (h<2.0/3)
      {
         h=(h-1.0/3)*2*Pi;
         rf=(1-s)/3;
         gf=(1+s*cos(h)/cos(Pi/3-h))/3;
         bf=1-rf-gf;
      } else
      {
         h=(h-2.0/3)*2*Pi;
         gf=(1-s)/3;
         bf=(1+s*cos(h)/cos(Pi/3-h))/3;
         rf=1-gf-bf;
      }
   float max=rf>gf ? rf : gf;
   max=max>bf ? max : bf;
   int ri, gi, bi;
   ri=(int) (rf*i/max*255);
   gi=(int) (gf*i/max*255);
   bi=(int) (bf*i/max*255);
   *r=ri<=255 ? ri : 255;
   *g=gi<=255 ? gi : 255;
   *b=bi<=255 ? bi : 255;
}

static int XYtoHS(float x, float y, int radius,
                  float * h, float * s)
{
   x/=radius; y/=radius;
   float dist=sqrt(x*x+y*y);
   if (dist>1) return 0;

   *h=atan2(y, x)/2/Pi;
   if (*h<0) *h+=1; if (*h>1) *h=1;
   *s=dist;
   return 1;
}

static int XYtoRGB(float x, float y, int radius,
                   u_char * r, u_char * g, u_char * b)
{
   float rf, gf, bf;
   float dist=sqrt(x*x+y*y);
   if (dist>radius) return 0;

   float s=dist/radius;
   x/=dist; y/=dist;

   if (y>0 && x>-0.5)
   {
      bf=(1-s)/3;
      rf=(1+s*x/(0.5*x+SIN60*y))/3;
      gf=1-bf-rf;
   } else
      if (x<=-0.5)
      {
         rf=(1-s)/3;
         gf=(1-s*(-0.5*x+SIN60*y)/x)/3;
         bf=1-rf-gf;
      } else
      {
	 gf=(1-s)/3;
         bf=(1+s*(-0.5*x-SIN60*y)/(0.5*x-SIN60*y))/3;
         rf=1-gf-bf;
      }
   float max=rf>gf ? rf : gf;
   max=max>bf ? max : bf;
   int ri, gi, bi;
   ri=(int) (rf/max*255);
   gi=(int) (gf/max*255);
   bi=(int) (bf/max*255);

   *r=ri<=255 ? ri : 255;
   *g=gi<=255 ? gi : 255;
   *b=bi<=255 ? bi : 255;
   return 1;
}

//***************************************************************************
//******************************* QeCDPane **********************************
//***************************************************************************

bool QeCDPane::event(QEvent * ev)
{
   if (ev->type()==Event_User)
   {
      try
      {
	 DEBUG_MSG("QeCDPane::event(): resizing: width=" << width() << ", height=" << height() << "\n");
	 setMinimumWidth(wantedWidth(height()));
	 ActivateLayouts(this);
	 return FALSE;
      } catch(const GException & exc)
      {
	 showError(this, "Error", exc);
      }
   }
   return QWidget::event(ev);
}
   
void QeCDPane::resizeEvent(QResizeEvent *)
{
   DEBUG_MSG("QeCDPane::resizeEvent(): delaying resize...\n");
   QEvent * ev=new QEvent(Event_User);
   QApplication::postEvent(this, ev);
}

QeCDPane::QeCDPane(QWidget * parent, const char * name) :
      QFrame(parent, name)
{
   if (QApplication::style()==WindowsStyle)
      setFrameStyle(QFrame::WinPanel | QFrame::Sunken);
   else
      setFrameStyle(QFrame::Panel | QFrame::Sunken);
   setMargin(0);
   setBackgroundColor(white);
}

//***************************************************************************
//******************************* QeCPreview ********************************
//***************************************************************************

int QeCDPreview::wantedWidth(int height)
{
   return height;
}

void QeCDPreview::paintEvent(QPaintEvent * ev)
{
   try
   {
      if (!pix.isNull())
      {
	 GRect erect=Q2G(ev->rect());
	 GRect crect=Q2G(contentsRect());
	 GRect irect; irect.intersect(erect, crect);
	 if (!irect.isempty())
	 {
	    QPainter p(this);
	    p.fillRect(irect.xmin, irect.ymin, irect.width(),
		       irect.height(), QBrush(white, pix));
	    p.end();
	 }
      }
      QeCDPane::paintEvent(ev);
   } catch(const GException & exc)
   {
      showError(this, "Error", exc);
   }
}

void QeCDPreview::setColor(u_int32 color)
{
   pix=qeImager->getColorPixmap(30, 30, color);
   repaint(FALSE);
}

//***************************************************************************
//******************************* QeCDEdit **********************************
//***************************************************************************

int QeCDEdit::wantedWidth(int height)
{
   return height+3*IWIDTH;
}

void QeCDEdit::setH(float hh)
{
   if (hh!=h)
   {
      hideCross(); h=hh; drawCross();
      drawIScale(bg_pix);
      QPainter p(this);
      p.drawPixmap(srect.xmin, srect.ymin, bg_pix,
		   srect.xmin, srect.ymin, srect.width(), srect.height());
      p.end();

      emit hChanged(h);
   }
}

void QeCDEdit::setS(float ss)
{
   if (ss!=s)
   {
      hideCross(); s=ss; drawCross();
      drawIScale(bg_pix);
      QPainter p(this);
      p.drawPixmap(srect.xmin, srect.ymin, bg_pix,
		   srect.xmin, srect.ymin, srect.width(), srect.height());
      p.end();

      emit sChanged(s);
   }
}

void QeCDEdit::setI(float ii)
{
   if (ii!=i)
   {
      hideSlider(); i=ii; drawSlider();
      emit iChanged(i);
   }
}

void QeCDEdit::drawCross(void)
{
   int x=xc+(int) (s*radius*cos(h*2*Pi));
   int y=yc-(int) (s*radius*sin(h*2*Pi));

   cross_x=x; cross_y=y;

   QPainter p(this);
   p.setPen(black);
   p.drawLine(x-1, y-CROSS_SIZE, x-1, y+CROSS_SIZE);
   p.drawLine(x+1, y-CROSS_SIZE, x+1, y+CROSS_SIZE);
   p.drawLine(x-CROSS_SIZE, y-1, x+CROSS_SIZE, y-1);
   p.drawLine(x-CROSS_SIZE, y+1, x+CROSS_SIZE, y+1);

   p.setPen(white);
   p.drawLine(x, y-CROSS_SIZE, x, y+CROSS_SIZE);
   p.drawLine(x-CROSS_SIZE, y, x+CROSS_SIZE, y);
   p.end();
}

void QeCDEdit::hideCross(void)
{
   if (cross_x>=0 && cross_y>=0)
   {
      QPainter p(this);
      p.drawPixmap(cross_x-CROSS_SIZE, cross_y-CROSS_SIZE, bg_pix,
		   cross_x-CROSS_SIZE, cross_y-CROSS_SIZE,
		   2*CROSS_SIZE+1, 2*CROSS_SIZE+1);
      cross_x=cross_y=-1;
      p.end();
   }
}

void QeCDEdit::drawSlider(void)
{
   int y=srect.ymax-(int) (i*srect.height());
   int x=srect.xmin;
   int width=srect.width();

   slider_y=y;

   QPainter p(this);
   QPointArray poly(3);
   poly.setPoint(0, x, y);
   poly.setPoint(1, x-2*CROSS_SIZE, y-CROSS_SIZE);
   poly.setPoint(2, x-2*CROSS_SIZE, y+CROSS_SIZE);
   p.drawPolygon(poly);
   poly.setPoint(0, x+width, y);
   poly.setPoint(1, x+width+2*CROSS_SIZE, y-CROSS_SIZE);
   poly.setPoint(2, x+width+2*CROSS_SIZE, y+CROSS_SIZE);
   p.drawPolygon(poly);
   p.end();
}

void QeCDEdit::hideSlider(void)
{
   if (slider_y>=0)
   {
      QPainter p(this);
      p.drawPixmap(srect.xmin-2*CROSS_SIZE, slider_y-CROSS_SIZE, bg_pix,
		   srect.xmin-2*CROSS_SIZE, slider_y-CROSS_SIZE,
		   srect.width()+4*CROSS_SIZE+1, 2*CROSS_SIZE+1);
      slider_y=-1;
      p.end();
   }
}

void QeCDEdit::drawIScale(QPixmap & pixmap)
{
   if (pixmap.isNull()) return;
   
   u_char r, g, b;
   HSItoRGB(h, s, 1, &r, &g, &b);

   GP<GPixmap> gpix=GPixmap::create(srect.height(), srect.width());
   GPixmap &pix=*gpix;
   for(int y=1;y<srect.height()-1;y++)
   {
      GPixel pixel;
      pixel.r=r*y/(srect.height()-1);
      pixel.g=g*y/(srect.height()-1);
      pixel.b=b*y/(srect.height()-1);
      for(int x=1;x<srect.width()-1;x++)
         pix[y][x]=pixel;
      pix[y][0]=pix[y][srect.width()-1]=GPixel::BLACK;
   }
   for(int x=0;x<srect.width();x++)
      pix[0][x]=pix[srect.height()-1][x]=GPixel::BLACK;

   if (pixmap.depth()<15) pix.ordered_666_dither();
   else if (pixmap.depth()<24) pix.ordered_32k_dither();

   QePainter p(&pixmap);
   p.drawPixmap(GRect(srect.xmin, srect.ymin,
		      srect.width(), srect.height()), &pix);
   p.end();
}

void QeCDEdit::rebuildBGPixmap(void)
{
   int width=QeCDPane::width();
   int height=QeCDPane::height();

   if (width>0 && height>0)
   {
      GP<GPixmap> gpix=GPixmap::create(height, width, &GPixel::WHITE);
      GPixmap &pix = *gpix;
	 // Creating the round with all possible colors at I=1
      for(int x=-radius;x<=radius;x++)
	 for(int y=-radius;y<=radius;y++)
	 {
	    u_char r, g, b;
	    if (XYtoRGB(x, y, radius, &r, &g, &b))
	    {
	       GPixel & pixel=pix[y+yc][x+xc];
	       pixel.r=r; pixel.g=g; pixel.b=b;
	    }
	 }

      bg_pix=QPixmap(width, height);
   
      if (bg_pix.depth()<15) pix.ordered_666_dither();
      else if (bg_pix.depth()<24) pix.ordered_32k_dither();

      QePainter p(&bg_pix);
      p.drawPixmap(GRect(0, 0, width, height), &pix);
      p.drawArc(xc-radius, yc-radius, 2*radius, 2*radius, 0, 360*16);
      p.end();

      drawIScale(bg_pix);
   }
}

void QeCDEdit::processMotion(int x, int y)
{
   int dist=(x-xc)*(x-xc)+(y-yc)*(y-yc);
   if (dist<=radius*radius)
   {
	 // Modifying H and S
      float hh, ss;
      if (XYtoHS(x-xc, yc-y, radius, &hh, &ss))
      {
	 if (hh!=h) { h=hh; emit hChanged(h); }
	 if (ss!=s) { s=ss; emit sChanged(s); }
      }
   } else
      if (x>srect.xmin-2*MARGIN && x<srect.xmax+2*MARGIN &&
          y>srect.ymin && y<srect.ymax)
      {
	 float ii=(float) (srect.ymax-y)/srect.height();
	 if (ii!=i) { i=ii; emit iChanged(i); }
      }
}

void QeCDEdit::mousePressEvent(QMouseEvent * ev)
{
   try
   {
      processMotion(ev->x(), ev->y());
   } catch(const GException & exc)
   {
      showError(this, "Error", exc);
   }
}

void QeCDEdit::mouseMoveEvent(QMouseEvent * ev)
{
   try
   {
      processMotion(ev->x(), ev->y());
   } catch(const GException & exc)
   {
      showError(this, "Error", exc);
   }
}

void QeCDEdit::paintEvent(QPaintEvent * ev)
{
   try
   {
      if (bg_pix.isNull()) rebuildBGPixmap();

      QRect crect=contentsRect();
      if (!bg_pix.isNull())
      {
	 QPainter p(this);
	 p.drawPixmap(crect.x(), crect.y(), bg_pix, crect.x(), crect.y(),
		      crect.width(), crect.height());
	 p.end();
      }
      drawCross(); drawSlider();
   
      QeCDPane::paintEvent(ev);
   } catch(const GException & exc)
   {
      showError(this, "Error", exc);
   }
}

void QeCDEdit::resizeEvent(QResizeEvent * ev)
{
   try
   {
      bg_pix=QPixmap();

      QRect crect=contentsRect();
      int cwidth=crect.width();
      int cheight=crect.height();
      int width=QeCDPane::width();
      int height=QeCDPane::height();

      if (width>0 && height>0)
      {
	 radius=(min(cwidth-3*IWIDTH, cheight)-2*MARGIN)/2;
	 xc=crect.x()+MARGIN+radius; yc=height/2;
	 srect=GRect(width-MARGIN-2*IWIDTH, yc-radius, IWIDTH, 2*radius);

	 cross_x=cross_y=slider_y=-1;
      }
      QeCDPane::resizeEvent(ev);
   } catch(const GException & exc)
   {
      showError(this, "Error", exc);
   }
}

//***************************************************************************
//****************************** QeColorDialog ******************************
//***************************************************************************

void QeColorDialog::setH(float h)
{
   scale_h->setValue((int) (h*100));
}

void QeColorDialog::setS(float s)
{
   scale_s->setValue((int) (s*100));
}

void QeColorDialog::setI(float i)
{
   scale_i->setValue((int) (i*100));
}

void QeColorDialog::scroll(int value)
{
   const QObject * obj=sender();
   if (obj && obj->isWidgetType())
   {
      if (ignore_drag) return;

      float h, s, i;
      if (obj==scale_r || obj==scale_g || obj==scale_b)
      {
	 u_char r, g, b;
	 r=scale_r->value();
	 g=scale_g->value();
	 b=scale_b->value();
	 RGBtoHSI(r, g, b, &h, &s, &i);
	 ignore_drag=1;
	 scale_h->setValue((int) (h*100));
	 scale_s->setValue((int) (s*100));
	 scale_i->setValue((int) (i*100));
	 ignore_drag=0;
      } else
      {
	 h=scale_h->value()/100.0;
	 s=scale_s->value()/100.0;
	 i=scale_i->value()/100.0;
	 u_char r, g, b;
	 HSItoRGB(h, s, i, &r, &g, &b);
	 ignore_drag=1;
	 scale_r->setValue(r);
	 scale_g->setValue(g);
	 scale_b->setValue(b);
	 ignore_drag=0;
      }

      if (old_h!=h || old_s!=s || old_i!=i)
      {
	 u_char r, g, b;
	 HSItoRGB(h, s, i, &r, &g, &b);
	 new_pane->setColor(ColorDB::RGB_to_C32(r, g, b));

	 if (old_h!=h) emit hChanged(h);
	 if (old_s!=s) emit sChanged(s);
	 if (old_i!=i) emit iChanged(i);

	 old_h=h; old_s=s; old_i=i;
      }
   }
}

u_int32 QeColorDialog::color(void) const
{
   return ColorDB::RGB_to_C32(scale_r->value(),
			      scale_g->value(),
			      scale_b->value());
}

QeColorDialog::QeColorDialog(u_int32 color, QWidget * parent,
			     const char * name, bool modal) :
      QeDialog(parent, name, modal), ignore_drag(0)
{
   setCaption("DjVu: Color Editor");
   QWidget * start=startWidget();

   QVBoxLayout * vlay=new QVBoxLayout(start, 10, 8, "vlay");
   QHBoxLayout * hlay=new QHBoxLayout(8, "hlay");
   vlay->addLayout(hlay);
      // ******** 'Edit' frame
   QeGroupBox * edit_box=new QeGroupBox("Edit", start, "edit_box");
   hlay->addWidget(edit_box);
   QVBoxLayout * edit_vlay=new QVBoxLayout(edit_box, 10, 8, "edit_vlay");
   edit_vlay->addSpacing(edit_box->fontMetrics().height());
   QHBoxLayout * edit_hlay=new QHBoxLayout(8, "edit_hlay");
   edit_vlay->addLayout(edit_hlay);
      // **** Sliders array
   QGridLayout * edit_glay=new QGridLayout(6, 2, 8, "edit_glay");
   edit_hlay->addLayout(edit_glay);
   edit_glay->addColSpacing(1, 200);
   edit_glay->addWidget(new QeLabel("Red", edit_box), 0, 0);
   edit_glay->addWidget(new QeLabel("Green", edit_box), 1, 0);
   edit_glay->addWidget(new QeLabel("Blue", edit_box), 2, 0);
   edit_glay->addWidget(new QeLabel("Hue", edit_box), 3, 0);
   edit_glay->addWidget(new QeLabel("Satt.", edit_box), 4, 0);
   edit_glay->addWidget(new QeLabel("Intens.", edit_box), 5, 0);
   scale_r=new QeIScale(edit_box, "scale_r");
   scale_r->setRange(0, 255); scale_r->setTextWidth(3);
   edit_glay->addWidget(scale_r, 0, 1);
   scale_g=new QeIScale(edit_box, "scale_g");
   scale_g->setRange(0, 255); scale_g->setTextWidth(3);
   edit_glay->addWidget(scale_g, 1, 1);
   scale_b=new QeIScale(edit_box, "scale_b");
   scale_b->setRange(0, 255); scale_b->setTextWidth(3);
   edit_glay->addWidget(scale_b, 2, 1);
   scale_h=new QeIScale(edit_box, "scale_h");
   scale_h->setRange(0, 100); scale_h->setDecimalPoints(2);
   scale_h->setTextWidth(3);
   edit_glay->addWidget(scale_h, 3, 1);
   scale_s=new QeIScale(edit_box, "scale_s");
   scale_s->setRange(0, 100); scale_s->setDecimalPoints(2);
   scale_s->setTextWidth(3);
   edit_glay->addWidget(scale_s, 4, 1);
   scale_i=new QeIScale(edit_box, "scale_i");
   scale_i->setRange(0, 100); scale_i->setDecimalPoints(2);
   scale_i->setTextWidth(3);
   edit_glay->addWidget(scale_i, 5, 1);
   edit_vlay->activate();

      // **** Edit pane
   edit_pane=new QeCDEdit(edit_box, "edit_pane");
   edit_hlay->addWidget(edit_pane);

   QVBoxLayout * pvlay=new QVBoxLayout(8, "pvlay");
   hlay->addLayout(pvlay);
   
      // ******** New color preview
   QeGroupBox * new_box=new QeGroupBox("New color", start, "new_box");
   pvlay->addWidget(new_box);
   QVBoxLayout * new_vlay=new QVBoxLayout(new_box, 10, 8, "new_vlay");
   new_vlay->addSpacing(new_box->fontMetrics().height());
   new_vlay->addStrut(new_box->fontMetrics().boundingRect(new_box->title()).width()+5);
   new_pane=new QeCDPreview(new_box, "new_pane");
   new_vlay->addWidget(new_pane);
   new_vlay->activate();

      // ******** Old color preview
   QeGroupBox * old_box=new QeGroupBox("Old color", start, "old_box");
   pvlay->addWidget(old_box);
   QVBoxLayout * old_vlay=new QVBoxLayout(old_box, 10, 8, "old_vlay");
   old_vlay->addSpacing(old_box->fontMetrics().height());
   old_vlay->addStrut(old_box->fontMetrics().boundingRect(old_box->title()).width()+5);
   old_pane=new QeCDPreview(old_box, "old_pane");
   old_vlay->addWidget(old_pane);
   old_vlay->activate();

   QHBoxLayout * butt_lay=new QHBoxLayout(8, "butt_lay");
   vlay->addLayout(butt_lay);
   butt_lay->addStretch(1);
   QePushButton * ok_butt=new QePushButton("&OK", start, "ok_butt");
   ok_butt->setDefault(TRUE);
   butt_lay->addWidget(ok_butt);
   QePushButton * cancel_butt=new QePushButton("&Cancel", start, "cancel_butt");
   butt_lay->addWidget(cancel_butt);
   
   vlay->activate();

      // Connecting signals...
   connect(scale_r, SIGNAL(valueChanged(int)), this, SLOT(scroll(int)));
   connect(scale_g, SIGNAL(valueChanged(int)), this, SLOT(scroll(int)));
   connect(scale_b, SIGNAL(valueChanged(int)), this, SLOT(scroll(int)));
   connect(scale_h, SIGNAL(valueChanged(int)), this, SLOT(scroll(int)));
   connect(scale_s, SIGNAL(valueChanged(int)), this, SLOT(scroll(int)));
   connect(scale_i, SIGNAL(valueChanged(int)), this, SLOT(scroll(int)));

   connect(this, SIGNAL(hChanged(float)), edit_pane, SLOT(setH(float)));
   connect(this, SIGNAL(sChanged(float)), edit_pane, SLOT(setS(float)));
   connect(this, SIGNAL(iChanged(float)), edit_pane, SLOT(setI(float)));
   connect(edit_pane, SIGNAL(hChanged(float)), this, SLOT(setH(float)));
   connect(edit_pane, SIGNAL(sChanged(float)), this, SLOT(setS(float)));
   connect(edit_pane, SIGNAL(iChanged(float)), this, SLOT(setI(float)));

   connect(ok_butt, SIGNAL(clicked(void)), this, SLOT(accept(void)));
   connect(cancel_butt, SIGNAL(clicked(void)), this, SLOT(reject(void)));
   
      // Setting default values
   scale_r->setValue(ColorDB::C32_GetRed(color));
   scale_g->setValue(ColorDB::C32_GetGreen(color));
   scale_b->setValue(ColorDB::C32_GetBlue(color));

   new_pane->setColor(color);
   old_pane->setColor(color);
}
