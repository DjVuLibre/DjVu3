//C-  -*- C++ -*-
//C-
//C-  Copyright © 1999-2001, LizardTech, Inc. All Rights Reserved.
//C-              Unauthorized use prohibited.
//C-
// 
// $Id: qd_viewer_prefs.h,v 1.1 2001-05-30 17:29:32 mchen Exp $
// $Name:  $


#ifndef HDR_QD_VIEWER_PREFS
#define HDR_QD_VIEWER_PREFS

#ifdef __GNUC__
#pragma interface
#endif

#include "qd_prefs.h"

#include <qdialog.h>

#include "qt_fix.h"

// "Hyperlink Preferences box"
class QDHlinkPrefs : public QeGroupBox
{
   Q_OBJECT
private:
   QeCheckBox		* popup_butt;
   QeCheckBox		* border_butt;
   QeComboBox		* key_menu;
private slots:
   void		slotHotKeyChanged(int);
signals:
   void		sigHotKeyChanged(DjVuPrefs::HLButtType key);
public slots:
   void		slotMagHotKeyChanged(DjVuPrefs::MagButtType key);
public:
   bool		disablePopup(void) const;
   bool		simpleBorder(void) const;
   DjVuPrefs::HLButtType	hlinkKey(void) const;
   
   QDHlinkPrefs(DjVuPrefs * prefs, QWidget * parent=0, const char * name=0);
   ~QDHlinkPrefs(void) {}
};

// The "Viewer Preferences" dialog
class QDViewerPrefs : public QeDialog
{
   Q_OBJECT
private:
   QDGammaPrefs		* gamma_prefs;
   QDLensPrefs		* lens_prefs;
   QDOptimPrefs		* optim_prefs;
   QDHlinkPrefs		* hlink_prefs;
   QDCachePrefs		* cache_prefs;
   QDTbarPrefs		* tbar_prefs;
   
   class QeCheckBox	* global_butt;
   bool			cache_disabled;

   DjVuPrefs		* prefs;
private slots:
   void		slotGlobalToggled(bool);
protected slots:
   virtual void	done(int);
public:
      // Used to disable cache just for this instance.
      // Remember, that the cache can be disabled globally.
   void		disableCache(void);
   
   QDViewerPrefs(DjVuPrefs * prefs, QWidget * parent=0,
		 const char * name=0, bool modal=FALSE);
   ~QDViewerPrefs(void) {};
};

#endif
