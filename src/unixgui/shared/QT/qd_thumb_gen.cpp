//C-  -*- C++ -*-
//C-
//C-  Copyright © 1999-2001, LizardTech, Inc. All Rights Reserved.
//C-              Unauthorized use prohibited.
//C-
// 
// $Id: qd_thumb_gen.cpp,v 1.1 2001-05-29 22:05:31 bcr Exp $
// $Name:  $


#ifdef __GNUC__
#pragma implementation
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
