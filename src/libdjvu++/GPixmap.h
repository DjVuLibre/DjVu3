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
//C- $Id: GPixmap.h,v 1.7 1999-03-17 19:24:57 leonb Exp $

#ifndef _GPIXMAP_H_
#define _GPIXMAP_H_

/** @name GPixmap.h

    Files #"GPixmap.h"# and #"GPixmap.cpp"# implement class \Ref{GPixmap}.
    Instances of this class represent color images.  Each RGB pixel is
    represented by structure \Ref{GPixel}. The ``bottom left'' coordinate system
    is used consistently in the DjVu library.  Line zero of a GPixmap is the
    bottom line in the color image.  Pixels are organized from left to right
    within each line.
    
    {\bf ToDo} --- Support should be included for more sophisticated color
    correction schemes.  Support should also be added for blitting individual
    bitmaps with a predefined color.  This operation should support
    subsampling, anti-aliasing, and sub-pixel positionment, just like
    #GBitmap::blit#.
    
    @memo
    Generic support for color images.
    @author
    L\'eon Bottou <leonb@research.att.com>
    @version
    #$Id: GPixmap.h,v 1.7 1999-03-17 19:24:57 leonb Exp $# */
//@{

#ifdef __GNUC__
#pragma interface
#endif

#include "DjVuGlobal.h"
#include "GRect.h"
#include "GSmartPointer.h"
#include "ByteStream.h"
#include "GBitmap.h"


/** Color pixel as a RGB triple.  
    The colors are represented using three bytes named #r#, #g# and #b#.  The
    value of these bytes represent additive amounts of light.  Color white is
    represented by setting all three bytes to #255#.  Color black is
    represented by setting all three bytes to #0#.  This convention should not
    be confused with the convention adopted for class \Ref{GBitmap} where the
    pixel values represent an ink level.  */

struct GPixel
{
  /** Blue component. */
  unsigned char b;
  /** Green component. */
  unsigned char g;
  /** Red component. */
  unsigned char r;
  /** Returns true iff colors are identical. */
  friend int operator==(const GPixel & p1, const GPixel & p2);
  /** Returns true iff colors are different. */
  friend int operator!=(const GPixel & p1, const GPixel & p2);
  /** @name Predefined colors. */
  //@{ 
  /// GPixel::WHITE is initialized to #rgb:255/255/255#.
  static GPixel WHITE; 
  /// GPixel::BLACK is initialized to #rgb:0/0/0#.
  static GPixel BLACK; 
  /// GPixel::BLUE is initialized to #rgb:0/0/255#.
  static GPixel BLUE;  
  /// GPixel::GREEN is initialized to #rgb:0/255/0#.
  static GPixel GREEN; 
  /// GPixel::RED is initialized to #rgb:255/0/0#.
  static GPixel RED;
  //@}
};


/** RGB Color images.  
    Instances of class #GPixmap# represent color images as a two dimensional
    array of pixels \Ref{GPixel}.  The bracket operator returns a pointer to
    the pixels composing one line of the image.  This pointer can be used as
    an array to read or write the pixels of this particular line.  Following
    the general convention of the DjVu Reference Library, line zero is always
    the bottom line of the image.
 */

class GPixmap : public GPEnabled
{
public:
  virtual ~GPixmap();
  /** @name Construction. */
  //@{
  /** Constructs an empty GBitmap object.  The returned GPixmap has zero rows
      and zero columns.  Use function \Ref{init} to change the size of the
      image. */
  GPixmap();
  /** Constructs a GPixmap with #nrows# rows and #ncolumns# columns.  When the
      optional argument #filler# is specified, all pixels are initialized 
      with the corresponding color. */
  GPixmap(int nrows, int ncolumns, GPixel *filler=0);
  /** Constructs a GPixmap by copying the gray level image #ref#.
      The constructed GPixmap has the same size as #ref#.  The pixels
      are initialized with shades of grays copied from #ref#. */
  GPixmap(const GBitmap &ref);
  /** Constructs a GPixmap by copying the rectangle #rect# of the gray level
      image #ref#.  The constructed GPixmap has the same size as rectangle
      #rect#.  The pixels are initialized with shades of grays converted from
      the ink levels represented in #ref#.  This conversion depends on the
      number of gray levels in #ref#. */
  GPixmap(const GBitmap &ref, const GRect &rect);
  /** Copy constructors. Constructs a GPixmap by replicating the size and the
      contents of GPixmap #ref#. */
  GPixmap(const GPixmap &ref);
  /** Constructs a GPixmap by copying the rectangle #rect# of the color image #ref#.
      The constructed GPixmap has the same size as rectangle #rect#.
      The pixels are initialized with colors copied from #ref#. */
  GPixmap(const GPixmap &ref, const GRect &rect);
  /** Constructs a GPixmap by reading PPM data from ByteStream #ref#.
      See \Ref{PNM and RLE file formats} for more information. */
  GPixmap(ByteStream &ref);
  //@}

