//C-  -*- C++ -*-
//C-
//C-  Copyright © 1999-2001, LizardTech, Inc. All Rights Reserved.
//C-              Unauthorized use prohibited.
//C-
// 
// $Id: qd_move_page.h,v 1.1 2001-08-08 17:00:32 docbill Exp $
// $Name:  $


#ifndef HDR_QD_MOVE_PAGE
#define HDR_QD_MOVE_PAGE

#ifdef __GNUC__
#pragma interface
#endif

#include <qdialog.h>

#include "qt_fix.h"

class QDMovePageDialog : public QeDialog
{
   Q_OBJECT
private:
   int			src_page_num;
   int			down_shift_items;
   class QeComboBox	* goto_menu, * shift_menu;
private slots:
   void		slotGotoActivated(const QString & text);
   void		slotShiftActivated(const QString & text);
public:
   int		pageNum(void) const;
   QDMovePageDialog(int page_num, int pages_num,
		    QWidget * parent=0, const char * name=0);
   ~QDMovePageDialog(void) {}
};

#endif
