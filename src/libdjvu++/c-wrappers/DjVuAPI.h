/* File "$Id: DjVuAPI.h,v 1.21 2000-01-12 14:29:31 bcr Exp $"
 *
 * The main header file for the DjVu API
 */

#ifndef _DJVUAPI_H_
#define _DJVUAPI_H_

/* 
 * $Log: DjVuAPI.h,v $
 * Revision 1.21  2000-01-12 14:29:31  bcr
 * Removed djvu_run_image and exposed more of GBitmap to the rest of the libraries.
 *
 * Revision 1.20  2000/01/07 16:58:50  praveen
 * updated
 *
 * Revision 1.19  2000/01/06 04:34:03  bcr
 * Don't ask me.
 *
 * Revision 1.17  2000/01/05 19:39:48  praveen
 * *** empty log message ***
 *
 * Revision 1.16  2000/01/05 19:34:24  bcr
 * Generic header file.
 *
 * Revision 1.15  2000/01/04 20:01:46  bcr
 * Made streams resumable, and fixed a few minor bugs.
 *
 * Revision 1.14  2000/01/04 05:31:30  bcr
 * Refinement of previous patch, so we don't have unused variables.
 *
 * Revision 1.13  2000/01/04 04:48:37  bcr
 * Moved memory and progress callbacks exclusively into libdjvu++.
 * Added a text message for the progress callback, to indicate what
 * is currently being done.
 *
 * Revision 1.12  1999/12/22 18:29:04  bcr
 * Changed the libio++ functions to use ByteStream's.
 *
 * Revision 1.11  1999/12/20 20:11:46  bcr
 * Removed references to G4TiffStream, since this class is superceded by MTiffStream.
 *
 * Revision 1.10  1999/12/14 18:11:40  parag
 * C Wrapper for MTiff added
 *
 * Revision 1.9  1999/12/14 04:44:08  parag
 * C Wrapers for libio
 *
 * Revision 1.8  1999/12/03 00:23:10  parag
 * Added C Compatibility
 *
 * Revision 1.7  1999/11/24 19:21:50  orost
 * added djvu_pixel_rotate
 *
 * Revision 1.6  1999/11/23 15:54:08  orost
 * additions for DjVu3
 *
 * Revision 1.5  1999/11/18 00:17:12  parag
 * After changing Callback for djvu_fin
 *
 * Revision 1.4  1999/11/16 20:04:16  orost
 * Added in the functions for libimage
 *
 * Revision 1.3  1999/11/10 17:55:01  parag
 * Rotate90_CW
 *
 * Revision 1.2  1999/11/10 17:18:30  parag
 * Added Orientation Flags in run and pixel img
 *
 * Revision 1.1  1999/11/04 20:41:06  orost
 * It is a common file used by different libraries.
 * Mostly contains the image structures.
 *
 * Revision 1.2.2.17.2.1  1999/07/21 21:55:18  bcr
 * Added a memory debugging routines and fixed memory problems in DjVuMaskFind.cpp.
 * I've also added a DJVU_MASK_DISCARD_CONST flag for quicker decoding of low
 * resolution documents.
 *
 * Revision 1.2.2.17  1999/04/28 22:17:03  bcr
 *
 * Updates for documentation, and added resolution information to PNM files.
 *
 * Revision 1.2.2.16  1999/04/28 17:04:00  bcr
 * Changed to support resolution as part of the pixel image format.  This way
 * we can read resolution from BMP files, and keep it accurate across image
 * transformations.
 *
 * Revision 1.2.2.15  1999/03/27 06:39:37  bcr
 * I've disabled stripes for most documents, due to large letter problems.
 *
 * Revision 1.2.2.14  1999/03/02 07:07:29  bcr
 * Packaging correcitons
 *
 * Revision 1.2.2.13  1999/02/24 18:47:22  bcr
 * djvuencode now uses the progress reporting callback, and I've corrected a
 * couple of the options, such as crcbhalf.
 *
 * Revision 1.2.2.12  1999/02/18 18:20:18  orost
 * Added bmp routines.
 *
 * Revision 1.2.2.11  1999/02/15 22:27:31  bcr
 * I have updated the progress indicator to pass back an id from the parms
 * structure.  I've also modified, the order of djvu_render_full parameters and
 * renamed the function to djvu_render_area.
 *
 * I don't know why BmpStream.h keeps appearing in the commits.  I haven't made
 * any changes this time.
 *
 * Revision 1.2.2.10  1999/02/14 03:28:16  bcr
 * Added progress callback.
 * The automatic scaling now uses the reserved field.
 *
 * Revision 1.2.2.9  1999/02/12 04:56:08  bcr
 * Added in saving of original sizes.
 *
 * Revision 1.2.2.8  1999/02/10 22:24:36  bcr
 * I've finished updating the API to read the usefull data in the info
 * structure.  Now, if we can just add scale, I can consider this complete.
 *
 * Revision 1.2.2.7  1999/02/10 15:57:04  bcr
 * Hmmm. I really don't know.
 *
 * Revision 1.2.2.6  1999/02/10 15:19:28  bcr
 * Changed the high resolution and very low resolution values to be parameters
 * instead constants.
 *
 * Revision 1.2.2.5  1999/02/10 14:31:22  bcr
 * I have fixed the quality factors, and modified the DjVu routines to
 * automatically scale images below 225 DPI.
 *
 * Revision 1.2.2.4  1999/02/09 18:55:33  bcr
 * Corrected default background DPI to be 75 instead of 100.
 * Corrected the reverse video problem for the mask.
 *
 * Revision 1.2.2.3  1999/02/09 03:43:01  bcr
 * This is a complete re-write of the maskfind/libdjvu interphase.  It seems
 * to be working, but it still needs more testing.  For some reason the -2 option
 * is causing a core dump in my test programs, but I don't think the problem is
 * related.
 *
 * Revision 1.2.2.2  1999/02/02 06:49:53  bcr
 * Changed libMaskFind to use callback functions, instead of unresolved externals.
 * Also added the static_inline macro.
 *
 * Revision 1.2.2.1  1999/02/01 20:29:57  orost
 * I don't know why these changes were ommited.
 *
 * Revision 1.3  1999/02/01 07:07:10  bcr
 * The 2.0.0-prerelease version is now compiling.
 *
 * Revision 1.3  1999/01/31 16:49:37  bcr
 * All the files have been modified to list in comments if C++, revision,
 * author (if known), brief description, and finally the CVS log of
 * comments like this.
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

/*
 *  ------------------------------------------------------------------------
 */
