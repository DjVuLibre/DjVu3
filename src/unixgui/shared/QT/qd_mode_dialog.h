//C-  -*- C++ -*-
//C-
//C-  Copyright © 1999-2001, LizardTech, Inc. All Rights Reserved.
//C-              Unauthorized use prohibited.
//C-
// 
// $Id: qd_mode_dialog.h,v 1.1 2001-08-08 17:00:32 docbill Exp $
// $Name:  $


#ifndef HDR_QD_MODE_DIALOG
#define HDR_QD_MODE_DIALOG

#ifdef __GNUC__
#pragma interface
#endif

#include <qdialog.h>
#include <qcombobox.h>

#include "qt_fix.h"

// Use this class to edit the default color mode for a DjVu page.

class QDModeDialog : public QeDialog
{
   Q_OBJECT
private:
   QeComboBox	* menu;
public:
   int		getMode(void) const;

      // Mode comes in DjVuAnno format
   QDModeDialog(int mode, QWidget * parent=0,
		const char * name=0, bool modal=FALSE);
   ~QDModeDialog(void) {};
};

#endif
