//C-  -*- C++ -*-
//C-
//C- This software contains software code or other information proprietary to
//C- AT&T Corp. ("AT&T").  Unless you accept a license to use this software,
//C- you shall not use, execute, compile, modify, redistribute, reverse
//C- compile, disassemble, or otherwise reverse engineer the AT&T software or
//C- any derived work of the AT&T software.  The text of a license can be
//C- found in file "ATTLICENSE" or at the Internet website having the URL
//C- "http://www.djvu.att.com/open".
//C
//C- This software is provided to you "AS IS".  YOU ASSUME TOTAL
//C- RESPONSIBILITY AND RISK FOR USE OF THE AT&T SOFTWARE.  AT&T DOES NOT
//C- MAKE, AND EXPRESSLY DISCLAIMS, ANY EXPRESS OR IMPLIED WARRANTIES OF ANY
//C- KIND WHATSOEVER, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTIES OF
//C- MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE, WARRANTIES OF TITLE
//C- OR NON-INFRINGEMENT OF ANY INTELLECTUAL PROPERTY RIGHTS, ANY WARRANTIES
//C- ARISING BY USAGE OF TRADE, COURSE OF DEALING OR COURSE OF PERFORMANCE, OR
//C- ANY WARRANTY THAT THE AT&T SOFTWARE IS ERROR FREE OR WILL MEET YOUR
//C- REQUIREMENTS.
//C-
//C-     (C) AT&T Corp. All rights reserved.
//C-     AT&T is a registered trademark of AT&T Corp.
//C-
//C- $Id: GSmartPointer.cpp,v 1.3 1999-03-15 18:28:51 leonb Exp $

// File "$Id: GSmartPointer.cpp,v 1.3 1999-03-15 18:28:51 leonb Exp $"
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