#ifdef __cplusplus
extern "C" {
#endif

#ifdef __notdef
           }
#endif

/*
 *  ------------------------------------------------------------------------
 * DYNAMIC LINK LIBRARY STUFF
 */



/* 
 *      djvu_halfcoded_image
 *		See DjVuAPI-2_0.html#djvu_halfcoded_image
 */
             typedef struct djvu_halfcoded_image_struct
             {
               unsigned int w;         /* current width of the image in pixels */
               unsigned int h;         /* current height of the image in pixels */
               unsigned int original_w; /* original width of the image in pixels */
               unsigned int original_h; /* original height of the image in pixels */
               int memuse;
               char *info;
               unsigned int dpi;       /* So you can print to scale */
               float gamma;            /* Gamma factor used when compressing */
               void *privdata;
             }
             djvu_halfcoded_image;
/* 
 *           Deallocation:
 */
                  DJVUAPI void
                  djvu_halfcoded_image_free(djvu_halfcoded_image *);
/* 
 *                Deallocates any allocated pointers and the
 *                structure.  Only use if the structure was
 *                allocated by another DjVu API call.
 * 
 * ----------------------------------------------------------------------------
 * 
 * DjVu Parameter Types
 *
 * 	See DjVuAPI-2_0.html#DjVu Parameter Types
 * 
 *      djvu_mask_parms
 *
 * 		See DjVuAPI-2_0.html#djvu_mask_parms
 */
             typedef struct djvu_mask_parms_struct
             {
               /* size --
                * -- Size in bytes of the parameters data structure
                */
               size_t  size;
   
               /* filter_level:
                * Connected components with a score lower than this filter level are kept
                * Typical values range from -200 to 200 (defaults is 0).
                */
               int filter_level;
   
               /* multi_foreback:
                * if (multi_foreback>0), splits the luminance into 2 ranges
                * [0, multi_foreback-1] [multi_foreback, 255]
                * applies 2 separate forback analysis, and stitch the component together
                */
               int multi_foreback;
   
#define              DJVU_MASK_NO_COLOR_FILTER 1
               /* if the DJVU_MASK_NO_COLOR_FILTER is set, no color filter is done.
                */
#define              DJVU_MASK_NO_SIZE_FILTER 2
               /* if the DJVU_MASK_NO_SIZE_FILTER is set, no size filter is done.
                */
#define              DJVU_MASK_NO_INVERSION_FILTER 4
               /* if the DJVU_MASK_NO_INVERSION_FILTER flag is set, no inversion filter
                * rule is done.
                */
#define              DJVU_MASK_NO_FILTER \
                     (DJVU_MASK_NO_COLOR_FILTER|DJVU_MASK_NO_SIZE_FILTER|DJVU_MASK_NO_INVERSION_FILTER)
		/* This allows maskfind to resize the origninal image when
		 * neccissary rather than making a constant.
		 */
#define		     DJVU_MASK_DISCARD_CONST 8
                int flags;

                /* This is where you tell DjVu the source document's resolution.
                 */
		unsigned int dpi;

		/* Anything high_resolution_threshold or above will not be
		 * rescaled for detecting letters.  Anything below
                 * high_resolution_threshold, but not below the
                 * verylow_resolution_threshold we be upsampled by 2 for
	         * detecting letters.  Anything below
                 * verylow_resolution_threshold will be upsampled by 3.
                 */
                unsigned int high_resolution_threshold;
                unsigned int verylow_resolution_threshold;

		/* Set this to whatever you like.  It will be passed to your
		 * progress callback function.
		 */
		void *id;

	        /* Specify the stripe size in inches.  The smaller the
		 * stripe, the less memory required, but large letters
		 * are more likely to have problems.  0 means the whole
	         * document is done at once for best quality.
	         */
		int stripe_size;

#ifdef __cplusplus
                djvu_mask_parms_struct();
#endif
             }
             djvu_mask_parms;
