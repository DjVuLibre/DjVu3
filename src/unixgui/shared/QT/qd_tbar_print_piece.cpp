//C-  -*- C++ -*-
//C-
//C-  Copyright © 1999-2001, LizardTech, Inc. All Rights Reserved.
//C-              Unauthorized use prohibited.
//C-
// 
// $Id: qd_tbar_print_piece.cpp,v 1.2 2001-06-07 22:13:55 mchen Exp $
// $Name:  $


#ifdef __GNUC__
#pragma implementation
#endif

#include "qd_tbar_print_piece.h"
#include "debug.h"
#include "qlib.h"
#include "qd_toolbutt.h"
#include "djvu_base_res.h"
#include "cin_data.h"

#include "qt_fix.h"

//****************************************************************************
//***************************** QDTBarPrintPiece *****************************
//****************************************************************************

void
QDTBarPrintPiece::setEnabled(bool en)
{
   print_butt->setEnabled(en);
   find_butt->setEnabled(en);
   save_butt->setEnabled(en);
}

QDTBarPrintPiece::QDTBarPrintPiece(QDToolBar * toolbar) : QDTBarPiece(toolbar)
{
   QFrame * frame;
   
   frame=new QFrame(toolbar, "separator");
   frame->setFrameStyle(QFrame::VLine | QFrame::Sunken);
   frame->setMinimumWidth(10);
   toolbar->addLeftWidget(frame);
   
   find_butt=new QDToolButton(*CINData::get("ppm_vfind"), true,
			      IDC_SEARCH, toolbar, tr("Find"));
   connect(find_butt, SIGNAL(clicked(void)), this, SIGNAL(sigFind(void)));
   toolbar->addLeftWidget(find_butt);

   print_butt=new QDToolButton(*CINData::get("ppm_vprint"), true,
			       IDC_PRINT, toolbar, tr("Print"));
   connect(print_butt, SIGNAL(clicked(void)), this, SIGNAL(sigPrint(void)));
   toolbar->addLeftWidget(print_butt);

   save_butt=new QDToolButton(*CINData::get("ppm_vsave"), true,
			      IDC_SAVE_DOC, toolbar, tr("Save"));
   connect(save_butt, SIGNAL(clicked(void)), this, SIGNAL(sigSave(void)));
   toolbar->addLeftWidget(save_butt);
   
   toolbar->addPiece(this);
   toolbar->adjustPositions();
}
