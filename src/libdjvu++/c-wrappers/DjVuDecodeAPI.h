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
//C- $Id: DjVuDecodeAPI.h,v 1.13 2000-01-21 02:47:51 bcr Exp $
#endif

#ifndef _DJVU_DECODE_API_H
#define _DJVU_DECODE_API_H

/* 
 * $Log: DjVuDecodeAPI.h,v $
 * Revision 1.13  2000-01-21 02:47:51  bcr
 *
 * I forgot the RGB test as part of the isNative() test.
 *
 * Revision 1.12  2000/01/18 19:53:31  praveen
 * updated
 *
 * Revision 1.11  2000/01/17 07:34:15  bcr
 * Added GBitmap and GPixmap support to the libimage library.  There
 * is definitely some sort of heap corruption in libdjvu++, but I still
 * haven't found it.  Even something simple like set_grays() causing
 * problems.
 *
 * Revision 1.10  2000/01/16 13:13:54  bcr
 * Added a get_info() option to the Stream class.
 *
 * I found the orientation flags is ignored by most unix programs, so the
 * tiff images are now  oriented manually.
 *
 */

/** This file contains the definitions needed to decode images,
    and to manipulate the decoded images, and functions to export
    the decoding images to files.
*/
#include "DjVu.h"
#include <stdlib.h>

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
  DJVU_UNKNOWN=~0,/* Not known (yet) */
  DJVU_RLE=0x0,  /* black and white  (only valid for rle images) */
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
                 // Gets the width using bottom up cooridinates.
               inline int get_width(void) const;
                 // Gets the height using bottom up cooridinates.
               inline int get_height(void) const;
                 // Gets the horizontal DPI using bottom up cooridinates.
               inline unsigned int get_xdpi(void) const;
                 // Gets the vertical DPI using bottom up cooridinates.
               inline unsigned int get_ydpi(void) const;
                 // Tests if this image is in the "native" format.
               inline bool isNative(void) const;
                 // Does a rotate of 0,90,180, or 270 degress.
               inline void Rotate(const int angle);
		 // This does a vertical flip in the raw coordinate system.
               inline void VFlip(void);
                 // This flips using corrected bottom up cooridinates.
               inline void HFlip(void);
                 // This crops using cooridinates in the raw coordinate system.
               inline void CropRaw(const int x,const int y,
                 const unsigned int width,const unsigned int height);
                 // This crops using corrected bottom up cooridinates.
               inline void Crop(const int x,const int y,
                 const unsigned int width,const unsigned int height);
#endif
             }
             djvu_image;

#define DJVU_IMAGE_GET_WIDTH(image) \
     (((image)->orientation&DJVU_ROTATE90_CW)?(image)->h:(image)->w)

#define DJVU_IMAGE_GET_HEIGHT(image) \
     (((image)->orientation&DJVU_ROTATE90_CW)?(image)->w:(image)->h)

#define DJVU_IMAGE_GET_XDPI(image) \
     (((image)->orientation&DJVU_ROTATE90_CW)?(image)->ydpi:(image)->xdpi)

#define DJVU_IMAGE_GET_YDPI(image) \
     (((image)->orientation&DJVU_ROTATE90_CW)?(image)->xdpi:(image)->ydpi)

#define DJVU_IMAGE_IS_NATIVE(image) \
  (((image)->type==DJVU_RLE)?((image)->orientation==DJVU_TDLRNR):\
  (((image)->orientation==DJVU_BULRNR)&&((image)->type!=DJVU_RGB)&&\
    ((image)->pixsize==(unsigned int)((image)->type==DJVU_GRAY)?1:3)))

#define DJVU_IMAGE_ROTATE(image,angle) \
     { \
       djvu_image *IMAGE=(image); \
       int a;for(a=(((angle)%360)+405)%360;a>90;a-=90) \
         IMAGE->orientation^=((IMAGE->orientation&DJVU_ROTATE90_CW)? \
           (DJVU_BOTTOM_UP|DJVU_MIRROR|DJVU_ROTATE90_CW): \
             DJVU_ROTATE90_CW); \
     }

#define DJVU_IMAGE_VFLIP(image) \
     {(image)->orientation^=DJVU_BOTTOM_UP;}

