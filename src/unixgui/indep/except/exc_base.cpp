//C-  -*- C++ -*-
//C-
//C-  Copyright © 1999-2001, LizardTech, Inc. All Rights Reserved.
//C-              Unauthorized use prohibited.
//C-
// 
// $Id: exc_base.cpp,v 1.1 2001-05-29 22:05:30 bcr Exp $
// $Name:  $


#ifdef __GNUC__
#pragma implementation
#endif

#include "exc_base.h"
#include "debug.h"

#include <string.h>
#include <stdlib.h>

Exception::Exception(void)
{
   type="Exception"; aldel_type=0;
}

Exception::Exception(const char * _func, const char * _msg,
		     const char * _type, const char * _file, int _line) :
      GException(_msg, _file, _line, _func)
{
   DEBUG_MSG("Exception::Exception(func, msg, type, file, line): Initializing class\n");
   DEBUG_MAKE_INDENT(3);
   
   if (!_type) _type="Exception";
   if ((type=strdup(_type))) aldel_type=1;
   else
   {
      type="Exception";
      aldel_type=0;
   }

   DEBUG_MSG("type=" << type << "\n");
}

Exception::Exception(const Exception & exc) : GException(exc)
{
   DEBUG_MSG("Exception::Exception(exc): Initializing class\n");
   DEBUG_MAKE_INDENT(3);
   
   if ((type=strdup(exc.type))) aldel_type=1;
   else
   {
      type="Exception";
      aldel_type=0;
   }

   DEBUG_MSG("type=" << exc.type << "\n");
}

Exception::~Exception(void)
{
   DEBUG3_MSG("Exception::~Exception(): destroying class\n");
   DEBUG_MAKE_INDENT(3);
   
   DEBUG3_MSG("type=" << type << "\n");
   if (type && aldel_type) free(type);
}

void Exception::SetType(const char * _type)
{
   if (type && aldel_type) free(type); type=0;
   if (!_type) _type="Exception";
   if ((type=strdup(_type))) aldel_type=1;
   else
   {
      type="Exception";
      aldel_type=0;
   }
}

Exception & Exception::operator=(const Exception & exc)
{
   if (this!=&exc)
   {
      DEBUG_MSG("Exception::operator=() called\n");
      DEBUG_MAKE_INDENT(3);

      *((GException *) this)=exc;
      
      DEBUG_MSG("type=" << exc.type << "\n");
	 
      if (type && aldel_type) free(type); type=0;

      if ((type=strdup(exc.type))) aldel_type=1;
      else
      {
	 type="Exception";
	 aldel_type=0;
      }
   }
   return *this;
}
