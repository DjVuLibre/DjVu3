/*C-  -*- C -*-
 *C-
 *C- Copyright (c) 2000, LizardTech, Inc.  All Rights Reserved.
 *C-
 *C- $Id: DjVuAPI.h,v 1.53 2000-09-18 17:10:35 bcr Exp $
 */

/* The main header file for the DjVu API
 */

#ifndef _DJVUAPI_H_
#define _DJVUAPI_H_

/** @name DjVuAPI.h
  
   @memo #DjVuAPI.h# defines the functions needed to do in memory operations
   with the \Ref{DjVuDecodeAPI.h}, \Ref{DjVuBitonalAPI.h}, \Ref{DjVuPhotoAPI.h},
   and \Ref{DjVuDocumentAPI.h} API's. 
  
   The basic type defined here is called a 'dstream'.  A 'dstream' can be
   either a \Ref{djvu_import} or \Ref{djvu_export} stream.  Normal usage
   of the dstreams, is to pass a filename for input or output, and let the
   stream handle all the relevant IO functions.  However, there are times
   when direct memory access, or access to a device is required instead of
   direct disk access is required.  In those cases, the API's should be 
   initialized with dummy filenames, and then passed one of the 'dstreams'
   created with the API functions in #DjVuAPI.h#.
 */

/*@{*/

/*
 * $Log: DjVuAPI.h,v $
 * Revision 1.53  2000-09-18 17:10:35  bcr
 * Adding files.
 *
 * Revision 1.51  2000/07/11 19:28:54  bcr
 * Various fixes to the copyrights and such.  Applied AT&T's latest patches.
 *
 * Revision 1.50  2000/05/18 20:22:34  bcr
 * Added rules for packaging source files.
 *
 * Revision 1.49  2000/03/08 22:59:46  bcr
 * Updated the documentation.  I'm using Leon's libdjvu++ documentation
 * as a template.
 *
 */

#ifdef WIN32
#include <stddef.h>
#else
#ifdef __MWERKS__
#include <stddef.h>
#else
#include <sys/types.h>
#endif
#endif

#include "DjVu.h"
#include "DjVuDecodeAPI.h"
/* include "DjVuBitonalAPI.h" */
/* include "DjVuPhotoAPI.h" */
/* include "DjVuDocumentAPI.h" */

#ifdef __cplusplus
extern "C"
{
#ifndef __cplusplus
};
#endif
#endif


/* pre-declaration. */
#ifndef _DJVU_IMAGE_
#define _DJVU_IMAGE_
struct djvu_image_struct
#if 0
{}
#endif
;
typedef struct djvu_image_struct djvu_image;
#endif

/*
 *  ------------------------------------------------------------------------
 * DYNAMIC LINK LIBRARY STUFF
 */

/** @memo #djvuio_struct# defines the basic structure used for DjVu(tm) IO.

   Normal usage is to create #djvuio_struct# with one of the
   \Ref{Open Commands} routines.  Most of the options are private, and
   should not be changed.  However, there is a read only reference to the
   filename, so users can unlink the output in the even of errors.

   \begin{verbatim}
   typedef struct djvuio_struct* djvu_import;
   typedef struct djvuio_struct* djvu_export;
   \end{verbatim}

   The \Ref{djvu_import} stream is normally used to load images from disk,
   and the \Ref{djvu_export} stream is normally used to save images to disk.
   However, there are \Ref{Open Commands} to read or write to almost any
   type of device.
 */
struct djvuio_struct
{
  /** #filename# will only be non-zero when the stream is initialized with a 
      filename. */
  const char *filename;
  /** #priv# contains all real data fields for this structure. */
  void *priv;
};

/** ++ We use the #djvuio_type# to represent what type of input
    or output the current stream is.  Legal values are defined as
    DjVuIO_NONE, DjVuIO_PNM, DjVuIO_PPM, DjVuIO_PBM, DjVuIO_BMP,
    DjVuIO_PICT, DjVuIO_PS, DjVuIO_PDF, DjVuIO_TIFF, DjVuIO_LIBTIFF,
    DjVuIO_JPEG, DjVuIO_GIF, DjVuIO_DJVU, DjVuIO_RAW, DjVuIO_CALLBACK,
    and DjVuIO_RAW.  Not all values listed here are used.
 */
enum djvuio_type_enum
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
};

typedef enum djvuio_type_enum djvuio_type;


/** @name Open Commands
 
    @memo The #Open Commands# create \Ref{djvu_import} or \Ref{djvu_export}
    streams for reading and writing images.
 */
/*@{*/

DJVUAPI
#if 0
;
#endif
/** ++ Open the named file for reading. */
djvu_import
djvu_import_file( const char filename[]);

