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
//C- $Id: JB2Image.h,v 1.12.2.2 1999-07-13 19:53:06 leonb Exp $

#ifndef _JB2IMAGE_H
#define _JB2IMAGE_H

/** @name JB2Image.h

    Files #"JB2Image.h"# and #"JB2Image.cpp"# address the compression of
    bilevel images using the JB2 soft pattern matching scheme.  These files
    provide the complete decoder and the decoder back-end.  The JB2 scheme is
    optimized for images containing a large number of self-similar small
    components such as characters.  Typical text images can be compressed into
    files 3 to 5 times smaller than with G4/MMR and 2 to 4 times smaller than
    with JBIG1.

    {\bf JB2 and JBIG2} --- JB2 has strong similarities with the forthcoming
    JBIG2 standard developped by the "ISO/IEC JTC1 SC29 Working Group 1" which
    is responsible for both the JPEG and JBIG standards.  This is hardly
    surprising since JB2 was our own proposal for the JBIG2 standard
    and remained the only proposal for years.  The full JBIG2 standard however
    is significantly more complex and slighlty less efficient than JB2 because
    it addresses a broader range of applications.  Full JBIG2 compliance may
    be implemented in the future.

    {\bf JB2 Images} --- Class \Ref{JB2Image} is the central data structure
    implemented here.  A #JB2Image# is composed of an array of shapes
    and an array of blits.  Each shape contains a small bitmap representing an
    elementary blob of ink, such as a character or a segment of line art.
    Each blit instructs the decoder to render a particular shape at a
    specified position in the image.  Some compression is already achieved
    because several blits can refer to the same shape.  A shape can also
    contain a pointer to a parent shape.  Additional compression is achieved
    when both shapes are similar because each shape is encoded using the
    parent shape as a model.  A #"O"# shape for instance could be a parent for
    both a #"C"# shape and a #"Q"# shape.

    {\bf JB2 Dictionary} --- Class \Ref{JB2Dict} is a peculiar kind of
    JB2Image which only contains an array of shapes.  These shapes can be
    referenced from another JB2Dict/JB2Image.  This is arranged by setting the
    ``inherited dictionary'' of a JB2Dict/JB2Image using function
    \Ref{JB2Dict::set_inherited_dict}. Several JB2Images can use shapes from a
    same JB2Dict encoded separately.  This is how several pages of a same
    document can share information.
    
    {\bf Decoding JB2 data} --- The first step for decoding JB2 data consists of 
    creating an empty #JB2Image# object.  Function \Ref{JB2Image::decode} then
    reads the data and populates the #JB2Image# with the shapes and the blits.
    Function \Ref{JB2Image::get_bitmap} finally produces an anti-aliased image.

    {\bf Encoding JB2 data} --- The first step for decoding JB2 data also
    consists of creating an empty #JB2Image# object.  You must then use
    functions \Ref{JB2Image::add_shape} and \Ref{JB2Image::add_blit} to
    populate the #JB2Image# object.  Function \Ref{JB2Image::encode} finally
    produces the JB2 data.  Function #encode# sequentially encodes the blits
    and the necessary shapes.  The compression ratio depends on several
    factors:
    \begin{itemize}
    \item Blits should reuse shapes as often as possible.
    \item Blits should be sorted in reading order because this facilitates
          the prediction of the blit coordinates.
    \item Shapes should be sorted according to the order of first appearance
          in the sequence of blits because this facilitates the prediction of the
          shape indices.
    \item Shapes should be compared to all previous shapes in the shape array.
          The shape parent pointer should be set to a suitable parent shape if
          such a parent shape exists.  The parent shape should have almost the
          same size and the same pixels.
    \end{itemize}
    All this is quite easy to achieve in the case of an electronically
    produced document such as a DVI file or a PS file: we know what the
    characters are and where they are located.  If you only have a scanned
    image however you must first locate the characters (connected component
    analysis) and cut the remaining pieces of ink into smaller blobs.
    Ordering the blits and matching the shapes is then an essentially
    heuristic process.  Although the quality of the heuristics substantially
    effects the file size, misordering blits or mismatching shapes never
    effects the quality of the image.  The last refinement consists in
    smoothing the shapes in order to reduce the noise and maximize the
    similarities between shapes.
    
    {\bf ToDo} --- Some improvements have been planned for a long time: (a)
    Shapes eventually will contain information about the baseline: this could
    improve the handling of the character descenders and also will provide a
    more understandable way to superpose matching shapes.  (b) JB2 files
    eventually will be able to reference external shape dictionaries: common
    characters will be shared between document pages.  (c) There will be a way
    to specify a color for each shape: this is good for encoding
    electronically produced documents.
    
    {\bf References} 
    \begin{itemize}
    \item Paul G. Howard : {\em Text image compression using soft 
          pattern matching}, Computer Journal, volume 40:2/3, 1997.
    \item JBIG1 : \URL{http://www.jpeg.org/public/jbighomepage.htm}.
    \item JBIG2 draft : \URL{http://www.jpeg.org/public/jbigpt2.htm}.
    \end{itemize}

    @version
    #$Id: JB2Image.h,v 1.12.2.2 1999-07-13 19:53:06 leonb Exp $#
    @memo
    Coding bilevel images with JB2.
    @author
    Paul Howard <pgh@research.att.com> -- JB2 design\\
    L\'eon Bottou <leonb@research.att.com> -- this implementation */
