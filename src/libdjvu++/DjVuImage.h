//C-  -*- C++ -*-
//C-
//C-  Copyright (c) 1998 AT&T	
//C-  All Rights Reserved 
//C-
//C-  THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
//C-  The copyright notice above does not evidence any
//C-  actual or intended publication of such source code.
//C-
//C-  $Id: DjVuImage.h,v 1.1 1999-02-01 18:57:33 leonb Exp $

#ifndef _DJVUIMAGE_H
#define _DJVUIMAGE_H


/** @name DjVuImage.h

    Files #"DjVuImage.h"# and #"DjVuImage.cpp"# provide the main entry points
    for decoding and processing a DjVu image.  The major component is class
    \Ref{DjVuImage} which represents a document image using the DjVu layered
    scheme.
    


    {\bf ToDo: Multi-page Documents} --- The envisionned multi-page format for
    DjVu documents will considerably change the decoding interface for this
    class.  The responsibility of supervising the decoding tasks will be
    transferred to a new class DjVuDocument.  Individual streams composing the
    multi-page document will be processed by a new class DjVuFile.  Class
    DjVuImage will still contain the rendering function, but the DjVu image
    components will be accessed via pointers to the underlying DjVuFile
    objects.
    
    {\bf ToDo: Annotations} --- Class DjVuAnno should move into another
    set of source files in order to implement the annotation chunk parser,
    and to retrieve the hyperlink data structure given a position (x,y).
    
    @memo
    Decoding DjVu images.
    @author
    Leon Bottou <leonb@research.att.com>
    @version
    #$Id: DjVuImage.h,v 1.1 1999-02-01 18:57:33 leonb Exp $# */
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


/** DjVu format version. */
//@{
/** Current DjVu format version.  The value of this macro represents the
    version of the DjVu file format implemented by this release of the DjVu
    Reference Library. */
#define DJVUVERSION          18
/** Oldest DjVu format version supported by this library.  This release of the
    library cannot completely decode DjVu files whose version field is less
    than or equal to this number. */
#define DJVUVERSION_TOO_OLD  15
/** Newest DjVu format partially supported by this library.  This release of
    the library will attempt to decode files whose version field is smaller
    than this macro.  If the version field is greater than or equal to this
    number, the decoder will just throw a \Ref{GException}.  */
#define DJVUVERSION_TOO_NEW  20
//@}






class DjVuInfo : public GPEnabled
{
public:
  DjVuInfo();
  void decode(ByteStream &bs);
  void encode(ByteStream &bs);  
  unsigned int get_memory_usage() const;
  int    width;
  int    height;
  int    version;
  int    dpi;
  double gamma;
};



/** Annotation chunk.  
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



/** Decoder progress notifier.
 */
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
  /** This function is called whenever the image should be redrawn.
      It indicates that enough new data has been received. The image
      can be redisplayed with increased quality. */
  virtual void notify_redisplay(void);
};




class DjVuImage : public GPEnabled
{
public:
  // CONSTRUCTION
  DjVuImage();
  void init();
  // COMPONENTS
  GP<DjVuInfo>   get_info() const;
  GP<DjVuAnno>   get_annotation() const;
  GP<JB2Image>   get_stencil() const;
  GP<IWPixmap>   get_bg44() const;
  GP<GPixmap>    get_bgpm() const;
  GP<GPixmap>    get_fgpm() const;
  // UTILITIES
  int get_width() const;
  int get_height() const;
  int get_version() const;
  GString get_mimetype() const;
  unsigned int get_memory_usage() const;
  GString get_short_description() const;
  GString get_long_description() const;
  // DECODING
  void decode(ByteStream &bs, DjVuInterface *notifier=0);
  // CHECKING 
  int is_legal_color() const;
  int is_legal_bilevel() const;
  // RENDERING 
  GP<GPixmap>  get_pixmap(const GRect &rect, const GRect &all, double gamma=0) const;
  GP<GBitmap>  get_bitmap(const GRect &rect, const GRect &all, int align = 1) const;
  GP<GPixmap>  get_bg_pixmap(const GRect &rect, const GRect &all, double gamma=0) const;
  GP<GPixmap>  get_fg_pixmap(const GRect &rect, const GRect &all, double gamma=0) const;
  // RENDERING (LOW LEVEL)
  GP<GPixmap>  get_pixmap(const GRect &rect, int subs=1, double gamma=0) const;
  GP<GBitmap>  get_bitmap(const GRect &rect, int subs=1, int align = 1) const;
  GP<GPixmap>  get_bg_pixmap(const GRect &rect, int subs=1, double gamma=0) const;
  GP<GPixmap>  get_fg_pixmap(const GRect &rect, int subs=1, double gamma=0) const;
private:
  // HELPERS
  int apply_stencil(GPixmap *pm, const GRect &rect, int subsample, double gamma) const;
  // COMPONENTS
  GP<DjVuInfo> info;
  GP<DjVuAnno> anno;
  GP<GPixmap>  bgpm;
  GP<IWPixmap> bg44;
  GP<JB2Image> stencil;
  GP<GPixmap>  fgpm;
  // DECODER
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
  return ( info ? info->version : 0 );
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
DjVuImage::get_annotation() const
{
  return anno;
}

inline GP<JB2Image>   
DjVuImage::get_stencil() const
{
  return stencil;
}

inline GP<IWPixmap>   
DjVuImage::get_bg44() const
{
  return bg44;
}

inline GP<GPixmap>    
DjVuImage::get_bgpm() const
{
  return bgpm;
}

inline GP<GPixmap>    
DjVuImage::get_fgpm() const
{
  return fgpm;
}



// ----- THE END
#endif
