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
//C- $Id: DjVuGlobalMemory.cpp,v 1.1 1999-12-08 20:56:43 bcr Exp $


#ifdef NEED_DJVU_MEMORY
#ifndef NEED_DJVU_MEMORY_IMPLEMENTATION
#define NEED_DJVU_MEMORY_IMPLEMENTATION
#endif /* NEED_DJVU_MEMORY_IMPLEMENTATION */

#include "DjVuGlobal.h"
#include "GException.h"

static djvu_delete_callback *_djvu_delete_handler = 0;
static djvu_new_callback *_djvu_new_handler = 0;
static djvu_delete_callback *deleteArray_handler = 0;
static djvu_new_callback *newArray_handler = 0;

int
djvu_memoryObject_callback (
  djvu_delete_callback* delete_handler,
  djvu_new_callback* new_handler
) {
  if(delete_handler && new_handler)
  {
#ifdef UNIX
    _djvu_new_ptr=&_djvu_new;
    _djvu_delete_ptr=&_djvu_delete;
#endif
    _djvu_delete_handler=delete_handler;
    _djvu_new_handler=new_handler;
    return 1;
  }else
  {
#ifdef UNIX
    _djvu_new_ptr=(djvu_new_callback *)&(operator new);
    _djvu_delete_ptr=(djvu_delete_callback *)&(operator delete);
#endif
    _djvu_delete_handler=0;
    _djvu_new_handler=0;
    return (delete_handler||new_handler)?0:1;
  }
  return 0;
}

void *
_djvu_new(size_t siz)
{
  void *ptr;
#ifndef UNIX
  if(_djvu_new_handler)
  {
#endif
    if(!(ptr=(*_djvu_new_handler)(siz)))
    {
      THROW("Memory Exhausted");
    }
#ifndef UNIX
  }else
  {
#ifdef WIN32
          ptr = (void *) new char[siz] ;
#else
    ptr=operator new(siz);
#endif
  }
#endif
  return ptr;
}

void  
_djvu_delete(void *addr)
{
  if(addr)
  {
    if(_djvu_delete_handler)
    {
      (*_djvu_delete_handler)(addr);
    }else
    {
      operator delete(addr);
    }
  }
}

void *
_djvu_newArray(size_t siz)
{
  void *ptr;
#ifndef UNIX
  if(newArray_handler)
  {
#endif
    if(!(ptr=(*newArray_handler)(siz)))
    {
      THROW("Memory Exhausted");
    }
#ifndef UNIX
  }else
  {
#ifdef WIN32
          ptr = (void *) new char[siz] ;
#else
          ptr=operator new [] (siz);
#endif
  }
#endif
  return ptr;
}

void
_djvu_deleteArray(void *addr)
{
  if(addr)
  {
    if(deleteArray_handler)
    {
      (*deleteArray_handler)(addr);
    }else
    {
#ifdef WIN32
                delete [] (addr) ;
#else
        operator delete [] (addr);
#endif
    }
  }
}

#endif

