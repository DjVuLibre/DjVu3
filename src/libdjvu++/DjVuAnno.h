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
//C- $Id: DjVuAnno.h,v 1.7 1999-09-30 19:16:13 eaf Exp $

#ifndef _DJVUANNO_H
#define _DJVUANNO_H


/** @name DjVuAnno.h

    Files #"DjVuAnno.h"# and #"DjVuAnno.cpp"# implement the mechanism for
    annotating DjVuImages. Annotations are additional instructions to the
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
    #$Id: DjVuAnno.h,v 1.7 1999-09-30 19:16:13 eaf Exp $# */
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
    The annotation chunk contains directives for displaying DjVu image, such
    as hyperlinks, border color, centering, preferred zoom factor, etc.
    Directives are encoded in plain text using a lisp like syntax.

    This class can decode the contents of this chunk and encode them back
    when necessary. After a class objecthas been created and initialized by
    an #ANTa# chunk, access to the contents of the annotation chunk is
    easy and straightforward through the object variables.
*/

class DjVuAnno : public GPEnabled
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
   DjVuAnno();
   virtual ~DjVuAnno();
      /** Decode an annotation chunk.  The annotation data is simply read from
	  ByteStream #bs# until reaching an end-of-stream marker.  This
	  function is normally called after a call to
	  \Ref{IFFByteStream::get_chunk}(). */
   void decode(ByteStream &bs);
      /** Decode an annotation chunk. This is a "convenience" function, which
	  differs from the function above only by the source of data. */
   void	decode(const char * data);
      /** Same as \Ref{decode}() but adds the new data to one that has
	  been decoded before. */
   void merge(ByteStream & bs);
      /** Encodes the annotation chunk.  The annotation data is simply written
	  into ByteStream #bs# with no IFF header. This function is normally
	  called after a call to \Ref{IFFByteStream::put_chunk}(). */
   void encode(ByteStream &bs);
      /** Returns the number of bytes needed by this data structure. It's
	  used by caching routines to estimate the size of a \Ref{DjVuImage}. */
   unsigned int get_memory_usage() const;
      /// Encodes data back into raw annotation data.
   GString encode_raw(void) const;

      /// Returns TRUE if no features are specified.
   bool		is_empty(void) const;
private:
   GCriticalSection class_lock;

   void		decode(class GLParser & parser);
   
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


//@}

// ----- THE END
#endif
