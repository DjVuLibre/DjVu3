//C-  -*- C++ -*-
//C- DjVu® Reference Library (v. 3.0)
//C- 
//C- Copyright © 1999-2000 LizardTech, Inc. All Rights Reserved.
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
// $Id: DjVuGlobalMemory.cpp,v 1.10 2000-11-03 02:08:37 bcr Exp $
// $Name:  $

#ifdef NEED_DJVU_MEMORY
#ifndef NEED_DJVU_MEMORY_IMPLEMENTATION
#define NEED_DJVU_MEMORY_IMPLEMENTATION
#endif /* NEED_DJVU_MEMORY_IMPLEMENTATION */

#include "DjVuGlobal.h"
#include "GException.h"
#include <stdlib.h>
#include <string.h>
#include <new.h>

#ifdef UNIX
djvu_delete_callback *
_djvu_delete_ptr=(djvu_delete_callback *)&(operator delete);
djvu_delete_callback *
_djvu_deleteArray_ptr=(djvu_delete_callback *)&(operator delete []);
djvu_new_callback *
_djvu_new_ptr=(djvu_new_callback *)&(operator new);
djvu_new_callback *
_djvu_newArray_ptr=(djvu_new_callback *)&(operator new []);
#endif

static djvu_delete_callback *_djvu_delete_handler = 0;
static djvu_new_callback *_djvu_new_handler = 0;
static djvu_delete_callback *deleteArray_handler = 0;
static djvu_new_callback *newArray_handler = 0;

static djvu_free_callback *_djvu_free_handler = 0;
static djvu_realloc_callback *_djvu_realloc_handler = 0;
static djvu_calloc_callback *_djvu_calloc_handler = 0;
static djvu_malloc_callback *_djvu_malloc_handler = 0;

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

int 
djvu_set_memory_callbacks
(
  djvu_free_callback *free_handler,
  djvu_realloc_callback *realloc_handler,
  djvu_malloc_callback *malloc_handler,
  djvu_calloc_callback *calloc_handler
)
{
  if(free_handler && realloc_handler && malloc_handler)
  {
#ifdef UNIX
    _djvu_new_ptr=(djvu_new_callback *)&_djvu_new;
    _djvu_delete_ptr=(djvu_delete_callback *)&_djvu_delete;
#endif
    _djvu_new_handler=(djvu_new_callback *)malloc_handler;
    _djvu_delete_handler=(djvu_delete_callback *)free_handler;
    _djvu_malloc_handler=(djvu_malloc_callback *)malloc_handler;
    _djvu_free_handler=(djvu_free_callback *)free_handler;
    _djvu_realloc_handler=(djvu_realloc_callback *)realloc_handler;
    if(calloc_handler)
    {
      _djvu_calloc_handler=(djvu_calloc_callback *)&calloc_handler;
    }else
    {
      _djvu_calloc_handler=0;
    }
    return 1;
  }else
  {
#ifdef UNIX
    _djvu_new_ptr=(djvu_new_callback *)&(operator new);
    _djvu_delete_ptr=(djvu_delete_callback *)&(operator delete);
#endif
    _djvu_delete_handler=0;
    _djvu_new_handler=0;
    _djvu_malloc_handler=0;
    _djvu_free_handler=0;
    _djvu_realloc_handler=0;
    _djvu_calloc_handler=0;
    return !(_djvu_malloc_handler
      ||_djvu_free_handler
      ||_djvu_realloc_handler
      ||_djvu_calloc_handler);
  }
}

DJVUAPI void *
_djvu_new(size_t siz)
{
  void *ptr;
#ifndef UNIX
  if(_djvu_new_handler)
  {
#endif
    if(!(ptr=(*_djvu_new_handler)(siz?siz:1)))
    {
      G_THROW("DjVuGlobalMemory.exhausted	");
    }
#ifndef UNIX
  }else
  {
      ptr=::operator new(siz?siz:1);
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
    if(!(ptr=(*newArray_handler)(siz?siz:1)))
    {
      G_THROW("DjVuGlobalMemory.exhausted	");
    }
#ifndef UNIX
  }else
  {
      ptr=::new unsigned char[siz?siz:1];
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

void *
_djvu_malloc(size_t siz)
{
  return _djvu_malloc_handler?(*_djvu_malloc_handler)(siz?siz:1):malloc(siz?siz:1);
}

void *
_djvu_calloc(size_t siz, size_t items)
{
  void *ptr;
  if( _djvu_calloc_handler )
  {
    ptr = (*_djvu_calloc_handler)(siz?siz:1, items?items:1);
  }else if( _djvu_malloc_handler )
  {
    if((ptr = (*_djvu_malloc_handler)((siz?siz:1)*(items?items:1)))&&siz&&items)
    {
      memset(ptr,0,siz*items);
    }
  }else
  { 
    ptr = calloc(siz?siz:1, items?items:1);
  }
  return ptr;    
}

void *
_djvu_realloc(void* ptr, size_t siz)
{
  void *newptr;
  if( _djvu_realloc_handler )
  {
    newptr = (*_djvu_realloc_handler)(ptr, siz);
  }else
  {
    newptr = realloc(ptr, siz?siz:1);
  }
  return newptr;
}
 
void
_djvu_free(void *ptr)
{
  if(ptr)
  {
    if( _djvu_free_handler )
    {
      (*_djvu_free_handler)(ptr);
    }else
    {
      free(ptr);
    }
  }
}

#endif

