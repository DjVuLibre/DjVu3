/*C-  -*- C -*-
 *C-
 *C- This software may only be used by you under license from AT&T
 *C- Corp. ("AT&T"). A copy of AT&T's Source Code Agreement is available at
 *C- AT&T's Internet web site having the URL <http://www.djvu.att.com/open>.
 *C- If you received this software without first entering into a license with
 *C- AT&T, you have an infringing copy of this software and cannot use it
 *C- without violating AT&T's intellectual property rights.
 *C-
 *C- $Id: DjVuAPI.h,v 1.47 2000-02-29 07:04:48 bcr Exp $
 *
 * The main header file for the DjVu API
 */

#ifndef _DJVUAPI_H_
#define _DJVUAPI_H_

/* 
 * $Log: DjVuAPI.h,v $
 * Revision 1.47  2000-02-29 07:04:48  bcr
 * Added in an "optimization" that decreases the amount of preprocess time.
 * Fixed a bug in PnmStream rotated output.
 *
 * Revision 1.46  2000/02/29 03:39:07  bcr
 * Added a 'Strict' option to the Native() method.  Then this option is false we
 * don't require color and gray to be bottom-up.
 *
 * Revision 1.45  2000/02/28 04:07:57  bcr
 * Added missing functions.
 *
 * Revision 1.44  2000/02/26 18:53:20  bcr
 * Changes to the DOC++ comments.
 *
 */

#ifdef WIN32
#include <stddef.h>
#elif defined(__MWERKS__)
#include <stddef.h>
#else
#include <sys/types.h>
#endif

#include "DjVu.h"
#include "DjVuDecodeAPI.h"
#include "DjVuBitonalAPI.h"
#include "DjVuPhotoAPI.h"
#include "DjVuDocumentAPI.h"

/*
 *  ------------------------------------------------------------------------
 */
#ifdef __cplusplus
extern "C" {
#endif

#ifdef __notdef
           }
#endif


/* pre-declaration. */
#ifndef _DJVU_IMAGE_
#define _DJVU_IMAGE_
struct djvu_image_struct;
typedef struct djvu_image_struct djvu_image;
#endif

/*
 *  ------------------------------------------------------------------------
 * DYNAMIC LINK LIBRARY STUFF
 */

/*
 *  ------------------------------------------------------------------------
 * Basic IO interface.
 *
 *    The following routines create an instance of a djvuio structure
 *    defined as:
 */
struct djvuio_struct
{
  const char *filename;
  void *priv;
};
/* typedef struct djvuio_struct* djvu_import;
 * typedef struct djvuio_struct* djvu_export;
 *
 * In general import means to load a file from disk, and export means to
 * save a file to disk.  However, as you will see, it is possible to use
 * any type of device, not just a disk with this functions.
 * 
 * All of the following are legal values for the magic functions:
 */

typedef enum djvuio_type_enum
{
  DjVuIO_NONE=0,
  DjVuIO_PNM,
  DjVuIO_PPM,
  DjVuIO_PGM,
  DjVuIO_PBM,
  DjVuIO_BMP,
  DjVuIO_PICT,
  DjVuIO_PS,
  DjVuIO_PDF,
  DjVuIO_TIFF,
  DjVuIO_LIBTIFF,
  DjVuIO_JPEG,
  DjVuIO_GIF,
  DjVuIO_DJVU,
  DjVuIO_RAW,
  DjVuIO_CALLBACK,
  DjVuIO_UNKNOWN
} djvuio_type;


/* --- Open commands ---
 */

/* Open the named file for reading.
 */
DJVUAPI djvu_import
djvu_import_file( const char filename[]);

/* Open the named file for writing. 
 */
DJVUAPI djvu_export
djvu_export_file( const char filename[]);

/* Open a stream for reading from an already open fileno.  This can be any
 * device that is seekable.
 */
DJVUAPI djvu_import
djvu_import_fileno( int fileno);

/* Open a stream for writing from an already open fileno.  This can be any
 * device that is seekable.
 */
DJVUAPI djvu_export
djvu_export_fileno( int fileno);


