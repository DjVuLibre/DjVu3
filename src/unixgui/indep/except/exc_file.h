//C-  -*- C++ -*-
//C-
//C-  Copyright © 1999-2001, LizardTech, Inc. All Rights Reserved.
//C-              Unauthorized use prohibited.
//C-
// 
// $Id: exc_file.h,v 1.1 2001-05-29 22:05:30 bcr Exp $
// $Name:  $


#ifndef HDR_EXC_FILE
#define HDR_EXC_FILE

#ifdef __GNUC__
#pragma interface
#endif

#include "exc_base.h"

class File : public Exception
{
public:
   File(const char * func, const char * msg, const char * type=0,
	const char * file=0, const int line=0) :
      Exception(func, msg, type ? type : "File", file, line) {};
   virtual ~File(void) {};
};

#define CANT_OPEN_FILE(func, msg) CantOpenFile(func, msg, 0, __FILE__, __LINE__)

class CantOpenFile : public File
{
public:
   CantOpenFile(const char * func, const char * msg, const char * type=0,
		const char * file=0, const int line=0) :
      File(func, msg, type ? type : "CantOpenFile", file, line) {};
   virtual ~CantOpenFile(void) {};
};

#define WRONG_FILE_CONTENTS(func, msg) WrongFileContents(func, msg, 0, __FILE__, __LINE__)

class WrongFileContents : public File
{
public:
   WrongFileContents(const char * func, const char * msg, const char * type=0,
		     const char * file=0, const int line=0) :
      File(func, msg, type ? type : "WrongFileContents", file, line) {};
   virtual ~WrongFileContents(void) {};
};

#endif
