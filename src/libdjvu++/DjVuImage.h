//C-  -*- C++ -*-
//C- DjVu� Reference Library (v. 3.0)
//C- 
//C- Copyright � 1999-2000 LizardTech, Inc. All Rights Reserved.
//C- The DjVu Reference Library is protected by U.S. Pat. No.
//C- 6,058,214 and patents pending.
//C- 
//C- This software is subject to, and may be distributed under, the
//C- GNU General Public License, Version 2. The license should have
//C- accompanied the software or you may obtain a copy of the license
//C- from the Free Software Foundation at http://www.fsf.org .
//C- 
//C- The computer code originally released by LizardTech under this
//C- license and unmodified by other parties is deemed the "LizardTech
//C- Original Code."
//C- 
//C- With respect to the LizardTech Original Code ONLY, and subject
//C- to any third party intellectual property claims, LizardTech
//C- grants recipient a worldwide, royalty-free, non-exclusive license
//C- under patent claims now or hereafter owned or controlled by
//C- LizardTech that are infringed by making, using, or selling
//C- LizardTech Original Code, but solely to the extent that any such
//C- patent(s) is/are reasonably necessary to enable you to make, have
//C- made, practice, sell, or otherwise dispose of LizardTech Original
//C- Code (or portions thereof) and not to any greater extent that may
//C- be necessary to utilize further modifications or combinations.
//C- 
//C- The LizardTech Original Code is provided "AS IS" WITHOUT WARRANTY
//C- OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED
//C- TO ANY WARRANTY OF NON-INFRINGEMENT, OR ANY IMPLIED WARRANTY OF
//C- MERCHANTIBILITY OR FITNESS FOR A PARTICULAR PURPOSE.
//C- 
// 
// $Id: DjVuImage.h,v 1.34 2000-12-18 17:13:42 bcr Exp $
// $Name:  $

#ifndef _DJVUIMAGE_H
#define _DJVUIMAGE_H


/** @name DjVuImage.h

    Files #"DjVuImage.h"# and #"DjVuImage.cpp"# implement \Ref{DjVuImage}
    class produced as a result of decoding of DjVu files. In the previous
    version of the library both the rendering {\bf and} decoding have been
    handled by \Ref{DjVuImage}. This is no longer true. Now the
    \Ref{DjVuDocument} and \Ref{DjVuFile} classes handle decoding of both
    single page and multi page documents.

    For compatibility reasons though, we still support old-style decoding
    interface through the \Ref{DjVuImage} class for single page documents
    {\em only}. As before, the display programs can call the decoding
    function from a separate thread.  The user interface thread may call
    the rendering functions at any time. Rendering will be performed using
    the most recent data generated by the decoding thread. This multithreaded
    capability enables progressive display of remote images.

    {\bf Creating DjVu images} --- Class \Ref{DjVuImage} does not provide a
    direct way to create a DjVu image.  The recommended procedure consists of
    directly writing the required chunks into an \Ref{IFFByteStream} as
    demonstrated in program \Ref{djvumake}.  Dealing with too many encoding
    issues (such as chunk ordering and encoding quality) would indeed make the
    decoder unnecessarily complex.

    {\bf ToDo: Layered structure} --- Class #DjVuImage# currently contains an
    unstructured collection of smart pointers to various data structures.
    Although it simplifies the rendering routines, this choice does not
    reflect the layered structure of DjVu images and does not leave much room
    for evolution.  We should be able to do better.

    @memo
    Decoding DjVu and IW44 images.
    @author
    L\'eon Bottou <leonb@research.att.com> - initial implementation
    Andrei Erofeev <eaf@geocities.com> - multipage support
    @version
    #$Id: DjVuImage.h,v 1.34 2000-12-18 17:13:42 bcr Exp $# */
//@{


#ifdef __GNUC__
#pragma interface
#endif

#include "DjVuFile.h"

/* Obsolete class included for backward compatibility. */

class DjVuInterface
{
public:
  virtual void notify_chunk(const char *chkid, const char *msg) = 0;
  virtual void notify_relayout(void) = 0;
  virtual void notify_redisplay(void) = 0;
};


/** Main DjVu Image data structure.  This class defines the internal
    representation of a DjVu image.  This representation consists of a few
    pointers referencing the various components of the DjVu image.  These
    components are created and populated by the decoding function.  The
    rendering functions then can use the available components to compute a
    pixel representation of the desired segment of the DjVu image. */

