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
//C- $Id: DjVuGlobal.h,v 1.13 1999-07-21 19:27:36 leonb Exp $


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
    #$Id: DjVuGlobal.h,v 1.13 1999-07-21 19:27:36 leonb Exp $#
    @author
    L\'eon Bottou <leonb@research.att.com> -- empty file.\\
    Bill Riemers <bcr@sanskrit.lz.att.com> -- real work.  */
//@{


typedef unsigned int	u_int32;
typedef unsigned short	u_int16;


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
    the calls to function \Ref{ByteStream::read} in class \Ref{ByteStream}.

    Calls to macro # DJVU_PROGRESS(tag,n)# should be inserted in strategic
    places in your processor intensive code.  Argument #tag# is a string
    describing the nature of the running code.  Argument #n# is an integer
    which may be used to differentiate the various invocations of the macro
    when it is called from a loop.  Successive integers #n# must always be in
    increasing order.

    Suppose you insert the following calls in 'main':
    \begin{verbatim}
    int
    main(int argc, char **argv)
    {
       _djvu_start_progress(0, "logfile");
       do_the_real_work(argc,argv);
       _djvu_end_progress();
    }
    \end{verbatim}
    Execution is going to create a file #"logfile"# tracing all invocations
    of macro #DJVU_PROGRESS#.  Each line of this file has the following format
    where #tag# and #n# are the arguments of DJVU_PROGRESS.
    \begin{verbatim}
      { "filename.cpp",  "tag", n } // time index in ms.
    \end{verbatim}
    
    The next step consists in selecting a few calls for the progress indicator
    and creating a #DjVuProgressScale# array.  Each record contains a global
    percentage indicator, and three fields matching well chosen records from
    the log file.  In the case of program \Ref{c44}, for instance, this array
    could be defined as follows:
    \begin{verbatim}
    static DjVuProgressScale scale[] = 
    { { 20, "IWImage.cpp",  "decomposition", 0 },
      { 40, "IWImage.cpp",  "decomposition", 0 },
      { 60, "IWImage.cpp",  "decomposition", 0 },
      { 70, "IWImage.cpp",  "slice", 62 },
      { 80, "IWImage.cpp",  "slice", 77 },
      { 90, "IWImage.cpp",  "slice", 87 },
      { 0 } }; // last element
    \end{verbatim}
    
    We now define a callback function:
    \begin{verbatim}
    static void progress_cb(int n) { fprintf(stderr,"%d%%\n", n); }
    \end{verbatim}
    and we cause function #main# to call instead the 
    following variant of #_djvu_start_progress#:
    \begin{verbatim}
    _djvu_start_progress(scale, progress_cb).
    \end{verbatim}

    The progress indicator function maintains a pointer to a current position
    in the scale array.  Every time #DJVU_PROGRESS# is called, the macro
    searches a match, starting at the current position in the scale array.
    The filename and the tag must match exactly.  Once filename and tag match,
    the search algorithm checks whether argument #n# passed to #DJVU_PROGRESS#
    is greater than the argument in the scale array.  If it is smaller, the
    macro returns immediatly without looking forward in the scale array.  If
    it is greater, the macro sets the current position to the next record, and
    calls the callback function with the global percentage as its single
    argument.

    Note that this procedures involves a search in the scale array.  Different
    input data may indeed lead to sliglthy different sequences of calls to
    #DJVU_PROGRESS#.  It would be a stupid idea however to use a scale array
    with more than a hundred entries.  The search is also limited by using the
    integer argument when #DJVU_PROGRESS# is called from a loop.  The value of
    #n# must indicate which fraction of the loop has been already executed.
    You should always make sure that macro #DJVU_PROGRESS# is called with a
    constant large integer argument when the loop terminates.  This last call
    advances the current position past all the entries pertaining to this loop
    in the scale array.  */
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
extern "C" void _djvu_progress(const char*, const char*, int);
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


