/*C-  -*- C -*-
 *C-
 *C- This software may only be used by you under license from AT&T
 *C- Corp. ("AT&T"). A copy of AT&T's Source Code Agreement is available at
 *C- AT&T's Internet web site having the URL <http://www.djvu.att.com/open>.
 *C- If you received this software without first entering into a license with
 *C- AT&T, you have an infringing copy of this software and cannot use it
 *C- without violating AT&T's intellectual property rights.
 *C-
 *C- $Id: DjVuMultiAPI.h,v 1.18 2000-03-03 00:48:35 bcr Exp $
 */


#ifndef __DJVUMULTIAPI_H__
#define __DJVUMULTIAPI_H__

/** @name DjVuMultiAPI.h

    File #"DjVuMultiAPI.h"# contains the API for assembling multi page
    DjVu documents, from single page, and other multi page documents.
    This API does not contain any functions for encoding, rendering, or
    decoding.
    @author
    Bill C Riemers <bcr@att.com>
*/

/* 
 * $Log: DjVuMultiAPI.h,v $
 * Revision 1.18  2000-03-03 00:48:35  bcr
 * Be less restrictive with GString's.
 *
 * Revision 1.17  2000/02/26 18:53:20  bcr
 * Changes to the DOC++ comments.
 *
 */


#ifdef DOCXX_CODE
//@{
#endif

/*
 *  ------------------------------------------------------------------------
 * DYNAMIC LINK LIBRARY STUFF
 */

#ifndef DJVUAPI
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

#ifdef BUILD_LIB
#define DJVUAPI DLLEXPORT
#else
#define DJVUAPI DLLIMPORT
#endif  /*BUILD_LIB*/
#endif  /*DJVUAPI*/

#include "DjVu.h"

#ifdef __cplusplus
extern "C" {  
#ifndef __cplusplus
           } ;  // makes doc++ look nicer
#endif
#endif


/** @name djvu_combine_options structure

    @memo Option structure for the MultiPage API.

    This structure defines the options used by both \Ref{djvu_joinby both the \Ref{djvu_combine}.
    Each field somehow corresponds to one of the command line options of the
    corresponding utility program(s).
 */
struct djvu_combine_options_struct
{
  /** Size of the option structure.
      This field should be set to the value #sizeof(djvu_combine_options)# */
  size_t size;
  /** Combination type flag.
      When this flag is set to #1#, function \Ref{djvu_combine} generates
      a #BUNDLED# DjVu file (a single file containing all the pages).
      When this flag is set to #0#, function \Ref{djvu_combine} generates
      an #INDIRECT# DjVu file (an index file pointing to secondary files containing 
      each page). */
  int bundle;
  /** Recover level. 
      This option specifies what should happen when errors are encountered
      while assembling the multipage file.  Four options are provided upon 
      encountering an error within a particular chunk of a particular page.
      \begin{description}
      \item[0] Level 0 aborts the complete operation.
      \item[1] Level 1 skips the erroneous page.
      \item[2] Level 2 skips the erroneous chunk and all remaining chunk for this page.
      \item[3] Level 3 just copies the erroneous data into the output file.
      \end{description}
  */
  int recover_level;
  /** Number of files to assemble.  This positive integer indicates the number
      of elements in #filelist#. */
  int filecount;
  /** Help output destination.  When this integer is non zero, function
      \Ref{djvu_combine} outputs usage instructions for programs
      \Ref{djvujoin} and \Ref{djvubundle}.  Usage instructions are output to
      the file descriptor (as returned by the system function #open#)
      contained in #helpfileno#.  File descriptor #1# always corresponds to
      the standard output.  File descriptor #2# always corresponds to the
      standard error. */
  int helpfileno;
  /** Log output destination.  When this integer is non zero, function
      \Ref{djvu_combine} outputs various messages describing the successive
      operations.  Messages are output to the file descriptor (as returned by
      the system function #open#) contained in #helpfileno#.  File descriptor
      #1# always corresponds to the standard output.  File descriptor #2#
      always corresponds to the standard output. */
  int logfileno;
  /** Name of the calling program.
      This name is used for reporting errors. */
  const char *prog;
  /** Output file.
      This is the name of the output file, 
      which will either contain a bundled DjVu document or
      the index of an indirect DjVu document. */
  const char *output;
  /** List of input files.
      Field #filelist# is a pointer to an array of C strings
      representing the filenames of all the input files. */
  const char * const *filelist;
  /** Private data.
      This field is used internally to hold the C++ peer of this data structure.
      Function \Ref{djvu_combine_options_alloc} properly initializes this field.  
      Do not modify it. */
  void *priv;
};
typedef struct djvu_combine_options_struct djvu_combine_options;

/** @name DjVuMultiAPI C function calls
 */
/*@{*/
/** Allocates an instance of #djvu_combine_options Structure#.
    This function allocates a \Ref{djvu_combine_options} data structure
    and initializes its fields according to the command line arguments
    passed via arguments #argc# and #argv#.  Argument #reserved# should
    be set to zero.  You can then further modify the options at your convenience. */
DJVUAPI djvu_combine_options *
djvu_combine_options_alloc(struct djvu_parse *reserved, int argc, const char * const argv[]);

/** Frees an instance of #djvu_combine_options#.
    This function also releases the memory used by the internal data structures
    pointed by the field #priv# of #djvu_combine_options#. */
DJVUAPI void 
djvu_combine_options_free(djvu_combine_options *);

/** Assembles a multipage file.
    This function assembles a multipage file according to the
    specifications stored in #options#. A non-zero return value indicates a fatal error. */
DJVUAPI int 
djvu_combine(djvu_combine_options *options);

/** Tests for error messages.  After executing \Ref{djvu_combine}, the
    #options# data structure may contain a list of error messages.  This
    function tests whether there are error messages available. */
DJVUAPI int 
djvu_combine_haserror(const djvu_combine_options *options);

/** Returns an error message.  After executing \Ref{djvu_combine}, the
    #options# data structure may contain a list of error messages.  
    This function returns the first error message on the stack.  The message is then
    removed in such a way that successive calls of this function
    returns a successive error messages.  This function returns #0# when
    no error messages are available. */
DJVUAPI const char * 
djvu_combine_error(djvu_combine_options *options);

/** Prints all the errors to #stderr#.  
    This convenience function prints all
    error messages to #stderr# together with message #mesg#.  */
DJVUAPI void 
djvu_combine_perror(djvu_combine_options *options, const char *mesg);

/** Alias for #djvu_combine# for implementing program #djvubundle#.
    This function is similar to \Ref{djvu_combine}
    but first sets the flag #bundle# to #1#. */
DJVUAPI int djvu_bundle(djvu_combine_options*);

/** Prints usage instructions for program #djvubundle#. 
    Output is sent to file descriptor #fd#.  Occurrences of
    program name #djvubundle# are replaced by the user supplied
    program name #prog#. */
DJVUAPI void djvu_bundle_usage(int fd,const char *prog);

/** Alias for #djvu_combine# for implementing program #djvujoin#.
    This function is similar to \Ref{djvu_combine}
    but first sets the flag #bundle# to #1#. */
DJVUAPI int djvu_join(djvu_combine_options*);

/** Prints usage instructions for program #djvujoin#. 
    Output is sent to file descriptor #fd#.  Occurrences of
    program name #djvubundle# are replaced by the user supplied
    program name #prog#. */
DJVUAPI void djvu_join_usage(int fd,const char *prog);

/*@}*/

#ifdef DOCXX_CODE
//@}
#endif

#ifdef __cplusplus
#ifndef __cplusplus
{
#endif
}
#endif

#endif
