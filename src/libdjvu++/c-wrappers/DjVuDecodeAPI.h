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
//C- $Id: DjVuDecodeAPI.h,v 1.3 2000-01-07 22:44:29 orost Exp $
#endif

#ifndef _DJVU_DECODE_API_H
#define _DJVU_DECODE_API_H

/* 
 * $Log: DjVuDecodeAPI.h,v $
 * Revision 1.3  2000-01-07 22:44:29  orost
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
     PNM, BMP, TIFF, JPEG etc.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "DjVu.h"

#ifdef __cplusplus
extern "C" {
#endif


typedef enum {
  asciiBW='1',
  asciiGRAY,
  asciiCOLOR,
  binaryBW,
  binaryGRAY,
  binaryCOLOR
} pnm;


 /* Mirror and Rotate90 is added to represent the orientation of any tiff
  * image.
	*/
#define 						 DJVU_PIXEL_MIRROR        0x20
#define 						 DJVU_PIXEL_ROTATE90_CW   0x40
#define              DJVU_PIXEL_BOTTOM_UP     0x02

// This class is for PNM IO.
/*
 * DjVu Image Formats
 * 	See DjVuAPI-2_0.html#DjVu Image Formats
 *
 *      djvu_pixel_image
 *		See DjVuAPI-2_0.html#djvu_pixel_image
 */
             typedef struct djvu_pixel_image_struct
             {
               int flags;
#define              DJVU_PIXEL_RGB        0x04
#define              DJVU_PIXEL_BGR        0x08
#define              DJVU_PIXEL_GRAY       0x10
 /* Black and white is not officially supported since this a format for
  * gray scale and color documents.  But this flag is set when reading
  * Pnm files, so you know to convert to djvu_run_image format.  To test, use
  *    ((image->flags&DJVU_PIXEL_BLACKANDWHITE)==DJVU_PIXEL_BLACKANDWHITE)
  */
#define              DJVU_PIXEL_BLACKANDWHITE (0x20|DJVU_PIXEL_GRAY)
#define		     DJVU_PIXEL_HAS_DPI 0x40 /* This has a dpi field */
               int w;               /* Image width in Pixels */
               int h;               /* Image height in Pixels */
               size_t offset;       /* Byte offset to start of image in data pointer */
               size_t rowsize;      /* Bytes per row. */
               size_t pixsize;      /* Bytes per pixel */
               unsigned char *data; /* Pointer to image data */
               void *privdata;      /* reserved for internal use */
               unsigned int xdpi;   /* The X image resolution */ 
               unsigned int ydpi;   /* The Y image resolution */ 
#ifdef __cplusplus
                djvu_pixel_image_struct();
#endif
             }
             djvu_pixel_image;
#ifdef __cplusplus
             inline
             djvu_pixel_image_struct::djvu_pixel_image_struct()
             : flags(0), w(0), h(0), offset(0), rowsize(0), pixsize(0),
               data(0), privdata(0), xdpi(0), ydpi(0) {}
#endif
/* 
 *
 *           Deallocation:
 */
                  DJVUAPI void
                  djvu_pixel_image_free(djvu_pixel_image *);
/* 
 *                Deallocates any allocated pointers and the
 *                structure.  This should only be used when the
 *                structure was allocated by another DjVu API
 *                call.  Users may use the command:
 */
		  DJVUAPI djvu_pixel_image *
		  djvu_pixel_image_allocate(int rows,int cols,int grayscale);

typedef enum{DECODE_MASK, DECODE_FOREGROUND, DECODE_BACKGROUND, DECODE_ALL} Layer;
typedef enum{PNM, BMP, PS, TIFF, JPEG} OUTFORMAT;
enum{percent=100};

typedef struct{
   struct djvu_option* long_options;
   int page;
   unsigned char mode, subsample, vflip, hflip, togray, verbose;
   char *profile, *resize, *segment;
   unsigned int angle, jpeg_quality;
   char *inFile, *outFile;
   int optind;
   djvu_pixel_image* pimg;
   unsigned int Xsize,Xsubsample,Xupsample;
   unsigned int Ysize,Ysubsample,Yupsample;
   unsigned int xsiz, ysiz;
   int xmin, ymin, seg_w, seg_h;
   void *priv;
}djvu_decode_options;


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

