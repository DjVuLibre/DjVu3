//C-  -*- C++ -*-
//C-
//C-  Copyright © 1999-2001, LizardTech, Inc. All Rights Reserved.
//C-              Unauthorized use prohibited.
//C-
// 
// $Id: qd_editor_prefs.h,v 1.1 2001-08-08 17:00:32 docbill Exp $
// $Name:  $


#ifndef HDR_QD_EDITOR_PREFS
#define HDR_QD_EDITOR_PREFS

#ifdef __GNUC__
#pragma interface
#endif

#include <qdialog.h>
#include <qcombobox.h>

#include "qd_prefs.h"
#include "qt_fix.h"

// Class used to edit DjEditor's preferences. Most of the stuff is
// declared and implemented in qd_prefs.*
// Here we're just positioning needed boxes in the "Preferences" window

class QDEditorPrefs : public QeDialog
{
   Q_OBJECT
private:
   QDGammaPrefs		* gamma_prefs;
   //QDTbarPrefs		* tbar_prefs;
   QDOptimPrefs		* optim_prefs;
   QDLensPrefs		* lens_prefs;
   QDCachePrefs		* cache_prefs;
   DjVuPrefs		* prefs;
protected slots:
   virtual void	done(int);
public:
   QDEditorPrefs(DjVuPrefs * prefs, QWidget * parent=0,
		 const char * name=0, bool modal=FALSE);
   ~QDEditorPrefs(void) {};
};

#endif
