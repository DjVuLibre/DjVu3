//C-  -*- C++ -*-
//C-
//C-  Copyright (c) 1988 AT&T	
//C-  All Rights Reserved 
//C-
//C-  THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
//C-  The copyright notice above does not evidence any
//C-  actual or intended publication of such source code.
//C-
//C-  $Id: GBitmap.h,v 1.1.1.1 1999-01-22 00:40:19 leonb Exp $

#ifndef _GBITMAP_H_
#define _GBITMAP_H_

#ifdef __GNUC__
#pragma interface
#endif

#include "DjVuGlobal.h"
#include "GRect.h"
#include "ByteStream.h"
#include "GSmartPointer.h"


/** @name GBitmap.h

    Files #"GBitmap.h"# and #"GBitmap.cpp"# implement class \Ref{GBitmap}.
    Instances of these class represent bilevel or gray-level images. The
    "bottom left" coordinate system is used consistently in the DjVu library.
    Line zero of a bitmap is the bottom line in the bitmap.  Pixels are
    organized from left to right within each line.  As suggested by its name,
    class #GBitmap# was initially a class for bilevel images only.  It was
    extended to handle gray-level images when arose the need to render
    anti-aliased images.  This class has been a misnomer since then.

    {\bf ToDo} --- Class #GBitmap# can internally represent bilevel images
    using a run-length encoded representation.  Some algorithms may benefit
    from a direct access to this run information.

    @memo
    Generic support for bilevel and gray-level images.
    @author
    Leon Bottou <leonb@research.att.com>
    @version
    #$Id: GBitmap.h,v 1.1.1.1 1999-01-22 00:40:19 leonb Exp $#

 */
//@{


/** Bilevel and gray-level images.  Instances of class #GBitmap# represent
    bilevel or gray-level images.  Images are usually represented using one
    byte per pixel.  Value zero represents a white pixel.  A value equal to
    the number of gray levels minus one represents a black pixel.  The number
    of gray levels is returned by the function \Ref{get_grays} and can be
    manipulated by the functions \Ref{set_grays} and \Ref{change_grays}.

    The bracket operator returns a pointer to the bytes composing one line of
    the image.  This pointer can be used to read or write the image pixels.
    Following the general convention of the DjVu Reference Library, line zero
    is always the bottom line of the image.

    The memory organization is setup in such a way that you can safely read a
    few pixels located in a small border surrounding all four sides of the
    image.  The width of this border can be modified using the function
    \Ref{minborder}.  The border pixels are initialized to zero (e.g. white).
    You should never write anything into border pixels because they are shared
    between images and between lines.  */

class GBitmap : public GPEnabled
{
public:
  virtual ~GBitmap();
  /** @name Construction. */
  //@{
  /** Constructs an empty GBitmap object.  The returned GBitmap has zero rows
      and zero columns.  Use function \Ref{init} to change the size of the
      image. */
  GBitmap();
  /** Constructs a GBitmap with #nrows# rows and #ncolumns# columns.  All
      pixels are initialized to white. The optional argument #border#
      specifies the size of the optional border of white pixels surrounding
      the image.  The number of gray levels is initially set to #2#.  */
  GBitmap(int nrows, int ncolumns, int border=0);
  /** Copy constructor. Constructs a GBitmap by replicating the size, the
      border and the contents of GBitmap #ref#. */
  GBitmap(const GBitmap &ref);
  /** Constructs a GBitmap by copying the contents of GBitmap #ref#.  
      Argument #border# specifies the width of the optional border. */
  GBitmap(const GBitmap &ref, int border);
  /** Constructs a GBitmap by copying a rectangular segment #rect# of GBitmap
      #ref#.  The optional argument #border# specifies the size of the
      optional border of white pixels surrounding the image. */
  GBitmap(const GBitmap &ref, const GRect &rect, int border=0);
  /** Constructs a GBitmap by reading PBM, PGM or RLE data from ByteStream
      #ref# into this GBitmap. The optional argument #border# specifies the
      size of the optional border of white pixels surrounding the image.  See
      \Ref{PNM and RLE file formats} for more information.  */
  GBitmap(ByteStream &ref, int border=0);
  //@}

