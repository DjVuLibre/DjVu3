//C-  -*- C++ -*-
//C-
//C-  Copyright (c) 1998 AT&T	
//C-  All Rights Reserved 
//C-
//C-  THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
//C-  The copyright notice above does not evidence any
//C-  actual or intended publication of such source code.
//C-
//C-  $Id: DjVuImage.h,v 1.6 1999-02-18 22:46:06 leonb Exp $

#ifndef _DJVUIMAGE_H
#define _DJVUIMAGE_H


/** @name DjVuImage.h

    Files #"DjVuImage.h"# and #"DjVuImage.cpp"# implement the main interface
    for decoding and rendering DjVu Images or IW44 Images.  Class
    \Ref{DjVuImage} is the central component of this interface.  This class
    provides a decoding function and a few rendering functions.  Display
    programs can call the decoding function from a separate thread.  The user
    interface thread may call the rendering functions at any time.  Rendering
    will be performed using the most recent data generated by the decoding
    thread.  This multithreaded capability enabled progressive display of
    remote images.  Program \Ref{djvutopnm} illustrates how class #DjVuImage#
    can be used by a single threaded decoder.  This program first calls the
    decoding function and then renders the desired segment of the image.

    {\bf Creating DjVu images} --- Class \Ref{DjVuImage} does not provide a
    direct way to create a DjVu image.  The recommended procedure consists of
    directly writing the required chunks into an \Ref{IFFByteStream} as
    demonstrated in program \Ref{djvumake}.  Dealing with too many encoding
    issues (such as chunk ordering and encoding quality) would indeed make the
    decoder unnecessarily complex.

    {\bf ToDo: Plugin annotations} --- Class DjVuAnno should move into another
    set of source files in order to implement the annotation chunk parser, and
    to retrieve the hyperlink data structure given a position (x,y).
 
    {\bf ToDo: Multi-page Documents} --- The envisionned multi-page format for
    DjVu documents will considerably change the decoding interface for this
    class.  The responsibility of supervising the decoding tasks will be
    transferred to a new class DjVuDocument.  Individual streams composing the
    multi-page document will be processed by a new class DjVuFile.  Class
    DjVuImage will still contain the rendering function, but the DjVu image
    components will be accessed via pointers to the underlying DjVuFile
    objects.
    
    {\bf ToDo: Layered structure} --- Class #DjVuImage# currently contains an
    unstructured collection of smart pointers to various data structures.
    Although it simplifies the rendering routines, this choice does not
    reflect the layered structure of DjVu images and does not leave much room
    for evolution.  We should be able to do better.

    @memo
    Decoding DjVu and IW44 images.
    @author
    Leon Bottou <leonb@research.att.com>
    @version
    #$Id: DjVuImage.h,v 1.6 1999-02-18 22:46:06 leonb Exp $# */
//@{


#ifdef __GNUC__
#pragma interface
#endif

#include "GString.h"
#include "GThreads.h"
#include "GSmartPointer.h"
#include "ByteStream.h"
#include "IFFByteStream.h"
#include "JB2Image.h"
#include "IWImage.h"
#include "GBitmap.h"
#include "GPixmap.h"


/** @name DjVu version 
    @memo DjVu file format version. */
//@{
/** Current DjVu format version.  The value of this macro represents the
    version of the DjVu file format implemented by this release of the DjVu
    Reference Library. */
#define DJVUVERSION          20
/** Oldest DjVu format version supported by this library.  This release of the
    library cannot completely decode DjVu files whose version field is less
    than or equal to this number. */
#define DJVUVERSION_TOO_OLD  15
/** Newest DjVu format partially supported by this library.  This release of
    the library will attempt to decode files whose version field is smaller
    than this macro.  If the version field is greater than or equal to this
    number, the decoder will just throw a \Ref{GException}.  */
#define DJVUVERSION_TOO_NEW  22
//@}




/** Information component.
    Each instance of class #DjVuInfo# represents the information
    contained in the information chunk of a DjVu file.  This #"INFO"#
    chunk is always the first chunk of a DjVu file.
 */