class DjVuImage : public DjVuPort
{
public:
  // CONSTRUCTION
  /** @name Construction. */
  //@{
  /** Constructs an empty DjVu image. After the image has been constructed,
      it may be connected to an existing \Ref{DjVuFile} or left as is.

      In the former case #DjVuImage# will look for its decoded components
      (like #Sjbz# or #BG44#) by decending the hierarchy of \Ref{DjVuFile}s
      starting from the one passed to \Ref{connect}().

      In the latter case you can use \Ref{decode}() function to decode
      {\bf single-page} DjVu documents in the old-style way. */
  DjVuImage(void);
  /** Connects this #DjVuImage# to the passed \Ref{DjVuFile}. The #DjVuImage#
      will use this \Ref{DjVuFile} to retrieve components necessary for
      decoding. It will also connect itself to \Ref{DjVuFile} using the
      communication mechanism provided by \Ref{DjVuPort} and \Ref{DjVuPortcaster}.
      This will allow it to receive and relay messages and requests generated
      by the passed \Ref{DjVuFile} and any file included into it. */
  void		connect(const GP<DjVuFile> & file);
      
  //@}

  // COMPONENTS
  /** @name Components. */
  //@{
  /** Returns a pointer to a DjVu information component.
      This function returns a null pointer until the decoder
      actually processes an #"INFO"# chunk. */
  GP<DjVuInfo>    get_info() const;
  /** Returns a pointer to the IW44 encoded background component of a DjVu
      image.  This function returns a null pointer until the decoder actually
      processes an #"BG44"# chunk. */
  GP<IWPixmap>    get_bg44() const;
  /** Returns a pointer to the raw background component of a DjVu image. The
      background component is used for JPEG encoded backgrounds.  This
      function returns a null pointer until the decoder actually processes an
      #"BGjp"# chunk. */
  GP<GPixmap>     get_bgpm() const;
  /** Returns a pointer to the mask of the foreground component of a DjVu
      image. The mask of the foreground component is always a JB2 image in
      this implementation. This function returns a null pointer until the
      decoder actually processes an #"Sjbz"# chunk. */
  GP<JB2Image>    get_fgjb() const;
  /** Returns a pointer to the colors of the foreground component of a DjVu
      image. The mask of the foreground component is always a small pixmap in
      this implementation. This function returns a null pointer until the
      decoder actually processes an #"FG44"# chunk. */
  GP<GPixmap>     get_fgpm() const;
  /** Returns a pointer to a palette object containing colors for the 
      foreground components of a DjVu image.  These colors are only
      pertinent with respect to the JB2Image. */
  GP<DjVuPalette> get_fgbc() const;
  /** Returns a pointer to a ByteStream containing all the annotation
      chunks collected so far for this image.  Individual chunks can be
      retrieved using \Ref{IFFByteStream}. Returns NULL if no chunks have been
      collected yet. */
  GP<ByteStream> get_anno() const;
  /** Returns a pointer to a ByteStream containing all the hidden text.
      Returns NULL if no chunks have been collected yet. */
  GP<ByteStream> get_text() const;
  //@}

  // NEW STYLE DECODING
  /** @name New style decoding. */
  //@{
  /** The decoder is now started when the image is created
      by function \Ref{DjVuDocument::get_page} in \Ref{DjVuDocument}. 
      This function waits until the decoding thread terminates
      and returns TRUE if the image has been successfully decoded. */
  bool wait_for_complete_decode(void);
  //@}
  
