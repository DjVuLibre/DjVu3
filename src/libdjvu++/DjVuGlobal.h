//C-  -*- C++ -*-
//C- DjVu� Reference Library (v. 3.0)
//C- 
//C- Copyright � 1999-2001 LizardTech, Inc. All Rights Reserved.
//C- The DjVu Reference Library is protected by U.S. Pat. No.
//C- 6,058,214 and patents pending.
//C- 
//C- This software is subject to, and may be distributed under, the
//C- GNU General Public License, Version 2. The license should have
//C- accompanied the software or you may obtain a copy of the license
//C- from the Free Software Foundation at http://www.fsf.org .
//C- 
//C- The computer code originally released by LizardTech under this
//C- license and unmodified by other parties is deemed the "LizardTech
//C- Original Code."
//C- 
//C- With respect to the LizardTech Original Code ONLY, and subject
//C- to any third party intellectual property claims, LizardTech
//C- grants recipient a worldwide, royalty-free, non-exclusive license
//C- under patent claims now or hereafter owned or controlled by
//C- LizardTech that are infringed by making, using, or selling
//C- LizardTech Original Code, but solely to the extent that any such
//C- patent(s) is/are reasonably necessary to enable you to make, have
//C- made, practice, sell, or otherwise dispose of LizardTech Original
//C- Code (or portions thereof) and not to any greater extent that may
//C- be necessary to utilize further modifications or combinations.
//C- 
//C- The LizardTech Original Code is provided "AS IS" WITHOUT WARRANTY
//C- OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED
//C- TO ANY WARRANTY OF NON-INFRINGEMENT, OR ANY IMPLIED WARRANTY OF
//C- MERCHANTIBILITY OR FITNESS FOR A PARTICULAR PURPOSE.
// 
// $Id: DjVuGlobal.h,v 1.48 2001-04-06 18:59:35 bcr Exp $
// $Name:  $

#ifndef _DJVUGLOBAL_H
#define _DJVUGLOBAL_H

//Placement new support.
//There is no new.h for WinCE, so we define a placement new here.
// (lifted from wcealt.h which can not be included in the library header files
//	but which must be included with an ActiveX control based on MFC)
#ifdef UNDER_CE
	#ifndef __WCEALT_H__	// Placement new also defined in wcealt.h
		inline void * operator new(size_t, void * ptr) 
		{ 
			return ptr; 
		}
	#endif
#else	// Non-CE platforms support this in a sensible fashion.
	#include <new.h>
#endif


#ifndef DJVU_STATIC_LIBRARY
#ifdef WIN32 
#define DLLIMPORT __declspec(dllimport)
#define DLLEXPORT __declspec(dllexport)
#else
#define DLLIMPORT /**/
#define DLLEXPORT /**/
#endif
#else /* DJVU_STATIC_LIBRARY */
#define DLLIMPORT /**/
#define DLLEXPORT /**/
#endif /* DJVU_STATIC_LIBRARY */


/** @name DjVuGlobal.h 

    This file is included by all include files in the DjVu reference library.
    
	If compilation symbols #NEED_DJVU_MEMORY#, #NEED_DJVU_PROGRESS# 
	or #NEED_DJVU_NAMES# are defined, this file enables 
	features which are useful for certain applications of the
    DjVu Reference Library.  These features are still experimental and
    therefore poorly documented.
    
    @memo
    Global definitions.
    @version
    #$Id: DjVuGlobal.h,v 1.48 2001-04-06 18:59:35 bcr Exp $#
    @author
    L\'eon Bottou <leonb@research.att.com> -- empty file.\\
    Bill Riemers <bcr@lizardtech.com> -- real work.  */
//@{


/** @name DjVu Memory 

    This section is enabled when compilation symbol #NEED_DJVU_MEMORY# is
    defined.  Function #_djvu_memory_callback# can be used to redefine the C++
    memory allocation operators.  Some operating systems (e.g. Macintoshes)
    require very peculiar memory allocation in shared objects.  We redefine
    the operators #new# and #delete# as #STATIC_INLINE# because we do not
    want to export these redefined versions to other libraries.  */