//@{

#ifdef __GNUC__
#pragma interface
#endif

#include "DjVuGlobal.h"
#include "GString.h"
#include "Arrays.h"
#include "GSmartPointer.h"
#include "ZPCodec.h"
#include "GRect.h"
#include "GBitmap.h"

class JB2Dict;
class JB2Image;

/** Blit data structure.  A #JB2Image# contains an array of #JB2Blit# data
    structures.  Each array entry instructs the decoder to render a particular
    shape at a particular location.  Members #left# and #bottom# specify the
    coordinates of the bottom left corner of the shape bitmap.  All
    coordinates are relative to the bottom left corner of the image.  Member
    #shapeno# is the subscript of the shape to be rendered.  */

class JB2Blit {
public:
  /** Horizontal coordinate of the blit. */
  unsigned short left;
  /** Vertical coordinate of the blit. */
  unsigned short bottom;
  /** Index of the shape to blit. */
  unsigned int shapeno;
};


/** Shape data structure.  A #JB2Image# contains an array of #JB2Shape# data
    structures.  Each array entry represents an elementary blob of ink such as
    a character or a segment of line art.  Member #bits# points to a bilevel
    image representing the shape pixels.  Member #parent# is the subscript of
    the parent shape.  */

class JB2Shape { 
public: 
  /** Subscript of the parent shape.  The parent shape must always be located
      before the current shape in the shape array.  A negative value indicates
      that this shape has no parent.  Any negative values smaller than #-1#
      further indicates that this shape does not look like a character.  This
      is used to enable a few internal optimizations.  This information is
      saved into the JB2 file, but the actual value of the #parent# variable
      is not. */
  int parent; 
  /** Bilevel image of the shape pixels.  This must be a pointer to a bilevel
      #GBitmap# image.  This pointer can also be null. The encoder will just
      silently discard all blits referring to a shape containing a null
      bitmap. */
  GP<GBitmap> bits;
  /** Private user data. This long word is provided as a convenience for users
      of the JB2Image data structures.  Neither the rendering functions nor
      the coding functions ever access this value. */
  long userdata;
};



/** JB2 Dictionary callback.
    The decoding function call this callback function when they discover that
    the current JB2Image or JB2Dict needs a pre-existing shape dictionary. 
    The callback function must return a pointer to the dictionary or NULL
    if none is found. */

typedef GP<JB2Dict> JB2DecoderCallback ( void* );


/** Dictionary of JB2 shapes. */

class JB2Dict : public GPEnabled
{
public:
  // CONSTRUCTION
  /** Null Constructor.  Constructs an empty #JB2Dict# object.  You can then
      call the decoding function #decode#.  You can also manually set the
      image size using #add_shape#. */
  JB2Dict();
  // INITIALIZATION
  /** Resets the #JB2Image# object.  This function reinitializes both the shape
     and the blit arrays.  All allocated memory is freed. */
  void init();

