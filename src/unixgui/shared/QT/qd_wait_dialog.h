//C-  -*- C++ -*-
//C-
//C-  Copyright © 1999-2001, LizardTech, Inc. All Rights Reserved.
//C-              Unauthorized use prohibited.
//C-
// 
// $Id: qd_wait_dialog.h,v 1.1 2001-05-29 22:05:31 bcr Exp $
// $Name:  $


#ifndef HDR_QD_WAIT_DIALOG
#define HDR_QD_WAIT_DIALOG

#ifdef __GNUC__
#pragma interface
#endif

#include <qsemimodal.h>

#include "qt_fix.h"

class QDWaitDialog : public QSemiModal
{
   Q_OBJECT
private slots:
   void		slotClose(void) { close(); }
public:
   QDWaitDialog(const QString & msg, const char * butt_label,
		QWidget * parent=0, const char * name=0, bool modal=FALSE);
   ~QDWaitDialog(void) {}
};

#endif
