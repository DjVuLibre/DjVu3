/*C-  -*- C -*-
 *C-
 *C- This software may only be used by you under license from AT&T
 *C- Corp. ("AT&T"). A copy of AT&T's Source Code Agreement is available at
 *C- AT&T's Internet web site having the URL <http://www.djvu.att.com/open>.
 *C- If you received this software without first entering into a license with
 *C- AT&T, you have an infringing copy of this software and cannot use it
 *C- without violating AT&T's intellectual property rights.
 *C-
 *C- $Id: DjVuDocumentAPI.h,v 1.30 2000-07-04 00:51:50 mrosen Exp $
 */

#ifndef _DJVUDOCUMENT_H_
#define _DJVUDOCUMENT_H_

#include "DjVuBitonalAPI.h"
#include "DjVuPhotoAPI.h"

/** @name DjVuDocumentAPI.h

    @memo #DjVuDocumentAPI.h# defines the API for encoding document files.
    Document files consist of both a foreground layer with text and a
    background layer with images.
    @author
    Bill C Riemers 
*/

/*
 * $Log: DjVuDocumentAPI.h,v $
 * Revision 1.30  2000-07-04 00:51:50  mrosen
 * updated documentation
 *
 * Revision 1.29  2000/03/26 16:16:41  haffner
 * Added options
 *
 * Revision 1.28  2000/03/09 22:27:59  bcr
 * Updated the documentation, again.
 *
 * Revision 1.27  2000/03/08 22:59:46  bcr
 * Updated the documentation.  I'm using Leon's libdjvu++ documentation
 * as a template.
 *
 * Revision 1.26  2000/03/07 20:46:21  praveen
 * updated
 *
 * Revision 1.25  2000/03/07 00:01:16  bcr
 * Updated the document api documentation to build correctly.
 *
 */


/*@{*/

#ifdef __cplusplus
extern "C"
{
#ifndef __cplusplus
};
#endif
#endif

/** @memo #djvu_foreground_options# define foreground color compression options.

    When encoded as wavelet, options such as the number of chunks or the
    crcbdelay would be an overkill.

    Whereas the background image can use the \Ref{djvu_iw44_options} options,
    the foreground image has specific constraints, as it may be encoded
    within the JB2 image.
*/
struct djvu_foreground_options_struct
{ 

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
  
};

typedef struct djvu_foreground_options_struct djvu_foreground_options;


/** @memo #djvu_segmenter_options# define document segmentation options. 

    This structure allows complete control over the segmenation process
    that separates the foreground and background layers for each page.
    Adjusting the values here effect speed, compression quality, and
    compression ratio.
*/
struct djvu_segmenter_options_struct
{
  /** @name Quality slider options

      These options determine the output quality.

      Most are expressed as levels with values ranging from 0 to 100.
   */
  /*@{*/
  
  /** Pixel filter level.

      This pixel filter parameter corresponds to the Markov Transition
      probability.  Increase to remove very small shapes, smooth out edges and
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

  /** Shape filter level.

      \begin{description}
      \item[Option type] quality slider.
      \item[Range] 0..100
      \item[0] maximum foreground (filter is off)
      \item[100] maximum background
      \item[Default] 50
      \item[Optimization] Because of tremendous improvements in the foreback
      module, this parameter is not critical.
      \item[Command line] no
      \end{description}

      Shape (i.e. connected components) with a score higher than this filter
      level are kept.
  */
  int shape_filter_level;
  

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
      \item[thickening] can be turned off whne the resolution is very low.
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
      {\em Its most direct influence is a band of width edge_size around
      each character is not used to sample the background color.}

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

      {\em Its most direct influence is that any character with render_size
      or less pixels will be removed.}

      If is assumed that at this render size, characters have a reasonable
      size (typically more than 10 pixels and less than 100 pixels).

      \begin{description}
      \item[Option type] Pixel size.
      \item[Range] 1..6
      \item[0] Very clean or 100dpi images
      \item[4] High resolution images
      \item[Default] 3
      \item[Command line] no
      \end{description}
  */
  int render_size;


  /** Blurring size parameter. 

      Defines the size (in pixel) of blurring convolutions applied in the
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
  int blurring_size;

  /** Thickening (in pixels) applied to characters */
  int thickening;
  /** Size, in pixels , of the block used for local thresholding.

  */
  int block_size;

