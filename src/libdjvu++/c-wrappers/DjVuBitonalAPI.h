/*C-  -*- C -*-
 *C-
 *C- This software may only be used by you under license from AT&T
 *C- Corp. ("AT&T"). A copy of AT&T's Source Code Agreement is available at
 *C- AT&T's Internet web site having the URL <http://www.djvu.att.com/open>.
 *C- If you received this software without first entering into a license with
 *C- AT&T, you have an infringing copy of this software and cannot use it
 *C- without violating AT&T's intellectual property rights.
 *C-
 *C- $Id: DjVuBitonalAPI.h,v 1.3 2000-01-23 06:06:58 bcr Exp $
 */

#ifndef _DJVUBITONAL_H_
#define _DJVUBITONAL_H_

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


struct bitonaltodjvu_options_struct;
typedef struct bitonaltodjvu_options_struct bitonaltodjvu_options;

struct djvu_jb2_options_struct;
typedef struct djvu_jb2_options_struct djvu_jb2_options;

struct djvutobitonal_options_struct;
typedef struct djvutobitonal_options_struct djvutobitonal_options;

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
struct djvu_jb2_options_struct
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

};

/*@}*/


/*@{*/

/** @name bitonaltodjvu_options struct
    @memo Options used in bitonaltodjvu function 
*/

struct bitonaltodjvu_options_struct
{
/** This keeps a string delimited by hypens(-) and commas(,) */
  const char *page_range;

/** These are the compression options */
  djvu_jb2_options jb2;

/** They allow transformations to be done on the given input images. 
  vflip is verticle flip, hflip is horizontal flip, invert gives the
  negative of the image and rotateAngle will rotate image clockwise */
  int vflip, hflip, invert, rotateAngle;

/** warnfileno should be set to a fileno which is greater than zero to
    print warning messages to the fileno specified.  (Warnings are errors
    which are non-fatal. For example, specifying a negative dpi would
    produce a warning, and then processing would continue with the default
    dpi value. */
  int warnfileno;

/** logfileno should be set to a fileno which is greater than zero to
    print verbose messages to the fileno specified. */
  int logfileno;

/** helpfileno should be set to a fileno which is greater than zero to
    print warning messages to the fileno specified. */
  int helpfileno;

/** dpi should the resolution in dots per inch of input images. */
  int dpi;

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

#ifdef __cplusplus
  inline bitonaltodjvu_options_struct::bitonaltodjvu_options_struct();
#endif /* __cplusplus */

};

struct djvu_parse;

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

/** Returns a string of the first error message on the stack.  Each
    call erases the previous return value. */
DJVUAPI
const char * bitonaltodjvu_error(bitonaltodjvu_options [1]);

/** Prints all the errors to stderr */
DJVUAPI
void bitonaltodjvu_perror(bitonaltodjvu_options [1],const char *mesg);

/** This will print usage instructions to the specified output. */
DJVUAPI
void bitonaltodjvu_usage(int fd,const char *prog);

/*@}*/

/*@{*/

/** @name djvutobitonal_options struct
      
     @memo Options used in djvutobitonal function 
*/

struct djvutobitonal_options_struct
{
/** This keeps a string delimited by hypens(-) and commas(,) */
  const char *page_range;

/** This option should be non-zero, if we want to force the application
    to avoid color images */
  int disable_mask;

/** logfileno should be non-zero to print verbose processing details */
  int warnfileno;

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

#ifdef __cplusplus
  inline djvutobitonal_options_struct::djvutobitonal_options_struct();
#endif /* __cplusplus */
};


/** @name djvutobitonal_options_alloc function 
    This is the primary allocation routine for djvutobitonal_options.
    Even if the values specified are illegal, an options structure
    will be returned. */
DJVUAPI
djvutobitonal_options *
djvutobitonal_options_alloc(struct djvu_parse *,int,const char * const argv[]);

/** @name djvutobitonal_options_free function
    Deallocates the fields of the djvutobitonal_options structure.
    You should always use the free option, even if you did not use alloc
    so the data pointed to by priv is freed. */
DJVUAPI
void djvutobitonal_options_free(djvutobitonal_options *);

/** @name djvutobitonal function 
    This function converts the source multipage DjVu document to
    a bitonal image according to options structure. */
DJVUAPI
int djvutobitonal(djvutobitonal_options[1]);

/** A non-zero value indicates there are error messages.  Error
    messages are generated for both fatal errors, and errors
    that are recovered from.  */
DJVUAPI
int djvutobitonal_haserror(const djvutobitonal_options [1]);

/** Returns a string of the first error message on the stack.  Each
    call erases the previous return value. */
DJVUAPI
const char * djvutobitonal_error(djvutobitonal_options [1]);

/** Prints all the errors to stderr */
DJVUAPI
void djvutobitonal_perror(djvutobitonal_options [1],const char *mesg);

/** This will print usage instructions to the specified output. */
DJVUAPI
void djvutobitonal_usage(int fd,const char *prog);

/*@}*/

#ifdef __cplusplus
}

inline djvu_jb2_options_struct::djvu_jb2_options_struct() :
  pages_per_dict(10), compression(djvu_normal), halftone_off(0),
  tolerance_percent(-1), tolerance4_size(-1) {}

inline bitonaltodjvu_options_struct::bitonaltodjvu_options_struct() :
  page_range(0), jb2(), vflip(0), hflip(0), invert(0),
  rotateAngle(0), warnfileno(0), logfileno(0), helpfileno(0), filelist(0),
  filecount(0), output(0), prog(0), priv(0) {}

inline djvutobitonal_options_struct::djvutobitonal_options_struct() :
  page_range(0), disable_mask(0), warnfileno(0), logfileno(0), helpfileno(0),
  filelist(0), filecount(0), output(0), prog(0), priv(0) {}

#endif

#endif /* _DJVUBITONAL_H_ */

