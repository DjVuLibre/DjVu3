//C-  -*- C++ -*-
//C-
//C-  Copyright © 1999-2001, LizardTech, Inc. All Rights Reserved.
//C-              Unauthorized use prohibited.
//C-
// 
// $Id: exc_res.h,v 1.1 2001-05-29 22:05:30 bcr Exp $
// $Name:  $


#ifndef HDR_EXC_RES
#define HDR_EXC_RES

#ifdef __GNUC__
#pragma interface
#endif

#include "exc_base.h"

#define OUT_OF_RESOURCES(func, msg) OutOfResources(func, msg, 0, __FILE__, __LINE__)

class OutOfResources : public Exception
{
public:
   OutOfResources(const char * func, const char * msg, const char * type=0,
		  const char * file=0, const int line=0) :
      Exception(func, msg, type ? type : "OutOfResources", file, line) {};
   virtual ~OutOfResources(void) {};
};

#define OUT_OF_MEMORY(func, msg) OutOfMemory(func, msg, 0, __FILE__, __LINE__)

class OutOfMemory : public OutOfResources
{
public:
   OutOfMemory(const char * func, const char * msg, const char * type=0,
	       const char * file=0, const int line=0) :
      OutOfResources(func, msg, type ? type : "OutOfMemory", file, line) {};
   virtual ~OutOfMemory(void) {};
};

#endif