/* Open a user allocated buffer as a stream for reading.  The buffer should
 * not be freed until after the djvu_import stream is closed.  This type of
 * stream is most useful for memmap files,  or files read in whole from a 
 * database.
 */
DJVUAPI djvu_import
djvu_import_buffer ( const void *buf, const size_t bufsize );


/* Open a stream for reading from user defined callbacks functions.
 */
typedef int djvu_input_sub ( void *arg, void *data, size_t len );
typedef int djvu_seek_sub ( void *arg, long offset, int whence );

DJVUAPI djvu_import
djvu_import_callback( void *arg,djvu_input_sub *inpf,djvu_seek_sub *seekf);


/* Open a stream for writing to user defined callback functions.
 */
typedef int djvu_output_sub ( void *arg, const void *data, size_t len );

DJVUAPI djvu_export
djvu_export_callback(void *arg,djvu_input_sub *inpf,djvu_seek_sub *seekf);


/* -- Close Commands --
 */

/* Close a stream with callbacks.
 */
DJVUAPI void
djvu_import_close( djvu_import);

DJVUAPI void
djvu_export_close( djvu_export);

/* -- Reading Images --
 * It is assumed all operations are sequential
 */

/* This call will return a constant image structure that fills the data
 * fields for the next image, such as width, and height, but not the actual
 * image data.  This is very useful for operations like skipping bitonal 
 * pages, or pages that will require too much memory to process.
 */
DJVUAPI const djvu_image *
djvu_dstream_image_info(djvu_import io);

/* This actually retrieves the image.  Normally the structure returned will
 * be the same as above, but with the data actually filled in.  However, in
 * some cases, we will learn more information about an image while reading it.
 * A typical example would be a bitonal BMP file that was stored in a color
 * format.  The djvu_dstream_image_info call would indicate the image is 
 * color, but when actually reading the image, we discover the image is 
 * really bitonal and automatically reduce it to the DJVU_RLE image format.
 */
DJVUAPI djvu_image *
djvu_dstream_to_image(djvu_import io);

/* When dealing with multi-page documents, this is the way to skip a page.
 * If the format allows, we will simply increment a page counter.  If the
 * format doesn't allow it, we may have to actually read in the whole image
 * just to find where the next image begins.
 */
DJVUAPI void
djvu_dstream_skip_image(djvu_import io);

/* djvu_dstream_to_image() and djvu_dstream_image_info will return a NULL
 * under two conditions.  The first, most common condition is that the
 * EOF has been reached.  The second less likely condition, is that the
 * page that was requested was in an unsupported input format.  The
 * djvu_dstream_is_eof() function will return non-zero if we have read
 * the file.  If we haven't read the EOF, then you can use the 
 * djvu_dstream_skip_image() function to skip the page that can not
 * be read.
 */
DJVUAPI int
djvu_dstream_is_eof(djvu_import io);

/* -- Writing Images --  
 * It is assumed all output is sequential.  If writing to a file type
 * that does not accept multiple pages, it is the users responsibility
 * to check that only one page is written.
 */

DJVUAPI int
djvu_image_to_dstream(djvu_export io, djvu_image img[1]);


/* -- Error Handling --
 */

/* Find out if a stream has errors.
 */
DJVUAPI int
djvu_import_haserror( djvu_import);

DJVUAPI int
djvu_export_haserror( djvu_export);



/* Retrieve an error message from the stream.
 */
DJVUAPI const char *
djvu_import_error( djvu_import);

DJVUAPI const char *
djvu_export_error( djvu_export);


/* Print all error messages from a stream.
 */
DJVUAPI void
djvu_import_perror( djvu_import,const char *);

DJVUAPI void
djvu_export_perror( djvu_export,const char *);


/* Find out what type of file is being used on the stream.  If the
 * known_type argument is anything other than DjVuIO_UNKNOWN, then an error
 * will be set on the stream (see the djvu_import_haserror() function)
 * if the file type is not what the user specified.  The file type is
 * normally determined by scanning the first few bytes of input.
 */
DJVUAPI djvuio_type
djvu_import_magic(djvu_import known_type);

