//C-  -*- C++ -*-
//C-
//C-  Copyright (c) 1988 AT&T	
//C-  All Rights Reserved 
//C-
//C-  THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
//C-  The copyright notice above does not evidence any
//C-  actual or intended publication of such source code.
//C-
//C-  $Id: DjVuGlobal.h,v 1.1.1.1 1999-01-22 00:40:19 leonb Exp $


#ifndef _DJVUGLOBAL_H
#define _DJVUGLOBAL_H

/** @name DjVuGlobal.h 

    This file is included by all include files in the DjVu reference
    library. It does nothing unless compilation symbols #DJVU_MEMORY# or
    #DJVU_NAMES# are defined.  These compilation options are useful when
    compiling the DjVu reference library as a shared library.

    {\bf Warning} --- This is not an include file for including everything in
    the DjVu library. It should not include other files unless this is
    absolutely necessary.
    @memo
    Global definitions.
    @version
    #$Id: DjVuGlobal.h,v 1.1.1.1 1999-01-22 00:40:19 leonb Exp $#
    @author
    Leon Bottou <leonb@research.att.com> -- initial (empty) file.  */
//@{

/** @name DjVu Memory.

    This section is enabled when compilation symbol #DJVU_MEMORY# is defined.
    It redefines the C++ memory allocation operators.  Some operating systems
    (e.g. Macintoshes) require very peculiar memory allocation in shared
    objects.  We redefine new and delete as #static inline# because we do not
    want to export the redefined versions to other libraries.

    {\bf Note} --- This is {\em unfinished}. Do not enable.  */
//@{
//@}

#ifdef DJVU_MEMORY
#include <stdlib.h> // for type size_t

static inline void *
operator new(size_t sz)
{
  return djvu_new(sz);
}

static inline void
operator delete(void *addr)
{
  return djvu_delete(addr);
}

static inline void *
operator new [] (size_t sz)
{
  return djvu_new(sz);
}

static inline void
operator delete [] (void *addr)
{
  return djvu_delete(addr);
}

#endif // DJVU_MEMORY


/** @name DjVu Names.  

    This section is enabled when compilation symbol #DJVU_NAMES# is defined.
    This section redefines class names in order to unclutter the name space of
    shared objects.  This is useful on systems which automatically export all
    global symbols when building a shared object.

    {\bf Note} --- This is {\em unfinished}. Do not enable.  */
//@{
//@}

#ifdef DJVU_NAMES

#define GException   DJVU_GException
#define GString      DJVU_GString
// etc.. most are missing..

#endif // DJVU_NAMES

//@}
#endif // _DJVUGLOBAL_H_
