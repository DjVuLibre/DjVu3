//C-  -*- C++ -*-
//C-
//C-  Copyright © 1999-2001, LizardTech, Inc. All Rights Reserved.
//C-              Unauthorized use prohibited.
//C-
// 
// $Id: exc_misc.h,v 1.1 2001-05-29 22:05:30 bcr Exp $
// $Name:  $


#ifndef HDR_EXC_MISC
#define HDR_EXC_MISC

#ifdef __GNUC__
#pragma interface
#endif

#include "exc_base.h"

#define MISC_EXCEPTION(func, msg) MiscException(func, msg, 0, __FILE__, __LINE__)

class MiscException : public Exception
{
public:
   MiscException(const char * func, const char * msg, const char * type=0,
		 const char * file=0, const int line=0) :
      Exception(func, msg, type ? type : "MiscException", file, line) {};
   virtual ~MiscException(void) {};
};

#define BAD_ARGUMENTS(func, msg) BadArguments(func, msg, 0, __FILE__, __LINE__)

class BadArguments : public MiscException
{
public:
   BadArguments(const char * func, const char * msg, const char * type=0,
		const char * file=0, const int line=0) :
      MiscException(func, msg, type ? type : "BadArguments", file, line) {};
   virtual ~BadArguments(void) {};
};

#define DUMMY Dummy(0, __FILE__, __LINE__)

class Dummy : public MiscException
{
public:
   Dummy(const char * type=0, const char * file=0, const int line=0) :
      MiscException("dummy", "dummy", type ? type : "Dummy", file, line) {};
   virtual ~Dummy(void) {};
};

#define INTERRUPTED(func, msg) Interrupted(func, msg, 0, __FILE__, __LINE__)

class Interrupted : public MiscException
{
public:
   Interrupted(const char * func, const char * msg, const char * type=0,
	       const char * file=0, const int line=0) :
      MiscException(func, msg, type ? type : "Interrupted", file, line) {};
   virtual ~Interrupted(void) {};
};

#endif