  /** @name Initialization. */
  //@{
  /** Resets this GBitmap size to #nrows# rows and #ncolumns# columns and sets
      all pixels to white.  The optional argument #border# specifies the size
      of the optional border of white pixels surrounding the image.  The
      number of gray levels is initialized to #2#. */
  void init(int nrows, int ncolumns, int border=0);
  /** Initializes this GBitmap with the contents of the GBitmap #ref#.  The
      optional argument #border# specifies the size of the optional border of
      white pixels surrounding the image. */
  void init(const GBitmap &ref, int border=0);
  /** Initializes this GBitmap with a rectangular segment #rect# of GBitmap
      #ref#.  The optional argument #border# specifies the size of the
      optional border of white pixels surrounding the image. */
  void init(const GBitmap &ref, const GRect &rect, int border=0);
  /** Reads PBM, PGM or RLE data from ByteStream #ref# into this GBitmap.  The
      previous content of the GBitmap object is lost. The optional argument
      #border# specifies the size of the optional border of white pixels
      surrounding the image. See \Ref{PNM and RLE file formats} for more
      information. */
  void init(ByteStream &ref, int border=0);
  /** Assignment operator. Initializes this GBitmap by copying the size, the
      border and the contents of GBitmap #ref#. */
  GBitmap& operator=(const GBitmap &ref);
  /** Initializes all the GBitmap pixels to value #value#. */
  void fill(unsigned char value);
  //@}

  /** @name Accessing the pixels. */
  //@{
  /** Returns the number of rows (the image height). */
  unsigned int rows() const;
  /** Returns the number of columns (the image width). */
  unsigned int columns() const;
  /** Returns a constant pointer to the first byte of row #row#.
      This pointer can be used as an array to read the row elements. */
  const unsigned char *operator[] (int row) const;
  /** Returns a pointer to the first byte of row #row#.
      This pointer can be used as an array to read or write the row elements. */
  unsigned char *operator[] (int row);
  /** Returns the size of a row in memory.  This number is equal to the
      difference between pointers to pixels located in the same column in
      consecutive rows.  This difference can be larger than the number of
      columns in the image. */
  unsigned int rowsize() const;
  /** Makes sure that the border is at least #minimum# pixels large.  This
      function does nothing it the border width is already larger than
      #minimum#.  Otherwise it reorganizes the data in order to provide a
      border of #minimum# pixels. */
  void minborder(int minimum);
  //@}

  /** @name Managing gray levels. */
  //@{
  /** Returns the number of gray levels. 
      Value #2# denotes a gray level image. */
  int  get_grays() const;
  /** Sets the number of gray levels without changing the pixels.
      Argument #grays# must be in range #2# to #256#. */
  void set_grays(int grays);
  /** Changes the number of gray levels.  The argument #grays# must be in
      range #2# to #256#.  All the pixel values are then rescaled and clipped
      in range #0# to #grays-1#. */
  void change_grays(int grays);
  /** Binarizes a gray level image using a threshold.  The number of gray
      levels is reduced to #2# as in a bilevel image.  All pixels whose value
      was strictly greater than #threshold# are set to black. All other pixels
      are set to white. */
  void binarize_grays(int threshold=0);
  //@}

  /** @name Optimizing the memory usage.  
      The amount of memory used by bilevel images can be reduced using
      function \Ref{compress}, which encodes the image using a run-length
      encoding scheme.  The bracket operator decompresses the image on demand.
      A few highly optimized functions (e.g. \Ref{blit}) can use a run-length
      encoded bitmap without decompressing it.  */
  //@{
  /** Reduces the memory required for a bilevel image by using a run-length
      encoded representation.  Functions that need to access the pixel array
      will decompress the image on demand. */
  void compress();
  /** Returns the number of bytes allocated for this image. */
  unsigned int get_memory_usage() const;
  //@}

  /** @name Additive Blit.  
      The blit functions are designed to efficiently construct an anti-aliased
      image by copying smaller images at predefined locations.  The image of a
      page, for instance, is composed by copying the images of characters at
      predefined locations.  These functions are fairly optimized.  They can
      direclty use compressed GBitmaps (see \Ref{compress}).  We consider in
      this section that each GBitmap comes with a coordinate system defined as
      follows.  Position (#0#,#0#) corresponds to the bottom left corner of
      the bottom left pixel.  Position (#1#,#1#) corresponds to the top right
      corner of the bottom left pixel, which is also the bottom left corner of
      the second pixel of the second row.  Position (#w#,#h#), where #w# and
      #h# denote the size of the GBitmap, corresponds to the top right corner
      of the top right pixel. */

