//C-  -*- C++ -*-
//C-
//C- Copyright (c) 1999 AT&T Corp.
//C- Copyright (c) 2000 LizardTech Inc.
//C- All rights reserved.
//C-
//C- This software may only be used by you under license from LizardTech
//C- Inc. A copy of LizardTech's Source Code Agreement is available at
//C- LizardTech's Internet website having the URL <http://www.djvu.com/open>.
//C- If you received this software without first entering into a license with
//C- LizardTech, you have an infringing copy of this software and cannot use it
//C- without violating LizardTech's intellectual property rights.
//C-
//C- $Id: DjVuGlobal.cpp,v 1.24 2000-09-18 17:10:11 bcr Exp $

/** This file impliments the DjVuProgressTask elements.  The memory
    functions are implimented in a separate file, because only the memory
    functions should be compiled with out overloading of the memory functions.
 */
  

#ifdef NEED_DJVU_PROGRESS
#include "DjVuGlobal.h"


// ----------------------------------------

#include "GOS.h"
#include "GThreads.h"
#include "GException.h"
#include "GContainer.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define INITIAL  500
#define INTERVAL 250

class DjVuProgressTask::Data : public GPEnabled
{
public:
  djvu_progress_callback *callback;
  DjVuProgressTask *head;
  const char *gtask;
  unsigned long lastsigdate;
  Data(djvu_progress_callback *_callback):
    callback(_callback), head(0), gtask(0), lastsigdate(0) {}
};

  
static GMap<void *,GP<DjVuProgressTask::Data> > *map_ptr=0;

djvu_progress_callback *
DjVuProgressTask::set_callback(djvu_progress_callback *_callback)
{ 
  djvu_progress_callback *retval=0;
  if(_callback)
  {
    if(!map_ptr)
    {
      map_ptr=new GMap<void *,GP<DjVuProgressTask::Data> >;
    }
    GMap<void *,GP<DjVuProgressTask::Data> > &map=*map_ptr;
    void *threadID=GThread::current();
    if(map.contains(threadID))
    {
      DjVuProgressTask::Data &data=*(map[threadID]);
      retval=data.callback;
      data.callback=_callback;
      data.head=0;
      data.gtask=0;
      data.lastsigdate=0;
    }else
    {
      map[threadID]=new Data(_callback);
    }
  }else if(map_ptr)
  {
    GMap<void *,GP<DjVuProgressTask::Data> > &map=*map_ptr;
    void *threadID=GThread::current();
    if(map.contains(threadID))
    {
      DjVuProgressTask::Data &data=*(map[threadID]);
      retval=data.callback;
      data.callback=0;
      data.head=0;
      data.gtask=0;
      data.lastsigdate=0;
      map.del(threadID);
    }
  }
  return retval;
}

DjVuProgressTask::DjVuProgressTask(const char *xtask,int nsteps)
  : task(xtask),parent(0), nsteps(nsteps), runtostep(0), gdata(0), data(0)
{
  //  gtask=task;
  if(map_ptr)
  {
    GMap<void *,GP<DjVuProgressTask::Data> > &map=*map_ptr;
    void *threadID=GThread::current();
    if(map.contains(threadID))
    {
      gdata=new GP<Data>;
      Data &d=*(data=((*(GP<Data> *)gdata)=map[threadID]));
      if(d.callback)
      {
        unsigned long curdate = GOS::ticks();
        startdate = curdate;
        if (!d.head)
          d.lastsigdate = curdate + INITIAL;
        parent = d.head;
        d.head = this;
      }
    }
  }
}

DjVuProgressTask::~DjVuProgressTask()
{
  if (data && data->callback)
  {
    if (data->head != this)
      G_THROW("DjVuProgress is not compatible with multithreading");
    data->head = parent;
    if (!parent)
    {
      unsigned long curdate = GOS::ticks();
      (*(data->callback))(data->gtask?data->gtask:"",curdate-startdate, curdate-startdate);
    }
  }
  delete (GP<Data> *)gdata;
}

void
DjVuProgressTask::run(int tostep)
{
  if(data)
  {
    data->gtask=task;
    if ((data->callback)&&(tostep>runtostep))
    {
      unsigned long curdate = GOS::ticks();
      if (curdate > data->lastsigdate + INTERVAL)
        signal(curdate, curdate);
      runtostep = tostep;
    }
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
      const unsigned long enddate = startdate+
        (unsigned long)(((float)(estdate-startdate) * (float)nsteps) / (float)inprogress);
      if (parent)
      {
        parent->signal(curdate, enddate);
      }
      else if (data && data->callback && curdate<enddate)
      {
        (*(data->callback))(data->gtask?data->gtask:"",curdate-startdate, enddate-startdate);
        data->lastsigdate = curdate;
      }
    }
}

// Progress callback
//
djvu_progress_callback *
djvu_set_progress_callback( djvu_progress_callback *callback )
{
   return DjVuProgressTask::set_callback(callback);
}

int djvu_supports_progress_callback(void) {return 1;}

#else

#ifndef HAS_DJVU_PROGRESS_TYPEDEF
extern "C"
{
  void *djvu_set_progress_callback(void *);
  int djvu_supports_progress_callback(void) {return 0;}
}
void *djvu_set_progress_callback(void *) { return 0; }
#else
int djvu_supports_progress_callback(void) {return 0;}
djvu_progress_callback *
djvu_set_progress_callback( djvu_progress_callback *) { return 0; }
#endif

#endif

