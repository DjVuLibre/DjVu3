//C-  -*- C++ -*-
//C-
//C-  Copyright © 1999-2001, LizardTech, Inc. All Rights Reserved.
//C-              Unauthorized use prohibited.
//C-
// 
// $Id: qt_hack.cpp,v 1.1 2001-08-08 17:00:32 docbill Exp $
// $Name:  $


#ifdef __GNUC__
#pragma implementation
#endif

#include "qt_hack.h"

#define private public
#define protected public
#include <qpaintdevice.h>

void HackQT(void)
{
#ifdef QT1
   QPaintDevice::x_defcmap=FALSE;
   QPaintDevice::x_defvisual=FALSE;
#endif
}