#ifdef __cplusplus
             inline
             djvu_mask_parms_struct::djvu_mask_parms_struct() 
             : size(sizeof(djvu_mask_parms)),filter_level(0),
             multi_foreback(0), flags(0), dpi(300),
             high_resolution_threshold(225), verylow_resolution_threshold(125),
             id(0), stripe_size(0) {}
#endif
/* 
 *           Initialization and allocation:
 */
                  DJVUAPI djvu_mask_parms *
                  djvu_mask_parms_init(djvu_mask_parms *parms,
                      const size_t parms_size);
/* 
 *                Initializes the specified djvu_mask_parms
 *                structure.  If parms is NULL, the structure
 *                will be allocated with calloc().  A NULL is
 *                returned only if the calloc() fails.
 *                parms_size only has meaning for none NULL
 *                values of parms.
 * 
 *           Deallocation:
 * 
 *                Use free() to deallocate this structure if it
 *                was allocated using djvu_mask_parms_init.
 * 
 *      djvu_encode_document_parms
 *
 * 		See DjVuAPI-2_0.html#djvu_encode_document_parms
 */
             typedef struct djvu_encode_document_parms_struct
             {
               /* size --
                * -- Size in bytes of the parameters data structure
                */
               size_t  size;
   
               /* target_gamma --
                * -- Specify the target system gamma of the rendering device
                *    for which the original image was designed.  (default 2.2)
                */
               float target_gamma;
   
               /* background_quality --
                * -- Specify a quality (an integer between 20 and 100)
                *    of the background compression. Default is 75.
                */
               unsigned int background_quality;
   
               /* lossless_mask --
                * -- Set flag to code the bilevel mask layer using
                *    a lossless scheme. This option can be useful
                *    when the page contains very small characters.
                */
#define              DJVU_DOCUMENT_LOSSLESS_MASK 1

               /* agressive_mask --
                * -- This is just the opposite of lossless.  Instead we try
                *    extra hard to remove noise from the foreground.  This
                *    option can be very useful if you need higher compression
                *    ratios, or you have lots of noise in your document.
                */ 
#define              DJVU_DOCUMENT_AGGRESSIVE_MASK 4 

               /* bypass_thickening
                * -- This flag bypass the character "thickening" algorithm
                *    which is designed to improve the visual appearance
                *    of a document with faded characters, or when zooming
                *    in very close.
                */
#define              DJVU_DOCUMENT_BYPASS_THICKENING 2
                int flags;
		unsigned int textcolor_dpi ; 
		unsigned int background_dpi ; 

		/* Set this to whatever you like.  It will be passed to your
		 * progress callback function.
		 */
		void *id;

#ifdef __cplusplus
                djvu_encode_document_parms_struct();
#endif
             }
             djvu_encode_document_parms;
