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
//C- $Id: DjVuGlobalUnix.cpp,v 1.2 1999-12-08 20:56:43 bcr Exp $

/** These are just the external globals needed for the Unix version.
    We could put this in DjVuGlobalMemory.cpp, but then even programs
    that don't use the memory callbacks, would have to link in the 
    functions for overloading them. */

#ifdef UNIX
#ifdef NEED_DJVU_MEMORY
#ifndef NEED_DJVU_MEMORY_IMPLEMENTATION
#define NEED_DJVU_MEMORY_IMPLEMENTATION
#endif
#include "DjVuGlobal.h"

djvu_delete_callback *_djvu_delete_ptr=(djvu_delete_callback *)&(operator delete);
djvu_delete_callback *_djvu_deleteArray_ptr=(djvu_delete_callback *)&(operator delete []);
djvu_new_callback *_djvu_new_ptr=(djvu_new_callback *)&(operator new);
djvu_new_callback *_djvu_newArray_ptr=(djvu_new_callback *)&(operator new []);
#endif
#endif

