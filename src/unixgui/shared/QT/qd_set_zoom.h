//C-  -*- C++ -*-
//C-
//C-  Copyright © 1999-2001, LizardTech, Inc. All Rights Reserved.
//C-              Unauthorized use prohibited.
//C-
// 
// $Id: qd_set_zoom.h,v 1.1 2001-05-29 22:05:31 bcr Exp $
// $Name:  $


#ifndef HDR_QD_SET_ZOOM
#define HDR_QD_SET_ZOOM

#ifdef __GNUC__
#pragma interface
#endif

#include <qdialog.h>
#include <qspinbox.h>

#include "qt_fix.h"

// Dialog for entering the exact zoom factor to use
class QDSetZoom : public QeDialog
{
   Q_OBJECT
private:
   QeSpinBox	* spin;
public:
   int		getZoom(void) const;

   QDSetZoom(int zoom, QWidget * parent=0, const char * name=0);
   virtual ~QDSetZoom(void) {};
};

inline int
QDSetZoom::getZoom(void) const
{
   return spin->value();
}

#endif
