//C-  -*- C++ -*-
//C-
//C-  Copyright © 1999-2001, LizardTech, Inc. All Rights Reserved.
//C-              Unauthorized use prohibited.
//C-
// 
// $Id: execdir.h,v 1.1 2001-05-29 22:05:30 bcr Exp $
// $Name:  $

 
#ifndef HDR_EXECDIR
#define HDR_EXECDIR

#ifdef __GNUC__
#pragma interface
#endif

#include "GString.h"

QString
getExecDir(QString argv0);

#endif