/* Find out what type of file is being used on the stream.  If the
 * known_type argument the is anything other than DjVuIO_UNKNOWN, then the
 * export type will be set to the specified type.  Otherwise the filename
 * field of the stream structure will be checked for an obvious extension
 * such as .jpg, or .bmp.  If the extension is not recognized, then the
 * it will be an error to try and write to the stream without setting
 * the type of output to use.
 */
DJVUAPI djvuio_type
djvu_export_magic(djvu_export);


/*  ------------------------------------------------------------------------
 * DjVu Pixel/Run Length Encoded Image Format
 *
 *    The djvu_image structure, is the primary form of dealing in memory
 *    with renderable images.  In the previous version of the DjVuAPI, 
 *    the image format was known as djvu_run_image for black and white,
 *    and djvu_pixel_image for color images.  While often, the distinction
 *    between black and white and color is very important, we found that
 *    actually having two separate formats meant programmers had to learn
 *    twice as many function calls, and write nearly twice as much code.
 *
 *    The types of djvu_image formats are now defined with an enum value.
 *    as follows:
 */

typedef enum djvu_image_types_enum
{
  DJVU_UNKNOWN=~0,/* Not known (yet) */
  DJVU_RLE=0x0,  /* black and white  (only valid for rle images) */
  DJVU_GRAY=0x1, /* gray (only valid for color images) */
  DJVU_RGB=0x2,  /* rgb (only valid for color images) */
  DJVU_BGR=0x3   /* bgr color (only valid for color images) */
} djvu_image_types;

 /*   Code that depends on the image format can check the type in the
  *   djvu_image structure.
  *
  *   The other aspect of images that is important to understand is the
  *   orientation.  There are four possible rotation values for an image
  *   which are 0 degrees, 90 degrees, 180 degrees, and 270 degrees.
  *   In addition the image can be mirrored backwards in any of these
  *   orientations, giving a possible of 8 orientations.  To sanely deal
  *   with these orientations, we have defined 3 mutually exclusive 
  *   bits.
  */
enum djvu_image_orientations_bits
{
  DJVU_BOTTOM_UP=0x1,  /* Upside down */
  DJVU_MIRROR=0x2,     /* Written backwards. (right to left) */
  DJVU_ROTATE90_CW=0x4 /* rotated 90 degrees */
};

 /*
  * By using combinations of these bits we can defined all possible 8
  * orientations.
  */
enum djvu_image_orientations
{
  DJVU_TDLRNR=0,                                     /* normal orientation */
  DJVU_BULRNR=DJVU_BOTTOM_UP,                               /* upside down */
  DJVU_TDRLNR=DJVU_MIRROR,                    /* backwards (right to left) */
  DJVU_BURLNR=DJVU_MIRROR|DJVU_BOTTOM_UP,                    /* rotate 180 */
  DJVU_TDLRCW=DJVU_ROTATE90_CW,                              /* rotated 90 */
  DJVU_BULRCW=DJVU_ROTATE90_CW|DJVU_BOTTOM_UP, /* backwards and rotate 180 */
  DJVU_TDRLCW=DJVU_ROTATE90_CW|DJVU_MIRROR,     /* backwards and rotate 90 */
  DJVU_BURLCW=DJVU_ROTATE90_CW|DJVU_MIRROR|DJVU_BOTTOM_UP    /* rotate 270 */
};

/* This structure is for internal use...
 */
struct _djvu_image_priv;
typedef struct _djvu_image_priv * djvu_image_priv;

/*
 * To be friendly for C++ users, we define C++ methods for the 
 * structure.  If you are using C, don't worry.  All the methods
 * are defined with Macros that are available from within C.
 * 
 * Now for the djvu_image structure:
 */
             struct djvu_image_struct
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
                 // Gets the width using bottom up coordinates.
               inline int get_width(void) const;
                 // Gets the height using bottom up coordinates.
               inline int get_height(void) const;
                 // Gets the horizontal DPI using bottom up coordinates.
               inline unsigned int get_xdpi(void) const;
                 // Gets the vertical DPI using bottom up coordinates.
               inline unsigned int get_ydpi(void) const;
                 // Tests if this image is in the "native" format.
               inline bool isNative(const bool Strict=true) const;
                 // Does a rotate of 0,90,180, or 270 degrees.
               inline void Rotate(const int angle);
		 // This does a vertical flip in the raw coordinate system.
               inline void VFlip(void);
                 // This flips using corrected bottom up coordinates.
               inline void HFlip(void);
                 // This crops using coordinates in the raw coordinate system.
               inline void CropRaw(const int x,const int y,
                 const unsigned int width,const unsigned int height);
                 // This crops using corrected bottom up coordinates.
               inline void Crop(const int x,const int y,
                 const unsigned int width,const unsigned int height);