#define DJVU_IMAGE_HFLIP(image) \
     {(image)->orientation^=DJVU_MIRROR;}

#define DJVU_IMAGE_CROP_RAW(image,x0,y0,width,height) \
     { \
       djvu_image *IMAGE=(image); \
       IMAGE->data+=(x0)*(IMAGE->pixsize)+(y0)*(IMAGE->rowsize); \
       IMAGE->w=(width); \
       IMAGE->h=(height); \
     }

#define DJVU_IMAGE_CROP(image,x0,y0,width,height) \
     { \
       djvu_image *IMAGE=(image); \
       int o=IMAGE->orientation,x=(x0),y=(y0),xw=(width),xh=(height); \
       if(o&DJVU_ROTATE90_CW) \
         DJVU_IMAGE_CROP_RAW(IMAGE,((o&DJVU_BOTTOM_UP)?(h-y-xh):y),((o&DJVU_MIRROR)?(w-x-xw):x),xh,xw) \
       else \
         DJVU_IMAGE_CROP_RAW(IMAGE,((o&DJVU_MIRROR)?(w-x-xw):x),((o&DJVU_BOTTOM_UP)?y:(h-y-xh)),xw,xh) \
     }

#ifdef __cplusplus
/** If you are using C++, you can use all the following methods
    to access and/or modify the djvu_image structure.  If you are using
    C, then you'll have to use the macros defined above...  */

               // This is just a simple constructor that zero's the values.
             inline djvu_image_struct::djvu_image_struct()
             : type(DJVU_UNKNOWN),orientation(0),w(0),h(0),pixsize(0),
               datasize(0),xdpi(0),ydpi(0),data(0),start_alloc(0),priv(0) {}
               // Gets the width using bottom up cooridinates.
             inline int djvu_image_struct::get_width(void) const
             {return DJVU_IMAGE_GET_WIDTH(this);}
               // Gets the height using bottom up cooridinates.
             inline int djvu_image_struct::get_height(void) const
             {return DJVU_IMAGE_GET_HEIGHT(this);}
               // Gets the horizontal DPI using bottom up cooridinates.
             inline unsigned int djvu_image_struct::get_xdpi(void) const
             {return DJVU_IMAGE_GET_XDPI(this);}
               // Gets the vertical DPI using bottom up cooridinates.
             inline unsigned int djvu_image_struct::get_ydpi(void) const
             {return DJVU_IMAGE_GET_YDPI(this);}
               // This tests if the image is in the "native" format.
             inline bool djvu_image_struct::isNative(void) const
             {return DJVU_IMAGE_IS_NATIVE(this);}
               // Does a rotate of 0,90,180, or 270 degress.
             inline void djvu_image_struct::Rotate(const int angle)
             DJVU_IMAGE_ROTATE(this,angle)
               // This flips using corrected bottom up cooridinates.
             inline void djvu_image_struct::VFlip(void)
             DJVU_IMAGE_VFLIP(this)
               // This flips using corrected bottom up cooridinates.
             inline void djvu_image_struct::HFlip(void)
             DJVU_IMAGE_HFLIP(this)
               // This crops using cooridinates in the raw coordinate system.
             inline void djvu_image_struct::CropRaw(const int x0,const int y0,
               const unsigned int width,const unsigned int height)
             DJVU_IMAGE_CROP_RAW(this,x0,y0,width,height)
               // This crops using corrected bottom up cooridinates.
             inline void djvu_image_struct::Crop(const int x0,const int y0,
               const unsigned int width,const unsigned int height)
             DJVU_IMAGE_CROP(this,x0,y0,width,height)
#endif


/** This free's an image allocated by the DjVu libraries.
 */
DJVUAPI void 
djvu_image_free(djvu_image *ximg);

/** This free's only he raw image data, not the structure itself.
 */
DJVUAPI void
djvu_image_free_data(djvu_image ximg[1]);

/** This routine allocates memory for the specified image.
    An error is indicated by a NULL return.  No message is
    returned.
 */
DJVUAPI djvu_image *
djvu_image_allocate(unsigned int cols,unsigned int rows,size_t datasize);

/**  This routine converts the image to bottom up orientation for
    color and gray images, top down for RLE data.  The alpha channel,
    and all other padding bits are removed.  When possible, the image
    is reallocated to the amount of memory actually used.
 */
