/*C-  -*- C -*-
 *C-
 *C- This software may only be used by you under license from AT&T
 *C- Corp. ("AT&T"). A copy of AT&T's Source Code Agreement is available at
 *C- AT&T's Internet web site having the URL <http://www.djvu.att.com/open>.
 *C- If you received this software without first entering into a license with
 *C- AT&T, you have an infringing copy of this software and cannot use it
 *C- without violating AT&T's intellectual property rights.
 *C-
 *C- $Id: DjVuPhotoAPI.h,v 1.11 2000-01-26 21:51:13 bcr Exp $
 */

#ifndef _DJVUPHOTO_H_
#define _DJVUPHOTO_H_

#include "DjVuDecodeAPI.h"

/** @name djvuphoto.h
      functions used to convert multiple photo images to DjVu multipage 
      photo files.
*/

#ifdef __cplusplus
extern "C"
{
#endif

enum phototodjvu_type_enum
{
  djvu_crcbnone,
  djvu_crcbhalf,
  djvu_crcbnormal,
  djvu_crcbfull,
  djvu_jpeg
};
typedef enum phototodjvu_type_enum phototodjvu_type;

typedef struct djvu_iw44_options_struct
{
/** This is the gamma factor used for correcting the image lighting.
    If you don't know what this is just leave it as 2.2, the default. */
  float gamma;

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

/** nchunks specifies the size of each of the above arrays.  The arrays must
    be either null pointers, or nchunk size. */
  int nchunks;

/** pages_per_dict allows n number of pages to be matched together.
    This value should never be too high or too low. Best value  can
    be between 10 to 20 */
  int crcbdelay;

#ifdef __cplusplus
inline djvu_iw44_options_struct();
#endif /* __cplusplus */

} djvu_iw44_options;


/*@{*/

/** @name phototodjvu_options struct
    @memo Options used in the phototodjvu function 
*/

typedef struct phototodjvu_options_struct
{
/** The #djvu_process_options struct@ defines the pages to be parsed,
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

} phototodjvu_options;

struct djvu_parse;

/** @name phototodjvu_options_alloc function 
    This is the primary allocation routine for phototodjvu_options.
    Even if the values specified are illegal, an options structure
    will be returned. */
DJVUAPI
phototodjvu_options *
phototodjvu_options_alloc(struct djvu_parse *,int,const char * const argv[]);

/** @name phototodjvu_options_free function
    Deallocates the fields of the phototodjvu_options structure.
    You should always use the free option, even if you did not use alloc
    so the data pointed to by priv is freed. */
DJVUAPI
void phototodjvu_options_free(phototodjvu_options *);

/** @name phototodjvu function 
    This function converts the photo input files to a multipage DjVu document
    according to the options structure.
    Depending on the type of the input data, the function uses the proper
    stream derived from \Ref{DjVu_Stream} for decoding, while 
    \Ref{DjVu_PixImage.h} for transformations and \Ref{DjVmDoc.h}, 
    \Ref{JB2Matcher.h} for encoding.  A non-zero return value indicates a
    fatal error. */
DJVUAPI
int phototodjvu(phototodjvu_options[1]);

/** A non-zero value indicates there are error messages.  Error
    messages are generated for both fatal errors, and errors
    that are recovered from.  */
DJVUAPI
int phototodjvu_haserror(const phototodjvu_options [1]);

/** A non-zero value indicates there are warning messages.  Waring
    messages are generated for non-fatal problems, that may be an
    error, or could just be abnormal usage. */
DJVUAPI
int phototodjvu_haswarning(const phototodjvu_options [1]);

/** Returns a string of the first error message on the stack.  Each
    call erases the previous return value. */
DJVUAPI
const char * phototodjvu_error(phototodjvu_options [1]);

/** Returns a string of the first warning message on the stack.  Each
    call erases the previous return value. */
DJVUAPI
const char * phototodjvu_warning(phototodjvu_options [1]);

/** Prints all the errors to stderr */
DJVUAPI
void phototodjvu_perror(phototodjvu_options [1],const char *mesg);

/** This will print usage instructions to the specified output. */
DJVUAPI
void phototodjvu_usage(int fd,const char *prog);

/*@}*/ 

#ifdef __cplusplus
}

inline djvu_iw44_options_struct::djvu_iw44_options_struct() :
  gamma((float)2.2), compression(djvu_crcbnormal), slices(0),
  bytes(0), decibels(0), nchunks(0), crcbdelay(10) {}

inline phototodjvu_options_struct::phototodjvu_options_struct() :
  process(), transform(), iw44() {}

#endif

#endif /* _DJVUPHOTO_H_ */
