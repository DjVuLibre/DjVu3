//C-  -*- C++ -*-
//C-
//C-  Copyright © 1999-2001, LizardTech, Inc. All Rights Reserved.
//C-              Unauthorized use prohibited.
//C-
// 
// $Id: qd_set_zoom.cpp,v 1.2 2001-06-06 17:16:57 mchen Exp $
// $Name:  $


#ifdef __GNUC__
#pragma implementation
#endif

#include "djvu_base_res.h"

#include "qd_set_zoom.h"

#include <qlabel.h>
#include <qvalidator.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qkeycode.h>

#include "qt_fix.h"

class QDSetZoomVal : public QValidator
{
public:
   virtual void		fixup(QString &);
#ifdef QT1
   virtual State	validate(QString &, int &);
#else
   virtual State	validate(QString &, int &) const;
#endif
   
   QDSetZoomVal(QSpinBox * parent, const char * name=0) :
	 QValidator(parent, name) {};
};

void
QDSetZoomVal::fixup(QString & str)
{
#ifdef QT1
   str.detach();
#else
   str.truncate(0);
#endif

   QSpinBox * spin=(QSpinBox *) parent();
   spin->setValue(spin->value());
   str=spin->text();
}

QValidator::State
QDSetZoomVal::validate(QString & input, int & pos)
#ifndef QT1
const
#endif
{
   if (!input.length()) return Valid;
   
   bool status;
   int zoom=input.toInt(&status);
   if (!status) return Invalid;

   if (zoom>IDC_ZOOM_MAX-IDC_ZOOM_MIN) return Invalid;
   if (zoom<=0) return Invalid;
   if (zoom<5)
      if (input.length()==1) return Valid;
      else return Invalid;

   return Acceptable;
}

QDSetZoom::QDSetZoom(int zoom, QWidget * parent, const char * name) :
      QeDialog(parent, name, TRUE)
{
   setCaption(tr("DjVu: Set Zoom"));

   QWidget * start=startWidget();

      // Create the menu
   QVBoxLayout * vlay=new QVBoxLayout(start, 10, 10);
   QHBoxLayout * hlay=new QHBoxLayout(10);
   vlay->addLayout(hlay);
   QeLabel * label=new QeLabel(tr("Custom zoom"), start, "zoom_label");
   hlay->addWidget(label);
   spin=new QeSpinBox(5, IDC_ZOOM_MAX-IDC_ZOOM_MIN, 10, start, "goto_menu");
   spin->setValidator(new QDSetZoomVal(spin));
   spin->setValue(zoom);
   hlay->addWidget(spin);
   label=new QeLabel(" %", start, "% label");
   hlay->addWidget(label);

      // Create the buttons
   hlay=new QHBoxLayout(10);
   vlay->addLayout(hlay);
   hlay->addStretch(1);
   QePushButton * ok_butt=new QePushButton(tr("&OK"), start, "ok_butt");
   ok_butt->setDefault(TRUE);
   hlay->addWidget(ok_butt);
   QePushButton * cancel_butt=new QePushButton(tr("&Cancel"), start, "cancel_butt");
   hlay->addWidget(cancel_butt);

      // Connect signals
   connect(ok_butt, SIGNAL(clicked(void)), this, SLOT(accept(void)));
   connect(cancel_butt, SIGNAL(clicked(void)), this, SLOT(reject(void)));

   spin->setFocus();
}
