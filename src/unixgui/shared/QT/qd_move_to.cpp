//C-  -*- C++ -*-
//C-
//C-  Copyright © 1999-2001, LizardTech, Inc. All Rights Reserved.
//C-              Unauthorized use prohibited.
//C-
// 
// $Id: qd_move_to.cpp,v 1.1 2001-08-08 17:00:32 docbill Exp $
// $Name:  $


#ifdef __GNUC__
#pragma implementation
#endif

#include "qd_move_to.h"

#include "GString.h"

#include <stdlib.h>
#include <stdio.h>

#include <qpopupmenu.h>
#include <qcombobox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qpushbutton.h>

#include "qt_fix.h"

int
QDMoveToDialog::pageNum(void) const
{
   return atoi(menu->text(menu->currentItem()))-1;
}

QDMoveToDialog::QDMoveToDialog(int src_page_num, int pages_num,
			       QWidget * parent, const char * name) :
      QeDialog(parent, name, TRUE)
{
   QWidget * start=startWidget();

   setCaption("DjVu: Page number");

   QVBoxLayout * vlay=new QVBoxLayout(start, 10, 10, "vlay");

   QeLabel * label=new QeLabel("Please specify where page "+GUTF8String(src_page_num+1)+
			       "\nshould be moved to:", start);
   vlay->addWidget(label);

   QHBoxLayout * hlay=new QHBoxLayout(5);
   vlay->addLayout(hlay);

   label=new QeLabel("&Page number: ", start);
   hlay->addWidget(label, 1);

   menu=new QeComboBox(false, start, "page_num_menu");
   for(int page_num=0;page_num<pages_num;page_num++)
      if (page_num!=src_page_num)
      {
	 char buffer[64];
	 sprintf(buffer, "%d", page_num+1);
	 menu->insertItem(buffer);
      }
   label->setBuddy(menu);
   hlay->addWidget(menu);
   
   QHBoxLayout * butt_lay=new QHBoxLayout(10);
   vlay->addLayout(butt_lay);
   butt_lay->addStretch(1);
   QePushButton * ok_butt=new QePushButton("&OK", start, "ok_butt");
   butt_lay->addWidget(ok_butt);
   QePushButton * cancel_butt=new QePushButton("&Cancel", start, "cancel_butt");
   butt_lay->addWidget(cancel_butt);
   ok_butt->setDefault(TRUE);
   
   vlay->activate();

      // Connecting signals and slots
   connect(ok_butt, SIGNAL(clicked(void)), this, SLOT(accept(void)));
   connect(cancel_butt, SIGNAL(clicked(void)), this, SLOT(reject(void)));
}
