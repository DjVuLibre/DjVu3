//C-  -*- C++ -*-
//C-
//C-  Copyright © 1999-2001, LizardTech, Inc. All Rights Reserved.
//C-              Unauthorized use prohibited.
//C-
// 
// $Id: qd_mode_dialog.cpp,v 1.1 2001-08-08 17:00:32 docbill Exp $
// $Name:  $


#ifdef __GNUC__
#pragma implementation
#endif

#include "qd_mode_dialog.h"
#include "DjVuAnno.h"

#include "debug.h"

#include <qlayout.h>
#include <qpushbutton.h>
#include <qlabel.h>

#include "qt_fix.h"

// Mode comes in DjVuANT format
QDModeDialog::QDModeDialog(int mode, QWidget * parent,
			   const char * name, bool modal) :
      QeDialog(parent, name, modal)
{
   setCaption("DjVu: Recommended Mode");
   
   QWidget * start=startWidget();
   
   QVBoxLayout * vlay=new QVBoxLayout(start, 10, 15);
   QeLabel * label=new QeLabel("Please specify the recommended mode in which\n"
			       "the page should be displayed by the browser.\n", start);
   label->setAlignment(AlignCenter);
   vlay->addWidget(label);

   QHBoxLayout * hlay=new QHBoxLayout(10);
   vlay->addLayout(hlay);
   label=new QeLabel("Recommended mode:", start);
   hlay->addWidget(label, 1);
   menu=new QeComboBox(FALSE, start, "mode_menu");
   menu->insertItem("Color");
   menu->insertItem("Black and White");
   menu->insertItem("Foreground");
   menu->insertItem("Background");
   menu->insertItem("Default");
   hlay->addWidget(menu);
   switch(mode)
   {
      case DjVuANT::MODE_COLOR: menu->setCurrentItem(0); break;
      case DjVuANT::MODE_BW: menu->setCurrentItem(1); break;
      case DjVuANT::MODE_FORE: menu->setCurrentItem(2); break;
      case DjVuANT::MODE_BACK: menu->setCurrentItem(3); break;
      case DjVuANT::MODE_UNSPEC: menu->setCurrentItem(4); break;
      default: menu->setCurrentItem(0); break;
   };

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

   menu->setFocus();
}

int QDModeDialog::getMode(void) const
{
   switch(menu->currentItem())
   {
      case 0: return DjVuANT::MODE_COLOR;
      case 1: return DjVuANT::MODE_BW;
      case 2: return DjVuANT::MODE_FORE;
      case 3: return DjVuANT::MODE_BACK;
      case 4: return DjVuANT::MODE_UNSPEC;
   };
   return DjVuANT::MODE_COLOR;
}
