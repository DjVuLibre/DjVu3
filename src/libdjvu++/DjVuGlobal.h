//C-  -*- C++ -*-
//C-
//C-  Copyright (c) 1998 AT&T	
//C-  All Rights Reserved 
//C-
//C-  THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
//C-  The copyright notice above does not evidence any
//C-  actual or intended publication of such source code.
//C-
//C-  $Id: DjVuGlobal.h,v 1.7 1999-02-22 20:35:01 leonb Exp $


#ifndef _DJVUGLOBAL_H
#define _DJVUGLOBAL_H

/** @name DjVuGlobal.h 

    This file is included by all include files in the DjVu reference library.
    It does nothing unless compilation symbols #NEED_DJVU_MEMORY#,
    #NEED_DJVU_PROGRESS# or #NEED_DJVU_NAMES# are defined.  These compilation
    symbols enable features which are useful for certain applications of the
    DjVu Reference Library.  These features are still experimental and
    therefore poorly documented.
    
    @memo
    Global definitions.
    @version
    #$Id: DjVuGlobal.h,v 1.7 1999-02-22 20:35:01 leonb Exp $#
    @author
    Leon Bottou <leonb@research.att.com> -- empty file.\\
    Bill Riemers <bcr@sanskrit.lz.att.com> -- real work.  */
//@{





/** @name DjVu Memory 

    This section is enabled when compilation symbol #NEED_DJVU_MEMORY# is
    defined.  Function #_djvu_memory_callback# can be used to redefine the C++
    memory allocation operators.  Some operating systems (e.g. Macintoshes)
    require very peculiar memory allocation in shared objects.  We redefine
    the operators #new# and #delete# as #inline_as_macro# because we do not
    want to export these redefined versions to other libraries.  */
//@{
//@}

#ifdef NEED_DJVU_MEMORY
#include <new.h>

// Normally, this is the only functions we should need.
typedef void djvu_delete_callback(void *);
typedef void *djvu_new_callback(size_t);
void _djvu_memory_callback(djvu_delete_callback*, djvu_new_callback*);

#ifndef NEED_DJVU_MEMORY_IMPLEMENTATION
// We need to use this inline function in all modules, but we never want it to
// appear in the symbol table.  It seems different compilers need different
// directives to do this...
#ifndef inline_as_macro
#ifdef __GNUC__
#define inline_as_macro extern inline
#else
#define inline_as_macro inline
#endif
#endif
// This clause is used when overriding operator new
// because the standard has slightly changed.
#if defined( __GNUC__ ) && ( __GNUC__*1000 + __GNUC_MINOR__ >= 2091 )
#ifndef new_throw_spec
#define new_throw_spec throw(std::bad_alloc)
#endif
#ifndef delete_throw_spec
#define delete_throw_spec throw()
#endif
#endif
// Old style
#ifndef new_throw_spec
#define new_throw_spec
#endif
#ifndef delete_throw_spec
#define delete_throw_spec
#endif 
// Overrides
void *_djvu_new(size_t);
void  _djvu_delete(void *);
inline_as_macro void *
operator new(size_t sz) new_throw_spec
{ return _djvu_new(sz); }
inline_as_macro void
operator delete(void *addr) delete_throw_spec
{ return _djvu_delete(addr); }
inline_as_macro void *
operator new [] (size_t sz) new_throw_spec
{ return _djvu_new(sz); }
inline_as_macro void
operator delete [] (void *addr) delete_throw_spec
{ return _djvu_delete(addr); }
#endif // !NEED_DJVU_MEMORY_IMPLEMENTATION
#endif // NEED_DJVU_MEMORY




/** @name DjVu Progress  

    This section is enabled when compilation symbol #NEED_DJVU_PROGRESS# is
    defined.  This macro setups callback function that may be used to
    implement a progress indicator for the encoding routines.  The decoding
    routines do not need such a facility because it is sufficient to monitor
    the calls to function \Ref{ByteStream::read} in class \Ref{ByteStream}.  */
//@{
//@}

#ifdef NEED_DJVU_PROGRESS

// Normally, these are the only functions we should need.
struct DjVuProgressScale {
  int         percent;
  const char *match_filename;
  const char *match_tag;
  int         match_index;
};
typedef void djvu_progress_callback(int);
void _djvu_start_progress(DjVuProgressScale*, djvu_progress_callback*);
void _djvu_start_progress(DjVuProgressScale*, const char*);
void _djvu_end_progress();

// Implementation
void _djvu_progress(const char*, const char*, int);
#define DJVU_PROGRESS(tag,percent) _djvu_progress(__FILE__,tag,percent)
#else  // ! NEED_DJVU_PROGRESS
#define DJVU_PROGRESS(tag,percent) /**/
#endif // NEED_DJVU_PROGRESS





/** @name DjVu Names  

    This section is enabled when compilation symbol #NEED_DJVU_NAMES# is
    defined.  This section redefines class names in order to unclutter the
    name space of shared objects.  This is useful on systems which
    automatically export all global symbols when building a shared object.
    @args */
//@{
//@}

#ifdef NEED_DJVU_NAMES
/* The contents of this section may be generated by this shell command :
 * % egrep -h '^(class|struct) +[A-Z_][A-Za-z0-9_]*' *.h *.cpp |\
 *   sed -e 's:[a-z]*  *\([A-Za-z_][A-Za-z0-9_]*\).*:#define \1 DJVU_\1:g' |\
 *   sort
 */
#endif // NEED_DJVU_NAMES

//@}
#endif // _DJVUGLOBAL_H_

