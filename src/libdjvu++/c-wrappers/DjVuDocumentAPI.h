/*C-  -*- C -*-
 *C-
 *C- This software may only be used by you under license from AT&T
 *C- Corp. ("AT&T"). A copy of AT&T's Source Code Agreement is available at
 *C- AT&T's Internet web site having the URL <http://www.djvu.att.com/open>.
 *C- If you received this software without first entering into a license with
 *C- AT&T, you have an infringing copy of this software and cannot use it
 *C- without violating AT&T's intellectual property rights.
 *C-
 *C- $Id: DjVuDocumentAPI.h,v 1.3 2000-01-22 07:10:14 bcr Exp $
 */

#ifndef _DJVUDOC_H_
#define _DJVUDOC_H_

#ifndef DJVUAPI

#ifndef DJVU_STATIC_LIBRARY
#ifdef WIN32 
#define DLLIMPORT __declspec(dllimport)
#define DLLEXPORT __declspec(dllexport)
#else
#define DLLIMPORT /**/
#define DLLEXPORT /**/
#endif
#else /* DJVU_STATIC_LIBRARY */
#define DLLIMPORT /**/
#define DLLEXPORT /**/
#endif /* DJVU_STATIC_LIBRARY */

#ifdef BUILD_LIB
#define DJVUAPI DLLEXPORT
#else
#define DJVUAPI DLLIMPORT
#endif  /*BUILD_LIB*/

#endif /*DJVUAPI*/


/** @name djvuphoto.h
      functions used to convert multiple photo images to DjVu multipage 
      documents.
*/

