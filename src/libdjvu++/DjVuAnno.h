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
//C- $Id: DjVuAnno.h,v 1.8 1999-10-04 22:33:33 eaf Exp $

#ifndef _DJVUANNO_H
#define _DJVUANNO_H


/** @name DjVuAnno.h

    Files #"DjVuAnno.h"# and #"DjVuAnno.cpp"# implement the mechanism for
    annotating DjVuImages. Annotations are additional instructions for the
    plugin about how the image should be displayed. These instructions
    include:
    \begin{itemize}
       \item the background color
       \item image alignment in the browser (when the image is smaller than
             the viewport)
       \item hyperlinks definitions. Plugins v.2.0+ allows you to jump from
             page to page by supporting hyperlinks.
       \item start zoom
    \end{itemize}

    The annotations are stored inside annotation chunk called #ANTa# and
    are decoded like any other part of \Ref{DjVuImage} during the decoding
    stage. Class \Ref{DjVuAnno} is responsible for handling decode/encode
    requests.

    @memo Implements support for DjVuImage annotations
    @author Andrei Erofeev <eaf@geocities.com>
    @version
    #$Id: DjVuAnno.h,v 1.8 1999-10-04 22:33:33 eaf Exp $# */
//@{


#ifdef __GNUC__
#pragma interface
#endif

#include "GString.h"
#include "GThreads.h"
#include "GSmartPointer.h"
#include "ByteStream.h"
#include "DjVuGlobal.h"
#include "GMapAreas.h"
#include "GContainer.h"

/** Implements support for \Ref{DjVuImage} annotations.
    The exact format of annotations is not strictly defined. The only
    requirement is that they have to be stored as a sequence of chunks
    inside a #FORM:ANNO#.

    This file implements annotations, that DjVu plugin and encoders are
    using: contents of #ANT*# and #TXT*# chunks.

    Contents of the #FORM:ANNO# should be passed to \Ref{DjVuAnno::decode}()
    for parsing, which initializes \Ref{DjVuAnno::ANT} and \Ref{DjVuAnno::TXT}
    and fills them with decoded data.
*/

/** This class contains some trivial annotations of the page or of the document
    such as:
    \begin{itemize}
       \item {\bf Page background color}: The color of border around image
             displayed
       \item {\bf Page alignment}
       \item {\bf Initial zoom and mode}
       \item {\bf Hyperlinks and highlighted areas}.
    \end{itemize}

    All this information is put inside a textual chunk #ANTa# in pseudo-lisp
    format. Decoding and encoding are normally done by \Ref{DjVuAnno::decode}()
    and \Ref{DjVuAnno::encode}() functions. */
class DjVuANT : public GPEnabled
{
public:
   enum { MODE_UNSPEC=0, MODE_COLOR, MODE_FORE, MODE_BACK, MODE_BW };
   enum { ZOOM_STRETCH=-4, ZOOM_ONE2ONE=-3, ZOOM_WIDTH=-2,
	  ZOOM_PAGE=-1, ZOOM_UNSPEC=0 };
   enum { ALIGN_UNSPEC=0, ALIGN_LEFT, ALIGN_CENTER, ALIGN_RIGHT,
	  ALIGN_TOP, ALIGN_BOTTOM };

