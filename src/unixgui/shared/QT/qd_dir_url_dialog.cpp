//C-  -*- C++ -*-
//C-
//C-  Copyright © 1999-2001, LizardTech, Inc. All Rights Reserved.
//C-              Unauthorized use prohibited.
//C-
// 
// $Id: qd_dir_url_dialog.cpp,v 1.1 2001-08-08 17:00:32 docbill Exp $
// $Name:  $


static char RCSVersion[]="@(#) $Id: qd_dir_url_dialog.cpp,v 1.1 2001-08-08 17:00:32 docbill Exp $";

#ifdef __GNUC__
#pragma implementation
#endif

#include "qd_dir_url_dialog.h"

#include "debug.h"

#include <qlayout.h>
#include <qpushbutton.h>
#include <qlabel.h>

#include "qt_fix.h"

QDDirURLDialog::QDDirURLDialog(const char * url, QWidget * parent,
			       const char * name, bool modal) :
      QeDialog(parent, name, modal)
{
   DEBUG_MSG("QDDirURLDialog::QDDirURLDialog(): initializing\n");

   setCaption("DjVu: Directory File URL");
   
   QWidget * start=startWidget();
   
   QVBoxLayout * vlay=new QVBoxLayout(start, 10, 15);
   QeLabel * label=new QeLabel("Please specify the URL for the file with the document directory.\n"
			       "This file is used to organize multiple DjVu pages\n"
			       "into a single multipage document.\n\n"
			       "If the URL doesn't start with a standard prefix (http://, file:/, etc.),\n"
			       "it will be assumed relative to the page URL.\n", start);
   label->setAlignment(AlignCenter);
   vlay->addWidget(label);

   QHBoxLayout * hlay=new QHBoxLayout(10);
   vlay->addLayout(hlay);
   label=new QeLabel("URL:", start);
   hlay->addWidget(label);
   text=new QeLineEdit(start, "url_text");
   text->setText(url);
   hlay->addWidget(text, 1);

   QHBoxLayout * butt_lay=new QHBoxLayout(10);
   vlay->addLayout(butt_lay);
   butt_lay->addStretch(1);
   QePushButton * ok_butt=new QePushButton("&OK", start, "ok_butt");
   ok_butt->setDefault(TRUE);
   butt_lay->addWidget(ok_butt);
   QePushButton * cancel_butt=new QePushButton("&Cancel", start, "cancel_butt");
   butt_lay->addWidget(cancel_butt);

   vlay->activate();

   connect(ok_butt, SIGNAL(clicked(void)), this, SLOT(accept(void)));
   connect(cancel_butt, SIGNAL(clicked(void)), this, SLOT(reject(void)));
}
