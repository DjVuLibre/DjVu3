//C-  -*- C++ -*-
//C- DjVu� Reference Library (v. 3.0)
//C- 
//C- Copyright � 1999-2001 LizardTech, Inc. All Rights Reserved.
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
// 
// $Id: DjVuText.h,v 1.15 2001-07-03 00:21:13 mchen Exp $
// $Name:  $

#ifndef _DJVUTEXT_H
#define _DJVUTEXT_H



/** @name DjVuText.h

    Files #"DjVuText.h"# and #"DjVuText.cpp"# implement the mechanism for
    text in DjVuImages.

    This file implements annotations understood by the DjVu plugins 
    and encoders.


    using: contents of #TXT*# chunks.

    Contents of the #FORM:TEXT# should be passed to \Ref{DjVuText::decode}()
    for parsing, which initializes \Ref{DjVuText::TXT} 
    and fills them with decoded data. 
    @memo Implements support for DjVuImage hidden text.
    @author Andrei Erofeev <eaf@geocities.com>
    @version
    #$Id: DjVuText.h,v 1.15 2001-07-03 00:21:13 mchen Exp $# */
//@{

#ifdef __GNUC__
#pragma interface
#endif

#include "GMapAreas.h"

class ByteStream;

// -------- DJVUTXT --------

/** Description of the text contained in a DjVu page.  This class contains the
    textual data for the page.  It describes the text as a hierarchy of zones
    corresponding to page, column, region, paragraph, lines, words, etc...
    The piece of text associated with each zone is represented by an offset
    and a length describing a segment of a global UTF8 encoded string.  */

class DjVuTXT : public GPEnabled
{
protected:
  DjVuTXT(void) {}
public:
  /// Default creator
  static GP<DjVuTXT> create(void) {return new DjVuTXT();}

  /** These constants are used to tell what a zone describes.
      This can be useful for a copy/paste application. 
      The deeper we go into the hierarchy, the higher the constant. */
  enum ZoneType { PAGE=1, COLUMN=2, REGION=3, PARAGRAPH=4, LINE=5, WORD=6, CHARACTER=7 };
  /** Data structure representing document textual components.
      The text structure is represented by a hierarchy of rectangular zones. */
  struct Zone 
  {
    Zone();
    /** Type of the zone. */
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
    friend class DjVuTXT;
    void cleartext();
    void normtext(const char *instr, GUTF8String &outstr);
    unsigned int memuse() const;
    static const int version;
    void encode(
      const GP<ByteStream> &bs, const Zone * parent=0, const Zone * prev=0) const;
    void decode(const GP<ByteStream> &bs, int maxtext,
	            	const Zone * parent=0, const Zone * prev=0);
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
  GUTF8String textUTF8;
  static const char end_of_column    ;      // VT: Vertical Tab
  static const char end_of_region    ;      // GS: Group Separator
  static const char end_of_paragraph ;      // US: Unit Separator
  static const char end_of_line      ;      // LF: Line Feed
  /** Main zone in the document.
      This zone represent the page. */
  Zone page_zone;
  /** Tests whether there is a meaningful zone hierarchy. */
  int has_valid_zones() const;
  /** Normalize textual data.  Assuming that a zone hierarchy has been built
      and represents the reading order.  This function reorganizes the string
      #textUTF8# by gathering the highest level text available in the zone
      hierarchy.  The text offsets and lengths are recomputed for all the
      zones in the hierarchy. Separators are inserted where appropriate. */
  void normalize_text();
  /** Encode data for a TXT chunk. */
  void encode(const GP<ByteStream> &bs) const;
  /** Decode data from a TXT chunk. */
  void decode(const GP<ByteStream> &bs);
  /** Returns a copy of this object. */
  GP<DjVuTXT> copy(void) const;
  /// Write XML formated text.
  void writeText(ByteStream &bs,const int height) const;
  /// Get XML formatted text.
  GUTF8String get_xmlText(const int height) const;
  /** Searches the TXT chunk for the given string and returns a list of
      the smallest zones covering the text.
      @param string String to be found. May contain spaces as word separators.
      @param start_pos Position where to start searching. It may be negative
             or it may be bigger than the length of the \Ref{textUTF8}
	     string. If the #start_pos# is out of bounds, it will be fixed
	     before starting the search
	     \begin{itemize}
	        \item If #start_pos# is negative and we search forward,
		      the #start_pos# will be reset to #0#.
		\item If #start_pos# is too big and we search backward,
		      the #start_pos# will be reset to the #textUTF8.length()-1#.
		\item Otherwise the #start_pos# will remain unchanged, and
		      nothing will be found.
	     \end{itemize}
	     
	     If the function manages to find an occurrence of the string,
	     it will modify the #start_pos# to point to it. If no match has
	     been found, the #start_pos# will be reset to some big number
	     if searching forward and #-1# otherwise.
      @param search_fwd #TRUE# means to search forward. #FALSE# - backward.
      @param match_case If set to #FALSE# the search will be case-insensitive.
      @param whole_word If set to #TRUE# the function will try to find
	     a whole word matching the passed string. The word separators
	     are all blank and punctuation characters. The passed
	     string may {\bf not} contain word separators, that is it
	     {\bf must} be a whole word.

      {\bf WARNING:} The returned list contains pointers to Zones.
      {\bf DO NOT DELETE} these Zones.
      */
  GList<Zone *> search_string(GUTF8String string, int & start_pos,
			      bool search_fwd, bool match_case,
			      bool whole_word=false) const;

  GList<Zone *> find_text_in_rect(GRect target_rect, GUTF8String &text) const;

   // get all zones of zone type zone_type under node parent. zone_list
   // contains the return value
   void get_zones(int zone_type, const Zone *parent, GList<Zone *> & zone_list) const;
     
  /** Returns the number of bytes needed by this data structure. It's
      used by caching routines to estimate the size of a \Ref{DjVuImage}. */
  unsigned int get_memory_usage() const;
private:
  bool		search_zone(const Zone * zone, int start, int & end) const;
  Zone	*	get_smallest_zone(int max_type, int start, int & end) const;
  GList<Zone *>	find_zones(int string_start, int string_length) const;
};


class DjVuText : public GPEnabled
{
protected:
   DjVuText(void) {}
public:
   /// Default creator.
   static GP<DjVuText> create(void) {return new DjVuText();}

      /** Decodes a sequence of annotation chunks and merges contents of every
	  chunk with previously decoded information. This function
	  should be called right after applying \Ref{IFFByteStream::get_chunk}()
	  to data from #FORM:TEXT#. */
   void decode(const GP<ByteStream> &bs);

      /** Encodes all annotations back into a sequence of chunks to be put
	  inside a #FORM:TEXT#. */
   void	encode(const GP<ByteStream> &bs);

      /// Returns a copy of this object
   GP<DjVuText>	copy(void) const;

      /** Returns the number of bytes needed by this data structure. It's
	  used by caching routines to estimate the size of a \Ref{DjVuImage}. */
   inline unsigned int get_memory_usage() const;

   /// Write XML formated text.
   void writeText(ByteStream &bs,const int height) const;

   /// Get XML formatted text.
   GUTF8String get_xmlText(const int height) const;

   GP<DjVuTXT>  txt;
private: // dummy stuff
   static void decode(ByteStream *);
   static void	encode(ByteStream *);
};

//@}

inline unsigned int
DjVuText::get_memory_usage() const
{
  return (txt)?(txt->get_memory_usage()):0;
}


// ----- THE END
#endif


