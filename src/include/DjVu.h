/*C-  -*- C -*-
 *C-
 *C- This software may only be used by you under license from AT&T
 *C- Corp. ("AT&T"). A copy of AT&T's Source Code Agreement is available at
 *C- AT&T's Internet web site having the URL <http://www.djvu.att.com/open>.
 *C- If you received this software without first entering into a license with
 *C- AT&T, you have an infringing copy of this software and cannot use it
 *C- without violating AT&T's intellectual property rights.
 *C-
 *C- $Id: DjVu.h,v 1.13 2000-07-05 16:50:53 bcr Exp $
 */

#ifndef _DJVU_GLOBAL_API_H
#define _DJVU_GLOBAL_API_H

/** @name DjVu.h

    @memo #DjVu.h# defines the API structures needed for specialized
    applications, like overloading memory allocations, monitoring
    compression progress, and manually parsing command line arguments.
    Most user programs should not need these functions.
    @author
    Bill C Riemers 
*/
/*@{*/

/*
 * $Log: DjVu.h,v $
 * Revision 1.13  2000-07-05 16:50:53  bcr
 * Updated the documentation.
 *
 * Revision 1.12  2000/07/04 00:51:49  mrosen
 * updated documentation
 *
 * Revision 1.11  2000/03/10 14:57:35  haffner
 * Typos + cannot use "default" as a variable name!
 *
 * Revision 1.10  2000/03/08 22:59:46  bcr
 * Updated the documentation.  I'm using Leon's libdjvu++ documentation
 * as a template.
 *
 */

/* Predeclarations. (should go here) */

 
#include <stdlib.h>

#ifdef __cplusplus
extern "C"
{
#ifndef __cplusplus
};
#endif
#endif /* __cplusplus */

/** @name Memory Callbacks

    @memo #Memory Callbacks# are intended primarily for support on platforms
    when special code is required to allocate large blocks of memory.

    In the past we have observed small improvements in compression speed by
    enabling these callbacks.  However, the callbacks have the nasty side
    effects when trying to run a debugger or a code analyzer.  For that
    reason, the Unix version of the API is normally compiled without these
    hooks enabled.  In order to enable memory callbacks, both your code
    and the API functions must be compiled with NEED_DJVU_MEMORY defined.
*/
/*@{*/
/** ++ #djvu_free_callback# is for overloading free() and operator delete() */
typedef void djvu_free_callback (void *);
/** ++ #djvu_realloc_callback# is for overloading realloc() */
typedef void *djvu_realloc_callback (void *, size_t);
/** ++ #djvu_malloc_callback# is for overloading malloc() */
typedef void *djvu_malloc_callback (size_t);
/** ++ #djvu_calloc_callback# is for overloading calloc() */
typedef void *djvu_calloc_callback (size_t,size_t);

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


#ifdef NEED_DJVU_MEMORY
DJVUAPI
#if 0
;
#endif
/** ++ #djvu_set_memory_callbacks# allows the overloading of memory 
  functions, if and only if NEED_DJVU_MEMORY is defined at compile
  time for the user code, and the API libraries.  This interface is
  intended primarily for MacOS, and other platforms that require 
  the application program to allocate and deallocate all memory.

  These callbacks are not enabled in ALPHA and BETA builds under Unix.
*/
int djvu_set_memory_callbacks(
           djvu_free_callback *,
           djvu_realloc_callback *,
           djvu_malloc_callback *,
           djvu_calloc_callback *);

/* Replacement functions. */
DJVUAPI void * _djvu_new(size_t);
DJVUAPI void * _djvu_newArray(size_t);
DJVUAPI void _djvu_delete(void *);
DJVUAPI void _djvu_deleteArray(void *);

/* We also need classic memory functions, for routines that need realloc. */
DJVUAPI void *_djvu_malloc(size_t);
DJVUAPI void *_djvu_calloc(size_t, size_t);
DJVUAPI void *_djvu_realloc(void*, size_t);
DJVUAPI void _djvu_free(void*);

