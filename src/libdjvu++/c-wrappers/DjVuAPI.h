/* File "$Id: DjVuAPI.h,v 1.22 2000-01-16 13:13:54 bcr Exp $"
 *
 * The main header file for the DjVu API
 */

#ifndef _DJVUAPI_H_
#define _DJVUAPI_H_

/* 
 * $Log: DjVuAPI.h,v $
 * Revision 1.22  2000-01-16 13:13:54  bcr
 * Added a get_info() option to the Stream class.
 *
 * I found the orientation flags is ignored by most unix programs, so the
 * tiff images are now  oriented manually.
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

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _DJVUAPI_H */