#ifdef __cplusplus
             inline
             djvu_encode_document_parms_struct::djvu_encode_document_parms_struct() 
             : size(sizeof(djvu_encode_document_parms)),target_gamma((float)2.2e0),
               background_quality(75), flags(0),
	       textcolor_dpi(25), background_dpi(100), id(0)  {};
#endif
/* 
 *           Initialization and allocation:
 */
                  DJVUAPI djvu_encode_document_parms *
                  djvu_encode_document_parms_init(djvu_encode_document_parms *parms,
                      const size_t parms_size);
/* 
 *                Initializes the specified djvu_mask_parms
 *                structure.  If parms is NULL, the structure
 *                will be allocated with calloc().  A NULL is
 *                returned only if the calloc() fails.
 *                parms_size only has meaning for none NULL
 *                values of parms.
 * 
 *           Deallocation:
 * 
 *                Use free() to deallocate this structure if it
 *                was allocated using djvu_encode_document_parms_init.
 * 
 *      djvu_encode_photo_parms
 *
 * 		See DjVuAPI-2_0.html#djvu_encode_photo_parms
 */
             typedef struct djvu_encode_photo_parms_struct
             {
               /* size --
                * -- Size in bytes of the parameters data structure
                */
               size_t  size;
   
               /* target_gamma --
                * -- Specify the target system gamma of the rendering device
                *    for which the original image was designed. (Default 2.2)
                */
               float target_gamma;
   
               /* quality --
                * -- Specify a quality (an integer between 20 and 100)
                *    of the image compression. Default is 75.
                */
               unsigned int quality;
   
               /* DJVU_PHOTO_CRCB_FULL --
                * -- Set flag to avoid color sub-sampling.
                */
#define              DJVU_PHOTO_CRCB_FULL 1
               /* DJVU_PHOTO_CRCB_NONE --
                * -- Set flag to make eliminate all color
                */
#define              DJVU_PHOTO_CRCB_NONE 2

               /* To sub-sample half way, use
		* 	(DJVU_PHOTO_CRCB_NONE|DJVU_PHOTO_CRCB_FULL)
		* and to do normal subsampling, set none of these flags...
                */
                int flags;

		/* Set this to whatever you like.  It will be passed to your
		 * progress callback function.
		 */
		void *id;
#ifdef __cplusplus
                djvu_encode_photo_parms_struct(); 
#endif
             }
             djvu_encode_photo_parms;