#else

#ifndef _djvu_free
#define _djvu_free(ptr) free((ptr))
#endif
#ifndef _djvu_malloc
#define _djvu_malloc(siz) malloc((siz))
#endif
#ifndef _djvu_realloc
#define _djvu_realloc(ptr,siz) realloc((ptr),(siz))
#endif
#ifndef _djvu_calloc
#define _djvu_calloc(siz,items) calloc((siz),(items))
#endif

#endif /* NEED_DJVU_MEMORY */
/*@}*/

/** @name Progress Callbacks

    @memo #Progress Callbacks# are intended for monitoring compression
    progress.

    Progress Callbacks, can be used to monitor a single threaded process
    progression through the stages of compression.  For the Bitonal and
    Photo API's, we assume compression is normally very fast, so only
    the low level library calls have progress callbacks enabled.  This is
    sufficient for printing a '*', or some other indicator that the process
    is still alive.  For Document Encoding, we have tuned the progress
    callbacks to return an estimate of how much of the task has been
    completed fairly accurately.

    The progress callback should be a function of the form:
    \begin{verbatim}
    typedef void
    djvu_progress_callback (const char *task,unsigned long,unsigned long);
    \end{verbatim}
    The first unsigned long is how much time has spent compressing, and the
    second is an estimate how much time is required.  The task string indicates
    what part of the compression the encoder is working on.
*/
/*@{*/
typedef void
djvu_progress_callback (const char *task,unsigned long,unsigned long);

DJVUAPI
#if 0
;
#endif
/** ++ #djvu_set_progress_callback# sets the function that will be called
  with progress updates.  A value of 0 may be used if you nolonger wish to
  use progress callbacks.  The return value is the address of the function
  which was previously handling the progress callbacks.
*/
djvu_progress_callback *
djvu_set_progress_callback(djvu_progress_callback *);

/*@}*/

/** @name DjVu Parse Options

    @memo All DjVu(tm) SDK utilities use the same options parsing code
    to provide uniform support of command line arguments and profile
    variables.

    The #DjVu Parse Options# functions and structures are modeled after
    the standard unix getopt_long(3) library function.  However, we have
    made several very significant modifications to the options parsing.
    \begin{itemize}
    \item All options may be specified in profiles instead of the command
    line.
    \item We provide the \Ref{djvu_parse} structure that allows the passing of
    pre-parsed arguments, instead of just argv[] arrays.
    \item Error messages are placed on a stack, so they can be processed
    later, treated as warnings, or discarded.
    \end{itemize}
    Before using the #DjVu Parse Options# structures and functions, it
    is best to being by reading the manual page
    \URL[djvuprofile]{../SDKTools/djvuprofile.html}.
*/
/*@{*/
/** The #djvu_option# structure is very similar to the standard unix
    long_options structure for getopt_long(3), and the usage is almost
    identical.
    @memo The djvu_option structure wraps the DjVuParseOptions class 
  */
struct djvu_option
{
  /** #name# contains the long name without the leading double dash */
  const char *name;
  /** #has_arg# is 1 if the option has a value, and 2 if the value
      is optional.  Otherwise has_arg should be 0. */
  int has_arg;
  /** #flag# reserved for future use. */
  int *flag;
  /** #val# contains a single charactor represing the short option value.
      Use a '-' if there is no short option equivalent.
  */
  int val;
};

/** The \Ref{DjVuParseOptions} class from the DjVu(tm) Reference Library
    is wrapped with the #djvu_parse# structure.
*/
struct djvu_parse
{
  /** This is a pointer to a \Ref{DjVuParseOptions} object */
  void *Private;
};

DJVUAPI
#if 0
;
#endif
/** ++ #djvu_parse_init# constructs a \Ref{djvu_parse} structure using the
  profile name specified as the initial current profile.  No error will be
  reported if that profile does not exist.
*/
struct djvu_parse
djvu_parse_init(const char []);

