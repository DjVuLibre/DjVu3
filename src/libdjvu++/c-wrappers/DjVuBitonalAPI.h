/*C-  -*- C -*-
 *C-
 *C- This software may only be used by you under license from AT&T
 *C- Corp. ("AT&T"). A copy of AT&T's Source Code Agreement is available at
 *C- AT&T's Internet web site having the URL <http://www.djvu.att.com/open>.
 *C- If you received this software without first entering into a license with
 *C- AT&T, you have an infringing copy of this software and cannot use it
 *C- without violating AT&T's intellectual property rights.
 *C-
 *C- $Id: DjVuBitonalAPI.h,v 1.6 2000-01-26 04:40:46 bcr Exp $
 */

#ifndef _DJVUBITONAL_H_
#define _DJVUBITONAL_H_

#include "DjVuDecodeAPI.h"

/** @name djvubitonal.h
      functions used to convert multiple bitonal images to DjVu multipage 
      documents.
*/

#ifdef __cplusplus
extern "C"
{
#endif

/* Predeclarations. */
 
struct djvu_parse;


typedef enum bitonaltodjvu_type_enum
{
  djvu_normal,
  djvu_conservative,
  djvu_lossless,
  djvu_aggressive,
  djvu_pseudo
} bitonaltodjvu_type;

/*@{*/

/** @name djvu_jb2_options struct
      
    @memo Options to fine tune compression quality and speed.
*/
typedef struct djvu_jb2_options_struct
{
/** pages_per_dict allows n number of pages to be matched together.
    This value should never be too high or too low. Best values are
    between 10 to 20.  A value of 1 has the special meaning of 
    not using a shared dictionary, and should be when fewer than
    ten or so pages are being processed. */
  int pages_per_dict;

/** These decides which predefined set of options to use. Quality
  value is greatest in lossless. (lossless > normal > conservative
  > aggressive).  pseudo will create the djvu output with data
  stored in G4 format, that is same as input.  The relavent
  configuration file option is lossless, normal, conservative,
  aggressive, and pseudo as mutually exclusive options.
 */
  bitonaltodjvu_type compression;

/** Halftone detection is used for dithered images.  The default
    of zero, means halftone detection is on.  */
  int halftone_off;

/** These are tuning parameters which affect the compression
    and the quality of the output image.  These are intended
    primarily for use from profiles tuned to a particular
    input source. Normal usage is to set these values by adjusting
    the #compression# mode above. */
  int tolerance_percent, tolerance4_size;

#ifdef __cplusplus
inline djvu_jb2_options_struct();
#endif /* __cplusplus */

} djvu_jb2_options;

/*@}*/

/*@{*/

/** @name bitonaltodjvu_options struct
    @memo Options used in bitonaltodjvu function 
*/

typedef struct bitonaltodjvu_options_struct
{
  /** The #djvu_process_options struct@ defines the pages to be parsed,
    input, and output, and contains the pointer for storing errors. */
  djvu_process_options process;

  /** These are the transformation options.  These will take place before
    compression. */
  djvu_transform_options transform;

  /// These are the compression options.
  djvu_jb2_options jb2;

#ifdef __cplusplus
  inline bitonaltodjvu_options_struct();
#endif /* __cplusplus */

} bitonaltodjvu_options;

/** @name bitonaltodjvu_options_alloc function 
    This is the primary allocation routine for bitonaltodjvu_options.
    Even if the values specified are illegal, an options structure
    will be returned. */
DJVUAPI
bitonaltodjvu_options *
bitonaltodjvu_options_alloc(struct djvu_parse *,int,const char * const argv[]);

/** @name bitonaltodjvu_options_free function
    Deallocates the fields of the bitonaltodjvu_options structure.
    You should always use the free option, even if you did not use alloc
    so the data pointed to by priv is freed. */
DJVUAPI
void bitonaltodjvu_options_free(bitonaltodjvu_options *);

/** @name bitonaltodjvu function 
    This function converts the bitonal input files to a multipage DjVu document
    according to the options structure.
    Depending on the type of the input data, the function uses the proper
    stream derived from \Ref{DjVu_Stream} for decoding, while 
    \Ref{DjVu_PixImage.h} for transformations and \Ref{DjVmDoc.h}, 
    \Ref{JB2Matcher.h} for encoding.  A non-zero return value indicates a
    fatal error. */
DJVUAPI
int bitonaltodjvu(bitonaltodjvu_options[1]);

/** A non-zero value indicates there are error messages.  Error
    messages are generated for both fatal errors, and errors
    that are recovered from.  */
DJVUAPI
int bitonaltodjvu_haserror(const bitonaltodjvu_options [1]);

/** A non-zero value indicates there are warning messages.  Waring
    messages are generated for non-fatal problems, that may be an
    error, or could just be abnormal usage. */
DJVUAPI
int bitonaltodjvu_haswarning(const bitonaltodjvu_options [1]);

/** Returns a string of the first error message on the stack.  Each
    call erases the previous return value. */
DJVUAPI
const char * bitonaltodjvu_error(bitonaltodjvu_options [1]);

/** Returns a string of the first warning message on the stack.  Each
    call erases the previous return value. */
DJVUAPI
const char * bitonaltodjvu_warning(bitonaltodjvu_options [1]);

/** Prints all the errors to stderr */
DJVUAPI
void bitonaltodjvu_perror(bitonaltodjvu_options [1],const char *mesg);

/** This will print usage instructions to the specified output. */
DJVUAPI
void bitonaltodjvu_usage(int fd,const char *prog);

/*@}*/

#ifdef __cplusplus
}

inline djvu_jb2_options_struct::djvu_jb2_options_struct() :
  pages_per_dict(10), compression(djvu_normal), halftone_off(0),
  tolerance_percent(-1), tolerance4_size(-1) {}

inline bitonaltodjvu_options_struct::bitonaltodjvu_options_struct() :
  process(), transform(), jb2() {}

#endif

#endif /* _DJVUBITONAL_H_ */

