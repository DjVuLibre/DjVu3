//C-  -*- C++ -*-
//C-
//C-  Copyright © 1999-2001, LizardTech, Inc. All Rights Reserved.
//C-              Unauthorized use prohibited.
//C-
// 
// $Id: qd_move_page.cpp,v 1.1 2001-08-08 17:00:32 docbill Exp $
// $Name:  $


#ifdef __GNUC__
#pragma implementation
#endif

#include "qd_move_page.h"

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
QDMovePageDialog::pageNum(void) const
{
   return atoi(goto_menu->currentText())-1;
}

void
QDMovePageDialog::slotGotoActivated(const QString & qtext)
{
   const char * const text=qtext;
   int goto_page=atoi(text)-1;

   if (goto_page==src_page_num)
      shift_menu->setCurrentItem(src_page_num+1);
   else if (goto_page==0)
      shift_menu->setCurrentItem(src_page_num);
   else if (goto_page==goto_menu->count()-1)
      shift_menu->setCurrentItem(src_page_num+2);
   else if (goto_page<src_page_num)
      shift_menu->setCurrentItem(goto_page);
   else
      shift_menu->setCurrentItem(goto_page+2);
}

void
QDMovePageDialog::slotShiftActivated(const QString & qtext)
{
   const char * const text=qtext;
   int goto_page=0;
   if (strstr(text, "up"))
   {
      sscanf(text, "%d", &goto_page);
      goto_page=src_page_num-goto_page;
   } else if (strstr(text, "down"))
   {
      sscanf(text, "%d", &goto_page);
      goto_page=src_page_num+goto_page;
   } else if (strstr(text, "beginning"))
      goto_page=0;
   else if (strstr(text, "don't"))
      goto_page=src_page_num;
   else if (strstr(text, "end"))
      goto_page=goto_menu->count()-1;

   goto_menu->setCurrentItem(goto_page);
}

QDMovePageDialog::QDMovePageDialog(int _src_page_num, int pages_num,
				   QWidget * parent, const char * name) :
      QeDialog(parent, name, TRUE), src_page_num(_src_page_num)
{
   QWidget * start=startWidget();

   setCaption(tr("DjVu: Move page to..."));

   QVBoxLayout * vlay=new QVBoxLayout(start, 10, 10, "vlay");

   QeLabel * label=new QeLabel(tr("Please specify where the page ")+
			       QString::number(src_page_num+1)+
			       tr("\nshould be moved to:"), start);
   vlay->addWidget(label);

   QHBoxLayout * hlay=new QHBoxLayout(5);
   vlay->addLayout(hlay);

   label=new QeLabel(tr("&New page number: "), start);
   hlay->addWidget(label, 1);

   goto_menu=new QeComboBox(false, start, "goto_menu");
   for(int page_num=0;page_num<pages_num;page_num++)
   {
      char buffer[64];
      sprintf(buffer, "%d", page_num+1);
      goto_menu->insertItem(buffer);
   }
   label->setBuddy(goto_menu);
   hlay->addWidget(goto_menu);

   QFrame * sep=new QFrame(start, "sep");
   sep->setFrameStyle(QFrame::HLine | QFrame::Sunken);
   sep->setMinimumHeight(10);
   vlay->addWidget(sep);
   
   label=new QeLabel(tr("Or choose the shift from the\nmenu below:"), start);
   vlay->addWidget(label);

   hlay=new QHBoxLayout(5);
   vlay->addLayout(hlay);
   
   label=new QeLabel(tr("&Shift page: "), start);
   hlay->addWidget(label, 1);

   shift_menu=new QeComboBox(false, start, "shift_menu");
   const char *page=(const char *)tr("page");
   const char *pages=(const char *)tr("pages");
   for(int i=0;i<src_page_num;i++)
   {
      char buffer[64];
      sprintf(buffer, "%d %s up", src_page_num-i,
	      i==src_page_num-1 ? page : pages);
      shift_menu->insertItem(buffer);
   }
   down_shift_items=shift_menu->count();
   shift_menu->insertItem(tr("to the beginning"));
   shift_menu->insertItem(tr("don't shift"));
   shift_menu->insertItem(tr("to the end"));
   for(int i=src_page_num+1;i<pages_num;i++)
   {
      char buffer[64];
      sprintf(buffer, "%d %s down", i-src_page_num,
	      i==src_page_num+1 ? page : pages);
      shift_menu->insertItem(buffer);
   }
   label->setBuddy(shift_menu);
   hlay->addWidget(shift_menu);

   vlay->addSpacing(5);
   
   QHBoxLayout * butt_lay=new QHBoxLayout(10);
   vlay->addLayout(butt_lay);
   butt_lay->addStretch(1);
   QePushButton * ok_butt=new QePushButton(tr("&OK"), start, "ok_butt");
   butt_lay->addWidget(ok_butt);
   QePushButton * cancel_butt=new QePushButton(tr("&Cancel"), start, "cancel_butt");
   butt_lay->addWidget(cancel_butt);
   ok_butt->setDefault(TRUE);
   
   vlay->activate();

      // Connecting signals and slots
   connect(ok_butt, SIGNAL(clicked(void)), this, SLOT(accept(void)));
   connect(cancel_butt, SIGNAL(clicked(void)), this, SLOT(reject(void)));
   connect(shift_menu, SIGNAL(activated(const QString &)),
	   this, SLOT(slotShiftActivated(const QString &)));
   connect(goto_menu, SIGNAL(activated(const QString &)),
	   this, SLOT(slotGotoActivated(const QString &)));

   goto_menu->setCurrentItem(src_page_num);
   goto_menu->setFocus();
   slotGotoActivated(goto_menu->currentText());
}