class DjVuInfo : public GPEnabled
{
public:
  /** Constructs an empty DjVuInfo object.
      The #width# and #height# fields are set to zero.
      All other fields are initialized with suitable default values. */
  DjVuInfo();
  /** Decodes the DjVu #"INFO"# chunk.  This function reads binary data from
      ByteStream #bs# and populates the fields of this DjVuInfo object.  It is
      normally called after detecting an #"INFO"# chunk header with function
      \Ref{IFFByteStream::get_chunk}. */
  void decode(ByteStream &bs);
  /** Encodes the DjVu #"INFO"# chunk. This function writes the fields of this
      DjVuInfo object into ByteStream #bs#. It is normally called after
      creating an #"INFO"# chunk header with function
      \Ref{IFFByteStream::put_chunk}. */
  void encode(ByteStream &bs);  
  /** Returns the number of bytes used by this object. */
  unsigned int get_memory_usage() const;
  /** Width of the DjVu image (in pixels). */
  int width;
  /** Height of the DjVu image (in pixels). */
  int height;
  /** DjVu file version number.  This number characterizes the file format
      version used by the encoder to generate this DjVu image.  A decoder
      should compare this version number with the constants described in
      section "\Ref{DjVu version}". */
  int version;
  /** Resolution of the DjVu image (in pixels per 2.54 cm).  Display programs
      ca use this information to determine the natural magnification to use
      for rendering a DjVu image. */
  int dpi;
  /** Gamma coefficient of the display for which the image was designed.  The
      rendering functions can use this information in order to perform color
      correction for the intended display device. */
  double gamma;
  /** Reserved byte. The IFF padding rules give the opportunity to store an
      extra byte in the #"INFO"# chunk.  This is reserved for possible
      extensions, backward compatibility hacks and other dirty businesses. */
  unsigned char reserved;
};



/** Display annotation component.
    The annotation chunk contains directives for displaying DjVu image, such
    as hyperlinks, border color, centering, preferred zoom factor, etc.
    Directives are encoded in plain text using a lisp like syntax.
    
    {\bf Todo} --- The decoding/encoding functions should actually
    convert the annotation chunk into/from an abstract representation
    of the hyperlinks and display modes.  */

class DjVuAnno : public GPEnabled
{
public:
  /** Constructs an empty annotation object. */
  DjVuAnno();
  /** Decode an annotation chunk.  The annotation data is simply read from
      ByteStream #bs# until reaching an end-of-stream marker.  This function
      is normally called after a call to \Ref{IFFByteStream::get_chunk}. */
  void decode(ByteStream &bs);
  /** Encodes the annotation chunk.  The annotation data is simply written
      into ByteStream #bs# with no IFF header. This function is normally
      called after a call to \Ref{IFFByteStream::put_chunk}. */
  void encode(ByteStream &bs);
  /** Returns the number of bytes needed by this data structure. */
  unsigned int get_memory_usage() const;
  /** Raw annotation data. */
  GString raw;
private:
  GCriticalSection mutex;
};





/** Decoder progress notifier.  The DjVu decoder keeps a pointer to an
    instance of this abstract class.  The virtual functions defined by this
    class are called during the decoding process.  This callback system
    provides a way to monitor the decoding thread and implement a progressive
    display program. */

class DjVuInterface
{
public:
  /** This function is called after decoding each chunk.  
      Argument #chkid# contains the chunk id.  
      Argument #msg# contains a message describing the chunk data. */
  virtual void notify_chunk(const char *chkid, const char *msg);
  /** This function is called after decoding the INFO chunk.
      It indicates that the size of the image is now known. */
  virtual void notify_relayout(void);
  /** This function is called whenever the image should be redrawn.  It
      indicates that enough new data has been received and that the image can
      be redisplayed with increased quality. */
  virtual void notify_redisplay(void);
};




/** DjVu Image.  This class defines the internal representation of a DjVu
    image.  This representation consists of a few pointers referencing the
    various components of the DjVu image.  These components are created and
    populated by the decoding function.  The rendering functions then can use
    the available components to compute a pixel representation of the desired
    segment of the DjVu image. */

class DjVuImage : public GPEnabled
{
public:
  // CONSTRUCTION
  /** @name Construction. */
  //@{
  /** Constructs an empty DjVu image.  
      Function #decode# must then be called to
      populate this DjVu image object. */
  DjVuImage();
  /** Resets a DjVu image and release all memory. 
      Function #decode# must then be called to
      populate this DjVu image object. */
  void init();
  //@}

