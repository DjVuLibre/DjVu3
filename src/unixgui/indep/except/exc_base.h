//C-  -*- C++ -*-
//C-
//C-  Copyright © 1999-2001, LizardTech, Inc. All Rights Reserved.
//C-              Unauthorized use prohibited.
//C-
// 
// $Id: exc_base.h,v 1.1 2001-05-29 22:05:30 bcr Exp $
// $Name:  $


#ifndef HDR_EXC_BASE
#define HDR_EXC_BASE

#ifdef __GNUC__
#pragma interface
#endif

#include "GException.h"

#define STD_MSG		"Due to the lack of memory, the text of the\n"\
			"exception message HAS BEEN LOST."

#define EXCEPTION(func, msg) Exception(func, msg, 0, __FILE__, __LINE__)

class Exception : public GException
{
private:
   char		* type;
   int		aldel_type;
protected:
   Exception(void);
   Exception(const char * _func, const char * _msg, const char * _type=0,
	     const char * _file=0, int _line=0);
   Exception(const Exception & exc);
   void			SetType(const char * _type);
public:
   const char *		GetType(void) const { return type; };
   
   Exception & operator =(const Exception & exc);
   virtual ~Exception(void);
};

#define DECLARE_EXCEPTION(exc_name, exc_str_name, parent_name) \
class exc_name : public parent_name\
{\
public:\
   exc_name(void) : parent_name() {};\
   exc_name(const char * func, const char * msg, const char * type=0,\
	    const char * file=0, const int line=0) :\
      parent_name(func, msg, type ? type : exc_str_name, file, line) {};\
   exc_name(const Exception & exc) : parent_name(exc)\
   {\
      SetType(exc_str_name);\
   };\
   virtual ~exc_name(void) {};\
};

#endif
