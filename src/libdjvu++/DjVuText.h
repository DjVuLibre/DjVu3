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
//C- $Id: DjVuText.h,v 1.2 1999-09-02 19:21:58 leonb Exp $


#ifndef _DJVUTEXT_H_
#define _DJVUTEXT_H_

#ifdef __GNUC__
#pragma interface
#endif

#include "DjVuGlobal.h"

#include <string.h>
#include <new.h>
#include "GException.h"
#include "GSmartPointer.h"
#include "GString.h"
#include "ByteStream.h"
#include "GContainer.h"
#include "GRect.h"



/** @name DjVuText.h
    
    Files #"DjVuText.h"# and #"DjVuText.cpp"# implement the class
    \Ref{DjVuText} for describing the text contained in a DjVu page.  The text
    is described as a hierarchy of zones corresponding to page, column,
    region, paragraph, lines, words, etc...

    @memo 
    Encoding of textual information for a DjVu image.
    @version 
    #$Id: DjVuText.h,v 1.2 1999-09-02 19:21:58 leonb Exp $# 
    @author: 
    Steven Pigeon <pigeon@research.att.com> -- initial implementation \\
    L\'eon Bottou <leonb@research.att.com> -- made it square 
*/
//@{




/** Description of the text contained in a DjVu page.  This class contains the
    textual data for the page.  It describes the text as a hierarchy of zones
    corresponding to page, column, region, paragraph, lines, words, etc...
    The piece of text associated with each zone is represented by an offset
    and a length describing a segment of a global UTF8 encoded string.  */

class DjVuText : public GPEnabled
{
public:
  /** These constant are used to tell what a zone describes.
      This can be useful for a copy/paste application. 
      The deeper we go into the hierarchy, the higher the constant. */
  enum ZoneType { PAGE=1, COLUMN=2, REGION=3, PARAGRAPH=4, LINE=5, WORD=6, CHARACTER=7 };
  /** Data structure representing document textual components.
      The text structure is represented by a hierarchy of rectangular zones. */
  struct Zone 
  {
    Zone();
    /** Type fo the zone. */
    enum ZoneType ztype;
    /** Rectangle spanned by the zone */
    GRect rect;
    /** Position of the zone text in string #textUTF8#. */
    int text_start;
    /** Length of the zone text in string #textUTF8#. */
    int text_length;
    /** List of children zone. */
    GList<Zone> children;
    /** Appends another subzone inside this zone.  The new zone is initialized
        with an empty rectangle, empty text, and has the same type as this
        zone. */
    Zone *append_child();
  private:
    friend class DjVuText;
    void cleartext();
    void normtext(const char *instr, GString &outstr);
    static const int version = 0;
    void encode(ByteStream &bs) const;
    void decode(ByteStream &bs, int maxtext);
  };

  /** Textual data for this page.  
      The content of this string is encoded using the UTF8 code.
      This code corresponds to ASCII for the first 127 characters.
      Columns, regions, paragraph and lines are delimited by the following
      control character:
      \begin{tabular}{lll}
        {\bf Name} & {\bf Octal} & {\bf Ascii name} \\\hline\\
        {\tt DjVuText::end_of_column}    & 013 & VT, Vertical Tab \\
        {\tt DjVuText::end_of_region}    & 035 & GS, Group Separator \\
        {\tt DjVuText::end_of_paragraph} & 037 & US, Unit Separator \\
        {\tt DjVuText::end_of_line}      & 012 & LF: Line Feed
      \end{tabular} */
  GString textUTF8;
  static const char end_of_column    = 013;      // VT: Vertical Tab
  static const char end_of_region    = 035;      // GS: Group Separator
  static const char end_of_paragraph = 037;      // US: Unit Separator
  static const char end_of_line      = 012;      // LF: Line Feed
  /** Main zone in the document.
      This zone represent the page. */
  Zone main;
  /** Tests whether there is a meaningful zone hierarchy. */
  int has_valid_zones() const;
  /** Normalize textual data.
      Assuming that a zone hierarchy has been built and represents the reading order.
      This function reorganizes the string #textUTF8# by gathering the highest level text
      available in the zone hierarchy.  The text offsets and lengths are recomputed
      for all the zones in the hierarchy. */
  void normalize_text();
  /** Encode data for a chunk describing the page text. */
  void encode_text(ByteStream &bs) const;
  /** Encode data for a chunk describing the page zones. */
  void encode_zones(ByteStream &bs) const;
  /** Decode data from a chunk describing the page text. */
  void decode_text(ByteStream &bs);
  /** Decode data from a chunk describing the page zones. */
  void decode_zones(ByteStream &bs);
};




//@}

// ------------ THE END
#endif
      
      
             

    
