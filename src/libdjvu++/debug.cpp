//C-  -*- C++ -*-
//C-
//C-  Copyright (c) 1998 AT&T	
//C-  All Rights Reserved 
//C-
//C-  THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
//C-  The copyright notice above does not evidence any
//C-  actual or intended publication of such source code.
//C-
//C-  $Id: debug.cpp,v 1.1.1.2 1999-10-22 19:29:23 praveen Exp $


#include "debug.h"

#if ( DEBUGLVL > 0 )

#include "GThreads.h"
#include "GContainer.h"
#include <stdarg.h>
#include <stdio.h>
#include <errno.h>

#ifdef __GNUC__
#pragma implementation
#endif

#ifndef UNIX
#ifndef WIN32
#ifndef macintosh
#define UNIX
#endif
#endif
#endif

static GCriticalSection debug_lock;
#ifdef RUNTIME_DEBUG_ONLY
static int              debug_level = 0;
#else
static int              debug_level = DEBUGLVL;
#endif
static int              debug_id;
#ifdef UNIX
static FILE            *debug_file;
static int              debug_file_count;
#endif

#if THREADMODEL==NOTHREADS
static Debug debug_obj;
#else
static GMap<long, Debug> debug_map;
#endif

Debug::Debug()
  : block(0), indent(0)
{
  id = debug_id++;
#ifdef UNIX
  GCriticalSectionLock glock(&debug_lock);
  if (! debug_file)
    debug_file = fopen("/dev/tty", "w");
  if (! debug_file)
    debug_file = stderr;
  debug_file_count++;
#endif
}

Debug::~Debug()
{
#ifdef UNIX
  if (--debug_file_count == 0)
    {
      if (debug_file != stderr)
        fclose(debug_file);
      debug_file = 0;
    }
#endif
}

void   
Debug::format(const char *fmt, ... )
{
  if (! block)
    {
      va_list ap;
      va_start(ap, fmt);
      char buffer[256];
      vsprintf(buffer, fmt, ap);
      va_end(ap);
      GCriticalSectionLock glock(&debug_lock);
#ifdef UNIX
      fprintf(debug_file, "%s", buffer);
      fflush(debug_file);
#endif
#ifdef WIN32
      OutputDebugString(buffer);
#endif
    }
}

void   
Debug::set_debug_level(int lvl)
{
  debug_level = lvl;
}

void
Debug::modify_indent(int rindent)
{
  indent += rindent;
}

Debug& 
Debug::lock(int lvl, int noindent)
{
  int threads_num=1;
  debug_lock.lock();
  // Find Debug object
#if THREADMODEL==NOTHREADS
  // Get no-threads debug object
  Debug &dbg = debug_obj;
#else
  // Get per-thread debug object
  long threadid = (long) GThread::current();
  Debug &dbg = debug_map[threadid];
  threads_num=debug_map.size();
#endif
  // Check level
  dbg.block = (lvl > debug_level);
  // Output thread id and indentation
  if (! noindent)
    {
      if (threads_num>1) dbg.format("[T%d] ", dbg.id);
      int ind = dbg.indent;
      char buffer[257];
      memset(buffer,' ', sizeof(buffer));
      buffer[sizeof(buffer)-1] = 0;
      while (ind > (int)sizeof(buffer)-1)
        {
          dbg.format("%s", buffer);
          ind -= sizeof(buffer)-1;
        }
      if (ind > 0)
        {
          buffer[ind] = 0;
          dbg.format("%s", buffer);
        }
    }
  // Return
  return dbg;
}

void
Debug::unlock()
{
  debug_lock.unlock();
}

#define OP(type, fmt) \
Debug& Debug::operator<<(type arg)\
{ format(fmt, arg); return *this; }

Debug& Debug::operator<<(bool arg)
{
   format("%s", arg ? "TRUE" : "FALSE"); return *this;
}

OP(char, "%c")
OP(unsigned char, "%c")
OP(int, "%d")
OP(unsigned int, "%u")
OP(short int, "%hd")
OP(unsigned short int, "%hu")
OP(long, "%ld")
OP(unsigned long, "%lu")
OP(float, "%g")
OP(double, "%g")
OP(const void * const, "0x%08x")

Debug& Debug::operator<<(const char * const ptr) 
{
  char buffer[256];
  const char *s = ptr;
  char *d = buffer;
  while (s && *s && d<buffer+sizeof(buffer)-4)
    *d++ = *s++;
  if (! s)
    strcpy(d, "(null)");
  else if (* s)
    strcpy(d, "...");
  *d = 0;
  format("%s", buffer);
  return *this; 
}

Debug& Debug::operator<<(const unsigned char * const ptr) 
{ 
  return operator<<( (const char *) ptr );
}

DebugIndent::DebugIndent(int inc)
  : inc(inc)
{
  Debug &dbg = Debug::lock(0,1);
  dbg.modify_indent(inc);
  dbg.unlock();
}

DebugIndent::~DebugIndent()
{
  Debug &dbg = Debug::lock(0,1);
  dbg.modify_indent(-inc);
  dbg.unlock();
}

#endif