  // COMPONENTS
  /** @name Components. */
  //@{
  /** Returns a pointer to a DjVu information component.
      This function returns a null pointer until the decoder
      actually processes an #"INFO"# chunk. */
  GP<DjVuInfo>   get_info() const;
  /** Returns a pointer to a DjVu display annotation component.
      This function returns a null pointer until the decoder
      actually processes an #"ANTa"# chunk. */
  GP<DjVuAnno>   get_anno() const;
  /** Returns a pointer to the background component of a DjVu image. The
      background component is always an IW44 image in this
      implementation. This function returns a null pointer until the decoder
      actually processes an #"BG44"# chunk. */
  GP<IWPixmap>   get_bg44() const;
  /** Returns a pointer to the mask of the foreground component of a DjVu
      image. The mask of the foreground component is always a JB2 image in
      this implementation. This function returns a null pointer until the
      decoder actually processes an #"Sjbz"# chunk. */
  GP<JB2Image>   get_fgjb() const;
  /** Returns a pointer to the colors of the foreground component of a DjVu
      image. The mask of the foreground component is always a small pixmap in
      this implementation. This function returns a null pointer until the
      decoder actually processes an #"FG44"# chunk. */
  GP<GPixmap>    get_fgpm() const;
  //@}

  // UTILITIES
  /** @name Utilities */
  //@{
  /** Returns the width of the DjVu image. This function just extracts this
      information from the DjVu information component. It returns zero if such
      a component is not yet available. */
  int get_width() const;
  /** Returns the height of the DjVu image. This function just extracts this
      information from the DjVu information component. It returns zero if such
      a component is not yet available. */
  int get_height() const;
  /** Returns the format version the DjVu data. This function just extracts
      this information from the DjVu information component. It returns zero if
      such a component is not yet available. */
  int get_version() const;
  /** Returns the resolution of the DjVu image (in pixels per 2.54 cm).
      Display programs ca use this information to determine the natural
      magnification to use for rendering a DjVu image. */
  int get_dpi() const;
  /** Returns the gamma coefficient of the display for which the image was
      designed.  The rendering functions can use this information in order to
      perform color correction for the intended display device. */
  double get_gamma() const;
  /** Returns a MIME type string describing the DjVu data.  This information
      is auto-sensed by the decoder.  The MIME type can be #"image/djvu"# or
      #"image/iw44"# depending on the data stream. */
  GString get_mimetype() const;
  /** Returns the memory required to store this image.  This number includes
      the memory required by all the components of the DjVu image. */
  unsigned int get_memory_usage() const;
  /** Returns a short string describing the DjVu image. */
  GString get_short_description() const;
  /** Returns a verbose description of the DjVu image.  This description lists
      all the chunks with their size and a brief comment. */
  GString get_long_description() const;
  //@}

  // DECODING
  /** @name Decoding. */
  //@{
  /** Decodes DjVu data. This function reads binary data from the ByteStream
      #bs# and populates this DjVu image.  The decoder can process both IW44
      Image files and DjVu Image files. IW44 Image files are in fact processed
      as Color DjVu Images.  The member functions of the optional argument
      #notifier# are called at appropriate times during the decoding process.
      See class \Ref{DjVuInterface} for more details. */
  void decode(ByteStream &bs, DjVuInterface *notifier=0);
  //@}

  // CHECKING
  /** @name Checking for legal DjVu files. */
  //@{
  /** This function returns true if this object contains a well formed Color
      DjVu image. Calling function #get_pixmap# on a well formed color image
      should always return a non zero value.  Note that function #get_pixmap#
      works as soon as sufficient information is present, regardless of the
      fact that the image follows the rules or not. */
  int is_legal_color() const;
  /** This function returns true if this object contains a well formed Bilevel
      DjVu image.  Calling function #get_bitmap# on a well formed bilevel
      image should always return a non zero value.  Note that function
      #get_bitmap# works as soon as a foreground mask component is present,
      regardless of the fact that the image follows the rules or not. */
  int is_legal_bilevel() const;
  /** This function returns true if this object contains a well formed
      Compound DjVu image.  Calling function #get_bitmap# or #get_pixmap# on a
      well formed Compound DjVu image should always return a non zero value.
      Note that functions #get_bitmap# or #get_pixmap# works as soon as
      sufficient information is present, regardless of the fact that the image
      follows the rules or not.  */
  int is_legal_compound() const;
  //@}

