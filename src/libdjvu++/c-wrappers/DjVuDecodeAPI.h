#ifdef __cplusplus
//C-  -*- C++ -*-
//C-
//C- Copyright (c) 1999 AT&T Corp.  All rights reserved.
//C-
//C- This software may only be used by you under license from AT&T
//C- Corp. ("AT&T"). A copy of AT&T's Source Code Agreement is available at
//C- AT&T's Internet website having the URL <http://www.djvu.att.com/open>.
//C- If you received this software without first entering into a license with
//C- AT&T, you have an infringing copy of this software and cannot use it
//C- without violating AT&T's intellectual property rights.
//C-
//C- $Id: DjVuDecodeAPI.h,v 1.7 2000-01-13 16:04:01 bcr Exp $
#endif

#ifndef _DJVU_DECODE_API_H
#define _DJVU_DECODE_API_H

/* 
 * $Log: DjVuDecodeAPI.h,v $
 * Revision 1.7  2000-01-13 16:04:01  bcr
 * Changed the djvu_image flags field to type and orientation.
 *
 * Revision 1.6  2000/01/12 16:15:21  bcr
 * Unified the C interface names without _pixel, and updated libddjvu
 *
 * Revision 1.5  2000/01/12 14:29:31  bcr
 * Removed djvu_run_image and exposed more of GBitmap to the rest of the libraries.
 *
 * Revision 1.4  2000/01/08 22:44:32  parag
 * TIFF enum replaced with MTIFF as it conflicted with libtiff symbol
 *
 * Revision 1.3  2000/01/07 22:44:29  orost
 * added comments
 *
 * Revision 1.2  2000/01/07 16:58:50  praveen
 * updated
 *
 * Revision 1.1  2000/01/06 20:10:44  praveen
 * added DjVu decode interface file
 *
 * Revision 1.2  2000/01/06 16:18:30  praveen
 * updated
 *
 */

/*
     This file contains the decode, rendering and output functions
     needed to decode DjVu images to various other formats like,
     PNM, BMP, MTIFF, JPEG etc.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "DjVu.h"

#ifdef __cplusplus
extern "C" {
#endif


typedef enum
{
  asciiBW='1',
  asciiGRAY,
  asciiCOLOR,
  binaryBW,
  binaryGRAY,
  binaryCOLOR
} pnm;


/* These are the various color formats that may be specified in the flag
 * portion of the image types
 */
typedef enum djvu_image_types_enum
{
  DJVU_RLE=0,    /* black and white  (only valid for rle images) */
  DJVU_GRAY=0x1, /* gray (only valid for color images) */
  DJVU_RGB=0x2,  /* rgb (only valid for color images) */
  DJVU_BGR=0x3   /* bgr color (only valid for color images) */
} djvu_image_types;

 /* There are 8 possible image orientations.
  */
enum djvu_image_orientations_bits
{
  DJVU_BOTTOM_UP=0x1,
  DJVU_MIRROR=0x2,
  DJVU_ROTATE90_CW=0x4
};

 /* This is a listing of all eight combinations, usefull for switch()
  * statements. 
  */
enum djvu_image_orientations
{
  DJVU_TDLRNR=0,                                          /* 0 */
  DJVU_BULRNR=DJVU_BOTTOM_UP,                             /* 1 */
  DJVU_TDRLNR=DJVU_MIRROR,                                /* 2 */
  DJVU_BURLNR=DJVU_MIRROR|DJVU_BOTTOM_UP,                 /* 3 */
  DJVU_TDLRCW=DJVU_ROTATE90_CW,                           /* 4 */
  DJVU_BULRCW=DJVU_ROTATE90_CW|DJVU_BOTTOM_UP,            /* 5 */
  DJVU_TDRLCW=DJVU_ROTATE90_CW|DJVU_MIRROR,               /* 6 */
  DJVU_BURLCW=DJVU_ROTATE90_CW|DJVU_MIRROR|DJVU_BOTTOM_UP /* 7 */
};

/* This structure is for internal use...
 */
struct _djvu_image_priv;
typedef struct _djvu_image_priv * djvu_image_priv;

