//C-  -*- C++ -*-
//C-
//C-  Copyright © 1999-2001, LizardTech, Inc. All Rights Reserved.
//C-              Unauthorized use prohibited.
//C-
// 
// $Id: exc_msg.h,v 1.1 2001-05-29 22:05:30 bcr Exp $
// $Name:  $


#ifndef HDR_EXC_MSG
#define HDR_EXC_MSG

#ifdef __GNUC__
#pragma interface
#endif

#include "exc_base.h"

#define MESSAGE(func, msg) Message(func, msg, 0, __FILE__, __LINE__)

class Message : public Exception
{
public:
   Message(const char * func, const char * msg, const char * type=0,
	   const char * file=0, const int line=0) :
      Exception(func, msg, type ? type : "Message", file, line) {};
   virtual ~Message(void) {};
};

#define INFO_MESSAGE(func, msg) InfoMessage(func, msg, 0, __FILE__, __LINE__)

class InfoMessage : public Message
{
public:
   InfoMessage(const char * func, const char * msg, const char * type=0,
	       const char * file=0, const int line=0) :
      Message(func, msg, type ? type : "InfoMessage", file, line) {};
   virtual ~InfoMessage(void) {};
};

#define ERROR_MESSAGE(func, msg) ErrorMessage(func, msg, 0, __FILE__, __LINE__)

class ErrorMessage : public Message
{
public:
   ErrorMessage(const char * func, const char * msg, const char * type=0,
	        const char * file=0, const int line=0) :
      Message(func, msg, type ? type : "ErrorMessage", file, line) {};
   virtual ~ErrorMessage(void) {};
};

#endif