#endif
             };

/* As promised, now here are the macro's.  These are not intended to
 * be readable, just efficient! 
 */

/* This macro will obtained the width the image should be rendered at.
 * This is different than the img->w field, in that it corrects for
 * the rotated orientations.
 */
#define DJVU_IMAGE_GET_WIDTH(IMAGE) \
     (((IMAGE)->orientation&DJVU_ROTATE90_CW)?(IMAGE)->h:(IMAGE)->w)

/* This macro will obtained the height the image should be rendered at.
 * This is different than the img->h field, in that it corrects for
 * the rotated orientations.
 */
#define DJVU_IMAGE_GET_HEIGHT(IMAGE) \
     (((IMAGE)->orientation&DJVU_ROTATE90_CW)?(IMAGE)->w:(IMAGE)->h)

/* This macro will obtained the xdpi the image should be rendered at.
 * This is different than the img->xdpi field, in that it corrects for
 * the rotated orientations.
 */
#define DJVU_IMAGE_GET_XDPI(IMAGE) \
     (((IMAGE)->orientation&DJVU_ROTATE90_CW)?(IMAGE)->ydpi:(IMAGE)->xdpi)

/* This macro will obtained the ydpi the image should be rendered at.
 * This is different than the img->ydpi field, in that it corrects for
 * the rotated orientations.
 */
#define DJVU_IMAGE_GET_YDPI(IMAGE) \
     (((IMAGE)->orientation&DJVU_ROTATE90_CW)?(IMAGE)->xdpi:(IMAGE)->ydpi)

/* This macro will return true (non-zero) if the image is in what
 * is considered Native orientation and type.  "Native" is defined as
 * the image formats used by the internal library calls.  All library
 * calls are optimized to work fastest when dealing with a "Native" format
 * image.  In DjVu 3.0 Native is defined as top down for run length encoded
 * bitonal data (DJVU_TDLRNR && DJVU_RLE).  For gray scale, it is defined
 * as bottom up with a pixsize of 1 and a rowsize of w.  For color, it is
 * defined as BGR, bottom up, with a pixsize of 3, and a rowsize of 3*w.
 */
#define DJVU_IMAGE_IS_NATIVE(IMAGE) \
  (((IMAGE)->start_alloc == (IMAGE)->data)&&\
    (((IMAGE)->type==DJVU_RLE)?((IMAGE)->orientation==DJVU_TDLRNR):\
    (((IMAGE)->rowsize==((IMAGE)->w)*((IMAGE)->pixsize))&&\
      ((IMAGE)->orientation==DJVU_BULRNR)&&((IMAGE)->type!=DJVU_RGB)&&\
      ((IMAGE)->pixsize==(unsigned int)((IMAGE)->type==DJVU_GRAY)?1:3))))

/* This is a slightly less restrictive form of native.  We accept either
 * top down or bottom up, and accept RGB as well as BGR.
 */
#define DJVU_IMAGE_NEARLY_NATIVE(IMAGE) \
  (((IMAGE)->start_alloc == (IMAGE)->data)&&\
    (((IMAGE)->type==DJVU_RLE)?((IMAGE)->orientation==DJVU_TDLRNR):\
    (((IMAGE)->rowsize==((IMAGE)->w)*((IMAGE)->pixsize))&&\
      (((IMAGE)->orientation==DJVU_BULRNR)||\
        ((IMAGE)->orientation==DJVU_TDLRNR))&&\
      ((IMAGE)->pixsize==(unsigned int)((IMAGE)->type==DJVU_GRAY)?1:3))))