/*
 * DjVu Image Formats
 * 	See DjVuAPI-2_0.html#DjVu Image Formats
 *
 *      djvu_image
 *		See DjVuAPI-2_0.html#djvu_image
 */
             typedef struct djvu_image_struct
             {
               djvu_image_types type; /* RLE,GRAY,RGB, or BGR */
               int orientation;     /* We don't use the enum defined above */
				    /* since |, &, and ^ require int types. */
               int w;               /* Width in Pixels (current orientation)*/
               int h;               /* Height in Pixels (current orientation)*/
               size_t pixsize;      /* Bytes per pixel (includes padding) */
               size_t rowsize;      /* Bytes per row (includes padding) */
               size_t datasize;     /* Total Bytes (includes padding) */
               unsigned int xdpi;   /* The X image resolution */ 
               unsigned int ydpi;   /* The Y image resolution */ 
               unsigned char *data; /* Pointer to image data */
               unsigned char *start_alloc;/* To the actual allocation point. */
               djvu_image_priv priv;
#ifdef __cplusplus
               djvu_image_struct();
               inline int get_width(void) const
               {return(orientation&DJVU_ROTATE90_CW)?h:w;}
               inline int get_height(void) const
               {return(orientation&DJVU_ROTATE90_CW)?w:h;}
               inline unsigned int get_xdpi(void) const
               {return(orientation&DJVU_ROTATE90_CW)?ydpi:xdpi;}
               inline unsigned int get_ydpi(void) const
               {return(orientation&DJVU_ROTATE90_CW)?xdpi:ydpi;}
#endif
             }
             djvu_image;
#ifdef __cplusplus
             inline
             djvu_image_struct::djvu_image_struct()
             : type(DJVU_RLE),orientation(0),w(0),h(0),pixsize(0),datasize(0),xdpi(0),ydpi(0),
               data(0),start_alloc(0),priv(0) {}
#endif
/* 
 *
 *           Deallocation:
 */
                  DJVUAPI void
                  djvu_image_free(djvu_image *);
/* 
 *                Deallocates any allocated pointers and the
 *                structure.  This should only be used when the
 *                structure was allocated by another DjVu API
 *                call.  Users may use the command:
 */
		  DJVUAPI djvu_image *
		  djvu_image_allocate(int rows,int cols,int pixsize);

/* This is a BIG mess that still needs to be cleaned up!!! 
 */
typedef enum{DECODE_MASK, DECODE_FOREGROUND, DECODE_BACKGROUND, DECODE_ALL} Layer;
typedef enum{PNM, BMP, PS, MTIFF, JPEG} OUTFORMAT;
enum{percent=100};

typedef struct
{
   struct djvu_option* long_options;
   int page;
   unsigned char mode, subsample, vflip, hflip, togray, verbose;
   char *profile, *resize, *segment;
   unsigned int angle, jpeg_quality;
   char *inFile, *outFile;
   int optind;
   djvu_image* ximg;
   unsigned int Xsize,Xsubsample,Xupsample;
   unsigned int Ysize,Ysubsample,Yupsample;
   unsigned int xsiz, ysiz;
   int xmin, ymin, seg_w, seg_h;
   void *priv;
} djvu_decode_options;


/*
    All the djvutoxxxx_decode_options_alloc() functions take a pointer to
    DjVuPaseOptions object(passed as a void pointer), argc and argv as
    arguments. If there is no existing DjVuParseOptions Object, pass null
    value as the 1st argument, in which case the functions create one
    DjVuParseOptions object from the input argc and argv and read the
    options

*/
DJVUAPI
djvu_decode_options* djvutopnm_decode_options_alloc(void *opts, int argc, char *argv[]);
DJVUAPI
djvu_decode_options* djvutopgm_decode_options_alloc(void *opts, int argc, char *argv[]);
DJVUAPI
djvu_decode_options* djvutopbm_decode_options_alloc(void *opts, int argc, char *argv[]);
DJVUAPI
djvu_decode_options* djvutops_decode_options_alloc(void *opts, int argc, char *argv[]);
DJVUAPI
djvu_decode_options* djvutobmp_decode_options_alloc(void *opts, int argc, char *argv[]);
DJVUAPI
djvu_decode_options* djvutotiff_decode_options_alloc(void *opts, int argc, char *argv[]);
DJVUAPI
djvu_decode_options* djvutojpeg_decode_options_alloc(void *opts, int argc, char *argv[]);

/*
    All the djvu_decode_to_xxxx() functions take the decoder object as an
    input argument and perform the necessary conversions and output the
    image in respective formats.
    
    One should call djvu_decode_options_free() function to delete the
    decoder object, passing a pointer to decoder object as input argument.
    (i.e., pointer to djvu_decode_options object.)
*/
DJVUAPI
void djvu_decode_to_pnm(djvu_decode_options *d_obj);
DJVUAPI
void djvu_decode_to_pgm(djvu_decode_options *d_obj);
DJVUAPI
void djvu_decode_to_pbm(djvu_decode_options *d_obj);
DJVUAPI
void djvu_decode_to_bmp(djvu_decode_options *d_obj);
DJVUAPI
void djvu_decode_to_jpeg(djvu_decode_options *d_obj);
DJVUAPI
void djvu_decode_to_tiff(djvu_decode_options *d_obj);
DJVUAPI
void djvu_decode_to_ps(djvu_decode_options *d_obj);

