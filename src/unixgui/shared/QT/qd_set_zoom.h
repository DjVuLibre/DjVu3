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
// $Id: qd_set_zoom.h,v 1.4 2001-10-16 18:01:45 docbill Exp $
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
