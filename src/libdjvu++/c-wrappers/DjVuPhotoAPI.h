/*C-  -*- C -*-
 *C-
 *C- This software may only be used by you under license from AT&T
 *C- Corp. ("AT&T"). A copy of AT&T's Source Code Agreement is available at
 *C- AT&T's Internet web site having the URL <http://www.djvu.att.com/open>.
 *C- If you received this software without first entering into a license with
 *C- AT&T, you have an infringing copy of this software and cannot use it
 *C- without violating AT&T's intellectual property rights.
 *C-
 *C- $Id: DjVuPhotoAPI.h,v 1.15 2000-03-07 00:01:17 bcr Exp $
 */

#ifndef _DJVUPHOTO_H_
#define _DJVUPHOTO_H_

#include "DjVuDecodeAPI.h"

/** @name DjVuPhotoAPI.h

   #DjVuPhotoAPI.h# defines the API for wavelette encoding multi page photos.
   Most of the structures defined here are also used for the background
   when document encoding.
   @author
   Bill C Riemers <bcr@att.com>
*/

/*
 * $Log: DjVuPhotoAPI.h,v $
 * Revision 1.15  2000-03-07 00:01:17  bcr
 * Updated the document api documentation to build correctly.
 *
 * Revision 1.14  2000/03/05 18:13:37  bcr
 * More comment changes.
 *
 */

/* Predeclarations. */

/*@{*/

/** ++ #phototodjvu_type# is used to decide how aggressively to compress
  the chrominance information when wavelette encoding.  Possible values
  consist of djvu_crcbnone, djvu_crcbhalf, djvu_crcbnormal, djvu_crcbfull,
  and djvu_jpeg.
 */
enum phototodjvu_type_enum
{
  djvu_crcbnone,
  djvu_crcbhalf,
  djvu_crcbnormal,
  djvu_crcbfull,
  djvu_jpeg
};

typedef enum phototodjvu_type_enum phototodjvu_type;

#ifdef __cplusplus
extern "C"
{
#ifndef __cplusplus
};
#endif
#endif

/** @memo #djvu_iw44_options# define wavelette compression options.

   The #djvu_iw44_options# structure defines the options
   used by the photo encoder (and document encoder) for 
   wavelette encoding the image (background).
*/
struct djvu_iw44_options_struct
{
/** This is the gamma factor used for correcting the image lighting.
    If you don't know what this is just leave it as 2.2, the default. */
  float gamma;
  #define DEFAULT_GAMMA 2.2

/** These decides which predefined set of options to use. They are
  enum values, defined above. */
  phototodjvu_type compression;

/** Slice target.  Data generation for the current chunk stops if the total
    number of slices (in this chunk and all the previous chunks) reaches
    value #slice#.  Use a NULL array to bypass this test.  Alternatively,
    you may use #0# to bypass this test on selected chunks.  (You should 
    specify at least one test for each chunk.) */
  const int *slices;

/** Size target.  Data generation for the current chunk stops if the total
    data size (in this chunk and all the previous chunks), expressed in
    bytes, reaches value #size#.  A NULL array means this criteria will
    not be used.  Alternatively, #0# can be used to bypass this value
    on a particular chunk.  (You should always specify at least one test 
    for each chunk.) */
  const int    *bytes;

/** Decibel target.  Data generation for the current chunk stops if the
    estimated luminance error, expressed in decibels, reaches value
    #decibel#.  Use a NULL array to shortcut the computation of luminance
    and sensibly speeds up the encoding process. Alternatively, you may
    use #0.0# to bypass this test for just one chunk.  (You should always
    specify at least one test for each chunk.) */
  const float *decibels;

/** #nchunks# specifies the size of each of the above arrays.  The arrays must
    be either null pointers, or nchunk size. */
  int nchunks;

/** #crcbdelay# specifies the delay factor for chrominace encoding.  A value
    of 10 is typical. */
  int crcbdelay;

#ifdef __cplusplus
inline djvu_iw44_options_struct();
#endif /* __cplusplus */

};

typedef struct djvu_iw44_options_struct djvu_iw44_options;

/** @memo #phototodjvu_options# contains options correponding to phototodjvu.

  The values of the #phototodjvu_options# structure control all
  aspects of photo encoding.
*/
struct phototodjvu_options_struct
{
/** The \Ref{djvu_process_options} defines the pages to be parsed,
  input, and output, and contains the pointer for storing errors. */
  djvu_process_options process;

/** These are the transformation options.  These will take place before
    compression. */
  djvu_transform_options transform;

/** These options are the options that control the quality and speed
    of compression.
 */
  djvu_iw44_options iw44;

#ifdef __cplusplus
inline phototodjvu_options_struct();
#endif /* __cplusplus */

};

typedef struct phototodjvu_options_struct phototodjvu_options;

struct djvu_parse
#if 0
{}
#endif
;

/** @name DjVuPhotoAPI C function calls
 */

/*@{*/

DJVUAPI
#if 0
;
#endif
/** ++ This is the primary allocation routine for phototodjvu_options.
    Even if the values specified are illegal, an options structure
    will be returned. */
phototodjvu_options *
phototodjvu_options_alloc(struct djvu_parse *,int,const char * const argv[]);

DJVUAPI
#if 0
;
#endif
/** ++ Deallocates the fields of the phototodjvu_options structure.
    You should always use the free option, even if you did not use alloc
    so the data pointed to by priv is freed. */
void phototodjvu_options_free(phototodjvu_options *);

DJVUAPI
#if 0
;
#endif
/** ++ This function converts the photo input files to a multipage DjVu document
    according to the options structure.  A non-zero return value indicates a
    fatal error. */
int phototodjvu(phototodjvu_options[1]);

DJVUAPI
#if 0
;
#endif
/** ++ A non-zero value indicates there are error messages.  Error
    messages are generated for both fatal errors, and errors
    that are recovered from.  */
int phototodjvu_haserror(const phototodjvu_options [1]);

DJVUAPI
#if 0
;
#endif
/** ++ A non-zero value indicates there are warning messages.  Waring
    messages are generated for non-fatal problems, that may be an
    error, or could just be abnormal usage. */
int phototodjvu_haswarning(const phototodjvu_options [1]);

DJVUAPI
#if 0
;
#endif
/** ++ Returns a string of the first error message on the stack.  Each
    call erases the previous return value. */
const char * phototodjvu_error(phototodjvu_options [1]);

DJVUAPI
#if 0
;
#endif
/** ++ Returns a string of the first warning message on the stack.  Each
    call erases the previous return value. */
const char * phototodjvu_warning(phototodjvu_options [1]);

DJVUAPI
#if 0
;
#endif
/** ++ Prints all the errors to stderr */
void phototodjvu_perror(phototodjvu_options [1],const char *mesg);

DJVUAPI
#if 0
;
#endif
/** ++ This will print usage instructions to the specified output. */
void phototodjvu_usage(int fd,const char *prog);

/*@}*/ 

/*@}*/ 

#ifdef __cplusplus
#ifndef __cplusplus
{
#endif
};


inline djvu_iw44_options_struct::djvu_iw44_options_struct() :
  gamma((float)DEFAULT_GAMMA), compression(djvu_crcbnormal), slices(0),
  bytes(0), decibels(0), nchunks(0), crcbdelay(10) {}

inline phototodjvu_options_struct::phototodjvu_options_struct() :
  process(), transform(), iw44() {}

#endif

#endif /* _DJVUPHOTO_H_ */

