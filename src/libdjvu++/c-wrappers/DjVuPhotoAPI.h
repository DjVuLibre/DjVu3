/*C-  -*- C -*-
 *C-
 *C- This software may only be used by you under license from AT&T
 *C- Corp. ("AT&T"). A copy of AT&T's Source Code Agreement is available at
 *C- AT&T's Internet web site having the URL <http://www.djvu.att.com/open>.
 *C- If you received this software without first entering into a license with
 *C- AT&T, you have an infringing copy of this software and cannot use it
 *C- without violating AT&T's intellectual property rights.
 *C-
 *C- $Id: DjVuPhotoAPI.h,v 1.7 2000-01-23 20:29:17 bcr Exp $
 */

#ifndef _DJVUPHOTO_H_
#define _DJVUPHOTO_H_

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


/** @name djvuphoto.h
      functions used to convert multiple photo images to DjVu multipage 
      documents.
*/

#ifdef __cplusplus
extern "C"
{
#endif

#ifdef DOCXX_CODE
//@{
#endif /*DOCXX_CODE*/

/** @name phototodjvu_options struct
    @memo Options used in phototodjvu function 
*/

enum phototodjvu_type_enum
{
  djvu_crcbnone,
  djvu_crcbhalf,
  djvu_crcbnormal,
  djvu_crcbfull,
  djvu_pseudobg
};
typedef enum phototodjvu_type_enum phototodjvu_type;

struct phototodjvu_options_struct
{
/** This keeps a string delimited by hypens(-) and commas(,) */
  const char *page_range;

/** These decides which predefined set of options to use. They are
  enum values, defined above. */
  phototodjvu_type compression;

/** This is the gamma factor used for correcting the image lighting.
  If you don't know what this is just leave it as 2.2, the default. */
  float gamma;

/** Boolian values specified whether a vflip, hflip, should be done.
  vflip means to flip on the vertical axis, and hflip the horizontal
  axis., and invert means to reverse black and white. */
  int vflip, hflip;

/** Specify the angle the input image should be rotated.  This may be any
    multiple of 90 degrees.  Rotation is clockwise and takes place after
    any vflip or hflip commands. */
  int rotateAngle;

/** logfile should be non-NULL to print verbose processing details */
  int logfileno;

/** helpfile should be non-NULL to print usage instructions */
  int helpfileno;

/** dpi should the resolution in dots per inch of input images. */
  int dpi;

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
  be between 10 to 20*/
  int crcbdelay;

/** list of input filenames being the last. */
  const char * const * filelist;

/** Number of files in filelist. */
  int filecount;

/** The output filename (or directory) */
  const char *output;

/** The program name */
  const char *prog;

/** This is where all memory is allocated and errors are listed. */
  void *priv;
};
typedef struct phototodjvu_options_struct phototodjvu_options;

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

/** Returns a string of the first error message on the stack.  Each
    call erases the previous return value. */
DJVUAPI
const char * phototodjvu_error(phototodjvu_options [1]);

/** Prints all the errors to stderr */
DJVUAPI
void phototodjvu_perror(phototodjvu_options [1],const char *mesg);

/** This will print usage instructions to the specified output. */
DJVUAPI
void phototodjvu_usage(int fd,const char *prog);

#ifdef DOCXX_CODE
//@}
#endif /*DOCXX_CODE*/


#ifdef DOCXX_CODE
//@{
#endif /*DOCXX_CODE*/

/** @name djvutophoto_options struct
      
     @memo Options used in djvutophoto function 
*/

struct djvutophoto_options_struct
{
/** This keeps a string delimited by hypens(-) and commas(,) */
  const char *page_range;

/** This option should be non-zero, if we want to force color images to
    be reduced to gray scale */
  int togray;

/** Boolian values specified whether a vflip, hflip, should be done.
  vflip means to flip on the vertical axis, and hflip the horizontal
  axis., and invert means to reverse black and white. */
  int vflip, hflip;

/** Specify the angle the input image should be rotated.  This may be any
    multiple of 90 degrees.  Rotation is clockwise and takes place after
    any vflip or hflip commands. */
  int rotateAngle;

/** logfileno should be non-zero to print verbose processing details */
  int logfileno;

/** helpfileno should be non-zero to print usage instructions */
  int helpfileno;

/** list of input filenames being the last. */
  const char * const * filelist;

/** Number of files in filelist. */
  int filecount;

/** The output filename (or directory) */
  const char *output;

/** The program name */
  const char *prog;

/** This is where all memory is allocated and errors are listed. */
  void *priv;
};
typedef struct djvutophoto_options_struct djvutophoto_options;

struct djvu_parse;

/** @name djvutophoto_options_alloc function 
    This is the primary allocation routine for djvutophoto_options.
    Even if the values specified are illegal, an options structure
    will be returned. */
DJVUAPI
djvutophoto_options *
djvutophoto_options_alloc(struct djvu_parse *,int,const char * const argv[]);

/** @name djvutophoto_options_free function
    Deallocates the fields of the djvutophoto_options structure.
    You should always use the free option, even if you did not use alloc
    so the data pointed to by priv is freed. */
DJVUAPI
void djvutophoto_options_free(djvutophoto_options *);

/** @name djvutophoto function 
    This function converts the source multipage DjVu document to
    a photo image according to options structure. */
DJVUAPI
int djvutophoto(djvutophoto_options[1]);

/** A non-zero value indicates there are error messages.  Error
    messages are generated for both fatal errors, and errors
    that are recovered from.  */
DJVUAPI
int djvutophoto_haserror(const djvutophoto_options [1]);

/** Returns a string of the first error message on the stack.  Each
    call erases the previous return value. */
DJVUAPI
const char * djvutophoto_error(djvutophoto_options [1]);

/** Prints all the errors to stderr */
DJVUAPI
void djvutophoto_perror(djvutophoto_options [1],const char *mesg);

/** This will print usage instructions to the specified output. */
DJVUAPI
void djvutophoto_usage(int fd,const char *prog);

#ifdef DOCXX_CODE
//@}
#endif /*DOCXX_CODE*/

#ifdef __cplusplus
}
#endif

#endif /* _DJVUPHOTO_H_ */

