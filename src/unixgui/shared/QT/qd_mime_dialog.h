//C-  -*- C++ -*-
//C-
//C-  Copyright © 1999-2001, LizardTech, Inc. All Rights Reserved.
//C-              Unauthorized use prohibited.
//C-
// 
// $Id: qd_mime_dialog.h,v 1.1 2001-08-08 17:00:32 docbill Exp $
// $Name:  $


#ifndef HDR_QD_MIME_DIALOG
#define HDR_QD_MIDE_DIALOG

#ifdef __GNUC__
#pragma interface
#endif

#include <qdialog.h>

#include "qt_fix.h"

class QDMimeDialog : public QeDialog
{
   Q_OBJECT
private:
   class QeRadioButton	* again_butt, * dontask_butt, * dontcheck_butt;
public:
   bool		dontAsk(void) const;
   bool		dontCheck(void) const;
   
      // Mode comes in DjVuAnno format
   QDMimeDialog(const QString & mime_fname, QWidget * parent=0,
		const char * name=0, bool modal=FALSE);
   ~QDMimeDialog(void) {};
};

#endif
