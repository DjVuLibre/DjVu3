//C-  -*- C++ -*-
//C-
//C-  Copyright © 1999-2001, LizardTech, Inc. All Rights Reserved.
//C-              Unauthorized use prohibited.
//C-
// 
// $Id: qd_editor_prefs.cpp,v 1.1 2001-08-08 17:00:32 docbill Exp $
// $Name:  $


#ifdef __GNUC__
#pragma implementation
#endif

#include "qd_editor_prefs.h"
#include "qlib.h"
#include "debug.h"
#include "exc_msg.h"

#include <qlabel.h>
#include <qgroupbox.h>
#include <qlayout.h>
#include <stdio.h>
#include "qt_fix.h"

void
QDEditorPrefs::done(int rc)
{
   DEBUG_MSG("QDEditorPrefs::done() called\n");
   DEBUG_MAKE_INDENT(3);
   
   if (rc==Accepted)
   {
      DEBUG_MSG("OK pressed. Updating preferences structure\n");

      DjVuPrefs disk_prefs;
      disk_prefs.dScreenGamma=prefs->dScreenGamma=gamma_prefs->displGamma();
      disk_prefs.dPrinterGamma=prefs->dPrinterGamma=
			       gamma_prefs->match() ? 0 : gamma_prefs->printGamma();
      disk_prefs.fastZoom=prefs->fastZoom=optim_prefs->fastZoom();
      disk_prefs.fastThumb=prefs->fastThumb=optim_prefs->fastThumb();
      disk_prefs.optimizeLCD=prefs->optimizeLCD=optim_prefs->optimizeLCD();
      disk_prefs.magnifierSize=prefs->magnifierSize=lens_prefs->size();
      disk_prefs.magnifierScale=prefs->magnifierScale=lens_prefs->scale();
      disk_prefs.magnifierHotKey=prefs->magnifierHotKey=lens_prefs->hotKey();
      disk_prefs.mcacheSize=prefs->mcacheSize=cache_prefs->mcacheSize();

//        disk_prefs.toolBarOn=prefs->toolBarOn=tbar_prefs->enabled();
//        disk_prefs.toolBarAlwaysVisible=prefs->toolBarAlwaysVisible=tbar_prefs->visible();
//        disk_prefs.toolBarDelay=prefs->toolBarDelay=tbar_prefs->delay();
      
      disk_prefs.save();
   } else
   {
      DEBUG_MSG("Cancel pressed.\n");
   }
   QeDialog::done(rc);
}

QDEditorPrefs::QDEditorPrefs(DjVuPrefs * _prefs, QWidget * parent,
			     const char * name, bool modal) :
      QeDialog(parent, name, modal), prefs(_prefs)
{
   DEBUG_MSG("QDEditorPrefs::QDEditorPrefs(): Creating Editor Prefs dialog...\n");
   DEBUG_MAKE_INDENT(3);

   setCaption("DjVu: Editor Preferences");
   
   QWidget * start=startWidget();
   QVBoxLayout * top_vlay=new QVBoxLayout(start, 10, 5, "vlay");

   QHBoxLayout * hlay=new QHBoxLayout(5);
   top_vlay->addLayout(hlay);

   QVBoxLayout * vlay=new QVBoxLayout(5);
   hlay->addLayout(vlay);
   
   gamma_prefs=new QDGammaPrefs(prefs, start, "gamma_prefs");
   vlay->addWidget(gamma_prefs);

   cache_prefs=new QDCachePrefs(prefs, false, start, "cache_prefs");
   vlay->addWidget(cache_prefs);
   
   vlay=new QVBoxLayout(5);
   hlay->addLayout(vlay);
   
   optim_prefs=new QDOptimPrefs(prefs, start, "optim_prefs");
   vlay->addWidget(optim_prefs);

//     tbar_prefs=new QDTbarPrefs(prefs, start, "tbar_prefs");
//     vlay->addWidget(tbar_prefs);

   lens_prefs=new QDLensPrefs(prefs, start, "lens_prefs");
   vlay->addWidget(lens_prefs);
   
   QHBoxLayout * butt_lay=new QHBoxLayout(5);
   top_vlay->addLayout(butt_lay);
   butt_lay->addStretch(1);
   QePushButton * ok_butt=new QePushButton("&OK", start, "ok_butt");
   ok_butt->setDefault(TRUE);
   butt_lay->addWidget(ok_butt);
   QePushButton * cancel_butt=new QePushButton("&Cancel", start, "cancel_butt");
   butt_lay->addWidget(cancel_butt);
   
   top_vlay->activate();

      // Connecting signals and slots
   connect(ok_butt, SIGNAL(clicked(void)), this, SLOT(accept(void)));
   connect(cancel_butt, SIGNAL(clicked(void)), this, SLOT(reject(void)));
}
