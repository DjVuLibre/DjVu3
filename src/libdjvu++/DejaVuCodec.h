//C-  -*- C++ -*-
//C-
//C-  Copyright (c) 1988 AT&T	
//C-  All Rights Reserved 
//C-
//C-  THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
//C-  The copyright notice above does not evidence any
//C-  actual or intended publication of such source code.
//C-
//C-  $Id: DejaVuCodec.h,v 1.1.1.1 1999-01-22 00:40:19 leonb Exp $


#ifndef _DEJAVUCODEC_H
#define _DEJAVUCODEC_H

//**** File "$Id: DejaVuCodec.h,v 1.1.1.1 1999-01-22 00:40:19 leonb Exp $
// - Performs DEJAVU decode
// - Author: Leon Bottou, 07/1997


#ifdef __GNUC__
#pragma interface
#endif

#include "DjVuGlobal.h"
#include "GString.h"
#include "GSmartPointer.h"
#include "ByteStream.h"
#include "IFFByteStream.h"
#include "JB2Codec.h"
#include "IWCodec.h"
#include "GBitmap.h"
#include "GPixmap.h"


//**** DEJAVUVERSION
// The version of the decoder.
// Plugin will warn if version is less than required

#define DEJAVUVERSION          18
#define DEJAVUVERSION_TOO_OLD  15
#define DEJAVUVERSION_TOO_NEW  20


//**** Struct DejaVuInfo
// This structure is a view of the INFO chunk.

struct DejaVuInfo
{
  unsigned char width_hi, width_lo;      // Page width
  unsigned char height_hi, height_lo;    // Page height
  unsigned char version_lo, version_hi;  // DjVu version number
  unsigned char dpi_lo, dpi_hi;          // Resolution (dpi)
  char gamma10;                          // Gamma*10 (e.g. 22=2.2)
};


//**** Class DejaVuImage
// This class represents a DejaVu image

class DejaVuImage : public GPEnabled
{
public:
  // Construction
  DejaVuImage();
  // Utilities
  int          get_width() const;
  int          get_height() const;
  int          get_version() const;
  int          get_dpi() const;
  double       get_target_gamma() const;
  unsigned int get_memory_usage() const;
  int          get_suggested_scales(int maxscales, GRatio *scales);
  int	       is_color(void);
  // Get pixmap
  GP<GBitmap>   get_bitmap(GRatio scale=1, int align=1);
  GP<GBitmap>   get_bitmap(const GRect &rect, GRatio scale=1, int align = 1);
  GP<GPixmap>   get_background_pixmap(const GRect &rect, GRatio scale=1, double gamma=0);
  GP<GPixmap>   get_foreground_pixmap(const GRect &rect, GRatio scale=1, double gamma=0);
  GP<GPixmap>   get_color_pixmap(const GRect &rect, GRatio scale=1, double gamma=0);
  GString      get_short_description();
  GString      get_long_description();
public:
  // Page Information
  DejaVuInfo   info;
  // DjVu Components
  int          width;
  int          height;
  GP<GPixmap>   bgpm;
  GP<IWPixmap> bg44;
  GP<JB2Image> jb2stencil;
  GP<GPixmap>   fgpm;
  GString      annotation;
  GString      mimetype;
  // Plugin stuff ???
  GArray<unsigned char> orig_file;
private:
  // Friends
  friend class DejaVuDecoder;
  // Information
  GString desc;
  long filesize;
  long deltasize;
  // Helpers
  void add_description(const char *s);
  int apply_stencil(GPixmap *pm, const GRect &rect, GRatio scale, double gamma);
};


//**** Class DejaVuDecoder
// This class represents the decoder for format DEJAVU

class DejaVuDecoder : public GPEnabled
{
public:
  // constructor and destructor
  DejaVuDecoder(ByteStream &bs);
  virtual ~DejaVuDecoder();
  // decoding
  // Arguments to the callback are as follows:
  // callback(arg, redraw, chunk_name, message);
  // if (redraw), the image will be redrawn in the callback
  // if (message!=NULL), it will be displayed in the status line
  void decode(DejaVuImage * dimg,
	      void (*callback)(void *, int, const char *, const char *)=0,
	      void * arg=0);
  void decode(DejaVuImage * dimg, GString * chunk_name,
	      GString * message, int * do_redraw, int * call_again);
  ByteStream * GetByteStream(void) { return &bs; };
protected:
  // Implementation
  IFFByteStream *iff;
  ByteStream &bs;
  // Disable assignment semantic
  DejaVuDecoder(const DejaVuDecoder &ref);
  DejaVuDecoder& operator=(const DejaVuDecoder &ref);
  // Helpers
  void decode_djvu(DejaVuImage *dimg, GString * chunk_name,
		   GString * message, int * do_redraw, int * call_again);
  void decode_iw44(DejaVuImage *dimg, GString * chunk_name,
		   GString * message, int * do_redraw, int * call_again);
};




// INLINE

inline int
DejaVuImage::get_width() const
{
  return width;
}

inline int
DejaVuImage::get_height() const
{
  return height;
}


// ----- THE END
#endif
