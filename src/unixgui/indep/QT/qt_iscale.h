//C-  -*- C++ -*-
//C-
//C-  Copyright © 1999-2001, LizardTech, Inc. All Rights Reserved.
//C-              Unauthorized use prohibited.
//C-
// 
// $Id: qt_iscale.h,v 1.1 2001-05-29 22:05:29 bcr Exp $
// $Name:  $


#ifndef HDR_QT_ISCALE
#define HDR_QT_ISCALE

#ifdef __GNUC__
#pragma interface
#endif

#include <qslider.h>
#include <qlineedit.h>

#include "qt_fix.h"

class QeIScale : public QWidget
{
   Q_OBJECT
private:
   QLineEdit	* text;
   QSlider	* slider;

   int		decPoints;

   double	itof(int num);
   int		ftoi(double num);

   int		fixValue(int value);
   int		getOptimalTextWidth(void);
private slots:
   void		sliderMoved(int value);
   void		sliderReleased(void);
signals:
   void		valueChanged(int);
public:
   virtual bool	eventFilter(QObject *, QEvent * ev);
   
   void		setDecimalPoints(int points);
   void		setValue(int value);
   void		setStep(int inc);
   void		setRange(int min, int max);

   // Normally you don't have to use it unless you want smth special
   void		setTextWidth(int chars);

   int		value(void) const { return slider->value(); };
   
   QeIScale(QWidget * parent=0, const char * name=0);
   ~QeIScale(void) {};
};

#endif
