//C-  -*- C++ -*-
//C-
//C-  Copyright © 1999-2001, LizardTech, Inc. All Rights Reserved.
//C-              Unauthorized use prohibited.
//C-
// 
// $Id: qd_move_pages.h,v 1.1 2001-08-08 17:00:32 docbill Exp $
// $Name:  $


#ifndef HDR_QD_MOVE_PAGES
#define HDR_QD_MOVE_PAGES

#ifdef __GNUC__
#pragma interface
#endif

#include "GString.h"
#include "GContainer.h"

#include <qdialog.h>

#include "qt_fix.h"

class QDMovePagesDialog : public QeDialog
{
   Q_OBJECT
private:
   class QeComboBox	* menu;
   int			pages_num;
public:
   static QString	listToString(const GList<int> & l);
   
   int		shift(void) const;
   QDMovePagesDialog(const GList<int> & page_list, int pages_num,
		     QWidget * parent=0, const char * name=0);
   ~QDMovePagesDialog(void) {}
};

#endif
