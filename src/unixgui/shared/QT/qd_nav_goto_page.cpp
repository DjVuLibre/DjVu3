//C-  -*- C++ -*-
//C-
//C-  Copyright © 1999-2001, LizardTech, Inc. All Rights Reserved.
//C-              Unauthorized use prohibited.
//C-
// 
// $Id: qd_nav_goto_page.cpp,v 1.2 2001-06-06 14:53:58 mchen Exp $
// $Name:  $


#ifdef __GNUC__
#pragma implementation
#endif
#include <stdio.h>

#include "DjVuDocument.h"
#include "DjVuImage.h"

#include "qd_nav_goto_page.h"

#include <qlabel.h>
#include <qvalidator.h>
#include <qlayout.h>
#include <qpushbutton.h>

#include "qt_fix.h"

class QDPageNumVal : public QValidator
{
public:
   virtual void		fixup(QString &);
#ifdef QT1
   virtual State	validate(QString &, int &);
#else
   virtual State	validate(QString &, int &) const;
#endif

   QDPageNumVal(QComboBox * parent, const char * name=0) :
   QValidator(parent, name) {};
};

void
QDPageNumVal::fixup(QString & str)
{
#ifdef QT1
   str.detach();
#else
   str.truncate(0);
#endif

   QComboBox * menu=(QComboBox *) parent();
   menu->setEditText(str=menu->text(menu->currentItem()));
}

QDPageNumVal::State
QDPageNumVal::validate(QString & input, int & pos)
#ifndef QT1
const
#endif
{
   if (!input.length()) return Valid;
   
   bool status;
   int page=input.toInt(&status)-1;
   if (!status) return Invalid;

   if (page<0) return Invalid;

   QComboBox * menu=(QComboBox *) parent();
   if (page>=menu->count()) return Invalid;
   
   return Acceptable;
}

int
QDNavGotoPage::getPageNum(void) const
{
      // We can't use menu->currentItem() here because it won't work when
      // user presses "Enter" in the menu
   return atoi(menu->currentText())-1;
}

QDNavGotoPage::QDNavGotoPage(GP<DjVuDocument> &doc,
			     DjVuImage * dimg,
			     QWidget * parent, const char * name) :
      QeDialog(parent, name, TRUE)
{
   setCaption(tr("DjVu: Goto Page"));
   QWidget * start=startWidget();

      // Create the menu
   QVBoxLayout * vlay=new QVBoxLayout(start, 10, 10);
   QHBoxLayout * hlay=new QHBoxLayout(10);
   vlay->addLayout(hlay);
   QeLabel * label=new QeLabel(tr("Goto page"), start, "goto_label");
   hlay->addWidget(label);
   menu=new QeComboBox(TRUE, start, "goto_menu");
   menu->setInsertionPolicy(QComboBox::NoInsertion);
   menu->setValidator(new QDPageNumVal(menu));
   hlay->addWidget(menu);

      // Create the buttons
   hlay=new QHBoxLayout(10);
   vlay->addLayout(hlay);
   hlay->addStretch(1);
   QePushButton * ok_butt=new QePushButton(tr("&OK"), start, "ok_butt");
   ok_butt->setDefault(TRUE);
   hlay->addWidget(ok_butt);
   QePushButton * cancel_butt=new QePushButton(tr("&Cancel"), start, "cancel_butt");
   hlay->addWidget(cancel_butt);

      // Set menu contents
   int cur_page=doc->url_to_page(dimg->get_djvu_file()->get_url());
   int pages=doc->get_pages_num();
   for(int i=0;i<pages;i++)
   {
      char buffer[64];
      sprintf(buffer, "%d", i+1);
      menu->insertItem(buffer);
   };
   menu->setCurrentItem(cur_page);
   
      // Connect signals
   connect(ok_butt, SIGNAL(clicked(void)), this, SLOT(accept(void)));
   connect(cancel_butt, SIGNAL(clicked(void)), this, SLOT(reject(void)));
}