DJVUAPI
#if 0
;
#endif
/** ++ Open the named file for writing.  */
djvu_export
djvu_export_file( const char filename[]);

DJVUAPI
#if 0
;
#endif
/** ++ Open a stream for reading from an already open fileno.  The device the
    fileno refers to must be seek-able.
 */
djvu_import
djvu_import_fileno( int fileno);

DJVUAPI
#if 0
;
#endif
/** ++ Open a stream for writing from an already open fileno.  The device the
   fileno refers to must be seek-able.
 */
djvu_export
djvu_export_fileno( int fileno);


DJVUAPI
#if 0
;
#endif
/** ++ Open a user allocated buffer as a stream for reading.  The buffer should
    not be freed until after the djvu_import stream is closed.  This type of
    stream is most useful for memmap files,  or files read in whole from a 
    database.
 */
djvu_import
djvu_import_buffer ( const void *buf, const size_t bufsize );


typedef int djvu_input_sub ( void *arg, void *data, size_t len );
typedef int djvu_seek_sub ( void *arg, long offset, int whence );

DJVUAPI
#if 0
;
#endif
/** ++ Open a stream for reading from user defined callbacks functions.
  \begin{verbatim}
  typedef int djvu_input_sub ( void *arg, void *data, size_t len );
  typedef int djvu_seek_sub ( void *arg, long offset, int whence );
  \end{verbatim}
 */
djvu_import
djvu_import_callback( void *arg,djvu_input_sub *inpf,djvu_seek_sub *seekf);


typedef int djvu_output_sub ( void *arg, const void *data, size_t len );

DJVUAPI
#if 0
;
#endif
/** ++ Open a stream for writing to user defined callback functions.
  \begin{verbatim}
  typedef int djvu_output_sub ( void *arg, const void *data, size_t len );
  \end{verbatim}
 */
djvu_export
djvu_export_callback(void *arg,djvu_input_sub *inpf,djvu_seek_sub *seekf);

/*@}*/


/** @name Close Commands 

    @memo The #Close Commands# close any open file descriptors associated
          with a dstream, and deallocate any allocated memory.  Even
          constant pointers, like filename should not be reference after
          a dstream is closed.
 */
/*@{*/

DJVUAPI
#if 0
;
#endif
/** ++ Close an import stream.  */
void
djvu_import_close( djvu_import);

DJVUAPI
#if 0
;
#endif
/** ++ Close an export stream.  */
void
djvu_export_close( djvu_export);

/*@}*/

/** @name Reading Commands

  @memo The following sequential operations, allow a user to read the
  images on a dstream.  This is useful for debugging, preparing 
  icon files and any type of image filtering.
*/

/*@{*/
DJVUAPI
#if 0
;
#endif
/** ++ #djvu_dstream_image_info# will return a constant image structure that
   fills the data fields for the next image, such as width, and height, but
   not the actual image data.  This is very useful for operations like
   skipping bitonal pages, or pages that will require too much memory to
   process.
 */
const djvu_image *
djvu_dstream_image_info(djvu_import io);

DJVUAPI
#if 0
;
#endif
/** ++ #djvu_dstream_to_image# retrieves the next image.  Normally the
   structure returned will be the same as \Ref{djvu_image_image_info}, but
   with the data actually filled in.  However, in some cases, we will learn
   more information about an image while reading it.  A typical example would
   be a bitonal BMP file that was stored in a color format.  The
   \Ref{djvu_dstream_image_info} call will indicate the image is 
   color, but #djvu_dstream_to_image# will actually return an image in
   \Ref{DJVU_RLE} format.
 */
djvu_image *
djvu_dstream_to_image(djvu_import io);

DJVUAPI
#if 0
;
#endif
/** ++ When dealing with multi-page documents, this is the way to skip a page.
   If the format allows, we will simply increment a page counter.  If the
   input format doesn't allow for skipping pages, we will read in the image
   and then discard the results just so we can advance to the next image.
 */
void
djvu_dstream_skip_image(djvu_import io);

DJVUAPI
#if 0
;
#endif
/** ++ #djvu_dstream_is_eof# tests if an EOF has been read.

   \Ref{djvu_dstream_to_image} and \Ref{djvu_dstream_image_info} will
   return a NULL under two conditions.  The first, most common condition
   is that the EOF has been reached.  The second less likely condition,
   is that the page that was requested was in an unsupported input format.  The
   #djvu_dstream_is_eof# function will return non-zero if we have read
   the file.  If we haven't read the EOF, then you can use the 
   \Ref{djvu_dstream_skip_image} function to skip the page that can not
   be read.  Since skip images does not actually read an image for many
   import types, do not expect repeated use of the
   \Ref{djvu_dstream_skip_image} function to set the EOF flag.
 */
