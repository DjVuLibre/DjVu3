//C-  -*- C++ -*-
//C-
//C- Copyright (c) 1999 AT&T Corp.  All rights reserved.
//C-
//C- This software may only be used by you under license from AT&T
//C- Corp. ("AT&T"). A copy of AT&T's Source Code Agreement is available at
//C- AT&T's Internet website having the URL <http://www.djvu.att.com/open>.
//C- If you received this software without first entering into a license with
//C- AT&T, you have an infringing copy of this software and cannot use it
//C- without violating AT&T's intellectual property rights.
//C-
//C- $Id: DjVuGlobal.cpp,v 1.6 1999-03-17 19:24:56 leonb Exp $




// ----------------------------------------

#define NEED_DJVU_MEMORY_IMPLEMENTATION
#include "DjVuGlobal.h"


// ----------------------------------------

#ifdef NEED_DJVU_MEMORY
#include "GException.h"

static djvu_new_callback *newptr = (djvu_new_callback*) & ::operator new;
static djvu_delete_callback *delptr = (djvu_delete_callback*) & ::operator delete;

void *
_djvu_new(size_t sz)
{
  void *addr = (*newptr)(sz);
  if (! addr) THROW(GException::outofmemory);
  return addr;
}

void  
_djvu_delete(void *addr)
{
  if (addr) 
    (*delptr)(addr);
}

void 
_djvu_memory_callback(djvu_delete_callback *dp, djvu_new_callback *np)
{
  newptr = ( np ? np : (djvu_new_callback*) &::operator new );
  delptr = ( dp ? dp : (djvu_delete_callback*) & ::operator delete );
}

#endif



// ----------------------------------------

#ifdef NEED_DJVU_PROGRESS
#include "GOS.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

static DjVuProgressScale       *p_scale  = 0;
static djvu_progress_callback  *p_cb     = 0;
static unsigned long            p_start  = 0;
static FILE                    *p_log    = 0;

void 
_djvu_end_progress()
{
  p_scale = 0;
  p_cb = 0;
  if (p_log && p_log!=stderr) 
    fclose(p_log);
  p_log = 0;
}

void 
_djvu_start_progress(DjVuProgressScale *s, djvu_progress_callback *c)
{
  _djvu_end_progress();
  p_scale = s;
  p_cb = c;
  p_log = 0;
}

void 
_djvu_start_progress(DjVuProgressScale *s, const char *logname)
{
  _djvu_end_progress();
  p_scale = s;
  p_cb = 0;
  p_start = GOS::ticks();
  p_log = stderr;
  if (logname) 
    p_log = fopen(logname,"w");
  if (p_log) 
    fprintf(p_log, "------------------------------\n");
}


// Internal stuff
void 
_djvu_progress(const char *filename, const char *tag, int index)
{
  // Log current call
  if (p_log)
    fprintf(p_log, "  { \"%s\", \"%s\", %d },\t// time=%lu\n", 
            filename,tag,index, GOS::ticks()-p_start );
  // Search for a match
  DjVuProgressScale *scale = p_scale;
  for (; scale && scale->percent>0 && scale->percent<100; scale++)
    {
      // Check for a match
      if (scale->match_filename && strcmp(filename, scale->match_filename))
        continue;
      if (scale->match_tag && strcmp(tag, scale->match_tag))
        continue;
      if (index < scale->match_index)
        break;
      // We have a match
      if (p_log)
        fprintf(p_log, "  // got %d %%\n", scale->percent );
      if (p_cb)
        (*p_cb) (scale->percent);
      // Go to next entry
      for (scale++; scale->percent>0 && scale->percent<100; scale++)
        {
          if (scale->match_filename && strcmp(filename, scale->match_filename))
            break;
          if (scale->match_tag && strcmp(tag, scale->match_tag))
            break;
          if (index <= scale->match_index)
            break;
        }
      p_scale = scale;
      break;
    }
}

#endif
