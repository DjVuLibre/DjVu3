//C-  -*- C++ -*-
//C- DjVu® Reference Library (v. 3.0)
//C- 
//C- Copyright © 1999-2001 LizardTech, Inc. All Rights Reserved.
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
// $Id: XMLTags.h,v 1.3.2.1 2001-03-22 02:04:16 bcr Exp $
// $Name:  $

#ifndef _LT_XMLTAGS__
#define _LT_XMLTAGS__

#ifdef __GNUC__
#pragma interface
#endif

#include "GContainer.h"
#include "GString.h"

class lt_XMLContents;
class DjVuFile;
class DjVuDocument;
class ByteStream;
class XMLByteStream;
class GURL;

class lt_XMLTags : public GPEnabled
{
protected:
  lt_XMLTags(void);
  lt_XMLTags(const char n[]);
public:
  /// Default creator.
  static GP<lt_XMLTags> create(void) { return new lt_XMLTags(); }
  /// Default the specified tag.
  static GP<lt_XMLTags> create(const char n[]) { return new lt_XMLTags(n); }
  /// Non-virtual destructor.
  ~lt_XMLTags();

  inline void addtag(GP<lt_XMLTags> x);
  inline void addraw(GString raw);
  inline GPosition contains(GString name) const;
  inline const GPList<lt_XMLTags> & operator [] (const GString name) const;
  inline const GPList<lt_XMLTags> & operator [] (const GPosition &pos) const;
  void init(XMLByteStream &xmlbs);
  void init(GP<ByteStream> &bs);
  void init(const GURL & url);
  GPList<lt_XMLTags> getTags(char const tagname[]) const;
  static void ParseValues(char const *t, GMap<GString,GString> &args,bool downcase=true);
  static void getMaps(char const tagname[],char const argn[],
    GPList<lt_XMLTags> list, GMap<GString, GP<lt_XMLTags> > &map);
  void write(ByteStream &bs,bool const top=true) const;

  GString name;
  GMap<GString,GString> args;
  GString raw;
  GList<lt_XMLContents> content;
  GMap<GString,GPList<lt_XMLTags> > allTags;
};

class lt_XMLContents
{
public:
  lt_XMLContents(void);
  lt_XMLContents(GP<lt_XMLTags> tag);
  GP<lt_XMLTags> tag;
  GString raw;
  void write(ByteStream &bs) const;
};

inline void
lt_XMLTags::addtag (GP<lt_XMLTags> x)
{
  content.append(lt_XMLContents(x));
  allTags[x->name].append(x);
}

inline void
lt_XMLTags::addraw (GString r)
{
  GPosition pos=content;
  if(pos)
  {
    content[pos].raw+=r;
  }else
  {
    r+=raw;
  }
}

inline GPosition
lt_XMLTags::contains(GString name) const
{
  return allTags.contains(name);
}

inline const GPList<lt_XMLTags> &
lt_XMLTags::operator [] (const GString name) const
{
  return allTags[name];
}

inline const GPList<lt_XMLTags> &
lt_XMLTags::operator [] (const GPosition &pos) const
{
  return allTags[pos];
}

#endif /* _LT_XMLTAGS__ */