DJVUAPI djvu_image *
djvu_image_native(djvu_image *ximg,char *ebuf,size_t ebuf_size);
  
/** This routine copies the image to bottom up orientation for
    color and gray images, top down for RLE data.  The alpha channel,
    and all other padding bits are removed.
 */
DJVUAPI djvu_image *
djvu_image_copy_native(const djvu_image *ximg,char *ebuf,size_t ebuf_size);

/** This converts images to gray scale.
 */
DJVUAPI djvu_image *
djvu_image_gray(djvu_image *ximg,char *ebuf,size_t ebuf_size);

/** This copies an image in gray scale.
 */
DJVUAPI djvu_image *
djvu_image_copy_gray(const djvu_image *ximg,char *ebuf,size_t ebuf_size);

/** This run length encodes an image.  This means the image is converted
    to bitonal black and white by comparing each pixel to the specified
    threshold, with values below the threshold becoming black and above
    white.  The pixels are then encoded by storing the number of
    consecutive pixels of the same color, instead of bit mapping each
    pixel.
 */
DJVUAPI djvu_image*
djvu_image_rle(
  djvu_image *ximg,const int threshold,char *ebuf,size_t ebuf_size);

/** This run length encodes a copy of the image image.  This means the
    image is converted to bitonal black and white by comparing each pixel
    to the specified threshold, with values below the threshold becoming
    black and above white.  The pixels are then encoded by storing the
    number of consecutive pixels of the same color, instead of bit mapping
    each pixel.
 */
DJVUAPI djvu_image*
djvu_image_copy_rle(
  const djvu_image *ximg,const int threshold,
  char *ebuf,size_t ebuf_size);

/** This makes a copy of the image, resized to at the specified width and
    height.
 */
DJVUAPI djvu_image *
djvu_image_copy_resize(
  const djvu_image *ximg,const unsigned int width,const unsigned int height,
  char *ebuf,size_t ebuf_size);

/** This resizes the image to the specified width and height.
 */
DJVUAPI djvu_image *
djvu_image_resize(
  djvu_image *ximg,const unsigned int width,const unsigned int height,
  char *ebuf,size_t ebuf_size);

/** This makes a copy of the image headers, with the flags changed to
    the specified rotation.
 */
DJVUAPI void
djvu_image_const_rotate(const djvu_image *in_img, djvu_image *out_img, int angle);

/** This changes the flags to indicate a rotate.  It is exactly the same
    as DJVU_IMAGE_ROTATE(ximg,angle)
 */
DJVUAPI void
djvu_image_rotate(djvu_image *ximg,int angle);

/** This makes a copy of the image headers, with the flags changed to
    the specified crop size.
 */
DJVUAPI void
djvu_image_const_crop(
  const djvu_image *in_img, djvu_image *out_img, int x0,int y0,
  const unsigned width,const unsigned int height);

/** This changes the flags to indicate a crop.  It is exactly the same
    as DJVU_IMAGE_CROP(ximg,x0,y0,width,height)
 */
DJVUAPI void
djvu_image_crop(
  djvu_image *ximg,int x0,int y0,
  const unsigned width,const unsigned int height);

/** This makes a copy of the image headers, with the flags changed to
    indicate a vflip operation.
 */
DJVUAPI void 
djvu_image_const_vflip(const djvu_image *in_img, djvu_image *out_img);

/** This changes the flags to indicate a vflip.  It is exactly the same
    as DJVU_IMAGE_VFLIP(ximg)
 */
DJVUAPI void
djvu_image_vflip(djvu_image *ximg);

/** This makes a copy of the image headers, with the flags changed to
    indicate a hflip operation.
 */
DJVUAPI void 
djvu_image_const_hflip(const djvu_image *in_img, djvu_image *out_img);

/** This changes the flags to indicate a vflip.  It is exactly the same
    as DJVU_IMAGE_HFLIP(ximg)
 */
DJVUAPI void
djvu_image_hflip(djvu_image *ximg);

/* This is a BIG mess that still needs to be cleaned up!!! 
 */
typedef enum
  {DECODE_MASK, DECODE_FOREGROUND, DECODE_BACKGROUND, DECODE_ALL} Layer;
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