int
djvu_dstream_is_eof(djvu_import io);

/*@}*/

/** @name Writing Commands

 @memo The sequential #writing command# allow users to 
 directly export images to an \Ref{djvu_export} stream.  If the
 output file type does not accept multiple pages, then the calling
 code must not attempt to write multiple pages.
*/
/*@{*/

DJVUAPI
#if 0
;
#endif
/** ++ Append an image to a \Ref{djvu_export} stream. */
int
djvu_image_to_dstream(djvu_export io, djvu_image img[1]);

/*@}*/

/** @name Error Handling

    @memo Error messages generated by dstream's are not automatically
    reported.  Rather the errors are pushed onto a stack, which must
    be checked manually after each call to a 'dstream' command.
 */

/*@{*/
DJVUAPI
#if 0
;
#endif
/** ++ Find out if a \Ref{djvu_import} stream has errors.  */
int
djvu_import_haserror( djvu_import);

DJVUAPI
#if 0
;
#endif
/** ++ Find out if a \Ref{djvu_export} stream has errors.  */
int
djvu_export_haserror( djvu_export);



DJVUAPI
#if 0
;
#endif
/** ++ Pull the oldest \Ref{djvu_import} error off the stack. */
const char *
djvu_import_error( djvu_import);

DJVUAPI
#if 0
;
#endif
/** ++ Pull the oldest \Ref{djvu_export} error off the stack. */
const char *
djvu_export_error( djvu_export);


DJVUAPI
#if 0
;
#endif
/** ++ Pull all errors off the \Ref{djvu_import} stack, and print them. */
void
djvu_import_perror( djvu_import,const char *);

DJVUAPI
#if 0
;
#endif
/** ++ Pull all errors off the \Ref{djvu_export} stack, and print them. */
void
djvu_export_perror( djvu_export,const char *);
/*@}*/

/** @name Magic Commands

  @memo The #Magic Commands# are used to detect, or set what type of
  file the stream is using.
*/

/*@{*/
DJVUAPI
#if 0
;
#endif
/** ++ Find out what type of file is being used on the stream.  If the
   known_type argument is anything other than DjVuIO_UNKNOWN, then an error
   will be set on the stream (see the djvu_import_haserror() function)
   if the file type is not what the user specified.  The file type is
   normally determined by scanning the first few bytes of input.
 */
djvuio_type
djvu_import_magic(djvu_import known_type);

DJVUAPI
#if 0
;
#endif
/** ++ Find out what type of file is being used on the stream.  If the
   known_type argument the is anything other than DjVuIO_UNKNOWN, then the
   export type will be set to the specified type.  Otherwise the filename
   field of the stream structure will be checked for an obvious extension
   such as .jpg, or .bmp.  If the extension is not recognized, then the
   it will be an error to try and write to the stream without setting
   the type of output to use.
 */
djvuio_type
djvu_export_magic(djvu_export);
/*@}*/


/*  ------------------------------------------------------------------------
 */
/** @name DjVu Pixel/Run Length Encoded Image Format
  
      The djvu_image structure, is the primary form of dealing in memory
      with renderable images.  In the previous version of the DjVuAPI, 
      the image format was known as djvu_run_image for black and white,
      and djvu_pixel_image for color images.  While often, the distinction
      between black and white and color is very important, we found that
      actually having two separate formats meant programmers had to learn
      twice as many function calls, and write nearly twice as much code.
*/  
/*@{*/
/** ++ #djvu_image_types# defines the type of image.  Valid values are
  DJVU_UNKNOWN, DJVU_RLE, DJVU_GRAY, DJVU_RGB, and DJVU_BGR.
  Code that depends on the image format can check the type in the
  \Ref{djvu_image} structure.
*/
enum djvu_image_types_enum
{
  DJVU_UNKNOWN=~0,/* Not known (yet) */
  DJVU_RLE=0x0,  /* black and white  (only valid for rle images) */
  DJVU_GRAY=0x1, /* gray (only valid for color images) */
  DJVU_RGB=0x2,  /* rgb (only valid for color images) */
  DJVU_BGR=0x3   /* bgr color (only valid for color images) */
};

typedef enum djvu_image_types_enum djvu_image_types;

/** ++ #djvu_image_orientations_bits# defines 3 mutually exclusive
   bits to indicate the image orientation.

   There are four possible rotation values for an image
   which are 0 degrees, 90 degrees, 180 degrees, and 270 degrees.
   In addition the image can be mirrored backwards in any of these
   orientations, giving a possible of 8 orientations.  To sanely deal
   with these orientations, we have defined 3 mutually exclusive 
   bits.  These are DJVU_BOTTOM_UP, DJVU_MIRROR, and DJVU_ROTATE90_CW.
*/
enum djvu_image_orientations_bits
{
  DJVU_BOTTOM_UP=0x1,  /* Upside down */
  DJVU_MIRROR=0x2,     /* Written backwards. (right to left) */
  DJVU_ROTATE90_CW=0x4 /* rotated 90 degrees */
};