#ifdef __cplusplus
             inline
             djvu_encode_photo_parms_struct::djvu_encode_photo_parms_struct() 
             : size(sizeof(djvu_encode_photo_parms)),
               target_gamma((float)2.2e0),quality(75),
               flags(0), id(0) {};
#endif
/* 
 *           Initialization and allocation:
 */
                  DJVUAPI djvu_encode_photo_parms *
                  djvu_encode_photo_parms_init(djvu_encode_photo_parms *parms,
                      const size_t parms_size);
/* 
 *                Initializes the specified djvu_encode_photo_parms
 *                structure.  If parms is NULL, the structure
 *                will be allocated with calloc().  A NULL is
 *                returned only if the calloc() fails.
 *                parms_size only has meaning for none NULL
 *                values of parms.
 * 
 *           Deallocation:
 * 
 *                Use free() to deallocate this structure if it
 *                was allocated using djvu_encode_photo_parms_init.
 * 
 *      djvu_render_parms
 *
 * 		See DjVuAPI-2_0.html#djvu_render_parms
 */
             typedef enum { COLOR=0, BLACKANDWHITE, BACKGROUND, FOREGROUND } djvu_render_layer;
             typedef struct djvu_render_parms_struct
             {
               /* size --
                * -- Size in bytes of the parameters data structure
                */
               size_t  size;
   
               /* target_gamma --
                * -- Specify the target system gamma of the rendering device
                *    for which the original image was designed. (Default 2.2)
                */
               float target_gamma;
   
               /* dither_depth --
                * -- The depth of the screen for which the image should be dithered.
                *    Default is 24 which represents no dithering.  The resulting
                *    BGR image will only contain colors taken from a 6x6x6 color cube
                *    when depth < 15, and a 32x32x32 color cube when depth < 24.
                */
               int dither_depth;

               /* Normally gray images are stored as color.  So, if you render
                * only parts of an image, render does not know if the image is
                * gray or color, and it returns the section as color.  When this
                * flags is set, djvu_render will reduce the area being rendered
                * to gray scale if it contains no color information.
                */
#define	          DJVU_RENDER_ALLOW_REDUCTIONS 1
               int flags;

               /* Sometimes you only want to view one layer.  This is how.
                * Valid values are COLOR, FOREGROUND, BACKGROUND, and
                * BLACKANDWHITE.
                */
               djvu_render_layer layer;

		/* Set this to whatever you like.  It will be passed to your
		 * progress callback function.
		 */
               void *id;
#ifdef __cplusplus
               djvu_render_parms_struct(); 
#endif
             }
             djvu_render_parms;
#ifdef __cplusplus
             inline
             djvu_render_parms_struct::djvu_render_parms_struct() 
             : size(sizeof(djvu_render_parms)),target_gamma((float)2.2e0),
               dither_depth(24), flags(0),layer(COLOR),id(0) {}
#endif
/* 
 *           Initialization and allocation:
 */
                  DJVUAPI djvu_render_parms *
                  djvu_render_parms_init(djvu_render_parms *parms,
                      const size_t parms_size);
/* 
 *                Initializes the specified djvu_render_parms
 *                structure.  If parms is NULL, the structure
 *                will be allocated with calloc().  A NULL is
 *                returned only if the calloc() fails.
 *                parms_size only has meaning for none NULL
 *                values of parms.
 * 
 *           Deallocation:
 * 
 *                Use free() to deallocate this structure if it
 *                was allocated using djvu_render_parms_init.
 */ 

/*
 * ----------------------------------------------------------------------------
 * 
 * DjVu Input/Output Functions
 *
 *	See DjVuAPI-2_0.html#DjVu Input/Output Functions
 * 
 *      djvu_input_sub
 *
 *		See DjVuAPI-2_0.html#djvu_input_sub
 */ 
