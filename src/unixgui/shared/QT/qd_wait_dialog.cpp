//C-  -*- C++ -*-
//C-
//C-  Copyright © 1999-2001, LizardTech, Inc. All Rights Reserved.
//C-              Unauthorized use prohibited.
//C-
// 
// $Id: qd_wait_dialog.cpp,v 1.1 2001-05-29 22:05:31 bcr Exp $
// $Name:  $


#ifdef __GNUC__
#pragma implementation
#endif

#include "qd_wait_dialog.h"

#include "debug.h"
#include "GContainer.h"
#include "GString.h"

#include <qlayout.h>
#include <qpushbutton.h>
#include <qlabel.h>
#include <qpainter.h>
#include <qtimer.h>
#include <qpixmap.h>
#include "qt_fix.h"

//****************************************************************************
//******************************* QeWaitLabel ********************************
//****************************************************************************

#define STEPS		20

class QeWaitLabel : public QeLabel
{
   Q_OBJECT
private:
   QTimer		timer;
   int			cur_step;
   GArray<QPixmap>	pixmap_arr;
   void		drawPixmap(int i);
private slots:
   void		slotTimeout(void);
protected:
   virtual void	paintEvent(QPaintEvent * ev);
   virtual void resizeEvent(QResizeEvent * ev);
public:
   QeWaitLabel(QWidget * parent=0, const char * name=0);
   ~QeWaitLabel(void) {};
};

#define GET_R(i) (((i) >> 16) & 0xff)
#define GET_G(i) (((i) >> 8) & 0xff)
#define GET_B(i) ((i) & 0xff)

void
QeWaitLabel::drawPixmap(int num)
{
   if (num>=pixmap_arr.size())
      pixmap_arr.resize(num);

   int w=width(), h=height();
   QPixmap qpix(w, h, x11Depth());
   
   {
      QPainter p(&qpix);
      QBrush back_brush(backgroundColor());
      p.fillRect(0, 0, w, h, back_brush);
   
      p.setPen(foregroundColor());
      p.setBrush(foregroundColor());
      int r=(w<h ? w : h)/2;
      int x0=w/2, y0=h/2;
      if (r>10) r=10;

      int angle=180-cur_step*180/STEPS;
      QRect rect(x0-r, y0-r, 2*r, 2*r);
      p.drawArc(rect, 0, 360*16);
      p.drawPie(rect, angle*16, 90*16);
      p.drawPie(rect, (angle+180)*16, 90*16);
   }
   
   pixmap_arr[num]=qpix;
}

void
QeWaitLabel::resizeEvent(QResizeEvent * ev)
{
   setMinimumWidth(ev->size().height());
   pixmap_arr.empty();
}

void
QeWaitLabel::paintEvent(QPaintEvent * ev)
{
   if (cur_step>=pixmap_arr.size())
      drawPixmap(cur_step);
   if (cur_step<pixmap_arr.size())
   {
      QPainter p(this);
      QRect rect=ev->rect();
      p.drawPixmap(rect.x(), rect.y(), pixmap_arr[cur_step],
		   rect.x(), rect.y(), rect.width(), rect.height());
   }
}

void
QeWaitLabel::slotTimeout(void)
{
   if (++cur_step>=STEPS)
      cur_step=0;

   if (isVisible())
   {
      {
	 QPainter p(this);
	 QBrush brush(backgroundColor());
	 p.fillRect(rect(), brush);
      }
      QPaintEvent ev(QRect(0, 0, width(), height()));
      paintEvent(&ev);
   }
}

QeWaitLabel::QeWaitLabel(QWidget * parent, const char * name) :
      QeLabel(parent, name), cur_step(0)
{
   connect(&timer, SIGNAL(timeout(void)), this, SLOT(slotTimeout(void)));
   timer.start(100);
}

//****************************************************************************
//******************************* QeWaitDialog *******************************
//****************************************************************************

QDWaitDialog::QDWaitDialog(const QString & msg, const char * butt_label,
			   QWidget * parent, const char * name, bool modal) :
      QSemiModal(parent, name, modal)
{
   setCaption(tr("DjVu: Please wait..."));

   resize(0, 0);
   
   QVBoxLayout * vlay=new QVBoxLayout(this, 10, 5);

   QHBoxLayout * hlay=new QHBoxLayout(10);
   vlay->addLayout(hlay);
   
   QeLabel * label;
   label=new QeWaitLabel(this);
   hlay->addWidget(label);

   QString str=msg+tr("\nPlease stand by...");
   label=new QeLabel(str, this);
   hlay->addWidget(label);

   QFrame * sep=new QFrame(this, "sep");
   sep->setFrameStyle(QFrame::HLine | QFrame::Sunken);
   sep->setMinimumHeight(sep->sizeHint().height());
   vlay->addWidget(sep);

   hlay=new QHBoxLayout(10);
   vlay->addLayout(hlay);
   hlay->addStretch(1);
   QePushButton * close_butt=new QePushButton(butt_label, this, "close_butt");
   close_butt->setDefault(TRUE);
   hlay->addWidget(close_butt);
   hlay->addStretch(1);

   vlay->activate();

   connect(close_butt, SIGNAL(clicked(void)), this, SLOT(slotClose(void)));
}

#include "qd_wait_dialog_moc.inc"
