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
//C- $Id: DjVuGlobal.cpp,v 1.8 1999-09-21 21:06:19 leonb Exp $




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

DjVuProgress::CheckPoint *DjVuProgress::chk = 0;
DjVuProgress::Callback   *DjVuProgress::cb = 0;
unsigned long             DjVuProgress::base = 0;
void                     *DjVuProgress::log = 0;
int                       DjVuProgress::taglen = 0;
int                       DjVuProgress::tagmax = 0;
char                     *DjVuProgress::tagbuf = 0;


void 
DjVuProgress::end()
{
  if ((FILE*)log && (FILE*)log!=stderr) 
    fclose((FILE*)log);
  chk = 0;
  cb = 0;
  log = 0;
  tagmax = taglen = 0;
  delete [] tagbuf;
  tagbuf = 0;
}

void 
DjVuProgress::start(DjVuProgress::CheckPoint *s, DjVuProgress::Callback *c)
{
  end();
  chk = s;
  cb = c;
  log = 0;
}

void 
DjVuProgress::start(const char *logname)
{
  end();
  base = GOS::ticks();
  log = (void*)stderr;
  if (logname) 
    log = (void*)fopen(logname,"w");
}

DjVuProgress::Event::~Event()
{
  taglen = n;
  tagbuf[taglen] = 0;
}

DjVuProgress::Event::Event(const char *tag)
  : n(taglen)
{
  if (!log && !chk) 
    return;
  enter(tag);
}

DjVuProgress::Event::Event(int tag)
  : n(taglen)
{
  if (!log && !chk) 
    return;
  char buffer[16];
  sprintf(buffer,"%d", tag);
  enter(buffer);
}

void
DjVuProgress::Event::enter(const char *tag)
{
  if (!log && !chk) 
    return;
  // Check tag buffer
  int l = strlen(tag);
  if (taglen+l+2 > tagmax) {
    int newtagmax = tagmax + 256;
    char *newbuf = new char[newtagmax];
    strcpy(newbuf, tagbuf ? tagbuf : "");
    delete [] tagbuf;
    tagbuf = newbuf;
    tagmax = newtagmax;
  }
  // Append tag component
  if (taglen>0) {
    strcpy(tagbuf+taglen, ".");
    taglen += 1;
  }
  strcpy(tagbuf+taglen, tag);
  taglen += l;
  // Perform trace
  if (log)
    fprintf((FILE*)log, "  { %6ld, \"%s\" },\n", GOS::ticks()-base, tagbuf);
  // Scan checkpoints
  if (chk)
    {
      int lastpassed = -1;
      for (CheckPoint *k=chk; k->tag; k++)
        if (! k->passed)
          {
            char *s = tagbuf;
            char *d = (char*)(k->tag);
            for(;;)
              {
                if (*s>='0' && *s<='9' && *d>='0' && *d<='9')
                  {
                    int si = strtol(s, &s, 10);
                    int di = strtol(d, &d, 10);
                    if (si >= di)
                      continue;
                    break;
                  }
                if (*s==0 && *d==0)
                  { 
                    k->passed = 1; 
                    lastpassed = k-chk;
                    break; 
                  }
                if (*s++ != *d++)
                  break;
              }
          }
      // Callback
      if (lastpassed>=0 && cb)
        (*cb)(lastpassed);
    }
}


#endif

