//C-  -*- C++ -*-
//C-
//C-  Copyright © 1999-2001, LizardTech, Inc. All Rights Reserved.
//C-              Unauthorized use prohibited.
//C-
// 
// $Id: exc_range.h,v 1.1 2001-05-29 22:05:30 bcr Exp $
// $Name:  $


#ifndef HDR_EXC_RANGE
#define HDR_EXC_RANGE

#ifdef __GNUC__
#pragma interface
#endif

#include "exc_base.h"

#define OUT_OF_RANGE(func, msg) OutOfRange(func, msg, 0, __FILE__, __LINE__)

class OutOfRange : public Exception
{
public:
   OutOfRange(const char * func, const char * msg, const char * type=0,
	      const char * file=0, const int line=0) :
      Exception(func, msg, type ? type : "OutOfRange", file, line) {};
   virtual ~OutOfRange(void) {};
};

#define INDEX_OUT_OF_RANGE(func, msg) IndexOutOfRange(func, msg, 0, __FILE__, __LINE__)

class IndexOutOfRange : public OutOfRange
{
public:
   IndexOutOfRange(const char * func, const char * msg, const char * type=0,
		   const char * file=0, const int line=0) :
      OutOfRange(func, msg, type ? type : "IndexOutOfRange", file, line) {};
   virtual ~IndexOutOfRange(void) {};
};

#endif
