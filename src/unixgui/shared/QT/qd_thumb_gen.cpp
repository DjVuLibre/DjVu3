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
// $Id: qd_thumb_gen.cpp,v 1.3.2.1 2001-10-23 21:16:48 leonb Exp $
// $Name:  $

#ifdef __GNUG__
#pragma implementation
#endif
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "qd_thumb_gen.h"

#include <qlabel.h>
#include <qvalidator.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qspinbox.h>
#include <qprogressdialog.h>

#include "qlib.h"
#include "qt_fix.h"

#define MIN_SIZE	10
#define MIN_SIZE_STR	"10"
#define MAX_SIZE	256
#define MAX_SIZE_STR	"256"

class QDThumbGenVal : public QValidator
{
public:
   virtual void		fixup(QString &);
#ifdef QT1
   virtual State	validate(QString &, int &);
#else
   virtual State	validate(QString &, int &) const;
#endif
   
   QDThumbGenVal(QSpinBox * parent, const char * name=0) :
	 QValidator(parent, name) {};
};

void
QDThumbGenVal::fixup(QString & str)
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
QDThumbGenVal::validate(QString & input, int & pos)
#ifndef QT1
const
#endif
{
   if (!input.length()) return Valid;
   
   bool status;
   int size=input.toInt(&status);
   if (!status) return Invalid;

   if (size<=0) return Invalid;
   if (size>MAX_SIZE) return Invalid;
   if (size<MIN_SIZE)
      if (input.length()==1) return Valid;
      else return Invalid;

   return Acceptable;
}

bool
QDThumbGen::progress_cb(int page_num)
{
   if (!progress_dialog)
   {
      progress_dialog=new QProgressDialog("", tr("Cancel"), doc->get_pages_num(),
					  this, "progress", TRUE);
      progress_dialog->setCaption(tr("DjVu Thumbnails integrator"));
      QeLabel * l=new QeLabel(tr("Generating thumbnails. Please wait..."), progress_dialog);
      progress_dialog->setLabel(l);
      progress_dialog->setMinimumWidth(l->sizeHint().width()+20);
      progress_dialog->setProgress(0);	// Required!!!
      progress_dialog->show();
   }
   progress_dialog->setProgress(page_num+1);
   emit sigProgress(page_num);
   return progress_dialog->wasCancelled();
}

void
QDThumbGen::done(int rc)
{
   if (rc==Accepted)
   {
      int size=spin->value();
      if (size<MIN_SIZE)
	 showError(this, "DjVu Error",
		   tr("Thumbnails size should not be less than ")+MIN_SIZE_STR+".");
      else if (size>MAX_SIZE)
	 showError(this, "DjVu Error",
		   tr("Thumbnails size should not be greater than ")+MAX_SIZE_STR+".");
      else
      {
	 QeDialog::done(rc);
	 int old_size=doc->get_thumbnails_size();
	 if (old_size>0 && size!=old_size)
         {
	    doc->remove_thumbnails();
         }
	 delete progress_dialog;
         progress_dialog=0;
         int page_num=0;
         do
         {
           page_num=doc->generate_thumbnails(size,page_num);
           if(progress_cb(page_num)) 
           {
             break;
           }
         } while(page_num>=0);
	 emit sigDocModified();
      }
   } else
   {
     QeDialog::done(rc);
   }
}

QDThumbGen::QDThumbGen(GP<DjVuDocEditor> &_doc,
		       QWidget * parent, const char * name) :
      QeDialog(parent, name, TRUE), doc(_doc), progress_dialog(0)
{
   setCaption(tr("DjVu: Thumbnails integrator"));
   
   QWidget * start=startWidget();

   QeLabel * label;
   QVBoxLayout * vlay=new QVBoxLayout(start, 10, 10);
   QString msg;
   int size=128;
   if (doc->get_thumbnails_num()>0)
   {
      size=doc->get_thumbnails_size();
      msg=tr("This document already contains some thumbnails\n")+
	 tr("of the size specified below. If you do not change it\n")+
	 tr("we will generate only missing thumbnails.\n")+
	 tr("Otherwise all thumbnails will be regenerated.\n");
   } else
      msg=tr("This document does not contain any thumnbail images yet.");
   label=new QeLabel(msg, start);
   vlay->addWidget(label);
   
   QHBoxLayout * hlay=new QHBoxLayout(10);
   vlay->addLayout(hlay);
   label=new QeLabel(tr("Thumbnail image &size"), start);
   hlay->addWidget(label);
   spin=new QeSpinBox(MIN_SIZE, MAX_SIZE, 10, start);
   spin->setValidator(new QDThumbGenVal(spin));
   spin->setValue(size);
   label->setBuddy(spin);
   hlay->addWidget(spin);
   label=new QeLabel(tr(" pixels"), start);
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
