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
// $Id: qd_welcome.h,v 1.5 2001-10-17 19:09:18 docbill Exp $
// $Name:  $


#ifndef HDR_QD_WELCOME
#define HDR_QD_WELCOME

#include "DjVuGlobal.h"
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