//@{
//@}

#ifdef NEED_DJVU_MEMORY

#include "DjVu.h"

// These define the two callbacks needed for C++
typedef void djvu_delete_callback(void *);
typedef void *djvu_new_callback(size_t);

// These functions allow users to set the callbacks.
int djvu_memoryObject_callback ( djvu_delete_callback*, djvu_new_callback*);
int djvu_memoryArray_callback ( djvu_delete_callback*, djvu_new_callback*);

// We need to use this inline function in all modules, but we never want it to
// appear in the symbol table.  It seems different compilers need different
// directives to do this...
#ifndef STATIC_INLINE
#ifdef __GNUC__
#define STATIC_INLINE extern inline
#else /* !__GNUC__ */
#define STATIC_INLINE static inline
#endif /* __GNUC__ */
#endif /* STATIC_INLINE */

// This clause is used when overriding operator new
// because the standard has slightly changed.
#if defined( __GNUC__ ) && ( __GNUC__*1000 + __GNUC_MINOR__ >= 2091 )
#ifndef new_throw_spec
#define new_throw_spec throw(std::bad_alloc)
#endif /* new_throw_spec */
#ifndef delete_throw_spec
#define delete_throw_spec throw()
#endif /* delete_throw_spec */
#endif /* __GNUC__ ... */
// Old style
#ifndef new_throw_spec
#define new_throw_spec
#endif /* new_throw_spec */
#ifndef delete_throw_spec
#define delete_throw_spec
#endif  /* delete_throw_spec */

#ifdef UNIX
extern djvu_new_callback *_djvu_new_ptr;
extern djvu_new_callback *_djvu_newArray_ptr;
extern djvu_delete_callback *_djvu_delete_ptr;
extern djvu_delete_callback *_djvu_deleteArray_ptr;

#ifndef NEED_DJVU_MEMORY_IMPLEMENTATION
void *operator new (size_t) new_throw_spec;
void *operator new[] (size_t) new_throw_spec;
void operator delete (void *) delete_throw_spec;
void operator delete[] (void *) delete_throw_spec;

STATIC_INLINE void *
operator new(size_t sz) new_throw_spec
{ return (*_djvu_new_ptr)(sz); }
STATIC_INLINE void
operator delete(void *addr) delete_throw_spec
{ return (*_djvu_delete_ptr)(addr); }
STATIC_INLINE void *
operator new [] (size_t sz) new_throw_spec
{ return (*_djvu_newArray_ptr)(sz); }
STATIC_INLINE void
operator delete [] (void *addr) delete_throw_spec
{ return (*_djvu_deleteArray_ptr)(addr); }
#endif /* NEED_DJVU_MEMORY_IMPLEMENTATION */

#else /* UNIX */

#ifndef NEED_DJVU_MEMORY_IMPLEMENTATION
STATIC_INLINE void *
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
{ _djvu_deleteArray(addr); }
#endif /* !NEED_DJVU_MEMORY_IMPLEMENTATION */

#endif /* UNIX */

#else

#define _djvu_free(ptr) free((ptr))
#define _djvu_malloc(siz) malloc((siz))
#define _djvu_realloc(ptr,siz) realloc((ptr),(siz))
#define _djvu_calloc(siz,items) calloc((siz),(items))

#endif /* NEED_DJVU_MEMORY */