#ifdef __cplusplus
extern "C"
{
#endif

/*@{*/


/** List of djvu_segmenter options */
struct djvu_segmenter_options_struct
{
  /** @name Quality slider options

      These options determine the output quality.

      Most are expressed as levels with values ranging from 0 to 100.

      
   */
  //@{
  
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
      \item[Optimization] Default chosen to avoid dropping of characters (it is only acceptable to drop dots). Generally, the optimal pix_filter_level corresponds to the smaller file size. Unfortunately, the optimum varies on document. For most documents, it is 50, but it is 25 for 200dpi documents.
      \item[Command line] yes
      \end{description}
      

  */
  int pix_filter_level;

  /** Virtual threshold level.

      This threshold level parameter corresponds to the difference between the
      foreground and the background standard deviations used by the foreback algorithm.

      It is advisable to decrease the threshold when parts of characters are lost in the background.


      \begin{description}
      \item[Option type] quality slider.
      \item[Range] 0..100
      \item[0] maximum foreground
      \item[100] maximum background
      \item[Default] 50
      \item[Optimization] Highly critical parameter. Current value best for 75% of documents.
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
      \item[Optimization] Because of tremendous improvements in the foreback module, thus pare-meter is no longer so critical. Has not been optimized recently.
      \item[Command line] no
      \end{description}

      Mark (i.e. connected components) with a score higher than this filter level are kept.
  */
  int mark_filter_level;
  

  /** inhibit_foreback level.

      This inhibit_foreback level parameter corresponds to the an a-priori cost
      added to the decision to peform foreground/background separation (vs. deciding that a given block should not be segmented).

      It is advisable to increase the inhibit_foreback to remove isolated speckles.


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
      \item[Optimization] Really case-by-case. This is still too much inversion for comics.
      \item[Command line] yes
      \end{description}
  */
  int inversion_level;

  //@}

  /** @name Size Options
      Sizes are expressed as a number of mask pixels (DOTs).
   */
  //@{
  
  /** @name Sizes depending on characteristics of the input document
      (resolution, noise).
      
      These sizes mostly depend on resolution and scanning conditions.
      (generally DPI/100. For instance, 300 dpi means render size=3).

      The reason to set them to different values are uncommon.

      \begin{description}
      \item[render_size] could be smaller than the others when very small fonts are used in a 300dpi document
      \item[smooth_size] can be reduced when the printing quality is excellent, and we do not want to lose the sharp edges.
      \item[edge_size] can be reduced when both the printing and scanning qualities are excellent, and that we want a high quality 300dpi background up to the edges of the characters.
      \end{description}
      
  */
  //@{
  /** Edge size parameter. 

      
      Defines the thickness (in pixel) of character edges, where the color is unreliable because of:
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

  /** Render Size: size of the "ideal" pixel, viewed at the standard rendering resolution.

      Parameter type: resolution.

      If is assumed that at this render size, characters have a reasonable size (typically more than 10 pixels and less than 100 pixels).
      

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

      
      Defines the size (in pixel) of smoothing convolutions applied in the segmenter.
      Generally very similar to edge size, handling similar problems:
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

  //@}
  
  /** @name Options for the subsampling process that happens after the  extraction of the selector mask.
  */
  //@{
   /** Subsampling for the foreground image.
      
      \begin{description}
      \item[Option type] Pixel size.
      \item[Range] 3..12
      \item[Default] 12
      \item[Command line] yes
      \end{description}
      
  */
  int fg_pixel_size;
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
  int bg_pixel_size;

  //@}

  /** The original luminance image and, as a consequence, the mask, have
      a resolution approximately #upsample_size# times 300dpi.
      
      The default value is 1.

      When it is set to two, shall all the other sizes be multiplied by 2?

      In theory, yes except for \Ref{smoothing_size}.

      In practice, no, as this imply that we process as 600dpi image as if it
      had the same quality as a 300dpi.  This is not true: 600dpi edges are
      much sharper.  Moreover, if one uses a 600dpi image input, he/she should
      also expect a higher quality Djvu output.

      More experiments are necessary, but I recommend leaving the other sizes
      to their 300dpi values.
      
      \begin{description}
      \item[Option type] Pixel size.
      \item[Range] 1..2
      \item[1] less than 300dpi
      \item[2] more than 300dpi
      \item[Default] 1
      \item[Command line] yes
      \end{description}
      
  */
  int upsample_size;
  //@}

 
  /** @name Flags
  */

  //@{
  /** Indicates the color of the foreground characters  varies in the same line.

      Normally, it is assumed that over a width of 32 pixels, the foreground color does not vary too much.

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

  //@}
#ifdef __cplusplus

  /** @name Profiles examples
   */
  //@{

  /** Standard profile
   */
  void standard()
    {
      threshold_level= 75;
      mark_filter_level= 50;
      /* must be conservative here, other characters are lost on 200dpi images, or documents with small low-contrast fonts. */
      pix_filter_level= 25;
      inversion_level= 25;
      inhibit_foreback_level=40;

      edge_size= 3;
      render_size= 3;
      smoothing_size= 3;
      fg_pixel_size= 12; 
      bg_pixel_size= 3; 
      upsample_size=1;

      
      high_variation_foreground= false;
      masksub_refine= true;
    }
  
  /** Dpi600 profile
   */
  void dpi600()
    {
      threshold_level= 75;
      mark_filter_level= 50;
      pix_filter_level= 50;
      inversion_level= 25;
      inhibit_foreback_level=40;
      
      edge_size= 3;
      render_size= 3;
      smoothing_size= 3;
      fg_pixel_size= 12; 
      bg_pixel_size= 4; 
      upsample_size=2;

      
      high_variation_foreground= false;
      masksub_refine= true;
    }
  

  /** Dpi200 profile
   */
  void dpi200()
    {
      threshold_level= 75;
      mark_filter_level= 25;
      pix_filter_level= 25;
      inversion_level= 25;
      inhibit_foreback_level=40;
      
      edge_size= 2;
      render_size= 3;
      smoothing_size= 3;
      fg_pixel_size= 8; 
      bg_pixel_size= 2; 
      upsample_size=1;

      
      high_variation_foreground= false;
      masksub_refine= true;
    }
  
  /** Dpi100 screen dump profile
   */
  void dpi100()
    {
      threshold_level= 75;
      mark_filter_level= 50;
      pix_filter_level= 50;
      inversion_level= 25;
      
      edge_size= 1;
      render_size= 1;
      smoothing_size= 2;
      fg_pixel_size= 6; 
      bg_pixel_size= 1; 
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
      bg_pixel_size=1
      high_variation_foreground= true
      \end{verbatim}
  */
  void comic()
    {
      threshold_level= 75;
      mark_filter_level= 50;
      pix_filter_level= 50;
      inversion_level= 25;
      inhibit_foreback_level=40;
      
      edge_size= 3;
      render_size= 3;
      smoothing_size= 3;
      fg_pixel_size= 12; 
      bg_pixel_size= 3; 
      upsample_size=1;
      
      
      high_variation_foreground= false;
      masksub_refine= true;
    }
  
  /** Ancient document profile.

      The ink may  have become very pale, so the segmenter must be more sensitive. As there are no photos, ugly artifacts due to over-segmentation are less likely.

      What differs from standard is:
      
      \begin{verbatim}
      pix_filter_level= 10
      threshold_level= 50
      \end{verbatim}
  */
  void ancient()
    {
      threshold_level= 75;
      mark_filter_level= 50;
      pix_filter_level= 50;
      inversion_level= 25;
      inhibit_foreback_level=40;
      
      edge_size= 3;
      render_size= 3;
      smoothing_size= 3;
      fg_pixel_size= 12; 
      bg_pixel_size= 3; 
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
  void very_clean()
    {
      threshold_level= 75;
      mark_filter_level= 50;
      pix_filter_level= 50;
      inversion_level= 25;
       inhibit_foreback_level=40;
     
      edge_size= 1;
      render_size= 1;
      smoothing_size= 1;
      fg_pixel_size= 12; 
      bg_pixel_size= 3; 
      upsample_size=1;
      
      
      high_variation_foreground= true;
      masksub_refine= false;
    }
  //@}

  /** C++ constructor */
  djvu_segmenter_options_struct() {standard();};
#endif
};

typedef struct djvu_segmenter_options_struct djvu_segmenter_options;

/*@}*/

#ifdef __cplusplus
}
#endif

#endif /* _DJVUDOC_H_ */

