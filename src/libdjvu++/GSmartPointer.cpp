//C-  -*- C++ -*-
//C-
//C-  Copyright (c) 1988 AT&T	
//C-  All Rights Reserved 
//C-
//C-  THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
//C-  The copyright notice above does not evidence any
//C-  actual or intended publication of such source code.
//C-
//C-  $Id: GSmartPointer.cpp,v 1.1 1999-01-22 00:40:19 leonb Exp $

// File "$Id: GSmartPointer.cpp,v 1.1 1999-01-22 00:40:19 leonb Exp $"
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
  if (count)
    THROW("Suspicious destruction of referenced GPEnabled object");
}

void
GPEnabled::destroy()
{
  if (count)
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
  int newcount = --count;
  gcsCounter.unlock();
  if (newcount==0)
    destroy();
}


// ------ GPBASE


GPBase&
GPBase::assign (GPEnabled *nptr)
{
  gcsCounter.lock();
  if (nptr) 
    {
      nptr->count++;
    }
  if (ptr)
    {
      GPEnabled *old = ptr;
      ptr = nptr;
      int newcount = --old->count;
      gcsCounter.unlock();      
      if (newcount == 0)
        old->destroy();
    }
  else
    {
      ptr = nptr;
      gcsCounter.unlock();
    }
  return *this;
}


