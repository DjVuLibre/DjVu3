//C-  -*- C++ -*-
//C-
//C-  Copyright © 1999-2001, LizardTech, Inc. All Rights Reserved.
//C-              Unauthorized use prohibited.
//C-
// 
// $Id: qd_move_to.h,v 1.1 2001-08-08 17:00:32 docbill Exp $
// $Name:  $


#ifndef HDR_QD_MOVE_TO
#define HDR_QD_MOVE_TO

#ifdef __GNUC__
#pragma interface
#endif

#include <qdialog.h>

#include "qt_fix.h"

class QDMoveToDialog : public QeDialog
{
   Q_OBJECT
private:
   class QeComboBox	* menu;
public:
   int		pageNum(void) const;
   QDMoveToDialog(int page_num, int pages_num,
		  QWidget * parent=0, const char * name=0);
   ~QDMoveToDialog(void) {}
};

#endif
