//C-  -*- C++ -*-
//C-
//C-  Copyright (c) 1988 AT&T	
//C-  All Rights Reserved 
//C-
//C-  THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
//C-  The copyright notice above does not evidence any
//C-  actual or intended publication of such source code.
//C-
//C-  $Id: GException.cpp,v 1.2 1999-01-27 22:15:23 leonb Exp $


#ifdef __GNUC__
#pragma implementation
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <new.h>
#include "GException.h"
#include "debug.h"


// File "$Id: GException.cpp,v 1.2 1999-01-27 22:15:23 leonb Exp $"
// - Author: Leon Bottou, 05/1997

static const char *outofmemory = "Out of memory";

GException::GException() 
  : cause(0), file(0), func(0), line(0)
{
}


GException::GException(const GException & exc) 
  : cause(0), file(exc.file), func(exc.func), line(exc.line)
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
  : cause(0), file(file), func(func), line(line)
{
  // good place to set a breakpoint and DEBUG message too. 
  // It'd hard to track exceptions which seem to go from nowhere
  DEBUG_MSG("GException::GException(): cause=" << (cause ? cause : "unknown") << "\n");

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
  fprintf(stderr, "*** %s", get_cause());
  if (file && line>0)
    fprintf(stderr, "\n*** (%s:%d)", file, line);    
  else if (file)
    fprintf(stderr, "\n*** (%s)", file);        
  if (msg) 
    fprintf(stderr, " %s", msg);
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
      abort();
    }
}


#endif



// ------ HACK TO SET NEW HANDLER


#if defined(_MSC_VER)


static int __cdecl  
throw_memory_error(size_t) 
{
  THROW(outofmemory);
  return 0;
}

class _SetNewHandler { // DJVU_CLASS
  int (*old_handler)(size_t);
public:
  _SetNewHandler() {
    old_handler = _set_new_handler(throw_memory_error);
  }
  ~_SetNewHandler() {
    _set_new_handler(old_handler);
  }
};



#else // ! MICROSOFT


static void 
throw_memory_error() 
{
  THROW(outofmemory);
}

class _SetNewHandler {  // DJVU_CLASS
  void (*old_handler)();
public:
  _SetNewHandler() {
    old_handler = set_new_handler(throw_memory_error);
  }
  ~_SetNewHandler() {
    set_new_handler(old_handler);
  }
};

#endif

static _SetNewHandler junk;