  // RENDERING 
  /** @name Rendering.  
      All these functions take two rectangles as argument.  Conceptually,
      these function first render the whole image into a rectangular area
      defined by rectangle #all#.  The relation between this rectangle and the
      image size define the appropriate scaling.  The rendering function then
      extract the subrectangle #rect# and return the corresponding pixels as a
      #GPixmap# or #GBitmap# object.  The actual implementation performs these
      two operation simultaneously for obvious efficiency reasons.  The best
      rendering speed is achieved by making sure that the size of rectangle
      #all# and the size of the DjVu image are related by an integer ratio. */
  //@{
  /** Renders the image and returns a color pixel image.  Rectangles #rect#
      and #all# are used as explained above. Color correction is performed
      according to argument #gamma#, which represents the gamma coefficient of
      the display device on which the pixmap will be rendered.  The default
      value, zero, means that no color correction should be performed. 
      This function returns a null pointer if there is not enough information
      in the DjVu image to properly render the desired image. */
  GP<GPixmap>  get_pixmap(const GRect &rect, const GRect &all, double gamma=0) const;
  /** Renders the mask of the foreground layer of the DjVu image.  This
      functions is a wrapper for \Ref{JB2Image::get_bitmap}.  Argument #align#
      specified the alignment of the rows of the returned images.  Setting
      #align# to #4#, for instance, will adjust the bitmap border in order to
      make sure that each row of the returned image starts on a double-word
      (four bytes) boundary.  This function returns a null pointer if there is
      not enough information in the DjVu image to properly render the desired
      image. */
  GP<GBitmap>  get_bitmap(const GRect &rect, const GRect &all, int align = 1) const;
  /** Renders the background layer of the DjVu image.  Rectangles #rect# and
      #all# are used as explained above. Color correction is performed
      according to argument #gamma#, which represents the gamma coefficient of
      the display device on which the pixmap will be rendered.  The default
      value, zero, means that no color correction should be performed.  This
      function returns a null pointer if there is not enough information in
      the DjVu image to properly render the desired image. */
  GP<GPixmap>  get_bg_pixmap(const GRect &rect, const GRect &all, double gamma=0) const;
  /** Renders the foreground layer of the DjVu image.  Rectangles #rect# and
      #all# are used as explained above. Color correction is performed
      according to argument #gamma#, which represents the gamma coefficient of
      the display device on which the pixmap will be rendered.  The default
      value, zero, means that no color correction should be performed.  This
      function returns a null pointer if there is not enough information in
      the DjVu image to properly render the desired image. */
  GP<GPixmap>  get_fg_pixmap(const GRect &rect, const GRect &all, double gamma=0) const;
  //@}

  // SUPERSEDED
  GP<GPixmap>  get_pixmap(const GRect &rect, int subs=1, double gamma=0) const;
  GP<GBitmap>  get_bitmap(const GRect &rect, int subs=1, int align = 1) const;
  GP<GPixmap>  get_bg_pixmap(const GRect &rect, int subs=1, double gamma=0) const;
  GP<GPixmap>  get_fg_pixmap(const GRect &rect, int subs=1, double gamma=0) const;
private:
  // HELPER
  int stencil(GPixmap *pm, const GRect &rect, int subs, double gcorr) const;
  // COMPONENTS
  GP<DjVuInfo>  info;    // INFO component
  GP<DjVuAnno>  anno;    // ANNOTATION component
  GP<IWPixmap>  bg44;    // BACKGROUND component
  GP<JB2Image>  fgjb;    // FOREGROUND MASK component
  GP<GPixmap>   fgpm;    // FOREGROUND COLOR component
  // DECODER INFO
  GString mimetype;
  GString description;
  long filesize;
};

//@}




// INLINE

inline int
DjVuImage::get_width() const
{
  return ( info ? info->width : 0 );
}

inline int
DjVuImage::get_height() const
{
  return ( info ? info->height : 0 );
}

inline int
DjVuImage::get_version() const
{
  return ( info ? info->version : DJVUVERSION );
}

inline int
DjVuImage::get_dpi() const
{
  return ( info ? info->dpi : 300 );
}


inline double
DjVuImage::get_gamma() const
{
  return ( info ? info->gamma : 2.2 );
}

inline GString
DjVuImage::get_mimetype() const
{
  return mimetype;
}


inline GP<DjVuInfo>   
DjVuImage::get_info() const
{
  return info;
}

inline GP<DjVuAnno>   
DjVuImage::get_anno() const
{
  return anno;
}

inline GP<IWPixmap>   
DjVuImage::get_bg44() const
{
  return bg44;
}

inline GP<JB2Image>   
DjVuImage::get_fgjb() const
{
  return fgjb;
}

inline GP<GPixmap>    
DjVuImage::get_fgpm() const
{
  return fgpm;
}



// ----- THE END
#endif
