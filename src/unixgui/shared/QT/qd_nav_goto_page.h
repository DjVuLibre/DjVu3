//C-  -*- C++ -*-
//C-
//C-  Copyright © 1999-2001, LizardTech, Inc. All Rights Reserved.
//C-              Unauthorized use prohibited.
//C-
// 
// $Id: qd_nav_goto_page.h,v 1.1 2001-05-29 22:05:31 bcr Exp $
// $Name:  $


#ifndef HDR_QD_NAV_GOTO_PAGE
#define HDR_QD_NAV_GOTO_PAGE

#ifdef __GNUC__
#pragma interface
#endif

#include "DjVuDocument.h"
#include <qdialog.h>
#include <qcombobox.h>

#include "qt_fix.h"

// Dialog for entering the exact page number to go to
class QDNavGotoPage : public QeDialog
{
   Q_OBJECT
private:
   QeComboBox	* menu;
public:
   int		getPageNum(void) const;
   
   QDNavGotoPage(GP<DjVuDocument> &doc,
		 class DjVuImage * dimg,
		 QWidget * parent=0, const char * name=0);
   virtual ~QDNavGotoPage(void) {};
};

#endif