/* This macro will set the flags to indicate a the specified rotation.
 * Quite simply, for a 90 degree rotation, it sets the 90 rotate flag
 * if it is not already set.  If the 90 degree rotation flag is set
 * it unsets the 90 degree rotation and then sets the MIRROR and BOTTOM_UP
 * flags.  Since MIRROR and BOTTOM_UP used in combination means the same
 * thing as a 180 degree rotation, this means by setting the MIRROR and
 * BOTTOM_UP and unsetting ROTATE90_CW, we have rotate 180-90 == 90 
 * degrees.  For higher angle rotations, it just keeps repeating the
 * this procedure.
 */
#define DJVU_IMAGE_ROTATE(image,angle) \
     { \
       djvu_image *IMAGE=(image); \
       int a;for(a=(((angle)%360)+405)%360;a>90;a-=90) \
         IMAGE->orientation^=((IMAGE->orientation&DJVU_ROTATE90_CW)? \
           (DJVU_BOTTOM_UP|DJVU_MIRROR|DJVU_ROTATE90_CW): \
             DJVU_ROTATE90_CW); \
     }

/* This flips the vertical axis, as defined by the image's current orientation.
 * simply by reversing the BOTTOM_UP flag.
 */
#define DJVU_IMAGE_VFLIP(IMAGE) \
     {(IMAGE)->orientation^=DJVU_BOTTOM_UP;}

/* This flips the horizontal axis, as defined by the image's current
 * orientation simply by reversing the MIRROR flag.
 */
#define DJVU_IMAGE_HFLIP(IMAGE) \
     {(IMAGE)->orientation^=DJVU_MIRROR;}

/* This crops away sections of the image by starting and the x0, y0 
 * coordinates as defined with the lower y0==0 indicating the last
 * row, in the uncorrected coordinate system.  This is referred to
 * by the version that corrects for orientation. 
 */
#define DJVU_IMAGE_CROP_RAW(image,x0,y0,width,height) \
     { \
       djvu_image *IMAGE=(image); \
       IMAGE->data+=(x0)*(IMAGE->pixsize)+(y0)*(IMAGE->rowsize); \
       IMAGE->w=(width); \
       IMAGE->h=(height); \
     }

/* This crops away sections of the image by starting and the x0, y0 
 * coordinates as defined in the image's current orientation.  (0,0)
 * refers to the bottom left corner.
 */
#define DJVU_IMAGE_CROP(image,x0,y0,width,height) \
     { \
       djvu_image *IMAGEX=(image); \
       int o=IMAGEX->orientation,x=(x0),y=(y0),xw=(width),xh=(height); \
       if(o&DJVU_ROTATE90_CW) \
         DJVU_IMAGE_CROP_RAW(IMAGEX,((o&DJVU_BOTTOM_UP)?((IMAGEX->w)-y-xh):y),((o&DJVU_MIRROR)?((IMAGEX->h)-x-xw):x),xh,xw) \
       else \
         DJVU_IMAGE_CROP_RAW(IMAGEX,((o&DJVU_MIRROR)?((IMAGEX->w)-x-xw):x),((o&DJVU_BOTTOM_UP)?y:((IMAGEX->h)-y-xh)),xw,xh) \
     }

