//C-  -*- C++ -*-
//C-
//C-  Copyright © 1999-2001, LizardTech, Inc. All Rights Reserved.
//C-              Unauthorized use prohibited.
//C-
// 
// $Id: qd_align_dialog.h,v 1.1 2001-08-08 17:00:32 docbill Exp $
// $Name:  $


#ifndef HDR_QD_ALIGN_DIALOG
#define HDR_QD_ALIGN_DIALOG

#ifdef __GNUC__
#pragma interface
#endif

#include <qdialog.h>
#include <qradiobutton.h>

#include "qt_fix.h"

// Class representing dialog used to edit page alignmentin DjEdit
// Alignments maybe -1/0/1 meaning left/center/right or bottom/center/top

class QDAlignDialog : public QeDialog
{
   Q_OBJECT
private:
   QeRadioButton	* left_butt, * hcenter_butt, * right_butt, * hdef_butt;
   QeRadioButton	* top_butt, * vcenter_butt, * bottom_butt, * vdef_butt;
public:
   int		horAlignment(void) const;
   int		verAlignment(void) const;

      // Alignments are in DjVuAnno format
   QDAlignDialog(int hor, int vert, QWidget * parent=0,
		 const char * name=0, bool modal=FALSE);
   ~QDAlignDialog(void) {};
};

#endif
