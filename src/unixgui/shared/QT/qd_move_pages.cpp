//C-  -*- C++ -*-
//C-
//C-  Copyright © 1999-2001, LizardTech, Inc. All Rights Reserved.
//C-              Unauthorized use prohibited.
//C-
// 
// $Id: qd_move_pages.cpp,v 1.1 2001-08-08 17:00:32 docbill Exp $
// $Name:  $


#ifdef __GNUC__
#pragma implementation
#endif

#include "qd_move_pages.h"

#include "GString.h"

#include <stdlib.h>
#include <stdio.h>

#include <qpopupmenu.h>
#include <qcombobox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qpushbutton.h>

#include "qt_fix.h"

static int
cmp(const void * ptr1, const void * ptr2)
{
   int num1=*(int *) ptr1;
   int num2=*(int *) ptr2;
   return num1<num2 ? -1 : num1>num2 ? 1 : 0;
}

static GList<int>
sortList(const GList<int> & list)
{
   GArray<int> a(list.size()-1);
   int cnt;
   GPosition pos;
   for(pos=list, cnt=0;pos;++pos, cnt++)
      a[cnt]=list[pos];
   
   qsort((int *) a, a.size(), sizeof(int), cmp);

   GList<int> l;
   for(int i=0;i<a.size();i++)
      l.append(a[i]);
   
   return l;
}

QString
QDMovePagesDialog::listToString(const GList<int> & l)
{
   GList<int> list=sortList(l);

   QString str;
   
   GPosition pos_min=list;
   while(pos_min)
   {
      int min=list[pos_min];
      int max=min;
      GPosition pos_max=pos_min;
      ++pos_max;
      while(pos_max && list[pos_max]==max+1)
      {
	 max=list[pos_max];
	 ++pos_max;
      }

      if (!str.isEmpty())
	 str+=", ";
      
      if (max>min+1) str+=QString::number(min+1)+"-"+QString::number(max+1);
      else if (max==min+1) str+=QString::number(min+1)+", "+QString::number(max+1);
      else str+=QString::number(min+1);

      pos_min=pos_max;
   }
   return str;
}

int
QDMovePagesDialog::shift(void) const
{
   int shift=0;
   GUTF8String str=GStringFromQString(menu->currentText());
   if (strstr(str, "eginning"))
      shift=-pages_num;
   else if (strstr(str, "end"))
      shift=pages_num;
   else if (strstr(str, "down"))
      sscanf(str, "%d", &shift);
   else if (strstr(str, "up"))
   {
      sscanf(str, "%d", &shift);
      shift=-shift;
   }
   return shift;
}

QDMovePagesDialog::QDMovePagesDialog(const GList<int> & page_list, int _pages_num,
				     QWidget * parent, const char * name) :
      QeDialog(parent, name, TRUE), pages_num(_pages_num)
{
   QWidget * start=startWidget();

   setCaption("DjVu: Move pages to...");

   QVBoxLayout * vlay=new QVBoxLayout(start, 10, 10, "vlay");

   QeLabel * label=new QeLabel(tr("Please specify where pages ")+listToString(page_list)+"\n"+
			       tr("should be moved to:"), start);
   vlay->addWidget(label);

   QHBoxLayout * hlay=new QHBoxLayout(5);
   vlay->addLayout(hlay);

   label=new QeLabel("&Move pages: ", start);
   hlay->addWidget(label, 1);

   int max_page=-1;
   int min_page=pages_num;
   for(GPosition pos=page_list;pos;++pos)
   {
      int page=page_list[pos];
      if (page>max_page) max_page=page;
      if (page<min_page) min_page=page;
   }
   
   menu=new QeComboBox(false, start, "page_num_menu");
   for(int i=0;i<max_page;i++)
   {
      char buffer[64];
      sprintf(buffer, "%d %s up", max_page-i,
	      i==max_page-1 ? "page" : "pages");
      menu->insertItem(buffer);
   }
   menu->insertItem("to the beginning");
   menu->insertItem("don't move");
   menu->setCurrentItem(menu->count()-1);
   menu->insertItem("to the end");
   for(int i=pages_num-1;i>min_page;i--)
   {
      char buffer[64];
      sprintf(buffer, "%d %s down", pages_num-i,
	      i==pages_num-1 ? "page" : "pages");
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

   menu->setFocus();
}
