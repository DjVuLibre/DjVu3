//C-  -*- C++ -*-
//C-
//C-  Copyright © 1999-2001, LizardTech, Inc. All Rights Reserved.
//C-              Unauthorized use prohibited.
//C-
// 
// $Id: qt_color_dialog.h,v 1.1 2001-05-29 22:05:29 bcr Exp $
// $Name:  $


#ifndef HDR_QT_COLOR_DIALOG
#define HDR_QT_COLOR_DIALOG

#ifdef __GNUC__
#pragma interface
#endif

#include "qt_iscale.h"

#include "GPixmap.h"
#include "GRect.h"
#include "int_types.h"

#include <qdialog.h>
#include <qframe.h>
#include <qpixmap.h>
#include <sys/types.h>

#include "qt_fix.h"

class QeCDPane : public QFrame
{
   Q_OBJECT
private:
protected:
   virtual void	resizeEvent(QResizeEvent *);
   virtual bool	event(QEvent *);
   virtual int	wantedWidth(int height)=0;
public:
   QeCDPane(QWidget * parent=0, const char * name=0);
   ~QeCDPane(void) {};
};

class QeCDPreview : public QeCDPane
{
   Q_OBJECT
private:
   QPixmap	pix;
protected:
   virtual int	wantedWidth(int height);
   virtual void	paintEvent(QPaintEvent *);
public:
   void		setColor(u_int32 color);
   
   QeCDPreview(QWidget * parent=0, const char * name=0) :
	 QeCDPane(parent, name) { setColor(0xffffff); };
   ~QeCDPreview(void) {};
};

class QeCDEdit : public QeCDPane
{
   Q_OBJECT
private:
   QPixmap	bg_pix;

   float	h, s, i;
   int		cross_x, cross_y, slider_y;
   int		xc, yc, radius;
   GRect	srect;

   void		drawCross(void);
   void		hideCross(void);
   void		drawSlider(void);
   void		hideSlider(void);
   void		drawIScale(QPixmap & pixmap);
   void		rebuildBGPixmap(void);
   void		processMotion(int x, int y);
protected:
   virtual int	wantedWidth(int height);
   virtual void	paintEvent(QPaintEvent *);
   virtual void	resizeEvent(QResizeEvent *);
   virtual void	mousePressEvent(QMouseEvent *);
   virtual void	mouseMoveEvent(QMouseEvent *);
signals:
   void		hChanged(float);
   void		sChanged(float);
   void		iChanged(float);
public slots:
   void		setH(float);
   void		setS(float);
   void		setI(float);
public:
   QeCDEdit(QWidget * parent=0, const char * name=0) :
	 QeCDPane(parent, name), h(0), s(0), i(0) { setMargin(5); };
   ~QeCDEdit(void) {};
};

class QeColorDialog : public QeDialog
{
   Q_OBJECT
private:
   QeIScale	* scale_r, * scale_g, * scale_b;
   QeIScale	* scale_h, * scale_s, * scale_i;
   QeCDEdit	* edit_pane;
   QeCDPreview	* new_pane, * old_pane;

   int		ignore_drag;
   float	old_h, old_s, old_i;
private slots:
   void		scroll(int);
protected:
signals:
   void		hChanged(float);
   void		sChanged(float);
   void		iChanged(float);
public slots:
   void		setH(float);
   void		setS(float);
   void		setI(float);
public:
   u_int32	color(void) const;
   
   QeColorDialog(u_int32 color, QWidget * parent=0,
		 const char * name=0, bool modal=FALSE);
   ~QeColorDialog(void) {};
};

#endif