/** ++ #djvu_image_orientations# defines all 8 possible orientations, using
   the three \Ref{djvu_image_orientations_bits}.

   \begin{itemize} 
   \item {\em DJVU_TDLRNR} for Top Down, Left to Right, No Rotation.
   \item {\em DJVU_BULRNR} for Bottom Up, Left to Right, No Rotation.
   \item {\em DJVU_TDRLNR} for Top Down, Right to Left, No Rotation.
   \item {\em DJVU_BURLNR} for Bottom Up, Right to Left, No Rotation.
   \item {\em DJVU_TDLRCW} for Top Down, Left to Right, 90 degree CW rotation.
   \item {\em DJVU_BULRCW} for Bottom Up, Left to Right, 90 degree CW rotation.
   \item {\em DJVU_TDRLCW} for Top Down, Right to Left, 90 degree CW rotation.
   \item {\em DJVU_BURLCW} for Bottom Up, Right to Left, 90 degree CW rotation.
   \end{itemize} 
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
struct _djvu_image_priv
#if 0
{}
#endif
;
typedef struct _djvu_image_priv * djvu_image_priv;

/** #djvu_image# defines the primary structure for in memory raw
  image references.

  To be friendly for C++ users, we define C++ methods for the 
  structure.  If you are using C, don't worry.  All the methods
  are defined with \Ref{Image Macros} that are available from within C.
*/
     struct djvu_image_struct
     {
       /** \Ref{djvu_image_types} #type# may be one of RLE,GRAY,RGB, or BGR. */
       djvu_image_types type;
       /** #orientation# can be an integer value of the
           \Ref{djvu_image_orientations}. */
       int orientation;
       /** The #width# in pixels, should not exceed 32767. */
       int w;
       /** The #height# in pixels, should not exceed 32767. */
       int h;
       /** #pixsize# is bytes per pixel, normally 1 for gray, or 3 for color,
           and undefined for RLE.  However, other values greater than the 
           normal values are legal. */
       size_t pixsize;
       /** #rowsize# is bytes per row.  Must be at least
           \Ref{pixsize}*\Ref{width}. (Not defined for RLE.) */
       size_t rowsize;      /* Bytes per row (includes padding) */
       /** Total number of allocated bytes.  Must be greater than
           \Ref{rowsize}*\Ref{height}.  For RLE it must be the total number
           of bytes to the end of the RLE data. */
       size_t datasize;
       /** The image resolution on the X axis. */
       unsigned int xdpi;
       /** The image resolution on the Y axis. */
       unsigned int ydpi;
       /** The start of the image data. */
       unsigned char *data;
       /** The start of memory allocation.  May be less than data, but never
           greater than data. */
       unsigned char *start_alloc;
       /** Structure for controlling the deallocation and reallocation of
           internally allocated images. */
       djvu_image_priv priv;
#ifdef __cplusplus
         /// A basic C++ constructor.
       djvu_image_struct();
         /// Gets the width using bottom up coordinates.
       inline int get_width(void) const;
         /// Gets the height using bottom up coordinates.
       inline int get_height(void) const;
         /// Gets the horizontal DPI using bottom up coordinates.
       inline unsigned int get_xdpi(void) const;
         /// Gets the vertical DPI using bottom up coordinates.
       inline unsigned int get_ydpi(void) const;
         /// Tests if this image is in the "native" format.
       inline bool isNative(const bool Strict=true) const;
         /// Does a rotate of 0,90,180, or 270 degrees.
       inline void Rotate(const int angle);
         /// This does a vertical flip in the raw coordinate system.
       inline void VFlip(void);
         /// This flips using corrected bottom up coordinates.
       inline void HFlip(void);
         /// This crops using coordinates in the raw coordinate system.
       inline void CropRaw(const int x,const int y,
         const unsigned int width,const unsigned int height);
         /// This crops using corrected bottom up coordinates.
       inline void Crop(const int x,const int y,
         const unsigned int width,const unsigned int height);
#endif
      }; 

/** @name Image Macros

   @memo These macros implement most of the transformations that can be done
   simply by changing the \Ref{djvu_image} structure fields, without touching
   the actual raw image data. 

 */
/*@{*/
/** ++ #DJVU_IMAGE_GET_WIDTH# will obtained the width the image should be
   rendered at.  This is different than the img->w field, in that it corrects
   for the rotated orientations.
 */