      /** Background color. Is in #0x00RRBBGG# format. #0xffffffff# if
	  there were no background color records in the annotation chunk. */
   u_int32	bg_color;
      /** Initial zoom. Possible values are:
	  \begin{itemize}
	     \item #ZOOM_STRETCH# - the image is stretched to the viewport
	     \item #ZOOM_ONE2ONE# - the image is displayed pixel-to-pixel
	     \item #ZOOM_WIDTH# - "Fit width" mode
	     \item #ZOOM_PAGE# - "Fit page" mode
	     \item #ZOOM_UNSPEC# - the zoom is not specified in the annotation
	           chunk.
          \end{itemize} */
   int		zoom;
      /** Initial mode. Possible values are:
	  \begin{itemize}
	     \item #MODE_COLOR# - color mode
	     \item #MODE_FORE# - foreground mode
	     \item #MODE_BACK# - background mode
	     \item #MODE_BW# - black and white mode
	     \item #MODE_UNSPEC# - the mode is not specified in the annotation
	     chunk.
          \end{itemize} */
   int		mode;
      /** Horizontal page alignment. Possible values are #ALIGN_LEFT#,
	  #ALIGN_CENTER#, #ALIGN_RIGHT# and #ALIGN_UNSPEC#. */
   int		hor_align;
      /** Vertical page alignment. Possible values are #ALIGN_TOP#,
	  #ALIGN_CENTER#, #ALIGN_BOTTOM# and #ALIGN_UNSPEC#. */
   int		ver_align;
      /** List of defined map areas. They may be just areas of highlighting
	  or hyperlink. Please refer to \Ref{GMapArea}, \Ref{GMapRect},
	  \Ref{GMapPoly} and \Ref{GMapOval} for details. */
   GPList<GMapArea>	map_areas;

      /// Constructs an empty annotation object.
   DjVuANT();
   virtual ~DjVuANT();

      /** Returns TRUE if no features are specified or specified features
	  are not different from default ones */
   bool		is_empty(void) const;
      /** Decodes contents of annotation chunk #ANTa#. The chunk data is
	  read from ByteStream #bs# until reaching an end-of-stream marker.
	  This function is normally called after a call to
	  \Ref{IFFByteStream::get_chunk}(). */
   void	decode(ByteStream & bs);
      /** Same as \Ref{decode}() but adds the new data to what has
	  been decoded before. */
   void merge(ByteStream & bs);
      /** Encodes the #ANTa# chunk. The annotation data is simply written
	  into ByteStream #bs# with no IFF header. This function is normally
	  called after a call to \Ref{IFFByteStream::put_chunk}(). */
   void encode(ByteStream &bs);
      /// Encodes data back into raw annotation data.
   GString encode_raw(void) const;

      /// Returns a copy of this object
   GP<DjVuANT>	copy(void) const;
   
      /** Returns the number of bytes needed by this data structure. It's
	  used by caching routines to estimate the size of a \Ref{DjVuImage}. */
   unsigned int get_memory_usage() const;
private:
   void			decode(class GLParser & parser);
   
   static GString	read_raw(ByteStream & str);
   
   static u_int32	cvt_color(const char * color, u_int32 def);
   static unsigned char	decode_comp(char ch1, char ch2);
   static u_int32	get_bg_color(class GLParser & parser);
   static int		get_zoom(class GLParser & parser);
   static int		get_mode(class GLParser & parser);
   static int		get_hor_align(class GLParser & parser);
   static int		get_ver_align(class GLParser & parser);
   static GPList<GMapArea>get_map_areas(class GLParser & parser);
   static void		del_all_items(const char * name, class GLParser & parser);
};

/** This is a top-level class containing annotations of a DjVu document (or just
    a page). It has only two functions: \Ref{encode}() and \Ref{decode}().
    Both of them work with a sequence of annotation chunks from #FORM:ANNO#
    form. Basing on the name of the chunks they call #encode()# and
    #decode()# functions of the proper annotation structure (like
    \Ref{ANT}). The real work of encoding and decoding is done by lower-level
    classes. */
class DjVuAnno : public GPEnabled
{
public:
   GP<DjVuANT>	ant;
   
      /** Decodes a sequence of annotation chunks and merges contents of every
	  chunk with previously decoded information. This function
	  should be called right after applying \Ref{IFFByteStream::get_chunk}()
	  to data from #FORM:ANNO#. */
   void decode(ByteStream & bs);
      /** Encodes all annotations back into a sequence of chunks to be put
	  inside a #FORM:ANNO#. */
   void	encode(ByteStream & bs);

      /// Returns a copy of this object
   GP<DjVuAnno>	copy(void) const;

      /** Returns the number of bytes needed by this data structure. It's
	  used by caching routines to estimate the size of a \Ref{DjVuImage}. */
   unsigned int get_memory_usage() const;
};

//@}

// ----- THE END
#endif