  /** @name Initialization. */
  //@{
  /** Resets the GPixmap to #nrows# rows and #ncolumns# columns.  When the
      optional argument #filler# is specified, all pixels are initialized with
      the corresponding color.  The previous content of the GPixmap is discarded. */
  void init(int nrows, int ncolumns,  GPixel *filler=0);
  /** Resets the GPixmap by copying the size and the contents of the color
      image #ref#.  The previous content of the GPixmap is discarded. */
  void init(const GPixmap &ref);
  /** Resets the GPixmap by copying the rectangle #rect# of the color image #ref#.
      The previous content of the GPixmap is discarded. */
  void init(const GPixmap &ref, const GRect &rect);
  /** Resets the GPixmap by copying the size and the contents of the gray 
      level image #ref#.  The optional argument #ramp# is an array of 256 
      pixel values used for mapping the gray levels to color values. 
      Setting #ramp# to zero selects a linear ramp of shades of gray. */
  void init(const GBitmap &ref, GPixel *ramp=0);
  /** Resets the GPixmap by copying the rectangle #rect# of the gray level
      image #ref#.  The optional argument #ramp# is an array of 256 pixel
      values used for mapping the gray levels to color values.  Setting #ramp#
      to zero selects a linear ramp computed according to the maximal number
      of gray levels in #ref#. */
  void init(const GBitmap &ref, const GRect &rect, GPixel *ramp=0);
  /** Resets the GPixmap by reading PPM data from ByteStream #ref#.  See
      \Ref{PNM and RLE file formats} for more information. */
  void init(ByteStream &ref);
  /** Resets the GPixmap by copying the gray level image #ref#.  The pixels
      are initialized with shades of grays copied from #ref#. */
  GPixmap& operator=(const GBitmap &ref);
  /** Copy operator. Resets the GPixmap by copying the size and the contents
      of the color image #ref#.  The previous content of the GPixmap is
      discarded. */
  GPixmap& operator=(const GPixmap &ref);
  //@}

  /** @name Accessing pixels. */
  //@{
  /** Returns the number of rows (the image height). */
  unsigned int rows() const;
  /** Returns the number of columns (the image width). */
  unsigned int columns() const;
  /** Returns a constant pointer to the first GPixel in row #row#.  This
      pointer can be used as an array to read the row elements. */
  const GPixel * operator[] (int row) const;
  /** Returns a pointer to the first GPixel in row #row#.  This pointer can be
      used as an array to read or write the row elements. */
  GPixel * operator[] (int row);
  /** Returns the length (in pixels) of a row in memory.  This number is equal
      to the difference between pointers to pixels located in the same column
      in consecutive rows.  This difference may be larger than the number of
      columns in the image. */
  unsigned int rowsize() const;
  //@}

