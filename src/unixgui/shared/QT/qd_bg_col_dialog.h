//C-  -*- C++ -*-
//C-
//C-  Copyright © 1999-2001, LizardTech, Inc. All Rights Reserved.
//C-              Unauthorized use prohibited.
//C-
// 
// $Id: qd_bg_col_dialog.h,v 1.1 2001-08-08 17:00:32 docbill Exp $
// $Name:  $


#ifndef HDR_QD_BG_COL_DIALOG
#define HDR_QD_BG_COL_DIALOG

#ifdef __GNUC__
#pragma interface
#endif

#include "qt_color_menu.h"
#include "int_types.h"

#include <qdialog.h>

#include "qt_fix.h"

// DjVu page background color editor. The color is in 'u_int32' format
// defined in int_type.h. It has the format RRGGBB in hex. Depending
// if the color is one of standard defined in col_db.cpp it will either
// be shown as one of "standard" or will be shown as custom, which you
// may additionally edit using QeColorEditDialog

class QDBGColorDialog : public QeDialog
{
   Q_OBJECT
private:
   QeColorMenu	* menu;
public:
   u_int32	color(void) const { return menu->color(); };
   
   QDBGColorDialog(u_int32 color, QWidget * parent=0,
		   const char * name=0, bool modal=FALSE);
   ~QDBGColorDialog(void) {};
};

#endif
