//C-  -*- C++ -*-
//C-
//C-  Copyright © 1999-2001, LizardTech, Inc. All Rights Reserved.
//C-              Unauthorized use prohibited.
//C-
// 
// $Id: qd_tbar_rotate_piece.cpp,v 1.1 2001-05-29 22:05:31 bcr Exp $
// $Name:  $


#ifdef __GNUC__
#pragma implementation
#endif

#include "qd_tbar_rotate_piece.h"
#include "debug.h"
#include "qlib.h"
#include "qd_toolbutt.h"
#include "djvu_base_res.h"
#include "cin_data.h"

#include "qt_fix.h"

//****************************************************************************
//***************************** QDTBarRotatePiece *****************************
//****************************************************************************

void
QDTBarRotatePiece::setEnabled(bool en)
{
   rotate90_butt->setEnabled(en);
   rotate270_butt->setEnabled(en);
}

QDTBarRotatePiece::QDTBarRotatePiece(QDToolBar * toolbar) : QDTBarPiece(toolbar)
{
   QFrame * frame;
   
   frame=new QFrame(toolbar, "separator");
   frame->setFrameStyle(QFrame::VLine | QFrame::Sunken);
   frame->setMinimumWidth(10);
   toolbar->addLeftWidget(frame);
   
   rotate90_butt=new QDToolButton(*CINData::get("ppm_rotate90"), true,
				  IDC_ROTATE_90, toolbar, tr("Rotate +90"));
   connect(rotate90_butt, SIGNAL(clicked(void)), this, SLOT(slotRotate()));
   toolbar->addLeftWidget(rotate90_butt);

   rotate270_butt=new QDToolButton(*CINData::get("ppm_rotate270"), true,
				   IDC_ROTATE_270, toolbar, tr("Rotate -90"));
   connect(rotate270_butt, SIGNAL(clicked(void)), this, SLOT(slotRotate()));
   toolbar->addLeftWidget(rotate270_butt);
   
   toolbar->addPiece(this);
   toolbar->adjustPositions();
}


void
QDTBarRotatePiece::slotRotate(void)
{
   const QObject * obj=sender();
   if (obj && obj->isWidgetType() && obj->inherits("QDToolButton"))
   {
      const QDToolButton * butt=(QDToolButton *) obj;
      emit sigRotate(butt->cmd);
   }
}