  /** @name Resampling images. */
  //@{
  /** Resets this GPixmap with a subsampled segment of color image #src#.
      This function conceptually rescales image #src# by a factor #1:factor#,
      and copies rectangle #rect# of the subsampled image into the current GPixmap.
      The full subsampled image is copied if #rect# is a null pointer.
      Both operations are however performed together for efficiency reasons.
      Subsampling works by averaging the colors of the source pixels located
      in small squares of size #factor# times #factor#. */
  void downsample(const GPixmap *src, int factor, const GRect *rect=0);
  /** Resets this GPixmap with a oversampled segment of color image #src#.
      This function conceptually rescales image #src# by a factor #factor:1#,
      and copies rectangle #rect# of the oversampled image into the current
      GPixmap.  The full oversampled image is copied if #rect# is a null
      pointer.  Both operations are however performed together for efficiency
      reasons.  Oversampling works by replicating the color of the source
      pixels into squares of size #factor# times #factor#. */
  void upsample(const GPixmap *src, int factor, const GRect *rect=0);
  /** Resets this GPixmap with a rescaled segment of #src# (zoom 75%).  This
      function conceptually rescales image #src# by a factor #3:4#, and copies
      rectangle #rect# of the rescaled image into the current GPixmap.  The
      full rescaled image is copied if #rect# is a null pointer.  Both
      operations are however performed together for efficiency reasons.  This
      function has been superseded by class \Ref{GPixmapScaler}. */
  void downsample43(const GPixmap *src, const GRect *rect=0); 
  /** Resets this GPixmap with a rescaled segment of #src# (zoom 150%).  This
      function conceptually rescales image #src# by a factor #3:2# and copies
      rectangle #rect# of the rescaled image into the current GPixmap.  The
      full rescaled image is copied if #rect# is a null pointer.  Both
      operations are however performed together for efficiency reasons.  This
      function has been superseded by class \Ref{GPixmapScaler}. */
  void upsample23(const GPixmap *src, const GRect *rect=0);
  //@}

  /** @name Blitting and applying stencils. 
      This function is essential for rendering DjVu images
      composed of multiple layers. */
  //@{
  /** Paints the color image #pm# through the stencil #bm# into this image.
      This function conceptually computes an intermediate color image by first
      upsampling the GPixmap #pm# by a factor #pms:1# (see \Ref{upsample}),
      extracting the sub-image designated by rectangle #pmr# if such a
      rectangle is specified, and applying color correction #corr# (see
      \Ref{color_correct}).  This intermediate color image is then blended
      into the current GPixmap using the corresponding gray levels in GBitmap
      #bm#.  All these four functions are however performed together for
      efficiency reasons.
      
      {\bf Example}: Assume that the current GPixmap already contains the
      color corrected background layer of a particular rectangle #pmr# for a
      DjVu image at a given target resolution.  Assume that the GBitmap #bm#
      contains the corresponding region of the gray level mask layer at the
      same resolution.  Assume finally that the GPixmap #pm# contains the low
      resolution foreground color layer for the complete DjVu image.  This
      foreground color layer needs to be enlarged #pms# times to reach the
      target resolution.  A single call to function #stencil# will render the
      DjVu image. */
  void stencil(const GBitmap *bm, const GPixmap *pm, int pms=1, 
               const GRect *pmr=0, double corr=1.0);
  //@}
  
  /** @name Manipulating colors. */
  //@{
  /** Dithers the image to 216 colors.  This function applies an ordered
      dithering algorithm to reduce the image to 216 predefined colors.  These
      predefined colors are located on a color cube of 6x6x6 colors: the color
      RGB coordinates can only take the following values: #0#, #51#, #102#,
      #163#, #214# or #255#.  This is useful for displaying images on a device
      supporting a maximum of 256 colors. Arguments #xmin# and #ymin# control
      the position of the dithering grids.  This is useful for dithering tiled
      images. Arguments #xmin# and #ymin# must be the position of the bottom
      left corner of the tile contained in this GPixmap. Properly setting
      these arguments eliminates dithering artifacts on the tile
      boundaries. */
  void ordered_666_dither(int xmin=0, int ymin=0);
  /** Dithers the image to 32768 colors.  This function applies an ordered
      dithering algorithm to reduce the image to 32768 predefined colors.
      These predefined colors are located on a color cube of 32x32x32 colors:
      the color RGB coordinates can only take values in which the three least
      significant bits are set to #1#.  This is useful for displaying images
      with less than 24 bits per pixel.  Arguments #xmin# and #ymin# control
      the position of the dithering grids.  This is useful for dithering tiled
      images. Arguments #xmin# and #ymin# must be the position of the bottom
      left corner of the tile contained in this GPixmap. Properly setting
      these arguments eliminates dithering artifacts on the tile
      boundaries. */
  void ordered_32k_dither(int xmin=0, int ymin=0);
  /** Applies a luminance gamma correction factor of #corr#.  Values greater than
      #1.0# make the image brighter.  Values smaller than #1.0# make the image
      darker.  The documentation of program \Ref{ppmcoco} explains how to
      properly use this function.  Note that standard gamma correction sharply
      increases the contrast of the darkest parts of the image.  This effect
      reveals unpleasant data compression artifacts.  This is why we use an
      ad-hoc formula which limits this effect.  The resulting image is less
      accurate but more pleasant! */
  void color_correct(double corr);
  //@}
  
