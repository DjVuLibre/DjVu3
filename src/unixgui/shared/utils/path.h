//C-  -*- C++ -*-
//C-
//C-  Copyright © 1999-2001, LizardTech, Inc. All Rights Reserved.
//C-              Unauthorized use prohibited.
//C-
// 
// $Id: path.h,v 1.1 2001-08-08 17:38:05 docbill Exp $
// $Name:  $

 
#ifndef HDR_PATH
#define HDR_PATH

#ifdef __GNUC__
#pragma interface
#endif

#include "GString.h"

GUTF8String	GetLibraryPath(void);

// CHECK_PATHS is typically either ~/.netscape, or /usr/local/lib/netscape
#ifndef CHECK_PATHS
#define CHECK_PATHS	,"/usr/local/lib/netscape","/usr/lib/netscape","/opt/netscape"
#endif

#endif
