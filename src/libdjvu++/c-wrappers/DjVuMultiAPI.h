#ifndef __DJVUMULTIAPI_H__
#define __DJVUMULTIAPI_H__
/** @name DjVuMultiAPI.h
    This is the main interface definition file
    @author
    Praveen K Guduru <guduru@att.com>
    @version
    
*/

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

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif


struct djvu_combine_options
{
  /** This should be set to the value sizeof(djvu_combine_options) */
  size_t size;
  /** This flag indicates if the document should be bundled into a
      single file. */
  int bundle;
  /** Recover levels 0 - 3 are defined.  0 Aborts on all errors, while
      3 tries to recover from everything. */
  int recover_level;
  /** This should tell us the number of elements in the filelist */
  int filecount;
  /** If set, the save() command will write usage instructions to this stream */
  FILE *helpfile;
  /** If set, messages will be logged to this stream */
  FILE *logfile;
  /** This is the name of the calling program */
  const char *prog;
  /** This is the name of the output index or bundled file */
  const char *output;
  /** This is the filelist */
  const char * const *filelist;
  /** This is a C++ class for storing allocated memory and error messages.
      If you declare this structure manually, set this to 0. */
  void *priv;
};

/** Call the options structure djvu_join_options when 
    using it to create indirect index files. */
typedef struct djvu_combine_options djvu_join_options;

/** Call the options structure djvu_bundle_options when 
    using it to create a bundled file. */
typedef struct djvu_combine_options djvu_bundle_options;

struct djvu_parse;

/** This is the primary allocation routine for djvu_bundle_options.
    Even if the values specified are illegal, an options structure
    will be returned. */
DJVUAPI 
djvu_bundle_options *
djvu_bundle_options_alloc(struct djvu_parse *,int,const char * const argv[]);

/** You should always use the free option, even if you did not use alloc
    so the data pointed to by priv is freed. */
DJVUAPI 
void djvu_bundle_options_free(djvu_bundle_options *);

/** This will process the options and create the document.
    A non-zero return value indicates a fatal error. */
DJVUAPI 
int djvu_bundle(djvu_bundle_options[]);

/** A non-zero value indicates there are error messages.  Error
    messages are generated for both fatal errors, and errors
    that are recovered from.  */
DJVUAPI
int djvu_bundle_haserror(const djvu_bundle_options []);

/** Returns a string of the first error message on the stack.  Each
    call erases the previous return value. */
DJVUAPI
const char * djvu_bundle_error(djvu_bundle_options []);

/** Prints all the errors to stderr */
DJVUAPI
void djvu_bundle_perror(djvu_bundle_options []);

/** This will print usage instructions to the specified output. */
DJVUAPI
void djvu_bundle_usage(FILE *out,const char *prog);

/** This is the primary allocation routine for djvu_join_options.
    Even if the values specified are illegal, an options structure
    will be returned. */
DJVUAPI
djvu_join_options *
djvu_join_options_alloc(struct djvu_parse *,int,const char * const argv[]);

/** You should always use the free option, even if you did not use alloc
    so the data pointed to by priv is freed. */
DJVUAPI
void djvu_join_options_free(djvu_join_options *);

/** This will process the options and create the document.
    A non-zero return value indicates a fatal error. */
DJVUAPI
int djvu_join(djvu_join_options[]);

/** A non-zero value indicates there are error messages.  Error
    messages are generated for both fatal errors, and errors
    that are recovered from.  */
DJVUAPI
int djvu_join_haserror(const djvu_bundle_options []);

/** Returns a string of the first error message on the stack.  Each
    call erases the previous return value. */
DJVUAPI
const char * djvu_join_error(djvu_join_options []);

/** Prints all the errors to stderr */
DJVUAPI
void djvu_join_perror(djvu_join_options []);



/** This will print usage instructions to the specified output. */
DJVUAPI
void djvu_join_usage(FILE *out,const char *prog);

#ifdef __cplusplus
}
#endif

#endif
