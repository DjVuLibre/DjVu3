//C-  -*- C++ -*-
//C-
//C-  Copyright © 1999-2001, LizardTech, Inc. All Rights Reserved.
//C-              Unauthorized use prohibited.
//C-
// 
// $Id: int_types.h,v 1.1 2001-05-29 22:05:30 bcr Exp $
// $Name:  $


#ifndef HDR_INT_TYPES
#define HDR_INT_TYPES

#ifdef __GNUC__
#pragma interface
#endif

#ifdef UNIX
#include <sys/types.h>
// LYB->EAF:
//  I am not sure that all unix variants
//  define u_int, u_long and u_short.
typedef u_int	u_int32;
typedef u_short	u_int16;
#endif

#ifdef WIN32
#include <sys/types.h>
typedef unsigned long  u_long;
typedef unsigned int   u_int;
typedef unsigned short u_short;
typedef unsigned char  u_char;
typedef u_int	       u_int32;
typedef u_short	       u_int16;
#endif

#endif
