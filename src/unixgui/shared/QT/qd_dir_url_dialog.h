//C-  -*- C++ -*-
//C-
//C-  Copyright © 1999-2001, LizardTech, Inc. All Rights Reserved.
//C-              Unauthorized use prohibited.
//C-
// 
// $Id: qd_dir_url_dialog.h,v 1.1 2001-08-08 17:00:32 docbill Exp $
// $Name:  $


#ifndef HDR_QD_DIR_URL_DIALOG
#define HDR_QD_DIR_URL_DIALOG

#ifdef __GNUC__
#pragma interface
#endif

#include <qdialog.h>
#include <qlineedit.h>

#include "qt_fix.h"

// Used to edit directory URL.

class QDDirURLDialog : public QeDialog
{
   Q_OBJECT
private:
   QeLineEdit	* text;
public:
   const QString &dirURL(void) const { return text->text(); };
   
   QDDirURLDialog(const char * url, QWidget * parent=0,
		  const char * name=0, bool modal=FALSE);
   ~QDDirURLDialog(void) {};
};

#endif
