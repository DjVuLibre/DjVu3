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
// $Id: qt_color_dialog.h,v 1.4 2001-10-16 18:01:45 docbill Exp $
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
