//C-  -*- C++ -*-
//C-
//C-  Copyright © 1999-2001, LizardTech, Inc. All Rights Reserved.
//C-              Unauthorized use prohibited.
//C-
// 
// $Id: qt_imager.cpp,v 1.1 2001-05-29 22:05:29 bcr Exp $
// $Name:  $


#ifdef __GNUC__
#pragma implementation
#endif

#include "qt_imager.h"
#include "debug.h"
#include "exc_msg.h"

QeImager	* qeImager;

QeImager::QeImager(void)
{
   DEBUG_MSG("QeImager::QeImager(): Initializing class...\n");
   DEBUG_MAKE_INDENT(3);

   if (qeImager)
      throw ERROR_MESSAGE("QeImager::QeImager",
			  "Internal error: Attempt to initialize QeImager twice.");
   qeImager=this;
}