#define DJVU_IMAGE_GET_WIDTH(IMAGE) \
     (((IMAGE)->orientation&DJVU_ROTATE90_CW)?(IMAGE)->h:(IMAGE)->w)

/** ++ #DJVU_IMAGE_GET_HEIGHT# will obtained the height the image should be
   rendered at.  This is different than the img->h field, in that it corrects
   for the rotated orientations.
 */
#define DJVU_IMAGE_GET_HEIGHT(IMAGE) \
     (((IMAGE)->orientation&DJVU_ROTATE90_CW)?(IMAGE)->w:(IMAGE)->h)

/* ++ #DJVU_IMAGE_GET_XDPI# will obtained the xdpi the image should be
   rendered at.  This is different than the img->xdpi field, in that it
   corrects for the rotated orientations.
 */
#define DJVU_IMAGE_GET_XDPI(IMAGE) \
     (((IMAGE)->orientation&DJVU_ROTATE90_CW)?(IMAGE)->ydpi:(IMAGE)->xdpi)

/** ++ #DJVU_IMAGE_GET_YDPI# will obtained the ydpi the image should be
   rendered at.  This is different than the img->ydpi field, in that it
   corrects for the rotated orientations.
 */
#define DJVU_IMAGE_GET_YDPI(IMAGE) \
     (((IMAGE)->orientation&DJVU_ROTATE90_CW)?(IMAGE)->xdpi:(IMAGE)->ydpi)

/** ++ #DJVU_IMAGE_IS_NATIVE# return non-zero if the image is in what
   is considered Native orientation and type.  "Native" is defined as
   the image formats used by the internal library calls.  All library
   calls are optimized to work fastest when dealing with a "Native" format
   image.  In DjVu 3.0 Native is defined as top down for run length encoded
   bitonal data (DJVU_TDLRNR && DJVU_RLE).  For gray scale, it is defined
   as bottom up with a pixsize of 1 and a rowsize of w.  For color, it is
   defined as BGR, bottom up, with a pixsize of 3, and a rowsize of 3*w.
 */
#define DJVU_IMAGE_IS_NATIVE(IMAGE) \
  (((IMAGE)->start_alloc == (IMAGE)->data)&&\
    (((IMAGE)->type==DJVU_RLE)?((IMAGE)->orientation==DJVU_TDLRNR):\
    (((IMAGE)->rowsize==((IMAGE)->w)*((IMAGE)->pixsize))&&\
      ((IMAGE)->orientation==DJVU_BULRNR)&&((IMAGE)->type!=DJVU_RGB)&&\
      ((IMAGE)->pixsize==(unsigned int)((IMAGE)->type==DJVU_GRAY)?1:3))))

/** ++ #DJVU_IMAGE_NEARLY_NATIVE is a slightly less restrictive form of 
  \Ref{DJVU_IMAGE_IS_NATIVE}.  We accept either top down or bottom up, and
  accept RGB as well as BGR.
 */
#define DJVU_IMAGE_NEARLY_NATIVE(IMAGE) \
  (((IMAGE)->start_alloc == (IMAGE)->data)&&\
    (((IMAGE)->type==DJVU_RLE)?((IMAGE)->orientation==DJVU_TDLRNR):\
    (((IMAGE)->rowsize==((IMAGE)->w)*((IMAGE)->pixsize))&&\
      (((IMAGE)->orientation==DJVU_BULRNR)||\
        ((IMAGE)->orientation==DJVU_TDLRNR))&&\
      ((IMAGE)->pixsize==(unsigned int)((IMAGE)->type==DJVU_GRAY)?1:3))))

/** ++ #DJVU_IMAGE_ROTATION# will set the flags to indicate a the specified
   rotation.  Quite simply, for a 90 degree rotation, it sets the 90 rotate
   flag if it is not already set.  If the 90 degree rotation flag is set
   it unsets the 90 degree rotation and then sets the MIRROR and BOTTOM_UP
   flags.  Since MIRROR and BOTTOM_UP used in combination means the same
   thing as a 180 degree rotation, this means by setting the MIRROR and
   BOTTOM_UP and unsetting ROTATE90_CW, we have rotate 180-90 == 90 
   degrees.  For higher angle rotations, it just keeps repeating the
   this procedure.
 */
#define DJVU_IMAGE_ROTATE(image,angle) \
     { \
       djvu_image *IMAGE=(image); \
       int a;for(a=(((angle)%360)+405)%360;a>90;a-=90) \
         IMAGE->orientation^=((IMAGE->orientation&DJVU_ROTATE90_CW)? \
           (DJVU_BOTTOM_UP|DJVU_MIRROR|DJVU_ROTATE90_CW): \
             DJVU_ROTATE90_CW); \
     }

