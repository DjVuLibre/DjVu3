//C-  -*- C++ -*-
//C-
//C-  Copyright © 1999-2001, LizardTech, Inc. All Rights Reserved.
//C-              Unauthorized use prohibited.
//C-
// 
// $Id: qd_bg_col_dialog.cpp,v 1.1 2001-08-08 17:00:32 docbill Exp $
// $Name:  $


#ifdef __GNUC__
#pragma implementation
#endif

#include "qd_bg_col_dialog.h"

#include "debug.h"

#include <qlayout.h>
#include <qpushbutton.h>
#include <qlabel.h>

#include "qt_fix.h"

QDBGColorDialog::QDBGColorDialog(u_int32 color, QWidget * parent,
				 const char * name, bool modal) :
      QeDialog(parent, name, modal)
{
   setCaption("DjVu: Background Color");
   
   QWidget * start=startWidget();
   
   QVBoxLayout * vlay=new QVBoxLayout(start, 10, 10);
   QeLabel * label=new QeLabel("Please choose the color of the background\n"
			       "(frame around the page displayed)", start);
   label->setAlignment(AlignCenter);
   vlay->addWidget(label);

   QHBoxLayout * hlay=new QHBoxLayout(10);
   vlay->addLayout(hlay);
   label=new QeLabel("Background color", start);
   hlay->addWidget(label);
   menu=new QeColorMenu(color, start, "bg_col_menu");
   hlay->addWidget(menu);

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