  // OLD STYLE DECODING
  /** @name Old style decoding (backward compatibility). */
  //@{
  /** This function is here for backward compatibility. Now, with the
      introduction of multipage DjVu documents, the decoding is handled
      by \Ref{DjVuFile} and \Ref{DjVuDocument} classes. For single page
      documents though, we still have this wrapper. */
  void decode(ByteStream & str, DjVuInterface *notifier=0);
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
      such a component is not yet available.  This version number should
      be compared with the \Ref{DjVu version constants}. */
  int get_version() const;
  /** Returns the resolution of the DjVu image. This information is given in
      pixels per 2.54 cm.  Display programs can use this information to
      determine the natural magnification to use for rendering a DjVu
      image. */
  int get_dpi() const;
  /** Same as \Ref{get_dpi}() but instead of precise value returns the closest
      "standard" one: 25, 50, 75, 100, 150, 300, 600. If dpi is greater than
      700, it's returned as is. */
  int get_rounded_dpi() const;
  /** Returns the gamma coefficient of the display for which the image was
      designed.  The rendering functions can use this information in order to
      perform color correction for the intended display device. */
  double get_gamma() const;
  /** Returns a MIME type string describing the DjVu data.  This information
      is auto-sensed by the decoder.  The MIME type can be #"image/djvu"# or
      #"image/iw44"# depending on the data stream. */
  GString get_mimetype() const;
  /** Returns a short string describing the DjVu image.
      Example: #"2500x3223 in 23.1 Kb"#. */
  GString get_short_description() const;
  /** Returns a verbose description of the DjVu image.  This description lists
      all the chunks with their size and a brief comment, as shown in the
      following example.
      \begin{verbatim}
      DJVU Image (2325x3156) version 17:
       0.0 Kb   'INFO'  Page information.
       17.3 Kb  'Sjbz'  JB2 foreground mask (2325x3156)
       2.5 Kb   'BG44'  IW44 background (775x1052)
       1.0 Kb   'FG44'  IW44 foreground colors (194x263)
       3.0 Kb   'BG44'  IW44 background (part 2).
       0.9 Kb   'BG44'  IW44 background (part 3).
       7.1 Kb   'BG44'  IW44 background (part 4).
      Compression ratio: 676 (31.8 Kb)
      \end{verbatim} */
  GString get_long_description() const;
  /** Returns pointer to \Ref{DjVuFile} which contains this image in
      compressed form. */
  GP<DjVuFile> get_djvu_file(void) const;
  //@}

  // CHECKING
  /** @name Checking for legal DjVu files. */
  //@{
  /** This function returns true if this object contains a well formed {\em
      Photo DjVu Image}. Calling function #get_pixmap# on a well formed photo
      image should always return a non zero value.  Note that function
      #get_pixmap# works as soon as sufficient information is present,
      regardless of the fact that the image follows the rules or not. */
  int is_legal_photo() const;
  /** This function returns true if this object contains a well formed {\em
      Bilevel DjVu Image}.  Calling function #get_bitmap# on a well formed
      bilevel image should always return a non zero value.  Note that function
      #get_bitmap# works as soon as a foreground mask component is present,
      regardless of the fact that the image follows the rules or not. */
  int is_legal_bilevel() const;
  /** This function returns true if this object contains a well formed {\em
      Compound DjVu Image}.  Calling function #get_bitmap# or #get_pixmap# on
      a well formed compound DjVu image should always return a non zero value.
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
      make sure that each row of the returned image starts on a word (four
      byte) boundary.  This function returns a null pointer if there is not
      enough information in the DjVu image to properly render the desired
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

  // Inherited from DjVuPort.
  virtual void notify_chunk_done(const DjVuPort *, const char *name);

  // SUPERSEDED
  GP<GPixmap>  get_pixmap(const GRect &rect, int subs=1, double gamma=0) const;
  GP<GBitmap>  get_bitmap(const GRect &rect, int subs=1, int align = 1) const;
  GP<GPixmap>  get_bg_pixmap(const GRect &rect, int subs=1, double gamma=0) const;
  GP<GPixmap>  get_fg_pixmap(const GRect &rect, int subs=1, double gamma=0) const;
private:
  GP<DjVuFile>		file;
  bool			relayout_sent;
  
  // HELPERS
  int stencil(GPixmap *pm, const GRect &rect, int subs, double gcorr) const;
  GP<DjVuInfo>		get_info(const GP<DjVuFile> & file) const;
  GP<IWPixmap>		get_bg44(const GP<DjVuFile> & file) const;
  GP<GPixmap>		get_bgpm(const GP<DjVuFile> & file) const;
  GP<JB2Image>		get_fgjb(const GP<DjVuFile> & file) const;
  GP<GPixmap>		get_fgpm(const GP<DjVuFile> & file) const;
  GP<DjVuPalette>      get_fgbc(const GP<DjVuFile> & file) const;
};


inline GP<DjVuFile>
DjVuImage::get_djvu_file(void) const
{
   return file;
}

inline bool
DjVuImage::wait_for_complete_decode(void)
{
  if (file) 
    {
      file->wait_for_finish();
      return file->is_decode_ok();
    }
  return 0;
}

//@}







// ----- THE END
#endif