  /** Number of small block (used to compute the histograms) by which each large block (used to estimate the thresholding parameters) overlaps the next block.

      Overlapping gives more choice for block boundaries, avoiding to cut characters.
      \begin{itemize}
      \item A 0 value corresponds to no overlap. It results of blocking artifacts, as there is no choice on where to place the boundary.

      \item A 1 overlap is usually enough.
      \item A 2 overlap, making segmentation slower, is more robust when segmenting drawings.
      \end{itemize}

  */
  int block_overlap;

  /*@}*/
  
  /** @name Options for subsampling
      @memo -- These options are processed that happens after the extraction
      of the selector mask.  */

  /*@{*/
  /** Subsampling for the foreground image.
      
      \begin{description}
      \item[Option type] Pixel size.
      \item[Range] 3..12
      \item[Default] 12
      \item[Command line] yes
      \end{description}
  */
  int fg_subsample;

  /** Subsampling for the background image.      

      \begin{description}
      \item[Option type] Pixel size.
      \item[Range] 1..6
      \item[Default] 3
      \item[Command line] yes
      \end{description}
   */
  int bg_subsample;

  /** Upsampling for the mask image.      

      \begin{description}
      \item[Option type] Pixel size.
      \item[Range] 1..3
      \item[Default] 1
      \item[Command line] yes
      \end{description}
   */
  int mask_upsample;

  /** Subsampling target for viewing the image.

      \begin{description}
      \item[Option type] Pixel size.
      \item[Range] 1..6
      \item[1] 300dpi bg
      \item[6] 50dpi bg
      \item[Default] 3
      \item[Command line] yes
      \end{description}
   */
  int target_subsample;

  /*@}*/

  /** Special indicator for images which are more than 400dpi.

      In theory, resolution_multiplier=(image_dpi/400)+1.
      
      \begin{description}
      \item[Option type] Pixel size.
      \item[Range] 1,2
      \item[1] less than 400dpi
      \item[2] more than 400dpi
      \item[Default] 1
      \item[Command line] yes
      \end{description}
  */
  int resolution_multiplier;

  /*@}*/

 
  /** @name Flags

      @memo #Flags# are configuration file boolean options to control the 
      segmenter. */

  /*@{*/
  /** Indicates the color of the foreground characters varies in the same line.

      Normally, it is assumed that over a width of 32 pixels, the foreground
      color does not vary too much.

      However, this is not true in the following cases:
      \begin{itemize}
      \item Characters in the neighboring words have very different colors
      \item A character is next to a vertical line of different color
      \end{itemize}
   */
  int high_variation_foreground;


  /** Level of refinement in subsampling the foreground and the background.

      Parameter type: quality
   */
  int masksub_refine;


  /** Sample background color in small intervals between character.

     Generally causes larger files and less smooth background.
     Necessary when characters are thick (typically when upsampling).
  */
  int background_floss;

  /** Set some limits to the memory usage, especially in the filter .

      This mostly applies to the filter, whose memory usage can become very
      important when CCs are large.  The price to pay is that large CCs will
      be rejected in the background, without even attempting to break them.  

  */

  int limit_mem_usage;
  /*@}*/

#ifdef __cplusplus 
  inline djvu_segmenter_options_struct();
#endif 

};

typedef struct djvu_segmenter_options_struct djvu_segmenter_options;

/** @memo #documenttodjvu_options# -- options corresponding to documenttodjvu.

    The values of the #documenttodjvu_options# structure control all aspects
    of document encoding.
*/
struct documenttodjvu_options_struct
{
  /** The \Ref{djvu_process_options} structure #process# defines the pages to
    be parsed, input, and output, and contains the pointer for storing
    errors. */
  djvu_process_options process;

  /** #transform# contains the \Ref{djvu_transform_options} values.
    All transformations will take place before compression. */
  djvu_transform_options transform;

  /** #segment# contains the \Ref{djvu_segmenter_options} values.  These
    values control the separation of foreground and background. */
  djvu_segmenter_options segment;

  /** #foreground# contains the \Ref{djvu_foreground_options} values.  These
    values control the quality, speed, and format of the foreground
    layer compression. */
  djvu_foreground_options foreground;

