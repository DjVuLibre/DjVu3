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
// $Id: qt_iscale.cpp,v 1.3 2001-10-12 17:58:31 leonb Exp $
// $Name:  $

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif


#include "qt_iscale.h"

#include "debug.h"
#include "qlib.h"

#include <qlayout.h>
#include <qkeycode.h>
#include <ctype.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "qt_fix.h"

#ifndef QT1
#include <q1xcompatibility.h>
#endif

double QeIScale::itof(int num)
      // Converts integer-style value into float format taking into
      // account decPoints
{
   double result=num;
   for(int i=0;i<decPoints;i++) result/=10;
   return result;
}

int QeIScale::ftoi(double num)
      // Converts float-style value into integer format taking into
      // account decPoints
{
   for(int i=0;i<decPoints;i++) num*=10;
   return (int) floor(num+0.5);
}

int QeIScale::fixValue(int val)
{
   if (val<slider->minValue()) val=slider->minValue();
   if (val>slider->maxValue()) val=slider->maxValue();
   val=(val-slider->minValue()+slider->lineStep()/2)/slider->lineStep()*
       slider->lineStep()+slider->minValue();
   return val;
}

void QeIScale::sliderMoved(int val)
{
   int valf=fixValue(val);
   
   DEBUG2_MSG("QeIScale::sliderMoved(): value=" << val << ", fixed=" << valf << "\n");

   char buffer[28];
   sprintf(buffer, "%0.4g", itof(valf));
   text->setText(buffer);
      //text->selectAll();

   emit valueChanged(valf);
}

void QeIScale::sliderReleased(void)
{
   int val=slider->value();
   int valf=fixValue(val);
   
   DEBUG2_MSG("QeIScale::sliderReleased(): value=" << val << ", fixed=" << valf << "\n");

   slider->setValue(valf);
}

bool QeIScale::eventFilter(QObject *, QEvent * ev)
{
   try
   {
      if (ev->type()==Event_FocusOut ||
	  (ev->type()==Event_KeyPress && ((QKeyEvent *) ev)->key()==Key_Return))
      {
	 DEBUG2_MSG("QeIScale::eventFilter(): focus just left the QLineEdit\n");
	 QString str=text->text();
	 char * ptr;
	 strtod(str, &ptr);
	 while(*ptr && !isspace(*ptr)) ptr++;
	 if (!*ptr)
	 {
	       // Looks like float
	    int value=ftoi(atof(str));
	    slider->setValue(fixValue(value));
	 };

	    // If Return has been pressed - don't let a dialog interpret it
	 if (ev->type()==Event_KeyPress) return TRUE;
	 else return FALSE;
      };
   } catch(const GException & exc)
   {
      showError(this, "Error", exc);
   };

      // Don't want the selection to disappear
      //if (ev->type()==Event_Leave) return TRUE;
   
   return FALSE;
}

void QeIScale::setDecimalPoints(int points)
{
   decPoints=points;
   sliderMoved(slider->value());
   setTextWidth(getOptimalTextWidth());
}

void QeIScale::setRange(int min, int max)
{
   slider->setRange(min, max);
   slider->setValue(fixValue(slider->value()));
   setTextWidth(getOptimalTextWidth());
}

void QeIScale::setValue(int value)
{
   slider->setValue(fixValue(value));
}

void QeIScale::setStep(int step)
{
   slider->setSteps(step, step);
   slider->setValue(fixValue(slider->value()));
}

int QeIScale::getOptimalTextWidth(void)
{
   int min=abs(slider->minValue());
   int max=abs(slider->maxValue());
   int abs_max=min<max ? max : min;

   return abs_max>0 ? (int) (floor(log10(abs_max)+0.0001)+1) : 2;
}

void QeIScale::setTextWidth(int chars)
      // Normally you don't have to use it unless you want smth special
{
   chars++;
   
   char buffer[128];
   if (chars>120) chars=120;
   for(int i=0;i<chars;i++) buffer[i]='0';
   buffer[chars]=0;
   
   int width=text->fontMetrics().boundingRect(buffer).width();
   text->setMinimumSize(width+5, text->sizeHint().height()+4);
   ActivateLayouts(text);
}

QeIScale::QeIScale(QWidget * parent, const char * name) :
      QWidget(parent, name), decPoints(0)
{
   QHBoxLayout * lay=new QHBoxLayout(this, 0, 5, "iscale_lay");
   slider=new QSlider(QSlider::Horizontal, this, "iscale_slider");
   lay->addWidget(slider, 1);
   text=new QLineEdit(this, "iscale_text");
   lay->addWidget(text, 0);
   lay->activate();

      // Setting text's font
   QFont font=text->font();
   font.setFamily("Courier");
   text->setFont(font);
   
      // Setting min sizes
   setTextWidth(3);
   slider->setMinimumWidth(text->minimumSize().width());

      // Connecting the signals
   connect(slider, SIGNAL(valueChanged(int)), this, SLOT(sliderMoved(int)));
   connect(slider, SIGNAL(sliderMoved(int)), this, SLOT(sliderMoved(int)));
   connect(slider, SIGNAL(sliderReleased(void)), this, SLOT(sliderReleased(void)));
   text->installEventFilter(this);

      // Setting default values
   setRange(0, 100);
   setStep(1);
   setValue(50);
}