/** ++ #DJVU_IMAGE_VFLIP# flips the vertical axis, as defined by the image's
   current orientation, simply by reversing the BOTTOM_UP bit.
 */
#define DJVU_IMAGE_VFLIP(IMAGE) \
     {(IMAGE)->orientation^=DJVU_BOTTOM_UP;}

/** ++ #DJVU_IMAGE_HFLIP# flips the horizontal axis, as defined by the image's
   current orientation, simply by reversing the MIRROR bit.
 */
#define DJVU_IMAGE_HFLIP(IMAGE) \
     {(IMAGE)->orientation^=DJVU_MIRROR;}

/** ++ #DJVU_IMAGE_CROP_RAW# crops away sections of the image by starting and
   the x0, y0 coordinates as defined with the lower y0==0 indicating the last
   row, in the uncorrected coordinate system.  This is referred to
   by the version that corrects for orientation. 
 */
#define DJVU_IMAGE_CROP_RAW(image,x0,y0,width,height) \
     { \
       djvu_image *IMAGE=(image); \
       IMAGE->data+=(x0)*(IMAGE->pixsize)+(y0)*(IMAGE->rowsize); \
       IMAGE->w=(width); \
       IMAGE->h=(height); \
     }

/** ++ #DJVU_IMAGE_CROP# crops away sections of the image by starting and the
  x0, y0 coordinates as defined in the image's current orientation.  (0,0)
  refers to the bottom left corner.
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

/*@}*/

#ifdef __cplusplus
/*  If you are using C++, you can use all the following methods
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

DJVUAPI
#if 0
;
#endif
/** ++ #djvuio_image_free# frees an image allocated by the DjVu libraries.  This
   call should not be used on images that have been allocated manually.
 */
void 
djvuio_image_free(djvu_image *ximg);

DJVUAPI
#if 0
;
#endif
/** ++ This frees an image data.  It is always safe to call this routine.
   However, if you manually allocated the start_alloc pointer, then this
   routine will not actually do anything.
 */
void
djvuio_image_free_data(djvu_image ximg[1]);

DJVUAPI
#if 0
;
#endif
/** ++ This gets sets a pointer to the start of a row, a pointer to the end
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
int
djvuio_image_get_row(
  djvu_image img[1],
  const int h,
  unsigned char *startptr[1],
  unsigned char *stopptr[1]);

/*  ------------------------------------------------------------------------
 */
/** @name Advanced Image Manipulations
 
  The following set of routines are defined with the DjVu Image library,
  and are not available in the \Ref{DjVuBitonalAPI.h}.

  Most of the #Advanced Image Manipulation# routines below have two versions,
  a version that will operate directly on the image passed, and a version that
  will perform the operation while copying from a constant image.
 */
/*@{*/
DJVUAPI
#if 0
;
#endif
/** ++  #djvu_image_free# is the same as \Ref{djvuio_image_free_data}(), except
   in a different library.  Unless you have special linking requirements, you
   should always use #djvu_image_free#.
 */
void 
djvu_image_free(djvu_image *ximg);

DJVUAPI
#if 0
;
#endif
/** ++ #djvu_image_free_data# is the same as djvuio_image_free_data(), but 
   linked in a different library.  Unless you have special linking requirements,
   always use the #djvu_image_free_data# routine.
 */
void
djvu_image_free_data(djvu_image ximg[1]);

DJVUAPI
#if 0
;
#endif
/** ++ #djvu_image_get_row# is the same as \Ref{djvuio_image_get_row}, but in
  a different library.  Unless you have special linking requirements, always
  use the #djvu_image_get_row# routine.
 */
int
djvu_image_get_row(
  djvu_image img[1],
  const int h,
  unsigned char *startptr[1],
  unsigned char *stopptr[1]);

DJVUAPI
#if 0
;
#endif
/** ++ #djvu_image_allocate# allocates memory for the specified image.
  An error is indicated by a NULL return.  No message is
  returned.
 */
djvu_image *
djvu_image_allocate(unsigned int cols,unsigned int rows,size_t datasize);

/** ++ #djvu_image_native# converts the image to bottom up orientation for
  color and gray images, top down for RLE data.  The alpha channel,
  and all other padding bits are removed.  When possible, the image
  is reallocated to the amount of memory actually used.  The
  application program should free the image in the event of an error.
 */
djvu_image *
djvu_image_native(djvu_image *ximg,char *ebuf,size_t ebuf_size);
  
