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
//C- $Id: DjVuGlobal.h,v 1.15 1999-09-21 20:51:26 leonb Exp $


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
    #$Id: DjVuGlobal.h,v 1.15 1999-09-21 20:51:26 leonb Exp $#
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

    {\bf Code tracing macros} ---
    Monitoring the progress of such complex algorithms requires significant
    code support.  This is achieved by inserting {\em code tracing macros}
    in strategic regions of the code.  Tagged trace events are generated 
    whenever the execution flow reaches these macros.

    Macro #DJVU_PROGRESS(tag)# generates a such tagged event when it gets
    executed. The event is said to remain ``active'' until the control flow
    leaves the C++ scope containing the tag macro. The argument #tag# may be a
    string (describing the currently running task) or an integer (describing a
    percentage of completion for the task).  The actual tag of an event is
    formed by concatenating the tags of all active events separated by dots.

    {\bf Saving the trace} ---
    Suppose you insert the following calls in 'main':
    \begin{verbatim}
    int main(int argc, char **argv)
    {
       DjVuProgress::start("logfile");
       do_the_real_work(argc,argv);
       DjVuProgress::end();
    }
    \end{verbatim}
    Execution is going to create a file #"logfile"# recording all trace events.
    Each line of this file contains a time stamp (in milliseconds) and the event
    tag (composed by concatenating the tags of all active events), as in the
    following example:
    \begin{verbatim}
    {     11, "mbwdjvu" },
    {     23, "mbwdjvu.jb2image" },
    {     54, "mbwdjvu.jb2image.15" },
    {     69, "mbwdjvu.jb2image.31" },
    {     79, "mbwdjvu.jb2image.46" },
    {     90, "mbwdjvu.jb2image.62" },
    {    103, "mbwdjvu.jb2image.77" },
    {    114, "mbwdjvu.jb2image.93" },
    \end{verbatim}

    {\bf Selecting checkpoints} --- The next step consists of selecting a few
    events to be used as checkpoints for ascertaining the progress of the
    algorithm.  There is no systematic way to do this since the sequence of
    events depend a lot on the program inputs.  Use good judgment to select
    one to four dozen events should be enough for all purposes.

    The checkpoints should be gathered in an array of #CheckPoint# records.
    Each record contains a #tag#, an integer #userdata# that you may use as
    you wish, and a flag #passed# that must be initialized to zero, as in the
    following example: 
    \begin{verbatim} 
    static DjVuProgress::CheckPoint checkpoints[] = 
    { { "mbwdjvu.jb2image.10", 10, 0 },
      { "mbwdjvu.jb2image.30", 30, 0 },
      { "mbwdjvu.jb2image.50", 50, 0 },
      { "mbwdjvu.jb2image.70", 70, 0 },
      { "mbwdjvu.jb2image.90", 90, 0 },
      { 0 } };
    \end{verbatim}.

    Flag #passed# is set by the library when an event tag ``matches'' the
    checkpoint tag.  A match means that both tags have the same letters, and
    that numbers in the event tag are greater or equal to the numbers in the
    checkpoint tag.

    {\bf Tracking checkpoints} --- You can now initialize the progress
    indicator system using another variant of #DjVuProgress::start#.  This
    function takes a checkpoint array and a callback function which is called
    whenever a checkpoint is passed.  This callback function can test which
    checkpoints are passed, can access the #userdata# field as appropriate,
    and should use appropriate logic in order to display a useful progress
    indication.
    \begin{verbatim}
    static void callback(int chk) 
    { // A minimal callback
       printf("%d%%\n", checkpoints[chk].userdata); 
    }
     int main(int argc, char **argv)
    {  // A main routine
       DjVuProgress::start(checkpoints, callback);
       do_the_real_work(argc,argv);
       DjVuProgress::end();
    }
    \end{verbatim}
    The callback function can of course become much more complex when the
    program has many modes of operation resulting in many different sequences
    of events.  Large checkpoint arrays can hurt the pewrformance.  It is
    possible however to call #DjVuProgress::start# and redefine the checkpoint
    array within the callback function. The resulting system is a state
    machine mimicking the large features of the control flow. */
//@{
//@}

#ifdef NEED_DJVU_PROGRESS

#define DJVU_PROGRESS(tag) DjVuProgress::Event _event_(tag)

class DjVuProgress 
{
public:
  struct CheckPoint { const char *tag; int userdata; int passed; };
  typedef void Callback(int checkpoint);
  static void start(const char *filename);
  static void start(CheckPoint *checkpoints, Callback *callback);
  static void end();
public:
  class Event { 
  public:
    Event(const char *tag);
    Event(int tag);
    ~Event();
  private:
    void enter(const char *tag);
    int n;
  };
private:
  friend class Event;
  static CheckPoint *chk;
  static Callback *cb;
  static unsigned long base;
  static void *log;
  static int taglen;
  static int tagmax;
  static char *tagbuf;
};

#else  // ! NEED_DJVU_PROGRESS

#ifndef DJVU_PROGRESS
#define DJVU_PROGRESS(tag) /**/
#endif

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


