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
//C- "$Id: djvubitonal.h,v 1.1 2000-01-07 21:48:24 eaf Exp $"
//C- -- Bitonal To DjVu
//C- Author: Parag Deshmukh (Dec 99), Andrei Erofeev (Jan 2000)
#endif  /* __cplusplus */

#ifndef _DJVUBITONAL_H_
#define _DJVUBITONAL_H_

#include <stdio.h>

/** @name djvubitonal.h
      functions used to convert multiple bitonal images to DjVu multipage 
      documents.
*/

#ifdef DOCXX_CODE
//@{
#endif /*DOCXX_CODE*/

/** @name bitonaltodjvu_options struct
      
     @memo Options used in bitonaltodjvu function 
*/

enum bitonaltodjvu_type_enum
{
  djvu_normal,
  djvu_conservative,
  djvu_lossless,
  djvu_aggressive,
  djvu_pseudo
};
typedef enum bitonaltodjvu_type_enum bitonaltodjvu_type;

struct bitonaltodjvu_options_struct
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
  bitonaltodjvu_type compression;
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
typedef struct bitonaltodjvu_options_struct bitonaltodjvu_options;

struct djvu_parse;

/** @name bitonaltodjvu_options_alloc function 
    This is the primary allocation routine for bitonaltodjvu_options.
    Even if the values specified are illegal, an options structure
    will be returned. */
bitonaltodjvu_options *
bitonaltodjvu_options_alloc(struct djvu_parse *,int,const char * const argv[]);

/** @name bitonaltodjvu_options_free function
    Deallocates the fields of the bitonaltodjvu_options structure.
    You should always use the free option, even if you did not use alloc
    so the data pointed to by priv is freed. */
void bitonaltodjvu_options_free(bitonaltodjvu_options *);

/** @name bitonaltodjvu function 
    This function converts the bitonal input files to a multipage DjVu document
    according to the options structure.
    Depending on the type of the input data, the function uses the proper
    stream derived from \Ref{DjVu_Stream} for decoding, while 
    \Ref{DjVu_PixImage.h} for transformations and \Ref{DjVmDoc.h}, 
    \Ref{JB2Matcher.h} for encoding.  A non-zero return value indicates a
    fatal error. */
int bitonaltodjvu(bitonaltodjvu_options[]);

/** A non-zero value indicates there are error messages.  Error
    messages are generated for both fatal errors, and errors
    that are recovered from.  */
int bitonaltodjvu_haserror(const bitonaltodjvu_options []);

/** Returns a string of the first error message on the stack.  Each
    call erases the previous return value. */
const char * bitonaltodjvu_error(bitonaltodjvu_options []);

/** Prints all the errors to stderr */
void bitonaltodjvu_perror(bitonaltodjvu_options []);

/** This will print usage instructions to the specified output. */
void bitonaltodjvu_usage(FILE *out,const char *prog);

#ifdef DOCXX_CODE
//@}
#endif /*DOCXX_CODE*/


#ifdef DOCXX_CODE
//@{
#endif /*DOCXX_CODE*/

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
typedef struct djvutobitonal_options_struct djvutobitonal_options;

struct djvu_parse;

/** @name djvutobitonal_options_alloc function 
    This is the primary allocation routine for djvutobitonal_options.
    Even if the values specified are illegal, an options structure
    will be returned. */
djvutobitonal_options *
djvutobitonal_options_alloc(struct djvu_parse *,int,const char * const argv[]);

/** @name djvutobitonal_options_free function
    Deallocates the fields of the djvutobitonal_options structure.
    You should always use the free option, even if you did not use alloc
    so the data pointed to by priv is freed. */
void djvutobitonal_options_free(djvutobitonal_options *);

/** @name djvutobitonal function 
    This function converts the source multipage DjVu document to
    a bitonal image according to options structure. */
int djvutobitonal(djvutobitonal_options[]);

/** A non-zero value indicates there are error messages.  Error
    messages are generated for both fatal errors, and errors
    that are recovered from.  */
int djvutobitonal_haserror(const djvutobitonal_options []);

/** Returns a string of the first error message on the stack.  Each
    call erases the previous return value. */
const char * djvutobitonal_error(djvutobitonal_options []);

/** Prints all the errors to stderr */
void djvutobitonal_perror(djvutobitonal_options []);

/** This will print usage instructions to the specified output. */
void djvutobitonal_usage(FILE *out,const char *prog);

#ifdef DOCXX_CODE
//@}
#endif /*DOCXX_CODE*/

#endif /* _DJVUBITONAL_H_ */

