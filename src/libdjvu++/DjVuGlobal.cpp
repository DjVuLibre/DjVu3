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
//C- $Id: DjVuGlobal.cpp,v 1.17 2000-01-04 04:48:37 bcr Exp $

/** This file impliments the DjVuProgressTask elements.  The memory
    functions are implimented in a separate file, because only the memory
    functions should be compiled with out overloading of the memory functions.
 */
  

#include "DjVuGlobal.h"


// ----------------------------------------

#ifdef NEED_DJVU_PROGRESS
#include "GOS.h"
#include "GException.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define INITIAL  500
#define INTERVAL 250

djvu_progress_callback *_djvu_progress_ptr = 0;

DjVuProgressTask *DjVuProgressTask::head = 0;

djvu_progress_callback *&DjVuProgressTask::callback=_djvu_progress_ptr;

const char *DjVuProgressTask::task=0;

unsigned long DjVuProgressTask::lastsigdate = 0;

DjVuProgressTask::DjVuProgressTask(const char *xtask,int nsteps)
  : parent(0), nsteps(nsteps), runtostep(0)
{
  task=xtask;
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
      if (!parent)
        {
          unsigned long curdate = GOS::ticks();
          (*callback)(task?task:"",curdate-startdate, curdate-startdate);
        }
    }
}

void
DjVuProgressTask::run(const char *xtask,int tostep)
{
  task=xtask;
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
          (*callback)(task?task:"",curdate-startdate, enddate-startdate);
          lastsigdate = curdate;
        }
    }
}

// Progress callback
//
djvu_progress_callback *
djvu_set_progress_callback( djvu_progress_callback *callback )
{
   djvu_progress_callback *ncallback=_djvu_progress_ptr;
   _djvu_progress_ptr=callback;
   return ncallback; 
}
#else

djvu_progress_callback *
djvu_set_progress_callback( djvu_progress_callback *)
{
   return 0;
}

#endif