#if 0

             typedef int
             djvu_input_sub_new (
                 void *arg,      /* Argument passed to
                                  * the API routine */
                 unsigned char * data[],     /* Pointer to the data
                                  * being read */
                 const int len
#ifdef __cplusplus
								 ,                /* Length of the data
                                  * being read  
				  * retVal < len >=0 (EOF reached)
				  * retval > len     (Wants to send More Data)
				  * retval < 0       ( Error Condition) */
		 const int seek=0, /* seek == 0    (Allocate &/ Give data)
				    * seek == +n   (Skip next "n" data )
				    * seek == -1   (Deallocate if required) */
		 const int whence=1 /* Default to SEEK_CUR */
#endif
                            );
#endif
             typedef int
             djvu_input_sub (
                 void *arg,      /* Argument passed to
                                  * the API routine */
                 void * data,     /* Pointer to the data
                                  * being read */
                 size_t len
			);

             typedef int
             djvu_seek_sub (
                 void *arg,      /* Argument passed to
                                  * the API routine */
                 long offset,	 /* This is the same as fseek's offset */
                 int whence	 /* This is the same as fseek's whence */
			);

/* 
 *      djvu_output_sub
 *
 *		See DjVuAPI-2_0.html#djvu_output_sub
 */
             typedef int
             djvu_output_sub (
                 void *arg,          /* Argument passed to
                                      * the API routine */
                 const void *data,   /* Pointer to the data
                                      * being written */
                 size_t len          /* Length of the data
                                      * being written  */
                             );
   
/* 
 * ----------------------------------------------------------------------------
 *
 * 	See DjVuAPI-2_0.html#djvu_error_callback
 */
typedef void
djvu_error_callback ( const char cause[], const char file[], const int line);

/* DjVu Error Handling
 *
 * 	See DjVuAPI-2_0.html#DjVu Error Handling
 * 
 *      djvu_set_error_callback
 * 		See DjVuAPI-2_0.html#djvu_set_error_callback
 */
             DJVUAPI djvu_error_callback *
             djvu_set_error_callback( djvu_error_callback *callback);
		
/* 
 * ----------------------------------------------------------------------------
 * 
 * DjVu Image Conversions
 *
 * 	See DjVuAPI-2_0.html#DjVu Image Conversions
 * 
 *      djvu_mask
 *
 * 		See DjVuAPI-2_0.html#djvu_mask
 */
             DJVUAPI djvu_image *
             djvu_mask ( const djvu_mask_parms  *parms,
                         const djvu_image img[]
                         );
/* 
 *      djvu_encode_document
 *
 * 		See DjVuAPI-2_0.html#djvu_encode_document
 */
             DJVUAPI int
             djvu_encode_document( const djvu_encode_document_parms *parms,
                          const djvu_image     *mask,
                          const djvu_image   *img, /* Optional */
                          djvu_output_sub *outf,
                          void *arg
                          );
/* 
 *      djvu_encode_photo
 *
 * 		See DjVuAPI-2_0.html#djvu_encode_photo
 */
             DJVUAPI int
             djvu_encode_photo( const djvu_encode_photo_parms *parms,
                          const djvu_image   img[],
                          djvu_output_sub *outf,
                          void *arg
                          );
/* 
 *      djvu_decode
 *
 * 		See DjVuAPI-2_0.html#djvu_decode
 */
             DJVUAPI djvu_halfcoded_image *
             djvu_decode ( djvu_input_sub *inpf,
                           void *arg
                           );
/* 
 *      djvu_render  // Depreciated, use djvu_render_full instead.
 *
 * 		See DjVuAPI-2_0.html#djvu_render
 */
             DJVUAPI djvu_image *
             djvu_render ( const djvu_render_parms *parms,
                           const djvu_halfcoded_image *dimg,
                           const int subsample,
                           const int xmin, const int ymin,
                           const int xmax, const int ymax
                         );
