/*C-  -*- C -*-
 *C-
 *C- Copyright (c) 2000, LizardTech, Inc.  All Rights Reserved.
 *C-
 *C- $Id: DjVuPhotoAPI.h,v 1.21 2000-09-18 17:10:36 bcr Exp $
 */

#ifndef _DJVUPHOTO_H_
#define _DJVUPHOTO_H_

#include "DjVuDecodeAPI.h"

/** @name DjVuPhotoAPI.h

   @memo #DjVuPhotoAPI.h# defines the API for wavelett encoding multi-page
   photos.  Most of the structures defined here are also used for the
   background layer when document encoding.
   @author
   Bill C Riemers 
*/

/*@{*/

/*
 * $Log: DjVuPhotoAPI.h,v $
 * Revision 1.21  2000-09-18 17:10:36  bcr
 * Adding files.
 *
 * Revision 1.19  2000/07/11 19:28:54  bcr
 * Various fixes to the copyrights and such.  Applied AT&T's latest patches.
 *
 * Revision 1.18  2000/07/04 00:51:50  mrosen
 * updated documentation
 *
 * Revision 1.17  2000/03/09 22:27:59  bcr
 * Updated the documentation, again.
 *
 * Revision 1.16  2000/03/08 22:59:46  bcr
 * Updated the documentation.  I'm using Leon's libdjvu++ documentation
 * as a template.
 *
 * Revision 1.15  2000/03/07 00:01:17  bcr
 * Updated the document api documentation to build correctly.
 *
 * Revision 1.14  2000/03/05 18:13:37  bcr
 * More comment changes.
 *
 */

/* Predeclarations. */

/** ++ #phototodjvu_type# is used to decide how aggressively to compress
  the chrominance information when wavelett encoding.  Possible values
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

/** @memo #djvu_iw44_options# define wavelett compression options.

   The #djvu_iw44_options# structure defines the options
   used by the photo encoder (and document encoder) for 
   wavelett encoding the image (background).
*/
struct djvu_iw44_options_struct
{
/** The #gamma# factor used for correcting the image lighting.
    If you don't know what this is just leave it as 2.2, the default. */
  float gamma;

/** #compression# is an enum type of \Ref{phototodjvu_type} for specifying
  what level of compression will be used. */
  phototodjvu_type compression;

/** #slices# is an array specifying the target number of slices total at
    the end of each chunk.

    Data generation for the current chunk stops if the total
    number of slices (in this chunk and all the previous chunks) reaches
    value #slice#.  Values between 65 and 140 are meaningful.  Use a NULL
    array to bypass this test.  Alternatively, you may use #0# to bypass
    this test on selected chunks.  (You should specify at least one test
    for each chunk.) */
  const int *slices;

/** #bytes# is an array specifying the target number of bytes total at the
    end of each chunk.

    Data generation for the current chunk stops if the total
    data size (in this chunk and all the previous chunks), expressed in
    bytes, reaches value #size#.  A NULL array means this criteria will
    not be used.  Alternatively, #0# can be used to bypass this value
    on a particular chunk.  (You should always specify at least one test 
    for each chunk.) */
  const int    *bytes;

/** #decibels# is an array specifying the target luminance error expressed
    in decibels for at the end of each chunk.

    Data generation for the current chunk stops if the estimated luminance
    error, expressed in decibels, reaches value #decibel#.  Meaningful values
    are between 16.0 and 64.0.  Use a NULL array to shortcut the computation
    of luminance and sensibly speeds up the encoding process.  Alternatively,
    you may use #0.0# to bypass this test for just one chunk.  (You should
    always specify at least one test for each chunk.) */
  const float *decibels;

/** #nchunks# specifies the size of each of the above arrays.  The arrays must
    be either null pointers, or nchunk size. */
  int nchunks;

/** #crcbdelay# specifies the delay factor for chrominance encoding.  A value
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
    If a \Ref{djvu_parse} structure has already been declared, it may
    be passed as the parse argument; otherwise, a NULL should be passed
    as the parse value.  Even if the values specified are illegal, an
    options structure will be returned. */
phototodjvu_options *
phototodjvu_options_alloc(
  struct djvu_parse *parse,int argc,const char * const argv[]);

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
    messages are generated for both fatal errors, and some non-fatal errors. */
int phototodjvu_haserror(const phototodjvu_options [1]);

DJVUAPI
#if 0
;
#endif
/** ++ A non-zero value indicates there are warning messages.  Warning
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
/** ++ Prints all the errors to stderr.  If the mesg arguments is not
  NULL, the mesg will be printed followed by a colon and a space befor
  each error message. */
void phototodjvu_perror(phototodjvu_options [1],const char *mesg);

DJVUAPI
#if 0
;
#endif
/** ++ This will print usage instructions to the specified fileno. */
void phototodjvu_usage(int fd,const char *prog);

/*@}*/ 

#ifdef __cplusplus
#ifndef __cplusplus
{
#endif
};


inline djvu_iw44_options_struct::djvu_iw44_options_struct() :
  gamma((float)2.2), compression(djvu_crcbnormal), slices(0),
  bytes(0), decibels(0), nchunks(0), crcbdelay(10) {}

inline phototodjvu_options_struct::phototodjvu_options_struct() :
  process(), transform(), iw44() {}

#endif

/*@}*/ 

#endif /* _DJVUPHOTO_H_ */