#ifdef __cplusplus
/** If you are using C++, you can use all the following methods
    to access and/or modify the djvu_image structure.  If you are using
    C, then you'll have to use the macros defined above...  */

               // This is just a simple constructor that zeros the values.
             inline djvu_image_struct::djvu_image_struct()
             : type(DJVU_UNKNOWN),orientation(0),w(0),h(0),pixsize(0), rowsize(0),
               datasize(0),xdpi(0),ydpi(0),data(0),start_alloc(0),priv(0) {}
               // Gets the width using bottom up coordinates.
             inline int djvu_image_struct::get_width(void) const
             {return DJVU_IMAGE_GET_WIDTH(this);}
               // Gets the height using bottom up coordinates.
             inline int djvu_image_struct::get_height(void) const
             {return DJVU_IMAGE_GET_HEIGHT(this);}
               // Gets the horizontal DPI using bottom up coordinates.
             inline unsigned int djvu_image_struct::get_xdpi(void) const
             {return DJVU_IMAGE_GET_XDPI(this);}
               // Gets the vertical DPI using bottom up coordinates.
             inline unsigned int djvu_image_struct::get_ydpi(void) const
             {return DJVU_IMAGE_GET_YDPI(this);}
               // This tests if the image is in the "native" format.
             inline bool djvu_image_struct::isNative(const bool Strict) const
             {return Strict?DJVU_IMAGE_IS_NATIVE(this):DJVU_IMAGE_NEARLY_NATIVE(this);}
               // Does a rotate of 0,90,180, or 270 degrees.
             inline void djvu_image_struct::Rotate(const int angle)
             DJVU_IMAGE_ROTATE(this,angle)
               // This flips using corrected bottom up coordinates.
             inline void djvu_image_struct::VFlip(void)
             DJVU_IMAGE_VFLIP(this)
               // This flips using corrected bottom up coordinates.
             inline void djvu_image_struct::HFlip(void)
             DJVU_IMAGE_HFLIP(this)
               // This crops using coordinates in the raw coordinate system.
             inline void djvu_image_struct::CropRaw(const int x0,const int y0,
               const unsigned int width,const unsigned int height)
             DJVU_IMAGE_CROP_RAW(this,x0,y0,width,height)
               // This crops using corrected bottom up coordinates.
             inline void djvu_image_struct::Crop(const int x0,const int y0,
               const unsigned int width,const unsigned int height)
             DJVU_IMAGE_CROP(this,x0,y0,width,height)
#endif

/** This frees an image allocated by the DjVu libraries.  This call should
   not be used on images that have been allocated manually.
 */
DJVUAPI void 
djvuio_image_free(djvu_image *ximg);

/** This frees an image data.  It is always safe to call this routine.
   However, if you manually allocated the start_alloc pointer, then this
   routine will not actually do anything.
 */
DJVUAPI void
djvuio_image_free_data(djvu_image ximg[1]);

/** This gets sets a pointer to the start of a row, a pointer to the end
    of the row, and returns the offset that should be used to loop over 
    the pixels.  This is appropriate for a loops like:
      int h;
      for(h=0;h<DJVU_IMAGE_GET_HEIGHT(image);++h)
      {
         unsigned char *pix, *end;
         const int offset=djvuio_image_get_row(image,h,&pix,&end);
         for(;pix!=end;pix+=offset)
         { 
            ... access each pixel pointed to by pix...
         }
      }
  */
DJVUAPI int
djvuio_image_get_row(
  djvu_image img[1],
  const int h,
  unsigned char *startptr[1],
  unsigned char *stopptr[1]);

/*
 *  ------------------------------------------------------------------------
 * Advanced image manipulations.
 *
 *    The following set of routines are defined with the djvuimage library,
 *    and are not available in the DjVuBitonalSDK.
 *
 *    Most routines below have two versions.  A version that will operate
 *    directly on the image passed, and a version that will copy from a
 *    a constant image.
 */

/** Same as djvuio_image_free_data(), but in a different library.  Useful if
   you are using the static libraries, and wish to control which library you 
   link from.
 */
DJVUAPI void 
djvu_image_free(djvu_image *ximg);

/** Same as djvuio_image_free_data(), but in a different library.  Useful if
   you are using the static libraries, and wish to control which library you 
   link from.
 */
DJVUAPI void
djvu_image_free_data(djvu_image ximg[1]);

/** Same as djvuio_image_get_row(), but in a different library.  Useful if you
   are using the static libraries, and wish to control which library you 
   link from.
 */
DJVUAPI int
djvu_image_get_row(
  djvu_image img[1],
  const int h,
  unsigned char *startptr[1],
  unsigned char *stopptr[1]);

/** This routine allocates memory for the specified image.
  An error is indicated by a NULL return.  No message is
  returned.
 */
DJVUAPI djvu_image *
djvu_image_allocate(unsigned int cols,unsigned int rows,size_t datasize);