/* 
 *      djvu_render_area
 *
 * 		See DjVuAPI-2_0.html#djvu_render
 */
             DJVUAPI djvu_image *
             djvu_render_area ( const djvu_render_parms *parms,
                           const djvu_halfcoded_image *dimg,
                           const int xmin, const int ymin,
                           const int xmax, const int ymax,
			   const int angle, const int width, const int height
                         );
/* 
 *      djvu_dstrea_to_image
 *
 * 		See DjVuAPI-2_0.html#djvu_dstream_to_image
 */
#ifdef NEED_JPEG_DECODER
DJVUAPI djvu_image * 
djvu_dstream_to_image(djvu_input_sub *, void *);
#endif
/* 
 *      djvu_image_flip
 *
 * 		See DjVuAPI-2_0.html#djvu_image_flip
 */
DJVUAPI djvu_image * 
djvu_image_flip(djvu_image []);
/* 
 *      djvu_image_mirror
 *
 * 		See DjVuAPI-2_0.html#djvu_image_mirror
 */
DJVUAPI djvu_image * 
djvu_image_mirror(djvu_image []);
/* 
 *      djvu_image_gray
 *
 * 		See DjVuAPI-2_0.html#djvu_image_gray
 */
DJVUAPI djvu_image *
djvu_image_gray(djvu_image []);
/* 
 *      djvu_image_to_native
 *
 * 		See DjVuAPI-2_0.html#djvu_image_to_native
 */
DJVUAPI djvu_image *
djvu_image_to_native(djvu_image []);
/* 
 *      djvu_image_to_pnm
 *
 * 		See DjVuAPI-2_0.html#djvu_image_to_pnm
 */
DJVUAPI int
djvu_image_to_pnm(djvu_image [],djvu_output_sub *,void *);
DJVUAPI int
djvu_image_to_bmp(djvu_image [],djvu_output_sub *,void *);
DJVUAPI int
djvu_image_to_pict(djvu_image [],djvu_output_sub *,void *);
DJVUAPI int
djvu_image_to_ps(djvu_image [],djvu_output_sub *,void *);
/* 
 *      djvu_pnm_to_run
 *
 * 		See DjVuAPI-2_0.html#djvu_pnm_to_run
 */

// alloc functions return a handle which is used for 
// further calls
DJVUAPI void *
djvu_alloc_mtiff_reader(djvu_input_sub * inpf,djvu_seek_sub *seekf,void * iarg);
DJVUAPI void *
djvu_alloc_mtiff_writer(djvu_output_sub * outf, void * oarg);
DJVUAPI djvu_image *
djvu_mtiff_to_dstream(void * handle, int pageNum);
DJVUAPI int
djvu_image_to_mtiff(void * handle, djvu_image pimg[]);
DJVUAPI void
djvu_free_mtiff(void * handle);

/* 
 *      djvu_image_to_bitonal
 *
 * 		See DjVuAPI-2_0.html#djvu_image_to_bitonal
 */
DJVUAPI djvu_image *
djvu_image_to_bitonal(const djvu_image pimg[],const int threshold);

/*
 * For freeing memory allocated by the callbacks
 */ 
DJVUAPI void
io_free_callback(void * callbackStruct);
/*
 *      djvu_image_copy_transformed
 *
 *		Not documented yet.
 */
DJVUAPI djvu_image *
djvu_image_copy_transformed
(const djvu_image im[],int angle,int w,int h);
/*
 *      djvu_image_transform
 *
 *		Not documented yet.
 */
DJVUAPI djvu_image *
djvu_image_transform
(djvu_image pimg[],int angle,int w,int h);
/*
 *      djvu_image_rotate
 *
 *		Not documented yet.
 */
DJVUAPI djvu_image *
djvu_image_rotate(djvu_image pimg[], int angle);

/* 
 *      djvu_counts_left
 *
 * 		Returns the number of counts left in the Limit License counter.
 */
DJVUAPI int
djvu_counts_left();

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _DJVUAPI_H */