/* 
   Following are the C wrapper functions to the member functions of
   the DjVuDecoder class. All these wrapper functions, except djvu_decode_options_alloc
   take the pointer to decoder object as an input argument 
*/

/* Creates and returns a decoder object */
DJVUAPI
djvu_decode_options* djvu_decode_options_alloc();

/* Sets the long options pointer to one of 'all_options',
   'non_jpeg_options', 'pgm_options' or 'pbm_options'
 */
DJVUAPI void
djvu_decode_set_long_options(
  djvu_decode_options *d_obj, struct djvu_option *long_opts);

/* Reads the command line options as per the long_options */
DJVUAPI void
djvu_decode_read_options(
  djvu_decode_options *d_obj, void *opts, int argc, char *argv[]);

/* Decodes and then renders the DjVu image by taking the pointer to decoder
   object and page number to decode as input arguments
 */
DJVUAPI djvu_image*
djvu_decode_and_render(djvu_decode_options *d_obj, int page_num);

/* Transforms the  decoded image  */
DJVUAPI djvu_image*
djvu_decode_do_transform(djvu_decode_options *d_obj);

/* Writes the final image to a file in the requested format */
DJVUAPI void
djvu_decode_write_image(
  djvu_decode_options *d_obj, djvu_image *ximg, OUTFORMAT fmt);

/* Frees the decoder object */
DJVUAPI void
djvu_decode_options_free(djvu_decode_options* d_obj);

/*
   This function sets the segment and full rectangles. The full rectangle
   is the rectangle into which the djvu image is decoded to and segment
   rectanlge is the rectangle which is a portion of the full rect. The final
   output image will have dimensions as that of segment rectangle. 'xmin' and
   'ymin' represent the bottom left corner of the rectangle and w, h represet
   the 'width' and 'height' of the rectangle. 'xmin' and 'ymin' for the full
   rectangle should always be zero. Otherwise exception will be thrown.
*/
DJVUAPI void
djvu_decode_set_rectangles(
  djvu_decode_options *d_obj,int seg_xmin,int seg_ymin,int seg_w,int seg_h,
  int all_xmin, int all_ymin, int all_w, int all_h);


/*
   Sets the VFLIP option for vertical flip to ON if the 
   input argument 'vf' is non-zero else sets it to OFF.
*/
DJVUAPI void
djvu_decode_set_vflip(djvu_decode_options *d_obj, char vf);

/*
   Sets the HFLIP option for horizontal flip to ON if the 
   input argument 'hf' is non-zero else sets it to OFF.
*/
DJVUAPI void
djvu_decode_set_hflip(djvu_decode_options *d_obj, char hf);

/* 
   Sets the GRAY option for color to gray image converion to ON if
   the input argument 'gray' is non-zero else sets it to OFF.
*/
DJVUAPI void
djvu_decode_set_gray(djvu_decode_options *d_obj, char gray);

/*
   Sets the angle of rotation to 'ang'. The output image will
   be oriented at an angle of 'ang' with respect to the original
   image. The input angles must be multiples of 90.
*/
DJVUAPI void
djvu_decode_set_angle(djvu_decode_options *d_obj, int ang);

/*
   Sets the 'input djvu file' to 'inf'. This facilitates the
   the use of same decoder object with same command line options
   to decode various djvu images.
*/
DJVUAPI void
djvu_decode_change_infile(djvu_decode_options *d_obj, char *inf);

/* Sets the 'output file' to 'outf'. */
DJVUAPI void
djvu_decode_change_outfile(djvu_decode_options *d_obj, char *outf);

/* Returns the current input file name */
DJVUAPI char*
djvu_decode_get_infile(djvu_decode_options *d_obj);

/* Returns the current output file name */
DJVUAPI char*
djvu_decode_get_outfile(djvu_decode_options *d_obj);

/*
   Sets the layer of the input djvu image to be decoded
   to one of DECODE_MASK for black & white mask layer,
   DECODE_FOREGROUND for foreground layer and DECODE_BACKGROUND
   to background layer.
*/
DJVUAPI void
djvu_decode_set_layer(djvu_decode_options *d_obj, Layer l);

/*
   Returns a non-zero value if any errors occur during decoding
   and writing the output to the specified format.
*/
DJVUAPI int
djvu_decode_has_error(djvu_decode_options *d_obj);

/*
   Returns the recent error from the list of errors and
   deletes it from the list of errors.
*/
DJVUAPI const char*
djvu_decode_error(djvu_decode_options *d_obj);

/*
   Prints all the errors in the list to stderr.
*/
DJVUAPI void
djvu_decode_perror(djvu_decode_options *d_obj);



#ifdef __cplusplus
};
#endif

#endif /* _DJVU_DECODEAPI_H */
