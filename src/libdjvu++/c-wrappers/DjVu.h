#ifdef __cplusplus
//C-  -*- C -*-
//C-
//C- This software may only be used by you under license from AT&T
//C- Corp. ("AT&T"). A copy of AT&T's Source Code Agreement is available at
//C- AT&T's Internet web site having the URL <http://www.djvu.att.com/open>.
//C- If you received this software without first entering into a license with
//C- AT&T, you have an infringing copy of this software and cannot use it
//C- without violating AT&T's intellectual property rights.
//C-
//C- $Id: DjVu.h,v 1.4 2000-01-07 00:28:07 bcr Exp $
#endif /* __cplusplus */

#include <stdlib.h>

#ifndef _DJVU_GLOBAL_H
#define _DJVU_GLOBAL_H

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
#ifndef DJVUAPI
#define DJVUAPI DLLEXPORT
#endif  /*DJVUAPI*/
#else
#ifndef DJVUAPI
#define DJVUAPI DLLIMPORT
#endif  /*DJVUAPI*/
#endif  /*BUILD_LIB*/
#endif DJVUAPI

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

typedef void djvu_free_callback (void *);
typedef void *djvu_realloc_callback (void *, size_t);
typedef void *djvu_malloc_callback (size_t);
typedef void *djvu_calloc_callback (size_t,size_t);

#ifdef NEED_DJVU_MEMORY
DJVUAPI int djvu_set_memory_callbacks(
           djvu_free_callback *,
           djvu_realloc_callback *,
           djvu_malloc_callback *,
           djvu_calloc_callback *);
#endif

#ifdef NEED_DJVU_MEMORY

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

typedef void
djvu_progress_callback (const char *task,unsigned long,unsigned long);

DJVUAPI djvu_progress_callback *
djvu_set_progress_callback(djvu_progress_callback *);

/** The #djvu_option# structure is very simmular to the standard unix
    long_options structure for getopt_long(3), and the usage is almost
    identical.
    @memo The djvu_option structure wraps the DjVuParseOptions class 
  */

struct djvu_option {
  /** This is the option long name without the leading double dash */
  const char *name;
  /** has_arg is one if the option has a value, and 2 if the value
      is optional.  Otherwise has_arg should be 0. */
  int has_arg;
  /** Currently this value is unused. */
  int *flag;
  /** This is a character representing the short option value.  Use '-'
      if there is no short option. */
  int val;
};

/** @name DjVuParseOptions C Wrappers
    The \Ref{DjVuParseOptions} class is wrapped by the following
    the following set of wrapper functions.
    @memo Wrapper functions for \Ref{DjVuParseOptions} */
#ifdef DOCXX_CODE
//@{
#endif /* DOCXX_CODE */

/** The #djvu_parse# structure is used for exporting and \Ref{DjVuParseOptions}
    methods from C++ into C.  */

struct djvu_parse
{
  /** This is a pointer to a \Ref{DjVuParseOptions} object */
  void *Private;
};

/** This is a wrapper for the C++ DjVuParseOptions profile constructor  */
struct djvu_parse
djvu_parse_init(const char []);

/** This is a wrapper for the C++ DjVuParseOptions config file constructor  */
struct djvu_parse
djvu_parse_config(const char [],const char []);

/** This is a wrapper for the C++ DjVuParseOptions copy constructor  */
struct djvu_parse
djvu_parse_copy(const struct djvu_parse);

/** This is a wrapper for the DjVuParseOptions::ChangeProfile function. */
void
djvu_parse_change_profile(struct djvu_parse,const char []);

  /** This is a wrapper for the DjVuParseOptions destructor */
void
djvu_parse_free(struct djvu_parse);

  /** This is a wrapper for the DjVuParseOptions::GetValue function */
const char *
djvu_parse_value(struct djvu_parse,const char []);

  /** This is a wrapper for the DjVuParseOptions::GetInteger function */
int
djvu_parse_integer(struct djvu_parse,const char [],const int);

  /** This is a wrapper for the DjVuParseOptions::GetInteger function */
int
djvu_parse_number(struct djvu_parse,const char [],const int);

  /** This is a wrapper for the DjVuParseOptions::ParseArguments function */
int
djvu_parse_arguments
(struct djvu_parse,int,const char * const *,const struct djvu_option []);

  /** This is a wrapper for the DjVuParseOptions::HasError function */
int
djvu_parse_haserror(struct djvu_parse);

  /** This is a wrapper for the DjVuParseOptions::GetError function */
const char *
djvu_parse_error(struct djvu_parse);

  /** This is a wrapper for the DjVuParseOptions::perror function */
void
djvu_parse_perror(struct djvu_parse,const char *mesg);

  /** This is a wrapper for the DjVuParseOptions::ConfigFilename function */
const char *
djvu_parse_configfile(struct djvu_parse,const char[],int);

#ifdef DOCXX_CODE
//@}
#endif /* DOCXX_CODE */
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __DJVU_GLOBAL_H__ */


