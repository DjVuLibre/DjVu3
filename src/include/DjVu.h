/*C- -*- C -*-
 *C-
 *C- DjVu® Reference Library (v. 3.5)
 *C- 
 *C- Copyright © 2000 LizardTech, Inc. All Rights Reserved.
 *C- 
 *C- This software (the "Original Code") is subject to, and may be
 *C- distributed under, the GNU General Public License, Version 2.
 *C- The license should have accompanied the Original Code or you
 *C- may obtain a copy of the license from the Free Software
 *C- Foundation at http://www.fsf.org .
 *C- 
 *C- With respect to the Original Code, and subject to any third
 *C- party intellectual property claims, LizardTech grants recipient
 *C- a worldwide, royalty-free, non-exclusive license under patent
 *C- claims infringed by making, using, or selling Original Code
 *C- which are now or hereafter owned or controlled by LizardTech,
 *C- but solely to the extent that any such patent is reasonably
 *C- necessary to enable you to make, have made, practice, sell, or 
 *C- otherwise dispose of Original Code (or portions thereof) and
 *C- not to any greater extent that may be necessary to utilize
 *C- further modifications or combinations.
 *C- 
 *C- The Original Code is provided "AS IS" WITHOUT WARRANTY OF ANY
 *C- KIND, EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO
 *C- ANY WARRANTY OF NON-INFRINGEMENT, OR ANY IMPLIED WARRANTY OF 
 *C- MERCHANTIBILITY OF FITNESS FOR A PARTICULAR PURPOSE.
 * 
 * $Id: DjVu.h,v 1.25 2001-10-16 18:01:43 docbill Exp $
 * $Name:  $
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
           djvu_free_callback    *free_handler,
           djvu_realloc_callback *realloc_handler,
           djvu_malloc_callback  *malloc_handler,
           djvu_calloc_callback  *calloc_handler);

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
    typedef int
    djvu_progress_callback (const char *task,unsigned long,unsigned long);
    \end{verbatim}
    The first unsigned long is how much time has spent compressing, and the
    second is an estimate how much time is required.  The task string indicates
    what part of the compression the encoder is working on. The return value
    should normally be zero. A non-zero return value causes DjVu to halt
    encoding (throws an exception).
*/
/*@{*/
typedef int
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
  /** #val# contains a single charactor representing the short option value.
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
djvu_parse_init(const char name[]);

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
djvu_parse_copy(const struct djvu_parse opts);

DJVUAPI
#if 0
;
#endif
/** ++ #djvu_parse_change_profile# will switch the profile being used as
  the current profile.  A non-zero return status indicates the specified
  profile was not found.
*/
int
djvu_parse_change_profile(struct djvu_parse opts,const char name[]);

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
djvu_parse_value(struct djvu_parse opts,const char name[]);

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
djvu_parse_integer(struct djvu_parse opts,
                   const char name[],
                   const int errval);

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
djvu_parse_number(struct djvu_parse opts,
                  const char name[],
                  const int errval);

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
  struct djvu_parse opts,
  int argc,
  const char * const *argv,
  const struct djvu_option *lopts);

DJVUAPI
#if 0
;
#endif
/** ++ #djvu_parse_haserror# returns non-zero if there are error messages on
  the stack.
*/
int
djvu_parse_haserror(struct djvu_parse opts);

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
  would be open. */
const char *
djvu_parse_configfile(struct djvu_parse opts,
                      const char name[]);


DJVUAPI
#if 0
;
#endif
/** This replaces fprintf(stderr,...), but requires UTF8 encoded strings. */
void DjVuPrintErrorUTF8(const char *fmt, ...);

DJVUAPI
#if 0
;
#endif
/** This is identical to fprintf(stderr,...). */
void DjVuPrintErrorNative(const char *fmt, ...);

DJVUAPI
#if 0
;
#endif
/** This replaces printf(...), but requires UTF8 encoded strings. */
void DjVuPrintMessageUTF8(const char *fmt, ...);

DJVUAPI
#if 0
;
#endif
/** This is identical to printf(...). */
void DjVuPrintMessageNative(const char *fmt, ...);

DJVUAPI
#if 0
;
#endif
/** This writes untranslated messages to a log file.  Intended for 
    logging errors from non-I18N third party libraries.   The strings
    are assumed to be in Native MBS format. */
void DjVuPrintLogMessage(const char *fmt, ...);

DJVUAPI
#if 0
;
#endif
/** ++ Set the file name for DjVu to place 3rd party library error
    messages. Messages are discarded if no filename is set.
 */
void
djvu_set_logfilename( const char *filename );

DJVUAPI
#if 0
;
#endif
/** The format (fmt) and arguments define a MessageList to be looked
    up in the external messages and printed to stderr.  Requires UTF8
    encoded strings. */
void DjVuFormatErrorUTF8(const char *fmt, ...);

DJVUAPI
#if 0
;
#endif
/** The format (fmt) and arguments define a MessageList to be looked
    up in the external messages and printed to stderr.  Requires Native
    encoded strings. */
void DjVuFormatErrorNative(const char *fmt, ...);

DJVUAPI
#if 0
;
#endif
/** Prints the translation of message to stdout. (Must be in UTF8)*/
void DjVuWriteMessage( const char *message );

/**
 A C function to perform a message lookup. Arguments are a buffer to
 receive the translated message, a buffer size (bytes), and a
 message_list. The translated result is returned in msg_buffer encoded
 in UTF8 MBS encoding. In case of error, msg_buffer is empty
 (i.e., msg_buffer[0] == '\0').
 */
DJVUAPI
#if 0
;
#endif
void DjVuMessageLookUpUTF8( char *msg_buffer,
                         const unsigned int buffer_size, 
                         const char *message ); 

/**
 A C function to perform a message lookup. Arguments are a buffer to
 receive the translated message, a buffer size (bytes), and a
 message_list. The translated result is returned in msg_buffer encoded
 in Native MBS encoding. In case of error, msg_buffer is empty
 (i.e., msg_buffer[0] == '\0').
 */
DJVUAPI
#if 0
;
#endif
void DjVuMessageLookUpNative( char *msg_buffer,
                         const unsigned int buffer_size, 
                         const char *message ); 

/**
 A C function to set the program name used when looking for language files.
 The argument is expected to be argv[0] from main().  The UTF8 translation of
 the program name is returned.
 */
DJVUAPI
#if 0
;
#endif
const char *
djvu_programname( const char *programname );

/*@}*/

/**
 Macro ERR_MSG is used by LizardTech to maintain the message files.
 It is used by an external auditing script and has no effect on the program. 
 */
#ifndef ERR_MSG
#define ERR_MSG(x) x
#endif

#ifdef __cplusplus
#ifndef __cplusplus
{
#endif
};
#endif /* __cplusplus */

/*@}*/

#endif /* __DJVU_GLOBAL_API_H__ */


