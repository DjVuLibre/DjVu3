//C-  -*- C++ -*-
//C-
//C-  Copyright (c) 1998 AT&T	
//C-  All Rights Reserved 
//C-
//C-  THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
//C-  The copyright notice above does not evidence any
//C-  actual or intended publication of such source code.
//C-
//C-  $Id: GContainer.cpp,v 1.2 1999-02-01 18:32:32 leonb Exp $

// File "$Id: GContainer.cpp,v 1.2 1999-02-01 18:32:32 leonb Exp $"
// - Author: Leon Bottou, 05/1997

/* Put this into *one* file, which instantiates all the required containers
#ifdef __GNUC__
#pragma implementation
#endif
*/

#include "GContainer.h"

// Implemetation of class GCONTAINERBASE

GContainerBase::~GContainerBase()
{
}

void 
GContainerBase::first(GPosition &pos) const 
{ 
  pos=firstpos(); 
}

void 
GContainerBase::last(GPosition &pos) const 
{ 
  pos=lastpos(); 
}


// Implemetation of class GPOOL

void 
GPool::newchunk()
{
  const size_t maxalignment = 16;
  size_t increment = (elemsize > sizeof(void*) ? elemsize : sizeof(void*));
  size_t alignment = (increment > maxalignment ? maxalignment : increment);
  // allocate and register
  void *nchk = operator new (alignment + increment * chunksize);
  *(void**)nchk = chunklist;
  chunklist = nchk;
  // fill free list records
  unsigned char *nelt = (unsigned char *)nchk + alignment;
  for (unsigned int i=0; i<chunksize; i++)
    {
      xdelete((void*)(nelt));
      nelt += increment;
    }
}

GPool::~GPool()
{
  void *chk = chunklist;
  while (chk)
    {
      void *nchk = *(void**)(chk);
      operator delete (chk);
      chk = nchk;
    }
  chunklist = 0;
  freelist = 0;
}

