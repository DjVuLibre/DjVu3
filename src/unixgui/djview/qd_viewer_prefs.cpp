//C-  -*- C++ -*-
//C-
//C- DjVu� Unix Viewer (v. 3.5)
//C- 
//C- Copyright � 1999-2001 LizardTech, Inc. All Rights Reserved.
//C- The DjVu Reference Library is protected by U.S. Pat. No.
//C- 6,058,214 and patents pending.
//C- 
//C- This software is subject to, and may be distributed under, the
//C- GNU General Public License, Version 2. The license should have
//C- accompanied the software or you may obtain a copy of the license
//C- from the Free Software Foundation at http://www.fsf.org .
//C- 
//C- The computer code originally released by LizardTech under this
//C- license and unmodified by other parties is deemed the "LizardTech
//C- Original Code."
//C- 
//C- With respect to the LizardTech Original Code ONLY, and subject
//C- to any third party intellectual property claims, LizardTech
//C- grants recipient a worldwide, royalty-free, non-exclusive license
//C- under patent claims now or hereafter owned or controlled by
//C- LizardTech that are infringed by making, using, or selling
//C- LizardTech Original Code, but solely to the extent that any such
//C- patent(s) is/are reasonably necessary to enable you to make, have
//C- made, practice, sell, or otherwise dispose of LizardTech Original
//C- Code (or portions thereof) and not to any greater extent that may
//C- be necessary to utilize further modifications or combinations.
//C- 
//C- The LizardTech Original Code is provided "AS IS" WITHOUT WARRANTY
//C- OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED
//C- TO ANY WARRANTY OF NON-INFRINGEMENT, OR ANY IMPLIED WARRANTY OF
//C- MERCHANTIBILITY OR FITNESS FOR A PARTICULAR PURPOSE.
//C-
// 
// $Id: qd_viewer_prefs.cpp,v 1.5 2001-10-17 19:00:53 docbill Exp $
// $Name:  $


#include "djvu_file_cache.h"
#include "qd_viewer_prefs.h"
#include "qlib.h"
#include "debug.h"
#include "exc_msg.h"

#include <qlayout.h>
#include <qtooltip.h>
#include <qcheckbox.h>
#include <qcombobox.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <stdio.h>

#include "qt_fix.h"

//***************************************************************************
//******************************* QDHlinkPrefs ******************************
//***************************************************************************

bool
QDHlinkPrefs::disablePopup(void) const
{
   return popup_butt->isChecked();
}

bool
QDHlinkPrefs::simpleBorder(void) const
{
   return border_butt->isChecked();
}

DjVuPrefs::HLButtType
QDHlinkPrefs::hlinkKey(void) const
{
   return (DjVuPrefs::HLButtType) key_menu->currentItem();
}

void
QDHlinkPrefs::slotHotKeyChanged(int cur_item)
{
   emit sigHotKeyChanged((DjVuPrefs::HLButtType) cur_item);
}

void
QDHlinkPrefs::slotMagHotKeyChanged(DjVuPrefs::MagButtType key)
{
      // Make sure that we don't have this key selected

      // Translate it to HLButtType
   if (key!=DjVuPrefs::MAG_MID)
   {
      int our_key=DjVuPrefs::HLB_SHIFT;
      if (key==DjVuPrefs::MAG_CTRL)
	 our_key=DjVuPrefs::HLB_CTRL;
      else if (key==DjVuPrefs::MAG_SHIFT)
	 our_key=DjVuPrefs::HLB_SHIFT;
      else if (key==DjVuPrefs::MAG_ALT)
	 our_key=DjVuPrefs::HLB_ALT;

	 // See if we have it currently selected
      if (key_menu->currentItem()==our_key)
	 key_menu->setCurrentItem((our_key+1) % DjVuPrefs::HLB_ITEMS);
   }
}

QDHlinkPrefs::QDHlinkPrefs(DjVuPrefs * prefs, QWidget * parent, const char * name) :
      QeGroupBox(tr("Hyperlinks Preferences"), parent, name)
{
   DEBUG_MSG("QDHlinkPrefs::QDHlinkOptimPrefs(): Creating 'Hyperlinks Preferences' box...\n");
   DEBUG_MAKE_INDENT(3);

   QVBoxLayout * vlay=new QVBoxLayout(this, 10, 5, "hlink_vlay");
   vlay->addSpacing(fontMetrics().height());

   popup_butt=new QeCheckBox(tr("Disable popup messages"), this, "popup_butt");
   popup_butt->setChecked(!prefs->hlinksPopup);
   vlay->addWidget(popup_butt);
   
   border_butt=new QeCheckBox(tr("Draw using simple border"), this, "border_butt");
   border_butt->setChecked(prefs->hlinksBorder);
   vlay->addWidget(border_butt);
   
   QHBoxLayout * hlay=new QHBoxLayout(5, "hlink_hlay");
   vlay->addLayout(hlay);
   QeLabel * key_label=new QeLabel(tr("'Show all hyperlinks' key"), this);
   hlay->addWidget(key_label);
   hlay->addStretch(1);
   key_menu=new QeComboBox(this, "key_menu");
   for(int i=0;i<DjVuPrefs::HLB_ITEMS;i++)
      key_menu->insertItem(DjVuPrefs::hlb_names[i], i);
   key_menu->setCurrentItem(prefs->hlb_num);
   hlay->addWidget(key_menu);

   vlay->activate();

   QString key_tip=tr("Combination of keys to be pressed to display\nall the hyperlinks present on the page.");
   QToolTip::add(key_label, key_tip); QToolTip::add(key_menu, key_tip);

   connect(key_menu, SIGNAL(activated(int)),
	   this, SLOT(slotHotKeyChanged(int)));
}