  //@{
  /** Performs an additive blit of the GBitmap #bm#.  The GBitmap #bm# is
      first positionned above the current GBitmap in such a way that position
      (#u#,#v#) in GBitmap #bm# corresponds to position (#u#+#x#,#v#+#y#) in
      the current GBitmap.  The value of each pixel in GBitmap #bm# is then
      added to the value of the corresponding pixel in the current GBitmap.
      
      {\bf Example}: Assume for instance that the current GBitmap is initially
      white (all pixels have value zero).  This operation copies the pixel
      values of GBitmap #bm# at position (#x#,#y#) into the current GBitmap.
      Note that function #blit# does not change the number of gray levels in
      the current GBitmap.  You may have to call \Ref{set_grays} to specify
      how the pixel values should be interpreted. */
  void blit(const GBitmap *bm, int x, int y);
  /** Performs an additive blit of the GBitmap #bm# with anti-aliasing.  The
      GBitmap #bm# is first positionned above the current GBitmap in such a
      way that position (#u#,#v#) in GBitmap #bm# corresponds to position
      (#u#+#x#/#subsample#,#v#+#y#/#subsample#) in the current GBitmap.  This
      mapping results in a contraction of GBitmap #bm# by a factor
      #subsample#.  Each pixel of the current GBitmap can be covered by a
      maximum of #subsample^2# pixels of GBitmap #bm#.  The value of
      each pixel in GBitmap #bm# is then added to the value of the
      corresponding pixel in the current GBitmap.

      {\bf Example}: Assume for instance that the current GBitmap is initially
      white (all pixels have value zero).  Each pixel of the current GBitmap
      then contains the sum of the gray levels of the corresponding pixels in
      GBitmap #bm#.  There are up to #subsample*subsample# such pixels.  If
      for instance GBitmap #bm# is a bilevel image (pixels can be #0# or #1#),
      the pixels of the current GBitmap can take values in range #0# to
      #subsample*subsample#.  Note that function #blit# does not change the
      number of gray levels in the current GBitmap.  You may must call
      \Ref{set_grays} to indicate that there are #subsample^2+1# gray
      levels.  Since there is at most 256 gray levels, this also means that
      #subsample# should never be greater than #15#.

      {\bf Remark}: Arguments #x# and #y# do not represent a position in the
      coordinate system of the current GBitmap.  According to the above
      discussion, the position is (#x/subsample#,#y/subsample#).  In other
      words, you can position the blit with a sub-pixel resolution.  The
      resulting anti-aliasing changes are paramount to the image quality. */
  void blit(const GBitmap *shape, int x, int y, int subsample);
  //@}
  
  /** @name Saving images.  
      The following functions write PBM, PGM and RLE files.  PBM and PGM are
      well known formats for bilevel and gray-level images.  The RLE is a
      simple run-length encoding scheme for bilevel images. These files can be
      read using the ByteStream based constructor or initialization function.
      See \Ref{PNM and RLE file formats} for more information. */
  //@{
  /** Saves the image into ByteStream #bs# using the PBM format.
      Argument #raw# selects the "Raw PBM" or the "Ascii PBM" format.
      The image is saved as a bilevel image. All non zero pixels are
      considered black pixels. */
  void save_pbm(ByteStream &bs, int raw=1);
  /** Saves the image into ByteStream #bs# using the PGM format.
      Argument #raw# selects the "Raw PGM" or the "Ascii PGM" format. 
      The image is saved as a gray level image. */
  void save_pgm(ByteStream &bs, int raw=1);
  /** Saves the image into ByteStream #bs# using the RLE file format.
      The image is saved as a bilevel image. All non zero pixels are
      considered black pixels. */
  void save_rle(ByteStream &bs);
  //@}

  /** @name Stealing or borrowing the memory buffer (advanced). */
  //@{
  /** Steals the memory buffer of a GBitmap.  This function returns the
      address of the memory buffer allocated by this GBitmap object.  The
      offset of the first pixel in the bottom line is written into variable
      #offset#.  Other lines can be accessed using pointer arithmetic (see
      \Ref{rowsize}).  The GBitmap object no longer "owns" the buffer: you
      must explicitly de-allocate the buffer using #operator delete []#.  This
      de-allocation should take place after the destruction or the
      re-initialization of the GBitmap object.  This function will return a
      null pointer if the GBitmap object does not "own" the buffer in the
      first place.  */
  unsigned char *take_data(size_t &offset);
  /** Initializes this GBitmap by borrowing a memory segment.  The GBitmap
      then directly addresses the memory buffer #data# provided by the user.
      This buffer must be large enough to hold #w*h# bytes.  The GBitmap
      object does not "own" the buffer: you must explicitly de-allocate the
      buffer using #operator delete []#.  This de-allocation should take place
      after the destruction or the re-initialization of the GBitmap object.  */
  void borrow_data(unsigned char *data, int w, int h);
  //@}


protected:
  // bitmap components
  unsigned short nrows;
  unsigned short ncolumns;
  unsigned short border;
  unsigned short bytes_per_row;
  unsigned short grays;
  unsigned char *bytes;
  unsigned char *bytes_data;
  unsigned char *rle;
  unsigned int   rlelength;
private:
  // helpers
  static int zerosize;
  static unsigned char *zerobuffer;
  static void zeroes(int ncolumns);
  static unsigned int read_integer(char &lookahead, ByteStream &ref);
  static void euclidian_ratio(int a, int b, int &q, int &r);
  int encode(unsigned char **pruns) const;
  void decode(unsigned char *runs);
  void read_pbm_text(ByteStream &ref); 
  void read_pgm_text(ByteStream &ref); 
  void read_pbm_raw(ByteStream &ref); 
  void read_pgm_raw(ByteStream &ref); 
  void read_rle_raw(ByteStream &ref); 
public:
#ifdef DEBUG
  void check_border() const;
#endif
};


