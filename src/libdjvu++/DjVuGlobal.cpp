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
//C- $Id: DjVuGlobal.cpp,v 1.12 1999-09-23 13:09:26 leonb Exp $




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
#include "GException.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define INITIAL  500
#define INTERVAL 500

DjVuProgressTask *DjVuProgressTask::head = 0;

void (*DjVuProgressTask::callback)(unsigned long, unsigned long) = 0;

unsigned long DjVuProgressTask::lastsigdate = 0;

DjVuProgressTask::DjVuProgressTask(int nsteps)
  : parent(0), nsteps(nsteps), runtostep(0)
{
  if (callback)
    {
      unsigned long curdate = GOS::ticks();
      startdate = curdate;
      if (head==0)
        lastsigdate = curdate + INITIAL;
      parent = head;
      head = this;
    }
}

DjVuProgressTask::~DjVuProgressTask()
{
  if (callback)
    {
      if (head != this)
        THROW("DjVuProgress is not compatible with multithreading");
      head = parent;
      unsigned long curdate = GOS::ticks();
      if (!parent)
        (*callback)(curdate-startdate, curdate-startdate);
    }
}

void
DjVuProgressTask::run(int tostep)
{
  if (callback && tostep>runtostep)
    {
      unsigned long curdate = GOS::ticks();
      if (curdate > lastsigdate + INTERVAL)
        signal(curdate, curdate);
      runtostep = tostep;
    }
}

void
DjVuProgressTask::signal(unsigned long curdate, unsigned long estdate)
{
  int inprogress = runtostep;
  if (inprogress > nsteps)
    inprogress = nsteps;
  if (inprogress > 0)
    {
      unsigned long enddate = startdate;
      enddate += (estdate-startdate) * nsteps / inprogress;
      if (parent)
        {
          parent->signal(curdate, enddate);
        }
      else if (callback && curdate<enddate)
        {
          (*callback)(curdate-startdate, enddate-startdate);
          lastsigdate = curdate;
        }
    }
}

#endif