//***************************************************************************
//******************************* QDViewerPrefs *****************************
//***************************************************************************

void
QDViewerPrefs::done(int rc)
{
   DEBUG_MSG("QDViewerPrefs::done() called\n");
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
      disk_prefs.hlinksPopup=prefs->hlinksPopup=!hlink_prefs->disablePopup();
      disk_prefs.hlinksBorder=prefs->hlinksBorder=hlink_prefs->simpleBorder();
      disk_prefs.hlb_num=prefs->hlb_num=hlink_prefs->hlinkKey();
      disk_prefs.magnifierSize=prefs->magnifierSize=lens_prefs->size();
      disk_prefs.magnifierScale=prefs->magnifierScale=lens_prefs->scale();
      disk_prefs.magnifierHotKey=prefs->magnifierHotKey=lens_prefs->hotKey();
      disk_prefs.mcacheSize=prefs->mcacheSize=cache_prefs->mcacheSize();
      disk_prefs.pcacheSize=prefs->pcacheSize=cache_prefs->pcacheSize();

      disk_prefs.toolBarOn=prefs->toolBarOn=tbar_prefs->enabled();
      disk_prefs.toolBarAlwaysVisible=prefs->toolBarAlwaysVisible=tbar_prefs->visible();
      disk_prefs.toolBarDelay=prefs->toolBarDelay=tbar_prefs->delay();

      if (global_butt->isChecked()) disk_prefs.save();
   } else
   {
      DEBUG_MSG("Cancel pressed.\n");
   }
   QeDialog::done(rc);
}

void
QDViewerPrefs::slotGlobalToggled(bool state)
{
   bool en=get_file_cache()->is_enabled() && state && !cache_disabled;
   cache_prefs->enablePCache(en);
}

void
QDViewerPrefs::disableCache(void)
{
   cache_disabled=true;
   cache_prefs->enablePCache(false);
   cache_prefs->resetPCache();
}

QDViewerPrefs::QDViewerPrefs(DjVuPrefs * _prefs, QWidget * parent,
			     const char * name, bool modal) :
      QeDialog(parent, name, modal), cache_disabled(false), prefs(_prefs)
{
   DEBUG_MSG("QDViewerPrefs::QDViewerPrefs(): Creating Viewer Prefs dialog...\n");
   DEBUG_MAKE_INDENT(3);

   setCaption(tr("DjVu: Viewer Preferences"));
   QWidget * start=startWidget();
   QVBoxLayout * top_vlay=new QVBoxLayout(start, 10, 5, "top_vlay");

   QHBoxLayout * hlay=new QHBoxLayout(5);
   top_vlay->addLayout(hlay);

   QVBoxLayout * vlay=new QVBoxLayout(5);
   hlay->addLayout(vlay);

      // *** Left column
   
   gamma_prefs=new QDGammaPrefs(prefs, start, "gamma_prefs");
   vlay->addWidget(gamma_prefs);

   optim_prefs=new QDOptimPrefs(prefs, start, "optim_prefs");
   vlay->addWidget(optim_prefs);

      // *** Right column
   
   vlay=new QVBoxLayout(5);
   hlay->addLayout(vlay);
   
   hlink_prefs=new QDHlinkPrefs(prefs, start, "hlink_prefs");
   vlay->addWidget(hlink_prefs);

   cache_prefs=new QDCachePrefs(prefs, true, start, "cache_prefs");
   vlay->addWidget(cache_prefs);

   tbar_prefs=new QDTbarPrefs(prefs, start, "tbar_prefs");
   vlay->addWidget(tbar_prefs);

   lens_prefs=new QDLensPrefs(prefs, start, "lens_prefs");
   vlay->addWidget(lens_prefs);

   QHBoxLayout * butt_lay=new QHBoxLayout(5);
   top_vlay->addLayout(butt_lay);
   global_butt=new QeCheckBox(tr("Update preferences globally"), start, "global_butt");
   global_butt->setChecked(TRUE);
   butt_lay->addWidget(global_butt);
   butt_lay->addStretch(1);
   QePushButton * ok_butt=new QePushButton(tr("&OK"), start, "ok_butt");
   ok_butt->setDefault(TRUE);
   butt_lay->addWidget(ok_butt);
   QePushButton * cancel_butt=new QePushButton(tr("&Cancel"), start, "cancel_butt");
   butt_lay->addWidget(cancel_butt);
   
   top_vlay->activate();

      // Connecting signals and slots
   connect(ok_butt, SIGNAL(clicked(void)), this, SLOT(accept(void)));
   connect(cancel_butt, SIGNAL(clicked(void)), this, SLOT(reject(void)));
   connect(global_butt, SIGNAL(toggled(bool)), this, SLOT(slotGlobalToggled(bool)));

   connect(hlink_prefs, SIGNAL(sigHotKeyChanged(DjVuPrefs::HLButtType)),
	   lens_prefs, SLOT(slotHlHotKeyChanged(DjVuPrefs::HLButtType)));
   connect(lens_prefs, SIGNAL(sigHotKeyChanged(DjVuPrefs::MagButtType)),
	   hlink_prefs, SLOT(slotMagHotKeyChanged(DjVuPrefs::MagButtType)));
}
