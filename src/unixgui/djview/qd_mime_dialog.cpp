//C-  -*- C++ -*-
//C-
//C-  Copyright © 1999-2001, LizardTech, Inc. All Rights Reserved.
//C-              Unauthorized use prohibited.
//C-
// 
// $Id: qd_mime_dialog.cpp,v 1.2 2001-06-05 21:56:29 mchen Exp $
// $Name:  $


#ifdef __GNUC__
#pragma implementation
#endif

#include "qd_mime_dialog.h"

#include "prefs.h"
#include "debug.h"

#include <qlayout.h>
#include <qpushbutton.h>
#include <qlabel.h>
#include <qbuttongroup.h>
#include <qradiobutton.h>

#include "qt_fix.h"

bool
QDMimeDialog::dontAsk(void) const
{
   return dontask_butt->isChecked();
}

bool
QDMimeDialog::dontCheck(void) const
{
   return dontcheck_butt->isChecked();
}
      
QDMimeDialog::QDMimeDialog(const QString & mime_fname, QWidget * parent,
			   const char * name, bool modal) :
      QeDialog(parent, name, modal)
{
   setCaption(tr("DjVu: MIME type checker"));

   QWidget * start=startWidget();
   
   QVBoxLayout * vlay=new QVBoxLayout(start, 15, 15);
   QFrame * frame=new QFrame(start, "frame");
   frame->setFrameStyle(QFrame::Box | QFrame::Sunken);
   vlay->addWidget(frame);

   QVBoxLayout * frame_vlay=new QVBoxLayout(frame, 20, 10);
   
   QString msg=tr("We have just found that your MIME type file ")+mime_fname+
      tr(" should be updated in order for the DjVu plugin to work properly. Invalid ")+
      mime_fname+
      tr(" can prevent Netscape from correctly displaying DjVu documents loaded from the hard disc and")+
      tr(" can make Netscape use incorrect MIME type when sending DjVu documents via e-mail.\n\nWould you like to update the ")+mime_fname+tr(" now?");
   QeLabel * label=new QeLabel(msg, frame);
   label->setAlignment(AlignLeft | WordBreak);
   frame_vlay->addWidget(label);

   QFrame * sep=new QFrame(frame, "sep");
   sep->setFrameStyle(QFrame::HLine | QFrame::Sunken);
   sep->setMinimumHeight(sep->sizeHint().height());
   frame_vlay->addWidget(sep);

   again_butt=new QeRadioButton(tr("Next time &check this again"), frame);
   frame_vlay->addWidget(again_butt);
   dontask_butt=new QeRadioButton(tr("Next time do the update &silently"), frame);
   frame_vlay->addWidget(dontask_butt);
   dontcheck_butt=new QeRadioButton(tr("&Never do this check again"), frame);
   frame_vlay->addWidget(dontcheck_butt);

   label->setMinimumWidth(again_butt->sizeHint().width()*2);

   QButtonGroup * bg=new QButtonGroup(frame);
   bg->hide();
   bg->insert(again_butt);
   bg->insert(dontask_butt);
   bg->insert(dontcheck_butt);

   DjVuPrefs prefs;
   dontask_butt->setChecked(prefs.mimeDontAsk);
   dontcheck_butt->setChecked(prefs.mimeDontCheck && !prefs.mimeDontAsk);
   again_butt->setChecked(!prefs.mimeDontCheck && !prefs.mimeDontAsk);

   frame_vlay->activate();
   
   QHBoxLayout * butt_lay=new QHBoxLayout(10);
   vlay->addLayout(butt_lay);
   butt_lay->addStretch(1);
   QePushButton * yes_butt=new QePushButton(tr("&Yes"), start, "yes_butt");
   yes_butt->setDefault(TRUE);
   butt_lay->addWidget(yes_butt);
   QePushButton * no_butt=new QePushButton(tr("&No"), start, "no_butt");
   butt_lay->addWidget(no_butt);

   vlay->activate();

   connect(yes_butt, SIGNAL(clicked(void)), this, SLOT(accept(void)));
   connect(no_butt, SIGNAL(clicked(void)), this, SLOT(reject(void)));
}
