/*C-  -*- C -*-
 *C-
 *C- This software may only be used by you under license from AT&T
 *C- Corp. ("AT&T"). A copy of AT&T's Source Code Agreement is available at
 *C- AT&T's Internet web site having the URL <http://www.djvu.att.com/open>.
 *C- If you received this software without first entering into a license with
 *C- AT&T, you have an infringing copy of this software and cannot use it
 *C- without violating AT&T's intellectual property rights.
 *C-
 *C- $Id: DjVuDocumentAPI.h,v 1.12 2000-01-28 23:44:38 haffner Exp $
 */

#ifndef _DJVUDOC_H_
#define _DJVUDOC_H_

#include "DjVuBitonalAPI.h"
#include "DjVuPhotoAPI.h"

/** @name djvudocument.h
      functions used to convert multiple documents (contains color text or
      images with text) to DjVu multipage documents.
*/

#ifdef __cplusplus
extern "C"
{
#endif

  /*@{*/

/** List of options for the foreground image.

    Whereas the background image can use the #libdjvuphoto# wavelet options,
    the foreground image has specific constraints, as it may be encoded
    within the JB2 image.

    When encoded as wavelet, options such as the number of chunks or the
    crcbdelay would be an overkill.
    
*/

typedef struct djvu_foreground_options_struct
{ 
  /** Use a full resolution image when computing the foreground image, 
      to allow for a maximum color saturation */
  int high_saturation;

  /** Foreground colors are coded as a palette in the JB2Matcher */
  int color_jb2;

  /** JPEG-like quality slider for the foreground, ranging from 0 to 100.

      This quality slider controls:
      \begin{description}
      \item[Color JB2 foreground] The maximum number of colors allowed
      in the palette (up to 1024) (palette_ncolors) and the minimal size
      of the color cube area affected to a color palette entry
      (palette_boxsize).
      \item[Wavelet foreground] The number of slices
      \end{description}
  */
  int quality;
#ifdef __cplusplus 
  inline djvu_foreground_options_struct();
#endif
  
} djvu_foreground_options;


/** List of djvu_segmenter options. */

typedef struct djvu_segmenter_options_struct
{
  /** @name Quality slider options

      These options determine the output quality.

      Most are expressed as levels with values ranging from 0 to 100.
   */
  /*@{*/
  
  /** Pixel filter level.

      This pixel filter parameter corresponds to the Markov Transition
      probability.  Increase to remove very small marks, smooth out edges and
      remove halftoning.  Decrease to avoid dropping of characters.  This is
      the most efficient option level to manage the foreground/background
      tradeoff, always try to tune it first.
      
      \begin{description}
      \item[Option type] quality slider.
      \item[Range] 0..100
      \item[0] maximum foreground
      \item[100] maximum background
      \item[Default] 25
      \item[Optimization] Default chosen to avoid dropping of characters
      (it is only acceptable to drop dots). Generally, the optimal
      pix_filter_level corresponds to the smaller file size. Unfortunately,
      the optimum varies on document. For most documents, it is 50, but it
      is 25 for 200dpi documents.
      \item[Command line] yes
      \end{description}
      

  */
  int pix_filter_level;

  /** Virtual threshold level.

      This threshold level parameter corresponds to the difference between the
      foreground and the background standard deviations used by the foreback
      algorithm.

      It is advisable to decrease the threshold when parts of characters are
      lost in the background.


      \begin{description}
      \item[Option type] quality slider.
      \item[Range] 0..100
      \item[0] maximum foreground
      \item[100] maximum background
      \item[Default] 50
      \item[Optimization] Highly critical parameter. Current value best for
      75% of documents.
      \item[Command line] yes
      \end{description}

  */
  int threshold_level;

  /** Mark filter level.

      \begin{description}
      \item[Option type] quality slider.
      \item[Range] 0..100
      \item[0] maximum foreground (filter is off)
      \item[100] maximum background
      \item[Default] 50
      \item[Optimization] Because of tremendous improvements in the foreback
      module, thus pare-meter is no longer so critical. Has not been optimized
      recently.
      \item[Command line] no
      \end{description}

      Mark (i.e. connected components) with a score higher than this filter
      level are kept.
  */
  int mark_filter_level;
  

  /** inhibit_foreback level.

      This inhibit_foreback level parameter corresponds to the an a-priori cost
      added to the decision to perform foreground/background separation (vs.
      deciding that a given block should not be segmented).

      It is advisable to increase the inhibit_foreback to remove isolated
      speckles.

      \begin{description}
      \item[Option type] quality slider.
      \item[Range] 0..100
      \item[0] maximum text
      \item[100] maximum background
      \item[Default] 40
      \item[Optimization] Little
      \item[Command line] no
      \end{description}

  */
  int inhibit_foreback_level;

  
  /** Determines how aggressively regions should be inverted.

      \begin{description}
      \item[Option type] quality slider.
      \item[Range] 0..100
      \item[0] no inversion
      \item[100] all inversion
      \item[Default] 25
      \item[Optimization] Really case-by-case. This is still too much
      inversion for comics.
      \item[Command line] yes
      \end{description}
  */
  int inversion_level;

  /*@}*/

  /** @name Size Options
      Sizes are expressed as a number of mask pixels (DOTs).
   */
  /*@{*/
  
  /** @name Sizes depending on characteristics of the input document
      (resolution, noise).
      
      These sizes mostly depend on resolution and scanning conditions.
      (generally DPI/100. For instance, 300 dpi means render size=3).

      The reason to set them to different values are uncommon.

      \begin{description}
      \item[render_size] could be smaller than the others when very small
      fonts are used in a 300dpi document
      \item[smooth_size] can be reduced when the printing quality is excellent,
      and we do not want to lose the sharp edges.
      \item[edge_size] can be reduced when both the printing and scanning
      qualities are excellent, and that we want a high quality 300dpi
      background up to the edges of the characters.
      \end{description}
      
  */
  /*@{*/
  /** Edge size parameter. 

      Defines the thickness (in pixel) of character edges, where the color is
      unreliable because of:
      \begin{itemize}
      \item dithering in the printing process.
      \item bad color registration in the printer or the scanner
      \item ink blurring around in old documents
      \end{itemize}

      \begin{description}
      \item[Option type] Pixel size.
      \item[Range] 1..3
      \item[0] Very clean or 100dpi images
      \item[Default] 3
      \item[Command line] no
      \end{description}
  */
  int edge_size;

  /** Render Size: size of the "ideal" pixel, viewed at the standard rendering
      resolution.

      Parameter type: resolution.

      If is assumed that at this render size, characters have a reasonable
      size (typically more than 10 pixels and less than 100 pixels).
      

      \begin{description}
      \item[Option type] Pixel size.
      \item[Range] 1..4
      \item[0] Very clean or 100dpi images
      \item[4] High resolution images
      \item[Default] 3
      \item[Command line] no
      \end{description}
  */
  int render_size;


  /** Smoothing size parameter. 

      Defines the size (in pixel) of smoothing convolutions applied in the
      segmenter.  Generally very similar to edge size, handling similar
      problems:
      \begin{itemize}
      \item dithering in the printing process.
      \item noise in old documents
      \end{itemize}


      \begin{description}
      \item[Option type] Pixel size.
      \item[Range] 1..4
      \item[0] Very clean or 100dpi images
      \item[4] High resolution images
      \item[Default] 3
      \item[Command line] no
      \end{description}
  */
  int smoothing_size;

  /*@}*/
  
  /** @name Options for the subsampling process that happens after the
      extraction of the selector mask.  */

  /*@{*/
  /** Subsampling for the foreground image.
      
      \begin{description}
      \item[Option type] Pixel size.
      \item[Range] 3..12
      \item[Default] 12
      \item[Command line] yes
      \end{description}
  */
  int fg_subsampling;

  /** Subsampling for the background image.      

      \begin{description}
      \item[Option type] Pixel size.
      \item[Range] 1..6
      \item[1] 300dpi bg
      \item[6] 50dpi bg
      \item[Default] 3
      \item[Command line] yes
      \end{description}
   */
  int bg_subsampling;

  /*@}*/

  /** Special indicator for images which are more than 400dpi.

      In theory, upsample_size=(image_dpi/400)+1.
      
      \begin{description}
      \item[Option type] Pixel size.
      \item[Range] 1,2
      \item[1] less than 400dpi
      \item[2] more than 400dpi
      \item[Default] 1
      \item[Command line] yes
      \end{description}
  */
  int upsample_size;

  /*@}*/

 
  /** @name Flags */

  /*@{*/
  /** Indicates the color of the foreground characters varies in the same line.

      Normally, it is assumed that over a width of 32 pixels, the foreground
      color does not vary too much.

      However, this is not true in the following cases:
      \begin{itemize}
      \item Characters in the neighboring words have very different colors
      \item A character is next to a vertical line of different color
      \item Command line: no
      \end{itemize}
   */
  int high_variation_foreground;


  /** Level of refinement in subsampling the foreground and the background.

      Parameter type: quality
      
      Command line: no
   */
  int masksub_refine;

  /*@}*/

#ifdef __cplusplus 
  inline djvu_segmenter_options_struct();
  inline void standard(void);
  inline void dpi600(void);
  inline void dpi200(void);
  inline void dpi100(void);
  inline void comic(void);
  inline void ancient(void);
  inline void very_clean(void);
#endif 

} djvu_segmenter_options;

/** @name documenttodjvu_options struct
    @memo Options used in the documenttodjvu function
*/

typedef struct documenttodjvu_options_struct
{
  /** The #djvu_process_options struct# defines the pages to be parsed,
    input, and output, and contains the pointer for storing errors. */
  djvu_process_options process;

  /** These are the transformation options.  These will take place before
    compression. */
  djvu_transform_options transform;

  /** These options control the separation of the foreground and background. */
  djvu_segmenter_options segment;

  /** These options are the options that control the quality and speed
    and format of the foreground compression.  */
  djvu_foreground_options foreground;

  /** These options are the options that control the quality and speed
    of the mask layer compression.  */
  djvu_jb2_options jb2;

  /** These options are the options that control the quality and speed
    of the background compression.  */
  djvu_iw44_options iw44;

#ifdef __cplusplus
inline documenttodjvu_options_struct();
#endif /* __cplusplus */

} documenttodjvu_options;

struct djvu_parse;

/** @name documenttodjvu_options_alloc function
    This is the primary allocation routine for documenttodjvu_options.
    Even if the values specified are illegal, an options structure
    will be returned. */
DJVUAPI
documenttodjvu_options *
documenttodjvu_options_alloc(struct djvu_parse *,int,const char * const argv[]);

/** @name documenttodjvu_options_free function
    Deallocates the fields of the documenttodjvu_options structure.
    You should always use the free option, even if you did not use alloc
    so the data pointed to by priv is freed. */
DJVUAPI
void documenttodjvu_options_free(documenttodjvu_options *);

/** @name documenttodjvu function
    This function converts the photo input files to a multipage DjVu document
    according to the options structure.
    Depending on the type of the input data, the function uses the proper
    stream derived from \Ref{DjVu_Stream} for decoding, while
    \Ref{DjVu_PixImage.h} for transformations and \Ref{DjVmDoc.h},
    \Ref{JB2Matcher.h} for encoding.  A non-zero return value indicates a
    fatal error. */
DJVUAPI
int documenttodjvu(documenttodjvu_options[1]);

/** A non-zero value indicates there are error messages.  Error
    messages are generated for both fatal errors, and errors
    that are recovered from.  */
DJVUAPI
int documenttodjvu_haserror(const documenttodjvu_options [1]);

/** A non-zero value indicates there are warning messages.  Waring
    messages are generated for non-fatal problems, that may be an
    error, or could just be abnormal usage. */
DJVUAPI
int documenttodjvu_haswarning(const documenttodjvu_options [1]);

/** Returns a string of the first error message on the stack.  Each
    call erases the previous return value. */
DJVUAPI
const char * documenttodjvu_error(documenttodjvu_options [1]);

/** Returns a string of the first warning message on the stack.  Each
    call erases the previous return value. */
DJVUAPI
const char * documenttodjvu_warning(documenttodjvu_options [1]);

/** Prints all the errors to stderr */
DJVUAPI
void documenttodjvu_perror(documenttodjvu_options [1],const char *mesg);

/** This will print usage instructions to the specified output. */
DJVUAPI
void documenttodjvu_usage(int fd,const char *prog);

#ifdef __cplusplus
}
#endif