/** @name PNM and RLE file formats

    {\bf PNM} --- There are actually three PNM file formats: PBM for bilevel
    images, PGM for gray level images, and PPM for color images.  These
    formats are widely used by popular image manipulation packages such as
    NetPBM \URL{http://www.arc.umn.edu/GVL/Software/netpbm.html} or
    ImageMagick \URL{http://www.wizards.dupont.com/cristy/}.
    
    {\bf RLE} --- The RLE file format is a simple run-length encoding scheme
    for storing bilevel images.  Encoding or decoding a RLE encoded file is
    extremely simple. Yet RLE encoded files are usually much smaller than the
    corresponding PBM encoded files.  RLE files always begin with a header
    line composed of:\\
    - the two characters #"R4"#,\\
    - one or more blank characters,\\
    - the number of columns, encoded using characters #"0"# to #"9"#,\\
    - one or more blank characters,\\
    - the number of lines, encoded using characters #"0"# to #"9"#,\\
    - exactly one blank character (usually a line-feed character).

    The rest of the file encodes a sequence of numbers representing the
    lengths of alternating runs of white and black pixels.  Lines are encoded
    starting with the top line and progressing towards the bottom line.  Each
    line starts with a white run. The decoder knows that a line is finished
    when the sum of the run lengths for that line is equal to the number of
    columns in the image.  Numbers in range #0# to #191# are represented by a
    single byte in range #0x00# to #0xbf#.  Numbers in range #192# to #16383#
    are represented by a two byte sequence: the first byte, in range #0xc0# to
    #0xff#, encodes the six most significant bits of the number, the second
    byte encodes the remaining eight bits of the number. This scheme allows
    for runs of length zero, which are useful when a line starts with a black
    pixel, and when a very long run (whose length exceeds #16383#) must be
    split into smaller runs.

    @memo
    Simple image file formats.  */

//@}


// ---------------- IMPLEMENTATION

inline unsigned int
GBitmap::rows() const
{
  return nrows;
}

inline unsigned int
GBitmap::columns() const
{
  return ncolumns;
}

inline unsigned int 
GBitmap::rowsize() const
{
  return bytes_per_row;
}

inline int
GBitmap::get_grays() const
{
  return grays;
}

inline unsigned char *
GBitmap::operator[](int row) 
{
  if (!bytes)
    decode(rle);
  if (row<0 || row>=nrows) {
#ifdef DEBUG
    if (zerosize < bytes_per_row + border)
      THROW("debug: zerobuffer is too small");
#endif
    return zerobuffer + border;
  }
  return &bytes[row * bytes_per_row + border];
}

inline const unsigned char *
GBitmap::operator[](int row) const
{
  if (!bytes) 
    ((GBitmap*)this)->decode(rle);
  if (row<0 || row>=nrows) {
#ifdef DEBUG
    if (zerosize < bytes_per_row + border)
      THROW("debug: zerobuffer is too small");
#endif
    return zerobuffer + border;
  }
  return &bytes[row * bytes_per_row + border];
}

inline GBitmap& 
GBitmap::operator=(const GBitmap &ref)
{
  init(ref, ref.border);
  return *this;
}

inline void 
GBitmap::minborder(int minimum)
{
  if (border < minimum)
    {
      if (bytes)
        {
          GBitmap tmp(*this, minimum);
          delete [] bytes_data;
          delete [] rle;
          bytes_per_row = tmp.bytes_per_row;
          bytes = bytes_data = tmp.bytes_data;
          tmp.bytes = tmp.bytes_data = 0;
        }
      border = minimum;
      zeroes(border + ncolumns + border);
    }
}

inline void 
GBitmap::euclidian_ratio(int a, int b, int &q, int &r)
{
  q = a / b;
  r = a - b*q;
  if (r < 0)
  {
    q -= 1;
    r += b;
  }
}

// ---------------- THE END
#endif