  /** @name Miscellaneous. */
  //@{
  /** Returns the number of bytes allocated for this image. */
  unsigned int get_memory_usage() const;
  /** Saves the image into ByteStream #bs# using the PPM format.
      Argument #raw# selects the ``Raw PPM'' (1) or the ``Ascii PPM'' (0) format.
      See \Ref{PNM and RLE file formats} for more information. */
  void save_ppm(ByteStream &bs, int raw=1) const;
  //@}

  /** @name Stealing or borrowing the memory buffer (advanced). */
  //@{
  /** Steals the memory buffer of a GPixmap.  This function returns the
      address of the memory buffer allocated by this GPixmap object.  The
      offset of the first pixel in the bottom line is written into variable
      #offset#.  Other lines can be accessed using pointer arithmetic (see
      \Ref{rowsize}).  The GPixmap object no longer ``owns'' the buffer: you
      must explicitly de-allocate the buffer using #operator delete []#.  This
      de-allocation should take place after the destruction or the
      re-initialization of the GPixmap object.  This function will return a
      null pointer if the GPixmap object does not ``own'' the buffer in the
      first place.  */
  GPixel *take_data(size_t &offset);
  /** Initializes this GPixmap by borrowing a memory segment.  The GPixmap
      then directly addresses the memory buffer #data# provided by the user.
      This buffer must be large enough to hold #w*h# GPixels.  The GPixmap
      object does not ``own'' the buffer: you must explicitly de-allocate the
      buffer using #operator delete []#.  This de-allocation should take place
      after the destruction or the re-initialization of the GPixmap object.  */
  void borrow_data(GPixel *data, int w, int h); 
  //@}
  
  // Please ignore these two functions. Their only purpose is to allow
  // DjVu viewer compile w/o errors. eaf. 
  // Is this still useful ?. lyb.
  int get_grays(void) const { return 256; };
  void set_grays(int) {};\
  
protected:
  // data
  unsigned short nrows;
  unsigned short ncolumns;
  unsigned short nrowsize;
  GPixel *pixels;
  GPixel *pixels_data;
};

//@}

// INLINE --------------------------


inline int 
operator==(const GPixel & p1, const GPixel & p2)
{
  return p1.r==p2.r && p1.g==p2.g && p1.b==p2.b;
}

inline int 
operator!=(const GPixel & p1, const GPixel & p2)
{
  return p1.r!=p2.r || p1.g!=p2.g || p1.b!=p2.b;
}

inline unsigned int
GPixmap::rows() const
{
  return nrows;
}

inline unsigned int
GPixmap::columns() const
{
  return ncolumns;
}

inline unsigned int
GPixmap::rowsize() const
{
  return nrowsize;
}

inline GPixel *
GPixmap::operator[](int row)
{
  if (row<0 || row>=nrows || !pixels) return 0;
  return &pixels[row * nrowsize];
}

inline const GPixel *
GPixmap::operator[](int row) const
{
  if (row<0 || row>=nrows) return 0;
  return &pixels[row * nrowsize];
}

inline GPixmap & 
GPixmap::operator=(const GBitmap &ref)
{
  init(ref);
  return *this;
}

inline GPixmap & 
GPixmap::operator=(const GPixmap &ref)
{
  init(ref);
  return *this;
}



// ---------------------------------
#endif


