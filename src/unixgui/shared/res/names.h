//C-  -*- C++ -*-
//C-
//C-  Copyright © 1999-2001, LizardTech, Inc. All Rights Reserved.
//C-              Unauthorized use prohibited.
//C-
// 
// $Id: names.h,v 1.1 2001-08-08 17:38:05 docbill Exp $
// $Name:  $


#ifndef HDR_NAMES
#define HDR_NAMES

#ifdef __GNUC__
#pragma interface
#endif

#define REV_PIPE_NAME	".pipe"
#ifdef hpux
#define LIBRARY_NAME	"nsdejavu.sl"
#else
#define LIBRARY_NAME	"nsdejavu.so"
#endif
#define DJVIEW_NAME	"djview"
#define DJEDIT_NAME	"djedit"
#define DEJAVU_DIR	"DjVu/"
#define DJVU_URL	"http://www.lizardtech.com/pluginfiles/unix/3.0"

#endif
