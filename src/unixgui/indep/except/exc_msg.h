//C-  -*- C++ -*-
//C-
//C- DjVu� Unix Viewer (v. 3.5)
//C- 
//C- Copyright � 1999-2001 LizardTech, Inc. All Rights Reserved.
//C- The DjVu Reference Library is protected by U.S. Pat. No.
//C- 6,058,214 and patents pending.
//C- 
//C- This software is subject to, and may be distributed under, the
//C- GNU General Public License, Version 2. The license should have
//C- accompanied the software or you may obtain a copy of the license
//C- from the Free Software Foundation at http://www.fsf.org .
//C- 
//C- The computer code originally released by LizardTech under this
//C- license and unmodified by other parties is deemed the "LizardTech
//C- Original Code."
//C- 
//C- With respect to the LizardTech Original Code ONLY, and subject
//C- to any third party intellectual property claims, LizardTech
//C- grants recipient a worldwide, royalty-free, non-exclusive license
//C- under patent claims now or hereafter owned or controlled by
//C- LizardTech that are infringed by making, using, or selling
//C- LizardTech Original Code, but solely to the extent that any such
//C- patent(s) is/are reasonably necessary to enable you to make, have
//C- made, practice, sell, or otherwise dispose of LizardTech Original
//C- Code (or portions thereof) and not to any greater extent that may
//C- be necessary to utilize further modifications or combinations.
//C- 
//C- The LizardTech Original Code is provided "AS IS" WITHOUT WARRANTY
//C- OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED
//C- TO ANY WARRANTY OF NON-INFRINGEMENT, OR ANY IMPLIED WARRANTY OF
//C- MERCHANTIBILITY OR FITNESS FOR A PARTICULAR PURPOSE.
//C-
// 
// $Id: exc_msg.h,v 1.5 2001-10-17 19:04:26 docbill Exp $
// $Name:  $


#ifndef HDR_EXC_MSG
#define HDR_EXC_MSG

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
