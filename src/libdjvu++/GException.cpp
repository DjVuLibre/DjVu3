//C-  -*- C++ -*-
//C-
//C- This software contains software code or other information proprietary to
//C- AT&T Corp. ("AT&T").  Unless you accept a license to use this software,
//C- you shall not use, execute, compile, modify, redistribute, reverse
//C- compile, disassemble, or otherwise reverse engineer the AT&T software or
//C- any derived work of the AT&T software.  The text of a license can be
//C- found in file "ATTLICENSE" or at the Internet website having the URL
//C- "http://www.djvu.att.com/open".
//C
//C- This software is provided to you "AS IS".  YOU ASSUME TOTAL
//C- RESPONSIBILITY AND RISK FOR USE OF THE AT&T SOFTWARE.  AT&T DOES NOT
//C- MAKE, AND EXPRESSLY DISCLAIMS, ANY EXPRESS OR IMPLIED WARRANTIES OF ANY
//C- KIND WHATSOEVER, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTIES OF
//C- MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE, WARRANTIES OF TITLE
//C- OR NON-INFRINGEMENT OF ANY INTELLECTUAL PROPERTY RIGHTS, ANY WARRANTIES
//C- ARISING BY USAGE OF TRADE, COURSE OF DEALING OR COURSE OF PERFORMANCE, OR
//C- ANY WARRANTY THAT THE AT&T SOFTWARE IS ERROR FREE OR WILL MEET YOUR
//C- REQUIREMENTS.
//C-
//C-     (C) AT&T Corp. All rights reserved.
//C-     AT&T is a registered trademark of AT&T Corp.
//C-
//C- $Id: GException.cpp,v 1.7 1999-03-15 18:28:51 leonb Exp $


#ifdef __GNUC__
#pragma implementation
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <new.h>
#include "GException.h"
#ifdef DEBUGLVL
#include "debug.h"
#endif

// File "$Id: GException.cpp,v 1.7 1999-03-15 18:28:51 leonb Exp $"
// - Author: Leon Bottou, 05/1997

GException::GException() 
  : cause(0), file(0), func(0), line(0)
{
}

const char * const
GException::outofmemory = "Out of memory";

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
#ifdef DEBUG_MSG
  DEBUG_MSG("GException::GException(): cause=" << (cause ? cause : "unknown") << "\n");
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



// ------ MEMORY MANAGEMENT HANDLER

#ifndef NEED_DJVU_MEMORY
// This is not activated when C++ memory management
// is overidden.  The overriding functions handle
// memory exceptions by themselves.
#if defined(_MSC_VER)
// Microsoft is different!
static int throw_memory_error(size_t) { THROW(GException::outofmemory); return 0; }
static int (*old_handler)(size_t) = _set_new_handler(throw_memory_error);
#else // !_MSC_VER
// Standard C++
static void throw_memory_error() { THROW(GException::outofmemory); }
static void (*old_handler)() = set_new_handler(throw_memory_error);
#endif // !_MSC_VER
#endif // !NEED_DJVU_MEMORY
