//C-  -*- C++ -*-
//C-
//C-  Copyright © 1999-2001, LizardTech, Inc. All Rights Reserved.
//C-              Unauthorized use prohibited.
//C-
// 
// $Id: qd_pnote_editor.cpp,v 1.1 2001-08-08 17:00:32 docbill Exp $
// $Name:  $


#ifdef __GNUC__
#pragma implementation
#endif

#include "qd_pnote_editor.h"

#ifdef UNIX
#include "qx_pnote.h"
#else
#include "qt_pnote.h"
#endif

#include "debug.h"

#include <qlayout.h>
#include <qlabel.h>
#include <qgroupbox.h>
#include <qapplication.h>

#include "qt_fix.h"

GPopupNote QDPNoteEditor::getNote(void) const
{
   return GPopupNote(text->text(), back_menu->color(), fore_menu->color());
}

void QDPNoteEditor::slotClosePreview(void)
{
   if (pnote_shell)
   {
      qeApp->killWidget(pnote_shell);
      pnote_shell=0;
   };
}

void QDPNoteEditor::slotShowPreview(void)
{
   slotClosePreview();
   
#ifdef UNIX
   pnote_shell=new QxPopupNote(getNote(), prv_butt, "pnote_preview");
#else
   pnote_shell=new QePopupNote(1, getNote(), prv_butt, "pnote_preview");
#endif
   pnote_shell->showNote(prv_butt->width()/2, prv_butt->height()/2);

   connect(pnote_shell, SIGNAL(sigClosed(void)),
	   this, SLOT(slotClosePreview(void)));
}

QDPNoteEditor::QDPNoteEditor(const GPopupNote & pnote, QWidget * parent,
			     const char * name) :
      QeDialog(parent, name, FALSE), GPopupNote(pnote), pnote_shell(0)
{
   setCaption("DjVu: Popup Note Editor");
   
   QWidget * start=startWidget();
   
   QeLabel * label;
   QFont font;
   
   QVBoxLayout * vlay=new QVBoxLayout(start, 10, 10);
   QeGroupBox * col_grp=new QeGroupBox("Colors", start, "col_grp");
   vlay->addWidget(col_grp);
   QGridLayout * col_glay=new QGridLayout(col_grp, 3, 2, 10, 10, "col_glay");
   col_glay->addRowSpacing(0, col_grp->fontMetrics().height());
   label=new QeLabel("&Background color:", col_grp);
   col_glay->addWidget(label, 1, 0);
   back_menu=new QeColorMenu(back_color, col_grp, "back_menu");
   label->setBuddy(back_menu);
   col_glay->addWidget(back_menu, 1, 1);
   label=new QeLabel("&Foreground color:", col_grp);
   col_glay->addWidget(label, 2, 0);
   fore_menu=new QeColorMenu(fore_color, col_grp, "fore_menu");
   label->setBuddy(fore_menu);
   col_glay->addWidget(fore_menu, 2, 1);
   col_glay->activate();

   QeGroupBox * msg_grp=new QeGroupBox("Message", start, "msg_grp");
   vlay->addWidget(msg_grp);
   QVBoxLayout * msg_vlay=new QVBoxLayout(msg_grp, 10, 10, "msg_vlay");
   msg_vlay->addSpacing(msg_grp->fontMetrics().height());
   label=new QeLabel("Enter text of the note here", msg_grp);
   label->setAlignment(AlignCenter);
   msg_vlay->addWidget(label);
   text=new QMultiLineEdit(msg_grp, "pnote_text");
   font=text->font(); font.setFamily("courier"); text->setFont(font);
   text->setText(note);
   text->setFixedVisibleLines(10);
   text->setMinimumWidth(text->fontMetrics().width("1234567890")*4);
   msg_vlay->addWidget(text);
   msg_vlay->activate();

   QeGroupBox * prv_grp=new QeGroupBox("Preview", start, "prv_grp");
   vlay->addWidget(prv_grp);
   QVBoxLayout * prv_vlay=new QVBoxLayout(prv_grp, 10, 10, "prv_vlay");
   prv_vlay->addSpacing(prv_grp->fontMetrics().height());
   label=new QeLabel("WARNING.", prv_grp);
   label->setAlignment(AlignCenter);
   font=label->font(); font.setWeight(QFont::Bold); label->setFont(font);
   prv_vlay->addWidget(label);
   label=new QeLabel("The actual shape of the popup note may be different on\n"
		     "different systems. This preview just gives an idea\n"
		     "about how this message may be displayed", prv_grp);
   prv_vlay->addWidget(label);
   label->setAlignment(AlignCenter);
   QHBoxLayout * prv_hlay=new QHBoxLayout(10, "prv_hlay");
   prv_vlay->addLayout(prv_hlay);
   prv_hlay->addStretch(1);
   prv_butt=new QePushButton("&Preview", prv_grp, "prv_butt");
   prv_hlay->addWidget(prv_butt, 1);
   prv_hlay->addStretch(1);
   prv_vlay->activate();
   
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
   connect(prv_butt, SIGNAL(clicked(void)), this, SLOT(slotShowPreview(void)));
}

QDPNoteEditor::~QDPNoteEditor(void)
{
   DEBUG_MSG("QDPNoteEditor::~QDPNoteEditor(): destroying the preview shell\n");
   slotClosePreview();
}