  // INHERITED
  /** Returns the inherited dictionary. */
  GP<JB2Dict> get_inherited_dict() const;
  /** Returns the number of inherited shapes. */
  int get_inherited_shape_count() const;
  /** Sets the inherited dictionary. */
  void set_inherited_dict(GP<JB2Dict> dict);

  // ACCESSING THE SHAPE LIBRARY
  /** Returns the total number of shapes.
      Shape indices range from #0# to #get_shape_count()-1#. */
  int get_shape_count() const;
  /** Returns a pointer to shape #shapeno#.
      The returned pointer directly points into the shape array.
      This pointer can be used for reading or writing the shape data. */
  JB2Shape *get_shape(int shapeno);
  /** Returns a constant pointer to shape #shapeno#.
      The returned pointer directly points into the shape array.
      This pointer can only be used for reading the shape data. */
  const JB2Shape *get_shape(int shapeno) const;
  /** Appends a shape to the shape array.  This function appends a copy of
      shape #shape# to the shape array and returns the subscript of the new
      shape.  The subscript of the parent shape #shape.parent# must 
      actually designate an already existing shape. */
  int  add_shape(const JB2Shape &shape);

  // MEMORY OPTIMIZATION
  /** Compresses all shape bitmaps.  This function reduces the memory required
      by the #JB2Image# by calling \Ref{GBitmap::compress} on all shapes
      bitmaps.  This function is best called after decoding a #JB2Image#,
      because function \Ref{get_bitmap} can directly use the compressed
      bitmaps.  */
  void compress();
  /** Returns the total memory used by the JB2Image.
      The returned value is expressed in bytes. */
  unsigned int get_memory_usage() const;

  // CODING
  /** Encodes the JB2Dict into ByteStream #bs#.  
      This function generates the JB2 data stream without any header.   */
  void encode(ByteStream &bs) const;
  /** Decodes JB2 data from ByteStream #bs#. This function decodes the image
      size and populates the shape and blit arrays.  The callback function
      #cb# is called when the decoder determines that the ByteStream data
      requires a shape dictionary which has not been set with
      \Ref{JB2Dict::set_inherited_dict}. The callback receives argument #arg#
      and must return a suitable dictionary which will be installed as the
      inherited dictionary.  The callback should return null if no such
      dictionary is found. */
  void decode(ByteStream &bs, JB2DecoderCallback *cb=0, void *arg=0);

  
public:
  /** Comment string.  
      This variable holds an optional comment included in JB2 files. */
  GString comment;

private:
  int inherited_shapes;
  GP<JB2Dict> inherited_dict;
  DArray<JB2Shape> shapes;
  
};



/** Main JB2 data structure.  Each #JB2Image# consists of an array of shapes
    and an array of blits.  These arrays can be populated by hand using
    functions \Ref{add_shape} and \Ref{add_blit}, or by decoding JB2 data
    using function \Ref{decode}.  You can then use function \Ref{get_bitmap}
    to render anti-aliased images, or use function \Ref{encode} to generate
    JB2 data. */

class JB2Image : public JB2Dict
{
public:

  // CONSTRUCTION
  /** Null Constructor.  Constructs an empty #JB2Image# object.  You can then
      call the decoding function #decode#.  You can also manually set the
      image size using #set_dimension# and populate the shape and blit arrays
      using #add_shape# and #add_blit#. */
  JB2Image();

  // INITIALIZATION
  /** Resets the #JB2Image# object.  This function reinitializes both the shape
     and the blit arrays.  All allocated memory is freed. */
  void init();

  // DIMENSION
  /** Returns the width of the image.  
      This is the width value previously set with #set_dimension#. */
  int get_width() const;
  /** Returns the height of the image.  
      This is the height value previously set with #set_dimension#. */
  int get_height() const;
  /** Sets the size of the JB2Image.
      This function can be called at any time. 
      The corresponding #width# and the #height# are stored
      in the JB2 file. */
  void set_dimension(int width, int height);