/** @name DjVu Progress  

    This section is enabled when compilation symbol #NEED_DJVU_PROGRESS# is
    defined.  This macro setups callback function that may be used to
    implement a progress indicator for the encoding routines.  The decoding
    routines do not need such a facility because it is sufficient to monitor
    the calls to function \Ref{ByteStream::read} in class \Ref{ByteStream}.
    
    {\bf Code tracing macros} ---
    Monitoring the progress of such complex algorithms requires significant
    code support.  This is achieved by inserting {\em code tracing macros}
    in strategic regions of the code.  
    \begin{description}
    \item[DJVU_PROGRESS_TASK(name,task,nsteps)]  indicates that the current
         scope performs a task roughly divided in #nsteps# equal steps, with
	 the specified #task# string used in the callback.
    \item[DJVU_PROGRESS_RUN(name,tostep)] indicates that we are starting
         an operation which will take us to step #tostep#.  The operation
         will be considered finished when #DJVU_PROGRESS_RUN# will be called
         again with an argument greater than #tostep#.  The execution of
         this operation of course can be described by one subtask and so on.
    \end{description}
 
    {\bf Progress callback} --- Before defining the outermost task, you can
    store a callback function pointer into the static member variable
    #DjVuProgressTask::callback#.  This callback function is called
    periodically with two unsigned long arguments.  The first argument is the
    elapsed time. The second argument is the estimated total execution time.
    Both times are given in milliseconds.

    {\bf Important Note} --- This monitoring mechanism should not be used by
    multithreaded programs.  */
//@{

#ifndef HAS_DJVU_PROGRESS_CALLBACKS
#define HAS_DJVU_PROGRESS_CALLBACKS

#ifdef NEED_DJVU_PROGRESS
#include "DjVu.h"

extern djvu_progress_callback *_djvu_progress_ptr;

#define DJVU_PROGRESS_TASK(name,task,nsteps)  DjVuProgressTask task_##name(task,nsteps)
#define DJVU_PROGRESS_RUN(name,tostep)   { task_##name.run(tostep); }

class DjVuProgressTask
{
public:
  class Data;
  ~DjVuProgressTask();
  DjVuProgressTask(const char *task,int nsteps);
  void run(int tostep);
  const char *task;
  static djvu_progress_callback *set_callback(djvu_progress_callback *ptr=0);
private:
  DjVuProgressTask *parent;
  int nsteps;
  int runtostep;
  unsigned long startdate;
  // Statics
  void *gdata;
  Data *data;
  // Helpers
  void signal(unsigned long curdate, unsigned long estdate);
};


#else  // ! NEED_DJVU_PROGRESS

#define DJVU_PROGRESS_TASK(name,task,nsteps)
#define DJVU_PROGRESS_RUN(name,step)

#endif // ! NEED_DJVU_PROGRESS
#endif // HAS_DJVU_PROGRESS_CALLBACKS
//@}


/** @name General functions.

    This section contains functions that replace some of the standard
    system calls without any other header file dependancies.
 */

#ifndef DJVUAPI
#if 0
class dummy /* This makes doc++ ignore this block */
{
  private:
#endif
#ifndef DJVU_STATIC_LIBRARY
#ifdef WIN32
#define DLLIMPORT __declspec(dllimport)
#define DLLEXPORT __declspec(dllexport)
#else
#define DLLIMPORT /* */
#define DLLEXPORT /* */
#endif
#else /* DJVU_STATIC_LIBRARY */
#define DLLIMPORT /* */
#define DLLEXPORT /* */
#endif /* DJVU_STATIC_LIBRARY */
#ifdef BUILD_LIB
#define DJVUAPI DLLEXPORT
#else
#define DJVUAPI DLLIMPORT
#endif  /*BUILD_LIB*/
#if 0
};
#endif
#endif /*DJVUAPI*/ 

#ifdef __cplusplus
#define DJVUEXTERNCAPI(x) extern "C" DJVUAPI x
#else
#define DJVUEXTERNCAPI(x) DJVUAPI x
#endif

/** This replaces fprintf(stderr,...), but with UTF8 encoded strings. */
DJVUEXTERNCAPI(void DjVuPrintError(const char *fmt, ...));

/** This replaces printf(...), but requires UTF8 encoded strings. */
DJVUEXTERNCAPI(void DjVuPrintMessage(const char *fmt, ...));

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

#if defined(macintosh)
#define EMPTY_LOOP continue
#else
#define EMPTY_LOOP /* nop */
#endif

#endif /* _DJVUGLOBAL_H_ */

