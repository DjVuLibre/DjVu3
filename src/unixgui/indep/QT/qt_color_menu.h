//C-  -*- C++ -*-
//C-
//C-  Copyright © 1999-2001, LizardTech, Inc. All Rights Reserved.
//C-              Unauthorized use prohibited.
//C-
// 
// $Id: qt_color_menu.h,v 1.1 2001-05-29 22:05:29 bcr Exp $
// $Name:  $


#ifndef HDR_QT_COLOR_MENU
#define HDR_QT_COLOR_MENU

#ifdef __GNUC__
#pragma interface
#endif

#include <qcombobox.h>

#include "int_types.h"

#include "qt_fix.h"

class QeColorMenu : public QeComboBox
{
   Q_OBJECT
private:
   u_int32	custom_color;
private slots:
   void		itemSelected(int item);
public:
   u_int32	color(void) const;

   void		setDefaultItemLabel(const QString &label);
   
   QeColorMenu(u_int32 color, QWidget * parent=0, const char * name=0);
   ~QeColorMenu(void) {}
};

#endif