DJVUAPI
#if 0
;
#endif
/** ++ #djvu_image_copy_native# copies the image to bottom up orientation for
  color and gray images, top down for RLE data.  The alpha channel,
  and all other padding bits are removed.  The copy is a DEEP copy,
  meaning all image data will be duplicated.  Any errors will be
  indicated with a NULL return value, and a message written into
  the user supplied buffer (ebuf).
 */
djvu_image *
djvu_image_copy_native(const djvu_image *ximg,char *ebuf,size_t ebuf_size);

DJVUAPI
#if 0
;
#endif
/** ++ #djvu_image_gray# converts images to gray scale.   An error is
  indicated by returning a NULL value and writing a message into the user
  supplied buffer (ebuf).  The application program should free the image in
  the event of an error.
 */
djvu_image *
djvu_image_gray(djvu_image *ximg,char *ebuf,size_t ebuf_size);

DJVUAPI
#if 0
;
#endif
/** ++ #djvu_image_copy_gray# copies an image in gray scale.  This will be a
  DEEP copy, meaning all image data will be duplicated.  An error will be
  indicated by a NULL return value and writing a message into the user supplied 
  buffer (ebuf).
 */
djvu_image *
djvu_image_copy_gray(const djvu_image *ximg,char *ebuf,size_t ebuf_size);

DJVUAPI
#if 0
;
#endif
/** ++ #djvu_image_rle# run length encodes an image.  This means the image is
  converted to bitonal black and white by comparing each pixel to the specified
  threshold, with values below the threshold becoming black and above
  white.  The pixels are then encoded by storing the number of
  consecutive pixels of the same color, instead of bit mapping each
  pixel.  An error is indicated by returning a NULL value and writing
  a message into the user supplied buffer (ebuf).  The application
  program should free the image in the event of an error.
 */
djvu_image*
djvu_image_rle(
  djvu_image *ximg,const int threshold,char *ebuf,size_t ebuf_size);

DJVUAPI
#if 0
;
#endif
/** ++ #djvu_image_copy_rle# run length encodes a copy of the image image.
  This means the image is converted to bitonal black and white by comparing
  each pixel to the specified threshold, with values below the threshold
  becoming black and above white.  The pixels are then encoded by storing the
  number of consecutive pixels of the same color, instead of bit mapping
  each pixel.  This will be DEEP copy, meaning all image data is duplicated.
  An error is indicated by returning a NULL value and writing a message
  into the user supplied buffer (ebuf). 
 */
djvu_image*
djvu_image_copy_rle(
  const djvu_image *ximg,const int threshold,
  char *ebuf,size_t ebuf_size);

DJVUAPI
#if 0
;
#endif
/** ++ #djvu_image_copy_resize# resized the image to the specified width and
  height.  The application program should free the image in the event of an
  error.
 */
djvu_image *
djvu_image_resize(
  djvu_image *ximg,const unsigned int width,const unsigned int height,
  char *ebuf,size_t ebuf_size);

DJVUAPI
#if 0
;
#endif
/** ++ #djvu_image_copy_resize# creates a copy of the specified image resized
  to the specified width and height.  This will be DEEP copy, meaning all
  image data is duplicated.  An error is indicated by returning a NULL value
  and writing a message into the user supplied buffer (ebuf). 
 */
djvu_image *
djvu_image_copy_resize(
  const djvu_image *ximg,const unsigned int width,const unsigned int height,
  char *ebuf,size_t ebuf_size);

DJVUAPI
#if 0
;
#endif
/** ++ #djvu_image_const_rotate# makes a copy of the image headers, with the
  flags changed to the specified rotation.  The copy will be SHALLOW, meaning
  the new image structure still refers to the data in the original
  image, and the original image can not be deallocated while the copy
  is still in use.  If a DEEP copy is desired, use any of the above
  transforms, such as \Ref{djvu_image_copy_native}().
 */
void
djvu_image_const_rotate(
  const djvu_image in_img[1], djvu_image out_img[1], int angle);

DJVUAPI
#if 0
;
#endif
/** ++ #djvu_image_rotate# changes the flags to indicate a rotate.  It is
    exactly the same as \Ref{DJVU_IMAGE_ROTATE}(ximg,angle)
 */
void
djvu_image_rotate(djvu_image *ximg,int angle);

DJVUAPI
#if 0
;
#endif
/** ++ #djvu_image_const_crop# makes a copy of the image headers, with the
  flags changed to the specified crop size.  The copy will be SHALLOW, meaning
  the new image structure still refers to the data in the original
  image, and the original image can not be deallocated while the copy
  is still in use.  If a DEEP copy is desired, use any of the above
  transforms, such as \Ref{djvu_image_copy_native}().
 */
void
djvu_image_const_crop(
  const djvu_image in_img[1], djvu_image out_img[1], int x0,int y0,
  const unsigned width,const unsigned int height);

