//C-  -*- C++ -*-
//C-
//C-  Copyright © 1999-2001, LizardTech, Inc. All Rights Reserved.
//C-              Unauthorized use prohibited.
//C-
// 
// $Id: qd_align_dialog.cpp,v 1.1 2001-08-08 17:00:32 docbill Exp $
// $Name:  $


#ifdef __GNUC__
#pragma implementation
#endif

#include "qd_align_dialog.h"
#include "DjVuAnno.h"

#include "debug.h"

#include <qlayout.h>
#include <qpushbutton.h>
#include <qlabel.h>
#include <qbuttongroup.h>

#include "qt_fix.h"

int QDAlignDialog::horAlignment(void) const
{
   return
      left_butt->isChecked() ? DjVuANT::ALIGN_LEFT :
      hcenter_butt->isChecked() ? DjVuANT::ALIGN_CENTER :
      right_butt->isChecked() ? DjVuANT::ALIGN_RIGHT :
      DjVuANT::ALIGN_UNSPEC;
}

int QDAlignDialog::verAlignment(void) const
{
   return
      top_butt->isChecked() ? DjVuANT::ALIGN_TOP :
      vcenter_butt->isChecked() ? DjVuANT::ALIGN_CENTER :
      bottom_butt->isChecked() ? DjVuANT::ALIGN_BOTTOM :
      DjVuANT::ALIGN_UNSPEC;
}

QDAlignDialog::QDAlignDialog(int hor, int ver, QWidget * parent,
			     const char * name, bool modal) :
      QeDialog(parent, name, modal)
{
   setCaption("DjVu: Page Alignment");
   
   QWidget * start=startWidget();
   
   QVBoxLayout * vlay=new QVBoxLayout(start, 10, 10);
   QHBoxLayout * hlay=new QHBoxLayout(10);
   vlay->addLayout(hlay);
   QVBoxLayout * title_vlay=new QVBoxLayout(15);
   hlay->addLayout(title_vlay);
   title_vlay->addStretch(1);
   QeLabel * label=new QeLabel("DjVu page alignment.", start);
   label->setAlignment(AlignCenter);
   QFont font=label->font(); font.setWeight(QFont::Bold);
   font.setPointSize(font.pointSize()+2);
   label->setFont(font);
   title_vlay->addWidget(label);
   title_vlay->addStretch(1);
   label=new QeLabel("Please note, that the alignment is in effect\n"
		     "only when the corresponding document dimension\n"
		     "is less than the window's one.", start);
   label->setAlignment(AlignCenter);
   title_vlay->addWidget(label);
   title_vlay->addStretch(1);

   QeButtonGroup * hgrp=new QeButtonGroup("Horizontal", start, "hgrp");
   hlay->addWidget(hgrp);
   QVBoxLayout * hgrp_lay=new QVBoxLayout(hgrp, 10, 10, "hgrp_lay");
   hgrp_lay->addSpacing(hgrp->fontMetrics().height());
   hgrp_lay->addStrut(hgrp->fontMetrics().boundingRect(hgrp->title()).width()+5);
   left_butt=new QeRadioButton("&Left", hgrp, "left_butt");
   hgrp_lay->addWidget(left_butt);
   hcenter_butt=new QeRadioButton("C&enter", hgrp, "hcenter_butt");
   hgrp_lay->addWidget(hcenter_butt);
   right_butt=new QeRadioButton("&Right", hgrp, "right_butt");
   hgrp_lay->addWidget(right_butt);
   hdef_butt=new QeRadioButton("&Default", hgrp, "hdef_butt");
   hgrp_lay->addWidget(hdef_butt);
   hgrp_lay->activate();

   QeButtonGroup * vgrp=new QeButtonGroup("Vertical", start, "vgrp");
   hlay->addWidget(vgrp);
   QVBoxLayout * vgrp_lay=new QVBoxLayout(vgrp, 10, 10, "vgrp_lay");
   vgrp_lay->addSpacing(vgrp->fontMetrics().height());
   vgrp_lay->addStrut(vgrp->fontMetrics().boundingRect(vgrp->title()).width()+5);
   top_butt=new QeRadioButton("&Top", vgrp, "top_butt");
   vgrp_lay->addWidget(top_butt);
   vcenter_butt=new QeRadioButton("Ce&nter", vgrp, "vcenter_butt");
   vgrp_lay->addWidget(vcenter_butt);
   bottom_butt=new QeRadioButton("&Bottom", vgrp, "bottom_butt");
   vgrp_lay->addWidget(bottom_butt);
   vdef_butt=new QeRadioButton("&Default", vgrp, "vdef_butt");
   vgrp_lay->addWidget(vdef_butt);
   vgrp_lay->activate();

   QHBoxLayout * butt_lay=new QHBoxLayout(10);
   vlay->addLayout(butt_lay);
   butt_lay->addStretch(1);
   QePushButton * ok_butt=new QePushButton("&OK", start, "ok_butt");
   ok_butt->setDefault(TRUE);
   butt_lay->addWidget(ok_butt);
   QePushButton * cancel_butt=new QePushButton("&Cancel", start, "cancel_butt");
   butt_lay->addWidget(cancel_butt);

   vlay->activate();

      // *** Connecting signals
   connect(ok_butt, SIGNAL(clicked(void)), this, SLOT(accept(void)));
   connect(cancel_butt, SIGNAL(clicked(void)), this, SLOT(reject(void)));

      // *** Setting defaults
   switch(hor)
   {
      case DjVuANT::ALIGN_LEFT: left_butt->setChecked(TRUE); break;
      case DjVuANT::ALIGN_CENTER: hcenter_butt->setChecked(TRUE); break;
      case DjVuANT::ALIGN_RIGHT: right_butt->setChecked(TRUE); break;
      default:
      case DjVuANT::ALIGN_UNSPEC: hdef_butt->setChecked(TRUE); break;
   }

   switch(ver)
   {
      case DjVuANT::ALIGN_TOP: top_butt->setChecked(TRUE); break;
      case DjVuANT::ALIGN_CENTER: vcenter_butt->setChecked(TRUE); break;
      case DjVuANT::ALIGN_BOTTOM: bottom_butt->setChecked(TRUE); break;
      default:
      case DjVuANT::ALIGN_UNSPEC: vdef_butt->setChecked(TRUE); break;
   }

   left_butt->setFocus();
}