  /** #jb2# contains the \Ref{djvu_jb2_options} values.  These
    values control the quality, speed, and format of the mask layer
    compression. */
  djvu_jb2_options jb2;

  /** #iw44# contains the \Ref{djvu_iw44_options} values.  These
    values control the quality, speed, and format of the background layer
    compression. */
  djvu_iw44_options iw44;

#ifdef __cplusplus
inline documenttodjvu_options_struct();
#endif /* __cplusplus */

};

typedef struct documenttodjvu_options_struct documenttodjvu_options;

struct djvu_parse
#if 0
{}
#endif
;

/** @name DjVuDocumentAPI C function calls
 */

/*@{*/

DJVUAPI
#if 0
;
#endif
/** ++ #documenttodjvu_options_alloc# is the primary allocation routine for
    documenttodjvu_options.  If a \Ref{djvu_parse} structure has been
    allocated, it may be passed as the parse argument.  Otherwise a NULL
    value should be passed as parse.  Even if the values specified are
    illegal, an options structure will be returned. */
documenttodjvu_options *
documenttodjvu_options_alloc(
  struct djvu_parse *parse,int argc,const char * const argv[]);

DJVUAPI
#if 0
;
#endif
/** ++ #documenttodjvu_options_free# deallocates the fields of the
    \Ref{documenttodjvu_options} structure.  You should always use the
    free option, even if you did not use alloc so the data pointed to by
    the priv pointer is freed. */
void documenttodjvu_options_free(documenttodjvu_options *);

DJVUAPI
#if 0
;
#endif
/** ++ #documenttodjvu# converts the photo input files to a multipage DjVu
    document according to the options structure.  A non-zero return value
    indicates a fatal error. */
int documenttodjvu(documenttodjvu_options[1]);

DJVUAPI
#if 0
;
#endif
/** ++ #documenttodjvu_haserror# returns a non-zero value to indicate there are
    error messages on the stack.  Error messages are generated for both fatal
    errors, and some non-fatal errors. */
int documenttodjvu_haserror(const documenttodjvu_options [1]);

DJVUAPI
#if 0
;
#endif
/** ++ #documenttodjvu_haswarning# returns a non-zero value to indicate there
    are warning messages on the stack.  Waring messages are generated for
    non-fatal problems, that may be an error, or could just be abnormal usage. */
int documenttodjvu_haswarning(const documenttodjvu_options [1]);

DJVUAPI
#if 0
;
#endif
/** ++ #documenttodjvu_error# pops the first error message off the stack.  Each
    call erases the previous return value. */
const char * documenttodjvu_error(documenttodjvu_options [1]);

DJVUAPI
#if 0
;
#endif
/** ++ #documenttodjvu_warning# pops the first warning message off the stack.
    Each call erases the previous return value. */
const char * documenttodjvu_warning(documenttodjvu_options [1]);

DJVUAPI
#if 0
;
#endif
/** ++ #documenttodjvu_perror# removes all error messages from the stack
  and prints them to stderr.  When mesg is not NULL, the mesg string
  followed by a colon and a blank will be printed before each error 
  message. */
void documenttodjvu_perror(documenttodjvu_options [1],const char *mesg);

DJVUAPI
#if 0
;
#endif
/** ++ #documenttodjvu_usage# will print usage instructions to the specified
   fileno. */
void documenttodjvu_usage(int fd,const char *prog);

/*@}*/

/*@}*/

#ifdef __cplusplus
#ifndef __cplusplus
{
#endif
};

inline
djvu_foreground_options_struct::djvu_foreground_options_struct() :
  color_jb2(false), quality(75)
{};

inline documenttodjvu_options_struct::documenttodjvu_options_struct() :
  process(), transform(), segment(), foreground(), jb2(), iw44() {}

inline djvu_segmenter_options_struct::djvu_segmenter_options_struct() :
  pix_filter_level(25), threshold_level(75), shape_filter_level(50),
  inhibit_foreback_level(40), inversion_level(25), edge_size(3),
  render_size(3), blurring_size(3), 
  block_size(16), block_overlap(1),
  fg_subsample(12), bg_subsample(3),
  mask_upsample(3),
  resolution_multiplier(1), high_variation_foreground(false), masksub_refine(true) {}

#endif

#endif /* _DJVUDOCUMENT_H_ */

