/*C-  -*- C -*-
 *C-
 *C- This software may only be used by you under license from AT&T
 *C- Corp. ("AT&T"). A copy of AT&T's Source Code Agreement is available at
 *C- AT&T's Internet web site having the URL <http://www.djvu.att.com/open>.
 *C- If you received this software without first entering into a license with
 *C- AT&T, you have an infringing copy of this software and cannot use it
 *C- without violating AT&T's intellectual property rights.
 *C-
 *C- $Id: DjVuDecodeAPI.h,v 1.18 2000-01-30 01:18:42 bcr Exp $
 */

#ifndef _DJVUDECODE_H_
#define _DJVUDECODE_H_

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

#endif /*DJVUAPI*/


/** @name djvudecode.h
      Functions used to decode multiple page DjVu documents...
*/

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

/* Predeclarations. */
 
struct djvu_parse;

struct djvuio_struct;

typedef struct djvuio_struct* djvu_import;
typedef struct djvuio_struct* djvu_export;

typedef enum djvu_layer_type_enum
{
  DJVU_ALL,
  DJVU_MASK,
  DJVU_FOREGROUND,
  DJVU_BACKGROUND
} djvu_layer_type;

/*@{*/

/** @name djvu_transform_options struct
      
    @memo Options that effect the processing of images.
*/

typedef struct djvu_transform_options_struct
{
  /** The #hflip# transform reverses left-right orientation.  This 
    results in mirror imaged documents.  Set #hflip# to non-zero
    to perform this transformation.  */
  int hflip;
  /** The #vflip# transform flips the image upside down.  This is
    logically equivalent to a 180 degree rotation and then an hflip.
    Set #vflip# to a non-zero value to perform this transformation. */
  int vflip;
  /** The #rotate# transform rotates an image to the nearest 90 degree
    multiple of the angle specified.  For example, a value of 46 would
    result in a 90 degree rotation clockwise. */
  int rotateAngle;
  /** The #togray# option will reduce color images to gray scale.  Set
     #togray# to a non-zero value to perform this conversion. */
  int togray;
  /** The #tobitonal# option will reduce gray images to bitonal (RLE) images.
     To convert color documents to bitonal, use the #togray# above as well
     as the #tobitonal# option.  Set #tobitonal# to a non-zero value to
     perform this conversion. */
  int tobitonal;
  /** The #invert# option exchanges black with white and white with black
     in a bitonal (RLE) image.  There will be no effect on color and gray
     scale documents unless the #togray# and #tobitonal# flags have also
     been used respectively.  Set #invert# to a non-zero value to perform
     this conversion. */
  int invert;
  /** Specify the absolute size desired.  0,0 has the special meaning of 
     being unresized. */
  int hsize,vsize;
  /// The amount to upsample the horizontal axis.
  int hupsample;
  /// The amount to subsample the horizontal axis.
  int hsubsample;
  /// The amount to upsample the vertical axis.
  int vupsample;
  /// The amount to subsample the vertical axis.
  int vsubsample;
  /** The values xmin, ymin, seg_width, and seg_height, allow cropping
      (segmentation) of the image. */
  int xmin, ymin, seg_width, seg_height;
  /** The #dpi# option will override the dpi value specified in the file.
     The image itself is left unmodified, but the rendering and compression
     will be effected.  To use the #dpi# information in the image, then
     set this value to 0. */
  int dpi;
#ifdef __cplusplus
  inline djvu_transform_options_struct();
#endif /* __cplusplus */
} djvu_transform_options;

/*@}*/

/*@{*/

/** @name djvu_process_options struct
      
     @memo Options used in deciding page range, output devices, etc.
*/

typedef struct djvu_process_options_struct
{
  /// This keeps a string delimited by hypens(-) and commas(,)
  const char *page_range;

  /** warnfileno should be non-zero to print warning messages that may
    effect the processing.  If this is zero, then warnings will be
    spooled like error messages.  This will likely make the output less
    intuitive, since unlike errors, processing continues even after a 
    warning is issued. */
  int warnfileno;

  /// logfileno should be non-zero to print verbose processing details
  int logfileno;

  /// #helpfileno# should be non-zero to print usage instructions.
  int helpfileno;

  /// list of input filenames being the last
  const char * const * filelist;

  /// Number of files in filelist
  int filecount;

  /** You can specify an import stream instead of a filelist as defined in
    DjVuAPI.h.  When using an input_stream filelist and filecount should both
    be set to 0. */
  djvu_import input_stream;

  /// The output filename (or directory)
  const char *output;

  /** Instead of specifying an output filename, you can define a 
    djvu_export stream as defined in DjVuAPI.h.  When using an output_stream
    output should be set to zero.  */
  djvu_export output_stream;

  /// The program name
  const char *prog;

  /// This is where all memory is allocated and errors are listed.
  void *priv;

#ifdef __cplusplus
  inline djvu_process_options_struct();
#endif /* __cplusplus */
} djvu_process_options;

