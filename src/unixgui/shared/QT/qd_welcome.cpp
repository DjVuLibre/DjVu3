//C-  -*- C++ -*-
//C-
//C-  Copyright © 1999-2001, LizardTech, Inc. All Rights Reserved.
//C-              Unauthorized use prohibited.
//C-
// 
// $Id: qd_welcome.cpp,v 1.1 2001-05-29 22:05:31 bcr Exp $
// $Name:  $


#ifdef __GNUC__
#pragma implementation
#endif

#include "qd_welcome.h"
#include "qlib.h"
#include "debug.h"

#include <qmessagebox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qapplication.h>
#include <qpixmap.h>

#include "qt_fix.h"

#ifndef QT1
#include <q1xcompatibility.h>
#endif


bool QDWelcome::eventFilter(QObject *, QEvent * ev)
{
   if (ev->type()==Event_Resize)
      close_butt->setFixedSize(((QResizeEvent *) ev)->size());
   return FALSE;	// Continue dispatching normally
}

QDWelcome::QDWelcome(QWidget * parent, const char * name, bool modal) :
      QeDialog(parent, name, modal)
{
   DEBUG_MSG("QDWelcome::QDWelcome(): Creating Welcome dialog...\n");
   DEBUG_MAKE_INDENT(3);

   setCaption("Welcome to the DjVu Plug-in");

   QeLabel * label;

   QWidget * start=startWidget();
   QVBoxLayout * vlay=new QVBoxLayout(start, 10, 10, "vlay");
   QGridLayout * glay=new QGridLayout(3, 2, 10, "glay");
   vlay->addLayout(glay);
   QePushButton * prefs_butt=new QePushButton("&Preferences", start, "prefs_butt");
   prefs_butt->inflateHeight(3);
   glay->addWidget(prefs_butt, 0, 0);
   label=new QeLabel("Adjust gamma correction, caches, etc.", start, "prefs_label");
   glay->addWidget(label, 0, 1);
   QePushButton * help_butt=new QePushButton("&Help", start, "help_butt");
   help_butt->inflateHeight(3);
   glay->addWidget(help_butt, 1, 0);
   label=new QeLabel("Learn how to use DjVu", start, "help_label");
   glay->addWidget(label, 1, 1);
   QePushButton * about_butt=new QePushButton("&About", start, "about_butt");
   about_butt->inflateHeight(3);
   glay->addWidget(about_butt, 2, 0);
   label=new QeLabel("Credit and links", start, "about_label");
   glay->addWidget(label, 2, 1);

   QHBoxLayout * hlay=new QHBoxLayout(10);
   vlay->addLayout(hlay);
   label=new QeLabel(start, "icon");
   label->setPixmap(QMessageBox::standardIcon(QMessageBox::Information,
					      QApplication::style()));
   hlay->addWidget(label);
   label=new QeLabel("These functions and many others can be\n"
		     "accessed from within a DjVu document\n"
		     "by clicking the right mouse button.", start, "info");
   label->setAlignment(AlignCenter);
   hlay->addWidget(label, 1);

   hlay=new QHBoxLayout(10);
   vlay->addLayout(hlay);
   close_butt=new QePushButton("&Close", start, "close_butt");
   hlay->addWidget(close_butt);
   never_butt=new QeCheckBox("&Never show this window again", start, "never_butt");
   never_butt->setChecked(FALSE);
   hlay->addWidget(never_butt);

   close_butt->setDefault(TRUE);
   
   vlay->activate();

      // Connecting signals and slots
   connect(close_butt, SIGNAL(clicked(void)), this, SLOT(accept(void)));
   connect(close_butt, SIGNAL(clicked(void)), this, SIGNAL(closed(void)));
   connect(prefs_butt, SIGNAL(clicked(void)), this, SIGNAL(preferences(void)));
   connect(help_butt, SIGNAL(clicked(void)), this, SIGNAL(help(void)));
   connect(about_butt, SIGNAL(clicked(void)), this, SIGNAL(about(void)));

   connect(this, SIGNAL(sigCancelled(void)), this, SIGNAL(closed(void)));
   connect(this, SIGNAL(sigClosed(void)), this, SIGNAL(closed(void)));
   
   prefs_butt->installEventFilter(this);
}