DJVUAPI
#if 0
;
#endif
/** ++ #djvu_image_crop# changes the flags to indicate a crop.  It is exactly
  the same as \Ref{DJVU_IMAGE_CROP}(ximg,x0,y0,width,height)
 */
void
djvu_image_crop(
  djvu_image *ximg,int x0,int y0,
  const unsigned width,const unsigned int height);

DJVUAPI
#if 0
;
#endif
/** ++ #djvu_image_const_vflip# makes a copy of the image headers, with the
  flags changed to indicate a vflip operation.  The copy will be SHALLOW,
  meaning the new image structure still refers to the data in the original
  image, and the original image can not be deallocated while the copy
  is still in use.  If a DEEP copy is desired, use any of the above
  transforms, such as \Ref{djvu_image_copy_native}().
 */
void 
djvu_image_const_vflip(const djvu_image in_img[1], djvu_image out_img[1]);

/** ++ #djvu_image_vflip# changes the flags to indicate a vflip.  It is
    exactly the same as \Ref{DJVU_IMAGE_VFLIP}(ximg)
 */
void
djvu_image_vflip(djvu_image *ximg);

DJVUAPI
#if 0
;
#endif
/** ++ #djvu_image_const_hflip# makes a copy of the image headers, with the
  flags changed to indicate a hflip operation.  The copy will be SHALLOW,
  meaning the new image structure still refers to the data in the original
  image, and the original image can not be deallocated while the copy
  is still in use.  If a DEEP copy is desired, use any of the above
  transforms, such as \Ref{djvu_image_copy_native}().
 */
void 
djvu_image_const_hflip(const djvu_image in_img[1], djvu_image out_img[1]);

DJVUAPI
#if 0
;
#endif
/** ++ #djvu_image_hflip# changes the flags to indicate a vflip.  It is
  exactly the same as \Ref{DJVU_IMAGE_HFLIP}(ximg).
 */
void
djvu_image_hflip(djvu_image *ximg);

/*@}*/

/*@}*/

/*  ------------------------------------------------------------------------
 */

/** @name  Advanced Memory To Memory Callbacks

  The #Advanced Memory To Memory Callbacks# define callbacks needed to
  use the various API's for in memory operations with multi-page documents.
  Most of these are not well tested, and should be considered as 
  experimental.
 */

/*@{*/
DJVUAPI
#if 0
;
#endif
/** ++ #djvu_import_image# is a special type of \Ref{djvu_import}, intended
   for cases when you have defined your own decoding, and want to map it to
   a stream for use with API functions.  The image will be passed directly,
   and the encoder may modify or deallocate the image as needed.
*/
djvu_import
djvu_import_image ( djvu_image * );

DJVUAPI
#if 0
;
#endif
/** ++ #djvu_export_image# is a special type of djvu_export, intended for
   obtaining the pixel image in memory...  An upon successful decoding, the
   image will be stored in the pointed passed.  The returned image should
   be deallocated with \Ref{djvu_image_free}.
 */
djvu_export
djvu_export_image ( djvu_image *[], const int size );


typedef struct djvuio_struct* djvu_io_sub ( void *arg, int filecount );
typedef djvu_io_sub djvu_import_sub;

DJVUAPI
#if 0
;
#endif
/** ++ #djvu_import_streams# uses special callbacks to accept streams from
   arbitrary sources.  These are defined as:

   \begin{verbatim}
   typedef struct djvuio_struct* djvu_io_sub ( void *arg, int filecount );
   typedef djvu_io_sub djvu_import_sub;
   \end{verbatim}

   The idea is quite simply, each time this stream runs out of pages to
   decode, it will make a callback, to obtain the next stream.  If the return
   from that callback is NULL we assume we are done.  Note, each stream
   will be closed after processing.
 */
djvu_import
djvu_import_streams( void *arg, djvu_import_sub * );

typedef djvu_io_sub djvu_export_sub;

DJVUAPI
#if 0
;
#endif
/** ++ #djvu_export_streams# uses special callbacks to accept streams from
   arbitrary sources.  These are defined as:

   \begin{verbatim}
   typedef struct djvuio_struct* djvu_io_sub ( void *arg, int filecount );
   typedef djvu_io_sub djvu_export_sub;
   \end{verbatim}

 The idea is quite simply, after adding a single page to a stream type that
 does not accept multiple pages, we use the special callback to get a new
 stream for appending further pages to.
*/
djvu_export
djvu_export_streams( void *arg, djvu_export_sub * );

/*@}*/



#ifdef __cplusplus
#ifndef __cplusplus
{
#endif
};
#endif /* __cplusplus */

/*@}*/

#endif /* _DJVUAPI_H */


