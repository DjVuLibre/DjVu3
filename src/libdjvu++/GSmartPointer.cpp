//C-  -*- C++ -*-
//C- DjVu® Reference Library (v. 3.0)
//C- 
//C- Copyright © 1997-2000 LizardTech, Inc. All Rights Reserved.
//C- 
//C- This software (the "Original Code") is subject to, and may be
//C- distributed under, the GNU General Public License, Version 2.
//C- The license should have accompanied the Original Code or you
//C- may obtain a copy of the license from the Free Software
//C- Foundation at http://www.fsf.org .
//C- 
//C- With respect to the Original Code, and subject to any third
//C- party intellectual property claims, LizardTech grants recipient
//C- a worldwide, royalty-free, non-exclusive license under patent
//C- claims infringed by making, using, or selling Original Code
//C- which are now or hereafter owned or controlled by LizardTech,
//C- but solely to the extent that any such patent is reasonably
//C- necessary to enable you to make, have made, practice, sell, or 
//C- otherwise dispose of Original Code (or portions thereof) and
//C- not to any greater extent that may be necessary to utilize
//C- further modifications or combinations.
//C- 
//C- The Original Code is provided "AS IS" WITHOUT WARRANTY OF ANY
//C- KIND, EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO
//C- ANY WARRANTY OF NON-INFRINGEMENT, OR ANY IMPLIED WARRANTY OF 
//C- MERCHANTIBILITY OF FITNESS FOR A PARTICULAR PURPOSE.
// 
// $Id: GSmartPointer.cpp,v 1.16 2000-11-02 01:08:34 bcr Exp $
// $Name:  $

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
    G_THROW("GSmartPointer.suspicious");
}

void
GPEnabled::destroy()
{
  if (count >= 0)
    G_THROW("GSmartPointer.suspicious");
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

