//C-  -*- C++ -*-
//C-
//C-  Copyright © 1999-2001, LizardTech, Inc. All Rights Reserved.
//C-              Unauthorized use prohibited.
//C-
// 
// $Id: qd_welcome.h,v 1.1 2001-05-29 22:05:31 bcr Exp $
// $Name:  $


#ifndef HDR_QD_WELCOME
#define HDR_QD_WELCOME

#ifdef __GNUC__
#pragma interface
#endif

#include <qdialog.h>
#include <qcheckbox.h>
#include <qpushbutton.h>

#include "qt_fix.h"

// That "Welcome" dialog appearing once for every new plugin version.
// Connect the slots to smth useful if you want the "Help", "Preferences"
// etc. buttons to work

class QDWelcome : public QeDialog
{
   Q_OBJECT
private:
   QeCheckBox		* never_butt;
   QePushButton		* close_butt;
protected slots:
   virtual bool		eventFilter(QObject *, QEvent * ev);
signals:
   void		closed(void);
   void		preferences(void);
   void		help(void);
   void		about(void);
public:
   int		neverShowAgain(void) const { return never_butt->isChecked(); };
   
   QDWelcome(QWidget * parent=0, const char * name=0, bool modal=FALSE);
   ~QDWelcome(void) {};
};

#endif