/* Sets the long options pointer to one of 'all_options', 'non_jpeg_options', 'pgm_options' or 'pbm_options' */
DJVUAPI
void djvu_decode_set_long_options(djvu_decode_options *d_obj, struct djvu_option *long_opts);

/* Reads the command line options as per the long_options */
DJVUAPI
void djvu_decode_read_options(djvu_decode_options *d_obj, void *opts, int argc, char *argv[]);

/* Decodes and then renders the DjVu image by taking the pointer to decoder object
   and page number to decode as input arguments */
DJVUAPI
djvu_pixel_image* djvu_decode_and_render(djvu_decode_options *d_obj, int page_num);

/* Transforms the  decoded image */
DJVUAPI
djvu_pixel_image* djvu_decode_do_transform(djvu_decode_options *d_obj);

/* Writes the final image to a file in the requested format */
DJVUAPI
void djvu_decode_write_image(djvu_decode_options *d_obj, djvu_pixel_image *pimg, OUTFORMAT fmt);

/* Frees the decoder object */
DJVUAPI
void djvu_decode_options_free(djvu_decode_options* d_obj);

/*
   This function sets the segment and full rectangles. The full rectangle is the rectangle
   into which the djvu image is decoded to and segment rectanlge is the rectangle which
   is a portion of the full rect. The final output image will have dimensions as that of
   segment rectangle. 'xmin' and 'ymin' represent the bottom left corner of the rectangle
   and w, h represet the 'width' and 'height' of the rectangle. 'xmin' and 'ymin' for the
   full rectangle should always be zero. Otherwise exception will be thrown.
*/
DJVUAPI
void djvu_decode_set_rectangles(djvu_decode_options *d_obj,int seg_xmin,int seg_ymin,int seg_w,int seg_h,
                                int all_xmin, int all_ymin, int all_w, int all_h);


/*
   Sets the VFLIP option for vertical flip to ON if the 
   input argument 'vf' is non-zero else sets it to OFF.
*/
DJVUAPI
void djvu_decode_set_vflip(djvu_decode_options *d_obj, char vf);

/*
   Sets the HFLIP option for horizontal flip to ON if the 
   input argument 'hf' is non-zero else sets it to OFF.
*/
DJVUAPI
void djvu_decode_set_hflip(djvu_decode_options *d_obj, char hf);

/* 
   Sets the GRAY option for color to gray image converion to ON if
   the input argument 'gray' is non-zero else sets it to OFF.
*/
DJVUAPI
void djvu_decode_set_gray(djvu_decode_options *d_obj, char gray);

/*
   Sets the angle of rotation to 'ang'. The output image will
   be oriented at an angle of 'ang' with respect to the original
   image. The input angles must be multiples of 90.
*/
DJVUAPI
void djvu_decode_set_angle(djvu_decode_options *d_obj, int ang);

/*
   Sets the 'input djvu file' to 'inf'. This facilitates the
   the use of same decoder object with same command line options
   to decode various djvu images.
*/
DJVUAPI
void djvu_decode_change_infile(djvu_decode_options *d_obj, char *inf);

/* Sets the 'output file' to 'outf'. */
DJVUAPI
void djvu_decode_change_outfile(djvu_decode_options *d_obj, char *outf);

/* Returns the current input file name */
DJVUAPI
char* djvu_decode_get_infile(djvu_decode_options *d_obj);

/* Returns the current output file name */
DJVUAPI
char* djvu_decode_get_outfile(djvu_decode_options *d_obj);

/*
   Sets the layer of the input djvu image to be decoded
   to one of DECODE_MASK for black & white mask layer,
   DECODE_FOREGROUND for foreground layer and DECODE_BACKGROUND
   to background layer.
*/
DJVUAPI
void djvu_decode_set_layer(djvu_decode_options *d_obj, Layer l);

/*
   Returns a non-zero value if any errors occur during decoding
   and writing the output to the specified format.
*/
DJVUAPI
int djvu_decode_has_error(djvu_decode_options *d_obj);

/*
   Returns the recent error from the list of errors and
   deletes it from the list of errors.
*/
DJVUAPI
const char* djvu_decode_error(djvu_decode_options *d_obj);

/*
   Prints all the errors in the list to stderr.
*/
DJVUAPI
void djvu_decode_perror(djvu_decode_options *d_obj);



#ifdef __cplusplus
};
#endif

#endif // _DJVU_DECODEAPI_H