  // RENDERING
  /** Renders an anti-aliased gray level image.  This function renders the
      JB2Image as a bilevel or gray level image.  Argument #subsample#
      specifies the desired subsampling ratio in range #1# to #15#.  The
      returned image uses #1+subsample^2# gray levels for representing
      anti-aliased edges.  Argument #align# specified the alignment of the
      rows of the returned images.  Setting #align# to #4#, for instance, will
      adjust the bitmap border in order to make sure that each row of the
      returned image starts on a word (four byte) boundary. */
  GP<GBitmap> get_bitmap(int subsample = 1, int align = 1) const;
  /** Renders an anti-aliased gray level sub-image.  This function renders a
      segment of the JB2Image as a bilevel or gray level image.  Conceptually,
      this function first renders the full JB2Image with subsampling ratio
      #subsample# and then extracts rectangle #rect# in the subsampled image.
      Both operations of course are efficiently performed simultaneously.
      Argument #align# specified the alignment of the rows of the returned
      images, as explained above.  Argument #dispy# should remain null. */
  GP<GBitmap> get_bitmap(const GRect &rect, int subsample=1, int align=1, int dispy=0) const;

  // ACCESSING THE BLIT LIBRARY
  /** Returns the total number of blits.
      Blit indices range from #0# to #get_blit_count()-1#. */
  int get_blit_count() const;
  /** Returns a pointer to blit #blitno#.
      The returned pointer directly points into the blit array.
      This pointer can be used for reading or writing the blit data. */
  JB2Blit *get_blit(int blitno);
  /** Returns a constant pointer to blit #blitno#.
      The returned pointer directly points into the shape array.
      This pointer can only be used for reading the shape data. */
  const JB2Blit *get_blit(int blitno) const;
  /** Appends a blit to the blit array.  This function appends a copy of blit
      #blit# to the blit array and returns the subscript of the new blit.  The
      shape subscript #blit.shapeno# must actually designate an already
      existing shape. */
  int  add_blit(const JB2Blit &blit);

  // MEMORY OPTIMIZATION
  /** Returns the total memory used by the JB2Image.
      The returned value is expressed in bytes. */
  unsigned int get_memory_usage() const;

  // CODING
  /** Encodes the JB2Image into ByteStream #bs#.  
      This function generates the JB2 data stream without any header. */
  void encode(ByteStream &bs) const;
  /** Decodes JB2 data from ByteStream #bs#. This function decodes the image
      size and populates the shape and blit arrays.  The callback function
      #cb# is called when the decoder determines that the ByteStream data
      requires a shape dictionary which has not been set with
      \Ref{JB2Dict::set_inherited_dict}. The callback receives argument #arg#
      and must return a suitable dictionary which will be installed as the
      inherited dictionary.  The callback should return null if no such
      dictionary is found. */
  void decode(ByteStream &bs, JB2DecoderCallback *cb=0, void *arg=0);
  
private:
  // Implementation
  int width;
  int height;
  TArray<JB2Blit> blits;
};


//@}


// JB2DICT INLINE FUNCTIONS




inline int
JB2Dict::get_shape_count() const
{
  return inherited_shapes + shapes.size();
}

inline int
JB2Dict::get_inherited_shape_count() const
{
  return inherited_shapes;
}

inline GP<JB2Dict>
JB2Dict::get_inherited_dict() const
{
  return inherited_dict;
}

inline JB2Shape *
JB2Dict::get_shape(int shapeno)
{
  if (shapeno >= inherited_shapes)
    return & shapes[shapeno - inherited_shapes];
  else if (inherited_dict)
    return inherited_dict->get_shape(shapeno);
  return 0;
}

inline const JB2Shape *
JB2Dict::get_shape(int shapeno) const
{
  if (shapeno >= inherited_shapes)
    return & shapes[shapeno - inherited_shapes];
  else if (inherited_dict)
    return inherited_dict->get_shape(shapeno);
  return 0;
}


// JB2IMAGE INLINE FUNCTIONS

inline int
JB2Image::get_width() const
{
  return width;
}

inline int
JB2Image::get_height() const
{
  return height;
}


inline int
JB2Image::get_blit_count() const
{
  return blits.size();
}


inline JB2Blit *
JB2Image::get_blit(int blitno)
{
  return & blits[blitno];
}

inline const JB2Blit *
JB2Image::get_blit(int blitno) const
{
  return & blits[blitno];
}


#endif
