#ifdef __cplusplus
//C-  -*- C++ -*-
//C-
//C-  Copyright (c) 1999 AT&T
//C-  All Rights Reserved
//C-
//C-  THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
//C-  The copyright notice above does not evidence any
//C-  actual or intended publication of such source code.
//C-
//C- "$Id: djvutiffg4.h,v 1.2 2000-01-06 22:00:22 bcr Exp $"
//C- -- Tiff G4 To DjVu
//C- Author: Parag Deshmukh (Dec 99)
#endif  /* __cplusplus */

#ifndef _DJVUTIFFG4_H_
#define _DJVUTIFFG4_H_

#include <stdio.h>

/** @name TiffG4ToDjVu.h
      functions used to convert multiple Tiff G4 images to DjVu multipage 
      documents.
*/

#ifdef DOCXX_CODE
//@{
#endif /*DOCXX_CODE*/

/** @name tiffg4todjvu_options struct
      
     @memo Options used in tiffg4todjvu function 
*/

enum tiffg4todjvu_type_enum
{
  djvu_normal,
  djvu_conservative,
  djvu_lossless,
  djvu_aggressive,
  djvu_pseudo
};
typedef enum tiffg4todjvu_type_enum tiffg4todjvu_type;

struct tiffg4todjvu_options_struct
{
/** This keeps a string delimited by hypens(-) and commas(,) */
  const char *page_range;
/** These are tuning parameters which affect the compression
  and the quality of the output image */
  int tolerance_percent, tolerance4_size;
/** These decides which predefined set of options to use. They are
  boolean values either 0 or 1. Quality  value is greatest in 
  lossless. (lossless > normal > conservative > aggressive).
  pseudo wil create the djvu output with data stored in G4 format,
  that is same as input. */
  tiffg4todjvu_type compression;
/** Halftone detection is used for dithered images. */
  int halftone_off;
/** pages_per_dict allows n number of pages to be matched together.
  This value should never be too high or too low. Best value  can
  be between 10 to 20*/
  int pages_per_dict;
/** They allow transformations to be done on the given input images. 
  vflip is verticle flip, hflip is horizontal flip, invert gives the
  negative of the image and rotateAngle will rotate image clockwise */
  int vflip, hflip, invert, rotateAngle;
/** logfile should be non-NULL to print verbose processing details */
  FILE *logfile;
/** helpfile should be non-NULL to print usage instructions */
  FILE *helpfile;
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
};
typedef struct tiffg4todjvu_options_struct tiffg4todjvu_options;

struct djvu_parse;

/** @name tiffg4todjvu_options_alloc function 
    This is the primary allocation routine for tiffg4todjvu_options.
    Even if the values specified are illegal, an options structure
    will be returned. */
tiffg4todjvu_options *
tiffg4todjvu_options_alloc(struct djvu_parse *,int,const char * const argv[]);

/** @name tiffg4todjvu_options_free function
    Deallocates the fields of the tiffg4todjvu_options structure.
    You should always use the free option, even if you did not use alloc
    so the data pointed to by priv is freed. */
void tiffg4todjvu_options_free(tiffg4todjvu_options *);

/** @name tiffg4todjvu function 
    This function converts the Tiff G4 input files to multipage DjVu document
    according to options structure.
    The function mainly uses  \Ref{DjVu_MTiffStream.h} for decoding, while 
    \Ref{DjVu_PixImage.h} for transformations and \Ref{DjVmDoc.h}, 
    \Ref{JB2Matcher.h} for encoding.  A non-zero return value indicates a
    fatal error. */
int tiffg4todjvu(tiffg4todjvu_options[]);

/** A non-zero value indicates there are error messages.  Error
    messages are generated for both fatal errors, and errors
    that are recovered from.  */
int tiffg4todjvu_haserror(const tiffg4todjvu_options []);

/** Returns a string of the first error message on the stack.  Each
    call erases the previous return value. */
const char * tiffg4todjvu_error(tiffg4todjvu_options []);

/** Prints all the errors to stderr */
void tiffg4todjvu_perror(tiffg4todjvu_options [],const char *);

/** This will print usage instructions to the specified output. */
void tiffg4todjvu_usage(FILE *out,const char *prog);

#ifdef DOCXX_CODE
//@}
#endif /*DOCXX_CODE*/


#ifdef DOCXX_CODE
//@{
#endif /*DOCXX_CODE*/

/** @name djvutotiffg4_options struct
      
     @memo Options used in djvutotiffg4 function 
*/

struct djvutotiffg4_options_struct
{
/** This keeps a string delimited by hypens(-) and commas(,) */
  const char *page_range;
/** This option should be non-zero, if we want to force the application
    to avoid color images */
  int disable_mask;
/** logfile should be non-NULL to print verbose processing details */
  FILE *logfile;
/** helpfile should be non-NULL to print usage instructions */
  FILE *helpfile;
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
typedef struct djvutotiffg4_options_struct djvutotiffg4_options;

struct djvu_parse;

/** @name djvutotiffg4_options_alloc function 
    This is the primary allocation routine for djvutotiffg4_options.
    Even if the values specified are illegal, an options structure
    will be returned. */
djvutotiffg4_options *
djvutotiffg4_options_alloc(struct djvu_parse *,int,const char * const argv[]);

/** @name djvutotiffg4_options_free function
    Deallocates the fields of the djvutotiffg4_options structure.
    You should always use the free option, even if you did not use alloc
    so the data pointed to by priv is freed. */
void djvutotiffg4_options_free(djvutotiffg4_options *);

/** @name djvutotiffg4 function 
    This function converts the Tiff G4 input files to multipage DjVu document
    according to options structure.
    The function mainly uses  \Ref{DjVu_MTiffStream.h} for decoding, while 
    \Ref{DjVu_PixImage.h} for transformations and \Ref{DjVmDoc.h}, 
    \Ref{JB2Matcher.h} for encoding.  A non-zero return value indicates a
    fatal error. */
int djvutotiffg4(djvutotiffg4_options[]);

/** A non-zero value indicates there are error messages.  Error
    messages are generated for both fatal errors, and errors
    that are recovered from.  */
int djvutotiffg4_haserror(const djvutotiffg4_options []);

/** Returns a string of the first error message on the stack.  Each
    call erases the previous return value. */
const char * djvutotiffg4_error(djvutotiffg4_options []);

/** Prints all the errors to stderr */
void djvutotiffg4_perror(djvutotiffg4_options [],const char *);

/** This will print usage instructions to the specified output. */
void djvutotiffg4_usage(FILE *out,const char *prog);

#ifdef DOCXX_CODE
//@}
#endif /*DOCXX_CODE*/

#endif /* _DJVUTIFFG4_H_ */

