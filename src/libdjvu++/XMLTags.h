//C-  Copyright © 2000-2001, LizardTech, Inc. All Rights Reserved.
//C-              Unauthorized use prohibited.
//
// $Id: XMLTags.h,v 1.1 2001-01-17 00:14:55 bcr Exp $
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

class lt_XMLTags : public GPEnabled
{
public:
  lt_XMLTags(void);
  lt_XMLTags(const char n[]);
  ~lt_XMLTags();
  GString name;
  GMap<GString,GString> args;
  GString raw;
  GList<lt_XMLContents> content;
  GMap<GString,GPList<lt_XMLTags> > allTags;
  inline void addtag(GP<lt_XMLTags> x);
  inline void addraw(GString raw);
  inline GPosition contains(GString name) const;
  inline const GPList<lt_XMLTags> & operator [] (const GString name) const;
  inline const GPList<lt_XMLTags> & operator [] (const GPosition &pos) const;
  void init(XMLByteStream &xmlbs);
  void init(GP<ByteStream> &bs);
  void init(char const name[]);
  GPList<lt_XMLTags> getTags(char const tagname[]) const;
  static void ParseValues(char const *t, GMap<GString,GString> &args,bool downcase=true);
  static void getMaps(char const tagname[],char const argn[],
    GPList<lt_XMLTags> list, GMap<GString, GP<lt_XMLTags> > &map);
  void write(ByteStream &bs,bool const top=true) const;
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