/*@}*/

/*@{*/

/** @name djvu_decode_options struct
      
     @memo Options used in djvu_decode function 
*/

typedef struct djvu_decode_options_struct
{
  /** The #djvu_process_options struct# defines the pages to be parsed,
     input, and output, and contains the pointer for storing errors. */
  djvu_process_options process;

  /** These are the transformation options.  These will take place after
    rendering. */
  djvu_transform_options transform;

  /** #output_format# is string, that indicates the output format, specified
    by the normal file extension of "pnm","ppm","pgm","pbm","tif","jpg",
    "ps", or "pict".  The special value of "auto" or a NULL pointer means to 
    try and decide the output format based on the output file's extension. */
  const char *output_format;

  /** This specifies the layer to decode.
    Pages without the specified layer will be skipped. */
  djvu_layer_type layer;

#ifdef __cplusplus
  inline djvu_decode_options_struct();
#endif /* __cplusplus */

} djvu_decode_options;


/** @name djvu_decode_options_alloc function 
    This is the primary allocation routine for the
    #djvu_decode_options struct#.  Even if the values specified are
    illegal, an options structure will always be returned. */
DJVUAPI
djvu_decode_options *
djvu_decode_options_alloc(struct djvu_parse *,int,const char * const argv[]);

/** @name djvu_decode_options_free function
    Deallocates the fields of the #djvu_decode_options struct#.
    You should always use the free option, even if you did not use alloc
    so the data pointed to by priv is deallocated. */
DJVUAPI
void djvu_decode_options_free(djvu_decode_options *);

/** @name djvu_decode function 
    This function converts the source multipage DjVu document to
    a document image according to options structure. */
DJVUAPI
int djvu_decode(djvu_decode_options[1]);

/** A non-zero value indicates there are error messages.  Error
    messages are generated for both fatal errors, and errors
    that are recovered from.  */
DJVUAPI
int djvu_decode_haserror(const djvu_decode_options [1]);

/** A non-zero value indicates there are warning messages.  Warning
    messages are generated from non-fatal errors, and sometimes just
    abnormal, but correct usage. */
DJVUAPI
int djvu_decode_haswarning(const djvu_decode_options [1]);

/** Returns a string of the first error message on the stack.  Each
    call erases the previous return value. */
DJVUAPI
const char * djvu_decode_error(djvu_decode_options [1]);

/** Returns a string of the first warning message on the stack.  Each
    call erases the previous return value. */
DJVUAPI
const char * djvu_decode_warning(djvu_decode_options [1]);

/// Prints all the errors to stderr.
DJVUAPI
void djvu_decode_perror(djvu_decode_options [1],const char *mesg);

/// This will print usage instructions to the specified output.
DJVUAPI
void djvu_decode_usage(int fd,const char *prog);

/*@}*/

#ifdef __cplusplus
}

inline djvu_transform_options_struct::djvu_transform_options_struct() :
  hflip(0), vflip(0), rotateAngle(0), togray(0), tobitonal(0),
  invert(0), hsize(0), vsize(0), hupsample(1), hsubsample(1), vupsample(1),
  vsubsample(1), xmin(0), ymin(0), seg_width(0), seg_height(0), dpi(0) {}

inline djvu_process_options_struct::djvu_process_options_struct() :
  page_range(0), warnfileno(0), logfileno(0), helpfileno(0), filelist(0),
  filecount(0), input_stream(0), output(0), output_stream(0), prog(0),
  priv(0) {}

inline djvu_decode_options_struct::djvu_decode_options_struct() :
  process(), transform(), output_format(0), layer(DJVU_ALL) {}

#endif /* __cplusplus */

#endif /* _DJVUDECODE_H_ */