DJVUAPI
#if 0
;
#endif
/** ++ #djvu_parse_config# constructs a \Ref{djvu_parse} structure using the
  configuration file, and profile name specified.  No error will be reported
  if the file or the profile does not exist.
*/
struct djvu_parse
djvu_parse_config(const char config[],const char profile[]);

DJVUAPI
#if 0
;
#endif
/** ++ #djvu_parse_copy# copies the \Ref{djvu_parse} object with all profiles
  that have been parsed, without copying the argv[] vectors that have been
  parsed.  This is useful when you don't want to read all the profiles again,
  but you need to parse a different set of arguments. 
*/
struct djvu_parse
djvu_parse_copy(const struct djvu_parse);

DJVUAPI
#if 0
;
#endif
/** ++ #djvu_parse_change_profile# will switch the profile being used as
  the current profile.  A non-zero return status indicates the specified
  profile was not found.
*/
int
djvu_parse_change_profile(struct djvu_parse,const char []);

DJVUAPI
#if 0
;
#endif
/** ++ #djvu_parse_free# is neccissary to deallocate the memory used by
  the \Ref{djvu_parse} structure.
*/
void
djvu_parse_free(struct djvu_parse);

DJVUAPI
#if 0
;
#endif
/** ++ #djvu_parse_value# will return the value of the specified configuration
  or command line argument (without the leading "--").  A NULL return value
  indicates the specified variable has not been defined. 
*/
const char *
djvu_parse_value(struct djvu_parse,const char []);

DJVUAPI
#if 0
;
#endif
/** ++ #djvu_parse_integer# will call \Ref{djvu_parse_value} and perform an
  atoi() conversion.  Any string beginning with the letter 't' (or 'T') will
  be returned as 1, and any string beginning with the letter 'f' (or 'F') 
  will be returned as 0.  Any other non-numeric string will be returned with
  the value specified as default.
*/
int
djvu_parse_integer(struct djvu_parse,const char name[],
                   const int default_val);

DJVUAPI
#if 0
;
#endif
/** ++ #djvu_parse_number# will call \Ref{djvu_parse_value} and perform an
  atoi() conversion.  The same parsing rules as #djvu_parse_integer# will
  be used.  However, any non-numeric string will also add an error message
  to the stack. 
*/
int
djvu_parse_number(struct djvu_parse,const char [],const int);

DJVUAPI
#if 0
;
#endif
/** ++ #djvu_parse_arguments# will add the values of the arguments in the
  argv[] array.  The \Ref{djvu_option} structure is used to specify which
  arguments are legal, and whether each argument has a value.  You may
  call #djvu_parse_arguments# multiple times with different argv[] arrays.
  The return value indicates the first element in the argv[] array that
  was not a recognized option.
*/
int
djvu_parse_arguments(
  struct djvu_parse,int,const char * const *argv,const struct djvu_option []);

DJVUAPI
#if 0
;
#endif
/** ++ #djvu_parse_haserror# returns non-zero if there are error messages on
  the stack.
*/
int
djvu_parse_haserror(struct djvu_parse);

DJVUAPI
#if 0
;
#endif
/** ++ #djvu_parse_error# pops the top error message from the stack. */
const char *
djvu_parse_error(struct djvu_parse);

DJVUAPI
#if 0
;
#endif
/** ++ #djvu_parse_perror# removes all error messages from the stack and
  prints them. */
void
djvu_parse_perror(struct djvu_parse,const char *mesg);

DJVUAPI
#if 0
;
#endif
/** ++ #djvu_parse_configfile# is useful for finding what configuration file
  would be open.  Level should be 0 for the global configuration directory,
  1 for the user configuration file directory.  -1 has the special meaning
  of just return the path and don't actually search for a file name. */
const char *
djvu_parse_configfile(struct djvu_parse,const char[],int level);

/*@}*/

#ifdef __cplusplus
#ifndef __cplusplus
{
#endif
};
#endif /* __cplusplus */

/*@}*/

#endif /* __DJVU_GLOBAL_API_H__ */


