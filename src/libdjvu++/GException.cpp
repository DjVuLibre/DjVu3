//C-  -*- C++ -*-
//C- DjVu® Reference Library (v. 3.0)
//C- 
//C- Copyright © 1999-2000 LizardTech, Inc. All Rights Reserved.
//C- 
//C- This software (the "Original Code") is subject to, and may be
//C- distributed under, the GNU General Public License, Version 2.
//C- The license should have accompanied the Original Code or you
//C- may obtain a copy of the license from the Free Software
//C- Foundation at http://www.fsf.org .
//C- 
//C- With respect to the Original Code, and subject to any third
//C- party intellectual property claims, LizardTech grants recipient
//C- a worldwide, royalty-free, non-exclusive license under patent
//C- claims infringed by making, using, or selling Original Code
//C- which are now or hereafter owned or controlled by LizardTech,
//C- but solely to the extent that any such patent is reasonably
//C- necessary to enable you to make, have made, practice, sell, or 
//C- otherwise dispose of Original Code (or portions thereof) and
//C- not to any greater extent that may be necessary to utilize
//C- further modifications or combinations.
//C- 
//C- The Original Code is provided "AS IS" WITHOUT WARRANTY OF ANY
//C- KIND, EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO
//C- ANY WARRANTY OF NON-INFRINGEMENT, OR ANY IMPLIED WARRANTY OF 
//C- MERCHANTIBILITY OF FITNESS FOR A PARTICULAR PURPOSE.
// 
// $Id: GException.cpp,v 1.23 2000-11-02 01:08:34 bcr Exp $
// $Name:  $



#ifdef __GNUC__
#pragma implementation
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#ifdef UNDER_CE
#include <afx.h>
#else
#include <new.h>
#endif
#include "GException.h"
#include "DjVuMessage.h"
#include "debug.h"

// File "$Id: GException.cpp,v 1.23 2000-11-02 01:08:34 bcr Exp $"
// - Author: Leon Bottou, 05/1997

GException::GException() 
  : cause(0), file(0), func(0), line(0)
{
}

const char * const
GException::outofmemory = "GException.outofmemory";

GException::GException(const GException & exc) 
  : file(exc.file), func(exc.func), line(exc.line)
{
  if (exc.cause && exc.cause!=outofmemory) 
    {
      char *s = new char[strlen(exc.cause)+1];
      strcpy(s, exc.cause);
      cause = s;
    }
  else
    {
      cause = exc.cause;
    }
}

GException::GException (const char *xcause, const char *file, int line, const char *func)
  : file(file), func(func), line(line)
{
  // good place to set a breakpoint and DEBUG message too. 
  // It'd hard to track exceptions which seem to go from nowhere
#ifdef DEBUG_MSG
  DEBUG_MSG("GException::GException(): cause=" << (xcause ? xcause : "unknown") << "\n");
#endif
  if (xcause && xcause!=outofmemory) 
    {
      char *s = new char[strlen(xcause)+1];
      strcpy(s, xcause);
      cause = s;
    }
  else
    {
      cause = xcause;
    }
}

GException::~GException(void)
{
  if (cause && cause!=outofmemory ) 
    delete [] (char*)cause; 
  cause=file=func=0;
}

GException & 
GException::operator=(const GException & exc)
{
  if (cause && cause!=outofmemory) 
    delete [] (char*)cause;
  cause = 0;
  file = exc.file;
  func = exc.func;
  line = exc.line;
  if (exc.cause && exc.cause!=outofmemory) 
    {
      char *s = new char[strlen(exc.cause)+1];
      strcpy(s, exc.cause);
      cause = s;
    }
  else
    {
      cause = exc.cause;
    }
  return *this;
}

void
GException::perror(const char *msg) const
{
  fflush(NULL);
  fputs("*** ",stderr);
  DjVuMsg.perror(get_cause());
  if (file && line>0)
    fprintf(stderr, "\n*** (%s:%d)", file, line);    
  else if (file)
    fprintf(stderr, "\n*** (%s)", file);        
  if (msg) 
  {
    DjVuMsg.perror(msg);
  }
  if (func)
    fprintf(stderr, "\n*** %s", func);
  fprintf(stderr,"\n");
}

const char* 
GException::get_cause(void) const
{
  if (! cause)
    return "Invalid exception";
  return cause;
}


#ifdef USE_EXCEPTION_EMULATION

GExceptionHandler *GExceptionHandler::head = 0;

void
GExceptionHandler::emthrow(const GException &gex)
{
  if (head)
    {
      head->current = gex;
      longjmp(head->jump, -1);
    }
  else
    {
      gex.perror("Unhandled exception");
#ifndef UNDER_CE
      abort();
#else
      exit(EXIT_FAILURE);
#endif
    }
}

#else // ! USE_EXCEPTION_EMULATION

static int abort_on_exception = 0;

void 
#ifndef NO_LIBGCC_HOOKS
GExceptionHandler::exthrow(const GException &ex)
#else
GExceptionHandler::exthrow(const GException ex)
#endif /* NO_LIBGCC_HOOKS */
{
  if (abort_on_exception) 
    abort();
  throw ex;
}

void 
GExceptionHandler::rethrow(void)
{
  if (abort_on_exception) 
    abort();
  throw;
}

#endif



// ------ MEMORY MANAGEMENT HANDLER

#ifndef NEED_DJVU_MEMORY
// This is not activated when C++ memory management
// is overidden.  The overriding functions handle
// memory exceptions by themselves.
#if defined(_MSC_VER)
// Microsoft is different!
static int throw_memory_error(size_t) { G_THROW(GException::outofmemory); return 0; }
#ifndef UNDER_CE
// Does not exist under CE, check for null and throw an exception instead.
static int (*old_handler)(size_t) = _set_new_handler(throw_memory_error);
#endif
#else // !_MSC_VER
// Standard C++
static void throw_memory_error() { G_THROW(GException::outofmemory); }
static void (*old_handler)() = set_new_handler(throw_memory_error);
#endif // !_MSC_VER
#endif // !NEED_DJVU_MEMORY