#ifdef __cplusplus 
inline
djvu_foreground_options_struct::djvu_foreground_options_struct() :
  high_saturation(false), color_jb2(false), quality(75)
{};

inline documenttodjvu_options_struct::documenttodjvu_options_struct() :
  process(), transform(), segment(), foreground(), jb2(), iw44() {}

inline djvu_segmenter_options_struct::djvu_segmenter_options_struct() :
  pix_filter_level(25), threshold_level(75), mark_filter_level(50),
  inhibit_foreback_level(40), inversion_level(25), edge_size(3),
  render_size(3), smoothing_size(3), fg_subsampling(12), bg_subsampling(3),
  upsample_size(1), high_variation_foreground(false), masksub_refine(true) {}

// These examples will be modified to show configuration file examples,
// instead of inline functions at a future date.


  /** @name Profiles examples */
  /*@{*/

  /** Standard profile
   */
  inline void djvu_segmenter_options_struct::standard(void)
    {
      threshold_level= 75;
      mark_filter_level= 50;
      /* must be conservative here, other characters are lost on 200dpi
         images, or documents with small low-contrast fonts. */
      pix_filter_level= 25;
      inversion_level= 25;

      /* this parameters has very little influence on most images,
         but may be very dramatic on few */
      inhibit_foreback_level=40;

      edge_size= 3;
      render_size= 3;
      smoothing_size= 3;
      fg_subsampling= 12; 
      bg_subsampling= 3; 
      upsample_size=1;

      
      high_variation_foreground= false;
      masksub_refine= true;
    }
  
  /** Dpi600 profile */
  inline void djvu_segmenter_options_struct::dpi600(void)
    {
      threshold_level= 75;
      mark_filter_level= 50;
      pix_filter_level= 50;
      inversion_level= 25;
      inhibit_foreback_level=40;
      
      edge_size= 3;
      render_size= 3;
      smoothing_size= 3;
      fg_subsampling= 12; 
      bg_subsampling= 4; 
      upsample_size=2;

      
      high_variation_foreground= false;
      masksub_refine= true;
    }
  

  /** Dpi200 profile */
  inline void djvu_segmenter_options_struct::dpi200(void)
    {
      threshold_level= 75;
      mark_filter_level= 25;
      pix_filter_level= 25;
      inversion_level= 25;
      inhibit_foreback_level=40;
      
      edge_size= 2;
      render_size= 3;
      smoothing_size= 3;
      fg_subsampling= 8; 
      bg_subsampling= 2; 
      upsample_size=1;

      
      high_variation_foreground= false;
      masksub_refine= true;
    }
  
  /** Dpi100 screen dump profile */
  inline void djvu_segmenter_options_struct::dpi100(void)
    {
      threshold_level= 75;
      mark_filter_level= 50;
      pix_filter_level= 50;
      inversion_level= 25;
      
      edge_size= 1;
      render_size= 1;
      smoothing_size= 2;
      fg_subsampling= 6; 
      bg_subsampling= 1; 
      upsample_size=1;
      inhibit_foreback_level=40;

      
      high_variation_foreground= true;
      masksub_refine= true;
    }
  
  /** Comics profile.
      
      Is is necessary to avoid excessive inversion.

      What differs from standard is:
      
      \begin{verbatim}
      pix_filter_level= 50
      inversion_level=25
      bg_subsampling=1
      high_variation_foreground= true
      \end{verbatim}
  */
  inline void djvu_segmenter_options_struct::comic(void)
    {
      threshold_level= 75;
      mark_filter_level= 50;
      pix_filter_level= 50;
      inversion_level= 25;
      inhibit_foreback_level=40;
      
      edge_size= 3;
      render_size= 3;
      smoothing_size= 3;
      fg_subsampling= 12; 
      bg_subsampling= 3; 
      upsample_size=1;
      
      
      high_variation_foreground= false;
      masksub_refine= true;
    }
  
  /** Ancient document profile.

      The ink may  have become very pale, so the segmenter must be more
      sensitive. As there are no photos, ugly artifacts due to
      over-segmentation are less likely.

      What differs from standard is:
      
      \begin{verbatim}
      pix_filter_level= 10
      threshold_level= 50
      \end{verbatim}
  */
  inline void djvu_segmenter_options_struct::ancient(void)
    {
      threshold_level= 75;
      mark_filter_level= 50;
      pix_filter_level= 50;
      inversion_level= 25;
      inhibit_foreback_level=40;
      
      edge_size= 3;
      render_size= 3;
      smoothing_size= 3;
      fg_subsampling= 12; 
      bg_subsampling= 3; 
      upsample_size=1;
      
      
      high_variation_foreground= false;
      masksub_refine= true;
    }
  
  /** "very clean" profile for 300dpi screen dumps.
      What differs from standard is:
      
      \begin{verbatim}
      edge_size= 1;
      render_size= 1;
      smoothing_size= 1;
      high_variation_foreground= true;
      masksub_refine= false;
      \end{verbatim}
  */
  inline void djvu_segmenter_options_struct::very_clean(void)
    {
      threshold_level= 75;
      mark_filter_level= 50;
      pix_filter_level= 50;
      inversion_level= 25;
      inhibit_foreback_level=40;
     
      edge_size= 1;
      render_size= 1;
      smoothing_size= 1;
      fg_subsampling= 12; 
      bg_subsampling= 3; 
      upsample_size=1;
      
      
      high_variation_foreground= true;
      masksub_refine= false;
    }
  /*@}*/
#endif
  /*@}*/


#endif /* _DJVUDOC_H_ */