/** This routine converts the image to bottom up orientation for
  color and gray images, top down for RLE data.  The alpha channel,
  and all other padding bits are removed.  When possible, the image
  is reallocated to the amount of memory actually used.  The
  application program should free the image in the event of an error.
 */
DJVUAPI djvu_image *
djvu_image_native(djvu_image *ximg,char *ebuf,size_t ebuf_size);
  
/** This routine copies the image to bottom up orientation for
  color and gray images, top down for RLE data.  The alpha channel,
  and all other padding bits are removed.  The copy is a DEEP copy,
  meaning all image data will be duplicated.  Any errors will be
  indicated with a NULL return value, and a message written into
  the user supplied buffer (ebuf).
 */
DJVUAPI djvu_image *
djvu_image_copy_native(const djvu_image *ximg,char *ebuf,size_t ebuf_size);

/** This converts images to gray scale.   An error is indicated by returning
  a NULL value and writing a message into the user supplied buffer (ebuf). 
  The application program should free the image in the event of an error.
 */
DJVUAPI djvu_image *
djvu_image_gray(djvu_image *ximg,char *ebuf,size_t ebuf_size);

/** This copies an image in gray scale.  This will be a DEEP copy, meaning
  all image data will be duplicated.  An error will be indicated by a
  NULL return value and writing a message into the user supplied 
  buffer (ebuf).
 */
DJVUAPI djvu_image *
djvu_image_copy_gray(const djvu_image *ximg,char *ebuf,size_t ebuf_size);

/** This run length encodes an image.  This means the image is converted
  to bitonal black and white by comparing each pixel to the specified
  threshold, with values below the threshold becoming black and above
  white.  The pixels are then encoded by storing the number of
  consecutive pixels of the same color, instead of bit mapping each
  pixel.  An error is indicated by returning a NULL value and writing
  a message into the user supplied buffer (ebuf).  The application
  program should free the image in the event of an error.
 */
DJVUAPI djvu_image*
djvu_image_rle(
  djvu_image *ximg,const int threshold,char *ebuf,size_t ebuf_size);

/** This run length encodes a copy of the image image.  This means the
  image is converted to bitonal black and white by comparing each pixel
  to the specified threshold, with values below the threshold becoming
  black and above white.  The pixels are then encoded by storing the
  number of consecutive pixels of the same color, instead of bit mapping
  each pixel.  This will be DEEP copy, meaning all image data is duplicated.
  An error is indicated by returning a NULL value and writing a message
  into the user supplied buffer (ebuf). 
 */
DJVUAPI djvu_image*
djvu_image_copy_rle(
  const djvu_image *ximg,const int threshold,
  char *ebuf,size_t ebuf_size);

/** This makes a copy of the image, resized to at the specified width and
  height.  The application program should free the image in the event of
  an error.
 */
DJVUAPI djvu_image *
djvu_image_copy_resize(
  const djvu_image *ximg,const unsigned int width,const unsigned int height,
  char *ebuf,size_t ebuf_size);

/** This resizes the image to the specified width and height.  This will be
  DEEP copy, meaning all image data is duplicated.  An error is indicated
  by returning a NULL value and writing a message into the user supplied
  buffer (ebuf). 
 */
DJVUAPI djvu_image *
djvu_image_resize(
  djvu_image *ximg,const unsigned int width,const unsigned int height,
  char *ebuf,size_t ebuf_size);

/** This makes a copy of the image headers, with the flags changed to
  the specified rotation.  The copy will be SHALLOW, meaning the
  new image structure still refers to the data in the original
  image, and the original image can not be deallocated while the copy
  is still in use.  If a DEEP copy is desired, use any of the above
  transforms, such as djvu_image_copy_native().
 */
DJVUAPI void
djvu_image_const_rotate(
  const djvu_image in_img[1], djvu_image out_img[1], int angle);

/** This changes the flags to indicate a rotate.  It is exactly the same
    as DJVU_IMAGE_ROTATE(ximg,angle)
 */
DJVUAPI void
djvu_image_rotate(djvu_image *ximg,int angle);

