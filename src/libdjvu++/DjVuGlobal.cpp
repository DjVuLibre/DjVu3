//C-  -*- C++ -*-
//C-
//C-  Copyright (c) 1998 AT&T	
//C-  All Rights Reserved 
//C-
//C-  THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
//C-  The copyright notice above does not evidence any
//C-  actual or intended publication of such source code.
//C-
//C-  $Id: DjVuGlobal.cpp,v 1.1 1999-02-05 22:48:32 leonb Exp $




// ----------------------------------------

#define NEED_DJVU_MEMORY_IMPLEMENTATION
#include "DjVuGlobal.h"
#include "GOS.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>


// ----------------------------------------

#ifdef NEED_DJVU_MEMORY

static djvu_new_callback *newptr = (djvu_new_callback*) & ::operator new;
static djvu_delete_callback *delptr = (djvu_delete_callback*) & ::operator delete;

void *
_djvu_new(size_t sz)
{
  return (*newptr)(sz);
}

void  
_djvu_delete(void *addr)
{
  if (addr) (*delptr)(addr);
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
      if (scale->match_filename)
        if (strcmp(filename, scale->match_filename))
          continue;
      if (scale->match_tag)
        if (strcmp(tag, scale->match_tag))
          continue;
      if (scale->match_index)
        if (index < scale->match_index)
          break;
      // Match
      if (p_log)
        fprintf(p_log, "  // got %d %%\n", scale->percent );
      if (p_cb)
        (*p_cb) (scale->percent);
      p_scale = ++scale;
      break;
    }
}

#endif
