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
//C- $Id: DjVuText.cpp,v 1.2 1999-09-02 19:05:24 leonb Exp $


#ifdef __GNUC__
#pragma implementation
#endif

#include "DjVuText.h"
#include "BSByteStream.h"



DjVuText::Zone::Zone()
  : ztype(DjVuText::PAGE), text_start(0), text_length(0)
{
}



DjVuText::Zone *
DjVuText::Zone::append_child()
{
  Zone empty;
  empty.ztype = ztype;
  empty.text_start = 0;
  empty.text_length = 0;
  children.append(empty);
  return & children[children.lastpos()];
}



void
DjVuText::Zone::cleartext()
{
  text_start = 0;
  text_length = 0;
  for (GPosition i=children; i; ++i)
    children[i].cleartext();
}



void
DjVuText::Zone::normtext(const char *instr, GString &outstr)
{
  if (text_length == 0)
    {
      // Descend collecting text below
      text_start = outstr.length();
      for (GPosition i=children; i; ++i)
        children[i].normtext(instr, outstr);
      text_length = outstr.length() - text_start;
      // Ignore empty zones
      if (text_length == 0)
        return;
    }
  else
    {
      // Collect text at this level
      int new_start = outstr.length();
      outstr = outstr + GString(instr+text_start, text_length);
      text_start = new_start;
      // Clear textual information on lower level nodes
      for (GPosition i=children; i; ++i)
        children[i].cleartext();
    }
  // Determine standard separator
  char sep;
  switch (ztype)
    {
    case COLUMN:
      sep = end_of_column; break;
    case REGION:
      sep = end_of_region; break;
    case PARAGRAPH: 
      sep = end_of_paragraph; break;
    case LINE:
      sep = end_of_line; break;
    default:
      return;
    }
  // Add separator if not present yet.
  if (outstr[text_start+text_length-1] != sep)
    {
      outstr = outstr + GString(&sep, 1);
      text_length += 1;
    }
}


int 
DjVuText::Zone::check(int maxtext) const
{
  // Sanity checks
  if (ztype<PAGE || ztype>CHARACTER)
    return 0;
  if (rect.isempty())
    return 0;
  if (text_start<0 || text_start+text_length>maxtext)
    return 0;
  for (GPosition i=children; i; ++i)
    if (! children[i].check(maxtext) )
      return 0;
  return 1;
}



void 
DjVuText::Zone::encode(ByteStream &bs) const
{
  // Encode type
  bs.write8(ztype);
  // Encode rectangle
  bs.write24(rect.xmin);
  bs.write24(rect.xmax);
  bs.write24(rect.ymin);
  bs.write24(rect.ymax);
  // Encode text info
  bs.write24(text_start);
  bs.write24(text_length);
  // Encode number of children
  bs.write24(children.size());
  // Encode all children
  for (GPosition i=children; i; ++i)
    children[i].encode(bs);
}



void 
DjVuText::Zone::decode(ByteStream &bs)
{
  // Decode type
  ztype = (ZoneType) bs.read8();
  if ( ztype<PAGE || ztype>CHARACTER )
    THROW("Corrupted text zone hierarchy");
  // Decode rectangle
  rect.xmin = bs.read24();
  rect.xmax = bs.read24();
  rect.ymin = bs.read24();
  rect.ymax = bs.read24();
  // Decode text info
  text_start = bs.read24();
  text_length = bs.read24();
  // Get children size
  int size = bs.read24();
  // Process children
  children.empty();
  while (size-- > 0) {
    Zone *z = append_child();
    z->decode(bs);
  }
}



void 
DjVuText::normalize_text()
{
  GString newtextUTF8;
  main.normtext( (const char*)textUTF8, newtextUTF8);
  textUTF8 = newtextUTF8;
}



int 
DjVuText::has_valid_zones() const
{
  if ( !textUTF8)
    return 0;
  if ( main.children.isempty() )
    return 0;
  if (! main.check(textUTF8.length()))
    return 0;
  return 1;
}



void 
DjVuText::encode_zones(ByteStream &bs) const
{
  BSByteStream bsb(bs,50);
  // Encode version number
  bsb.write8(Zone::version);
  // Encode zones
  main.encode(bsb);
}



void 
DjVuText::decode_zones(ByteStream &bs)
{
  BSByteStream bsb(bs);
  // Check version number
  int version = bsb.read8();
  if (version != Zone::version)
    THROW("Unsupported version tag in text zone information");
  // Decode zones
  main.decode(bsb);
}



void 
DjVuText::encode_text(ByteStream &bs) const
{
  int textsize = textUTF8.length();
  int blocksize = 1 + (textsize>>10);
  if (blocksize < 10) blocksize = 10;
  if (blocksize > 1024) blocksize = 1024;
  BSByteStream bsb(bs, blocksize);
  bsb.writall( (void*)(const char*)textUTF8, textsize );
}



void 
DjVuText::decode_text(ByteStream &bs)
{
  textUTF8.empty();
  BSByteStream bsb(bs);
  for(;;) {
    char buffer[1024];
    int size = bsb.read(buffer, sizeof(buffer));
    if (size <= 0) break;
    textUTF8 = textUTF8 + GString(buffer, size);
  }
}