/** This makes a copy of the image headers, with the flags changed to
  the specified crop size.  The copy will be SHALLOW, meaning the
  new image structure still refers to the data in the original
  image, and the original image can not be deallocated while the copy
  is still in use.  If a DEEP copy is desired, use any of the above
  transforms, such as djvu_image_copy_native().
 */
DJVUAPI void
djvu_image_const_crop(
  const djvu_image in_img[1], djvu_image out_img[1], int x0,int y0,
  const unsigned width,const unsigned int height);

/** This changes the flags to indicate a crop.  It is exactly the same
  as DJVU_IMAGE_CROP(ximg,x0,y0,width,height)
 */
DJVUAPI void
djvu_image_crop(
  djvu_image *ximg,int x0,int y0,
  const unsigned width,const unsigned int height);

/** This makes a copy of the image headers, with the flags changed to
  indicate a vflip operation.  The copy will be SHALLOW, meaning the
  new image structure still refers to the data in the original
  image, and the original image can not be deallocated while the copy
  is still in use.  If a DEEP copy is desired, use any of the above
  transforms, such as djvu_image_copy_native().
 */
DJVUAPI void 
djvu_image_const_vflip(const djvu_image in_img[1], djvu_image out_img[1]);

/** This changes the flags to indicate a vflip.  It is exactly the same
    as DJVU_IMAGE_VFLIP(ximg)
 */
DJVUAPI void
djvu_image_vflip(djvu_image *ximg);

/** This makes a copy of the image headers, with the flags changed to
  indicate a hflip operation.  The copy will be SHALLOW, meaning the
  new image structure still refers to the data in the original
  image, and the original image can not be deallocated while the copy
  is still in use.  If a DEEP copy is desired, use any of the above
  transforms, such as djvu_image_copy_native().
 */
DJVUAPI void 
djvu_image_const_hflip(const djvu_image in_img[1], djvu_image out_img[1]);

/** This changes the flags to indicate a vflip.  It is exactly the same
  as DJVU_IMAGE_HFLIP(ximg)
 */
DJVUAPI void
djvu_image_hflip(djvu_image *ximg);

/** @name djvu_decode function
    This function converts the source multipage DjVu document to
    a document image according to options structure. */
/*
 *  ------------------------------------------------------------------------
 *  DjVu Image manipulation
 *
 *    The following set of routines that allow users to use the various 
 *    API libraries for memory to memory operations.
 */

/* This is a special type of djvu_import, intended for cases when you have
 * defined your own decoding, and want to map it to a stream for use with
 * one of the above functions.  The image will be passed directly and
 * may be modified by the encoder.
 *
DJVUAPI djvu_import
djvu_import_image ( djvu_image * );
 *
 * (This method is planned for the a future release of DjVu.
 *  contact djvu@research.att.com if you need this function
 *  implemented.)
 */

/* This is a special type of djvu_export, intended for obtaining the
 * pixel image in memory...  An apon successful decoding, the image
 * will be stored in the pointed passed.  The returned image should
 * be freed with djvu_image_free.
 */
DJVUAPI djvu_export
djvu_export_image ( djvu_image *[], const int size );

/* For compressions, we need to define another type of callback to
 * accept the streams from arbitrary sources.  The idea is quite
 * simply, each time this stream runs out of pages to decode, it
 * will make a callback, to obtain the next stream.  If the return
 * from that callback is NULL we assume we are done.  Note, each stream
 * will be closed when done.
 */
typedef struct djvuio_struct* djvu_io_sub ( void *arg, int filecount );
typedef djvu_io_sub djvu_import_sub;

DJVUAPI djvu_import
djvu_import_streams( void *arg, djvu_import_sub * );

/* For decompression, we need to define another type of callback to
 * accept the streams from arbitrary sources.  The idea is quite
 * simply, after adding a single page to a stream type that does not
 * accept multiple pages, we make this callback to get a new stream 
 * for appending further pages to.
 */
typedef djvu_io_sub djvu_export_sub;

DJVUAPI djvu_export
djvu_export_streams( void *arg, djvu_export_sub * );



#ifdef __cplusplus
#ifndef __cplusplus
{
#endif
}
#endif /* __cplusplus */

#endif /* _DJVUAPI_H */


