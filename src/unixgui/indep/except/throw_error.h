//C-  -*- C++ -*-
//C-
//C-  Copyright © 1999-2001, LizardTech, Inc. All Rights Reserved.
//C-              Unauthorized use prohibited.
//C-
// 
// $Id: throw_error.h,v 1.1 2001-05-29 22:05:30 bcr Exp $
// $Name:  $


#ifndef HDR_THROW_ERROR
#define HDR_THROW_ERROR

#ifdef __GNUC__
#pragma interface
#endif

#include "exc_base.h"
#include "GString.h"
#include <errno.h>

void ThrowError(const char * func, const GUTF8String & msg, int in_errno=-1);

#endif
