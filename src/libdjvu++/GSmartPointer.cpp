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
//C- $Id: GSmartPointer.cpp,v 1.9 2000-01-19 23:50:38 bcr Exp $

// File "$Id: GSmartPointer.cpp,v 1.9 2000-01-19 23:50:38 bcr Exp $"
// - Author: Leon Bottou, 05/1997

/* Put this into *one* file, which instantiates all the required containers
#ifdef __GNUC__
#pragma implementation
#endif
*/

#include "GThreads.h"
#include "GSmartPointer.h"
#include "GException.h"


// ------ STATIC CRITICAL SECTION

static GCriticalSection gcsCounter;


// ------ GPENABLED


GPEnabled::~GPEnabled()
{
  if (count > 0)
    THROW("Suspicious destruction of referenced GPEnabled object");
}

void
GPEnabled::destroy()
{
  if (count >= 0)
    THROW("Suspicious destruction of referenced GPEnabled object");
  delete this;
}

void 
GPEnabled::ref()
{
  gcsCounter.lock();
  count++;
  gcsCounter.unlock();
}

void 
GPEnabled::unref()
{
  gcsCounter.lock();
  if (! --count) 
    count = -1;
  gcsCounter.unlock();
  if (count < 0)
    destroy();
}


// ------ GPBASE


GPBase&
GPBase::assign (GPEnabled *nptr)
{
  gcsCounter.lock();
  if (nptr)
    {
      if (nptr->count >= 0)  
        nptr->count++;
      else
        nptr = 0;
    }
  if (ptr)
    {
      GPEnabled *old = ptr;
      ptr = nptr;
      if (! --old->count) 
        old->count = -1;
      gcsCounter.unlock();      
      if (old->count < 0)
        old->destroy();
    }
  else
    {
      ptr = nptr;
      gcsCounter.unlock();
    }
  return *this;
}


GPBase&
GPBase::assign (const GPBase &sptr)
{
  gcsCounter.lock();
  if (sptr.ptr) 
    {
      sptr.ptr->count++;
    }
  if (ptr)
    {
      GPEnabled *old = ptr;
      ptr = sptr.ptr;
      if (! --old->count) 
        old->count = -1;
      gcsCounter.unlock();      
      if (old->count < 0)
        old->destroy();
    }
  else
    {
      ptr = sptr.ptr;
      gcsCounter.unlock();
    }
  return *this;
}


