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
// $Id: DjVuText.cpp,v 1.28 2001-07-06 18:35:10 mchen Exp $
// $Name:  $

#ifdef __GNUC__
#pragma implementation
#endif

#include "DjVuText.h"
#include "IFFByteStream.h"
#include "BSByteStream.h"
#include "debug.h"
#include <ctype.h>

#ifdef min
#undef min
#endif
template<class TYPE>
static inline TYPE min(TYPE a,TYPE b) { return (a<b)?a:b; }

//***************************************************************************
//******************************** DjVuTXT **********************************
//***************************************************************************

const char DjVuTXT::end_of_column    = 013;      // VT: Vertical Tab
const char DjVuTXT::end_of_region    = 035;      // GS: Group Separator
const char DjVuTXT::end_of_paragraph = 037;      // US: Unit Separator
const char DjVuTXT::end_of_line      = 012;      // LF: Line Feed

const int DjVuTXT::Zone::version  = 1;

DjVuTXT::Zone::Zone()
  : ztype(DjVuTXT::PAGE), text_start(0), text_length(0), zone_parent(0)
{
}

DjVuTXT::Zone *
DjVuTXT::Zone::append_child()
{
  Zone empty;
  empty.ztype = ztype;
  empty.text_start = 0;
  empty.text_length = 0;
  empty.zone_parent=this;
  children.append(empty);
  return & children[children.lastpos()];
}

void
DjVuTXT::Zone::cleartext()
{
  text_start = 0;
  text_length = 0;
  for (GPosition i=children; i; ++i)
    children[i].cleartext();
}

void
DjVuTXT::Zone::normtext(const char *instr, GUTF8String &outstr)
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
      outstr = outstr + GUTF8String(instr+text_start, text_length);
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
    case WORD:
      sep = ' '; break;
    default:
      return;
    }
  // Add separator if not present yet.
  if (outstr[text_start+text_length-1] != sep)
    {
      outstr = outstr + GUTF8String(&sep, 1);
      text_length += 1;
    }
}

unsigned int 
DjVuTXT::Zone::memuse() const
{
  int memuse = sizeof(*this);
  for (GPosition i=children; i; ++i)
    memuse += children[i].memuse();
  return memuse;
}


#ifndef NEED_DECODER_ONLY
void 
DjVuTXT::Zone::encode(
  const GP<ByteStream> &gbs, const Zone * parent, const Zone * prev) const
{
  ByteStream &bs=*gbs;
  // Encode type
  bs.write8(ztype);
  
  // Modify text_start and bounding rectangle based on the context
  // (whether there is a previous non-zero same-level-child or parent)
  int start=text_start;
  int x=rect.xmin, y=rect.ymin;
  int width=rect.width(), height=rect.height();
  if (prev)
  {
    if (ztype==PAGE || ztype==PARAGRAPH || ztype==LINE)
    {
      // Encode offset from the lower left corner of the previous
      // child in the coord system in that corner with x to the
      // right and y down
      x=x-prev->rect.xmin;
      y=prev->rect.ymin-(y+height);
    } else // Either COLUMN or WORD or CHARACTER
    {
      // Encode offset from the lower right corner of the previous
      // child in the coord system in that corner with x to the
      // right and y up
      x=x-prev->rect.xmax;
      y=y-prev->rect.ymin;
    }
    start-=prev->text_start+prev->text_length;
  } else if (parent)
  {
    // Encode offset from the upper left corner of the parent
    // in the coord system in that corner with x to the right and y down
    x=x-parent->rect.xmin;
    y=parent->rect.ymax-(y+height);
    start-=parent->text_start;
  }
  // Encode rectangle
  bs.write16(0x8000+x);
  bs.write16(0x8000+y);
  bs.write16(0x8000+width);
  bs.write16(0x8000+height);
  // Encode text info
  bs.write16(0x8000+start);
  bs.write24(text_length);
  // Encode number of children
  bs.write24(children.size());
  
  const Zone * prev_child=0;
  // Encode all children
  for (GPosition i=children; i; ++i)
  {
    children[i].encode(gbs, this, prev_child);
    prev_child=&children[i];
  }
}
#endif

void 
DjVuTXT::Zone::decode(const GP<ByteStream> &gbs, int maxtext,
		      const Zone * parent, const Zone * prev)
{
  ByteStream &bs=*gbs;
  // Decode type
  ztype = (ZoneType) bs.read8();
  if ( ztype<PAGE || ztype>CHARACTER )
    G_THROW( ERR_MSG("DjVuText.corrupt_text") );

  // Decode coordinates
  int x=(int) bs.read16()-0x8000;
  int y=(int) bs.read16()-0x8000;
  int width=(int) bs.read16()-0x8000;
  int height=(int) bs.read16()-0x8000;

  // Decode text info
  text_start = (int) bs.read16()-0x8000;
//  int start=text_start;
  text_length = bs.read24();
  if (prev)
  {
    if (ztype==PAGE || ztype==PARAGRAPH || ztype==LINE)
    {
      x=x+prev->rect.xmin;
      y=prev->rect.ymin-(y+height);
    } else // Either COLUMN or WORD or CHARACTER
    {
      x=x+prev->rect.xmax;
      y=y+prev->rect.ymin;
    }
    text_start+=prev->text_start+prev->text_length;
  } else if (parent)
  {
    x=x+parent->rect.xmin;
    y=parent->rect.ymax-(y+height);
    text_start+=parent->text_start;
  }
  rect=GRect(x, y, width, height);
  // Get children size
  int size = bs.read24();

  // Checks
  if (rect.isempty() || text_start<0 || text_start+text_length>maxtext )
    G_THROW( ERR_MSG("DjVuText.corrupt_text") );

  // Process children
  const Zone * prev_child=0;
  children.empty();
  while (size-- > 0) 
  {
    Zone *z = append_child();
    z->decode(gbs, maxtext, this, prev_child);
    prev_child=z;
  }
}

void 
DjVuTXT::normalize_text()
{
  GUTF8String newtextUTF8;
  page_zone.normtext( (const char*)textUTF8, newtextUTF8 );
  textUTF8 = newtextUTF8;
}

int 
DjVuTXT::has_valid_zones() const
{
  if (!textUTF8)
    return false;
  if (page_zone.children.isempty() || page_zone.rect.isempty()) 
    return false;
  return true;
}


#ifndef NEED_DECODER_ONLY
void 
DjVuTXT::encode(const GP<ByteStream> &gbs) const
{
  ByteStream &bs=*gbs;
  if (! textUTF8 )
    G_THROW( ERR_MSG("DjVuText.no_text") );
  // Encode text
  int textsize = textUTF8.length();
  bs.write24( textsize );
  bs.writall( (void*)(const char*)textUTF8, textsize );
  // Encode zones
  if (has_valid_zones())
  {
    bs.write8(Zone::version);
    page_zone.encode(gbs);
  }
}
#endif

void 
DjVuTXT::decode(const GP<ByteStream> &gbs)
{
  ByteStream &bs=*gbs;
  // Read text
  textUTF8.empty();
  int textsize = bs.read24();
  char *buffer = textUTF8.getbuf(textsize);
  int readsize = bs.read(buffer,textsize);
  buffer[readsize] = 0;
  if (readsize < textsize)
    G_THROW( ERR_MSG("DjVuText.corrupt_chunk") );
  // Try reading zones
  unsigned char version;
  if ( bs.read( (void*) &version, 1 ) == 1) 
  {
    if (version != Zone::version)
      G_THROW( ERR_MSG("DjVuText.bad_version") "\t" + GUTF8String(version) );
    page_zone.decode(gbs, textsize);
  }
}

GP<DjVuTXT> 
DjVuTXT::copy(void) const
{
  return new DjVuTXT(*this);
}


bool
DjVuTXT::search_zone(const Zone * zone, int start, int & end) const
      // Will return TRUE if 'zone' contains beginning of the text
      // at 'start'. In this case it will also modify the 'end'
      // to point to the first character beyond the zone
{
  if (start>=zone->text_start && start<zone->text_start+zone->text_length)
  {
    if (end>zone->text_start+zone->text_length)
      end=zone->text_start+zone->text_length;
    return true;
  }
  return false;
}

static inline bool
intersects_zone(GRect box, const GRect &zone)
{
  return
    ((box.xmin < zone.xmin)
      ?(box.xmax >= zone.xmin)
      :(box.xmin <= zone.xmax))
    &&((box.ymin < zone.ymin)
      ?(box.ymax >= zone.ymin)
      :(box.ymin <= zone.ymax));
}

void
DjVuTXT::Zone::get_text_with_rect(
  const GRect &box, int &string_start, int &string_end) const
{
  GPosition pos=children;
  if(pos?box.contains(rect):intersects_zone(box,rect))
  {
    const int text_end=text_start+text_length;
    if(string_start == string_end)
    {
      string_start=text_start;
      string_end=text_end;
    }else
    {
      if (string_end < text_end)
        string_end=text_end;
      if(text_start < string_start)
        string_start=text_start;
    }
  }else if(pos&&intersects_zone(box,rect))
  {
    do
    {
      children[pos].get_text_with_rect(box,string_start,string_end);
    } while(++pos);
  }
}

void
DjVuTXT::Zone::find_zones(
  GList<Zone *> &list, const int string_start, const int string_end) const
{
  GPosition pos=children;
  const int text_end=text_start+text_length;
  if(text_start >= string_start)
  {
    if(text_end <= string_end)
    {
      list.append(const_cast<Zone *>(this));
    }else if(text_start < string_end)
    {
      if(pos)
      {
        do
        {
          children[pos].find_zones(list,string_start,string_end);
        } while (++pos);
      }else
      {
        list.append(const_cast<Zone *>(this));
      }
    }
  }else if( text_end > string_start)
  {
    do
    {
      children[pos].find_zones(list,string_start,string_end);
    } while (++pos);
  }
}

void
DjVuTXT::Zone::get_smallest(GList<GRect> &list) const
{
  GPosition pos=children;
  if(pos)
  {
    do
    {
      children[pos].get_smallest(list);
    } while (++pos);
  }else
  {
    list.append(rect);
  }
}

void
DjVuTXT::Zone::get_smallest( 
  GList<GRect> &list, const int padding) const
{
  GPosition pos=children;
  if(pos)
  {
    do
    {
      children[pos].get_smallest(list,padding);
    } while (++pos);
  }else if(zone_parent && zone_parent->ztype >= PARAGRAPH)
  {
    const GRect &xrect=zone_parent->rect;
    if(xrect.height() < xrect.width())
    {
      list.append(GRect(rect.xmin-padding,xrect.ymin-padding,rect.width()+2*padding,xrect.height()+2*padding));
    }else
    {
      list.append(GRect(xrect.xmin-padding,rect.ymin-padding,xrect.width()+2*padding,rect.height()+2*padding));
    }
  }else
  {
    list.append(GRect(rect.xmin-padding,rect.ymin-padding,rect.width()+2*padding,rect.height()+2*padding));
  }
}

void
DjVuTXT::get_zones(int zone_type, const Zone *parent, GList<Zone *> & zone_list) const 
   // get all the zones of  type zone_type under zone node parent
{
   // search all branches under parent
   const Zone *zone=parent;
   for( int cur_ztype=zone->ztype; cur_ztype<zone_type; ++cur_ztype )
   {
      GPosition pos;
      for(pos=zone->children; pos; ++pos)
      {
	 Zone *zcur=(Zone *)&zone->children[pos];
	 if ( zcur->ztype == zone_type )
	 {
	    GPosition zpos=zone_list;
	    if ( !zone_list.search(zcur,zpos) )
	       zone_list.append(zcur);
	 }
	 else if ( zone->children[pos].ztype < zone_type )
	    get_zones(zone_type, &zone->children[pos], zone_list);
      }
   }
}

DjVuTXT::Zone *
DjVuTXT::get_smallest_zone(int max_type, int start, int & end) const
      // Will return the smallest zone with type up to max_type containing
      // the text starting at start. If anything is found, end will
      // be modified to point to the first character beyond the zone
{
  if (!search_zone(&page_zone, start, end)) return 0;
  
  const Zone * zone=&page_zone;
  while(zone->ztype<max_type)
  {
    GPosition pos;
    for(pos=zone->children;pos;++pos)
       if (search_zone(&zone->children[pos], start, end))
	  break;
    if (pos) 
       zone=&zone->children[pos];
    else 
       break;
  }
  
  return (Zone *) zone;
}

GList<DjVuTXT::Zone *>
DjVuTXT::find_zones(int string_start, int string_length) const
      // For the string starting at string_start of length string_length
      // the function will generate a list of smallest zones of the
      // same type and will return it
{
  GList<Zone *> zone_list;
  
  {
    // Get rid of the leading and terminating spaces
    const int start=textUTF8.nextNonSpace(string_start,string_length);
    const int end=textUTF8.firstEndSpace(start,string_length+string_start-start);
    if (start==end)
      return zone_list;
    string_start=start;
    string_length=end-start;
  }
  
  int zone_type=CHARACTER;
  while(zone_type>=PAGE)
  {
    int start=string_start;
    int end=string_start+string_length;
    
    while(start<end)
    {
      start=textUTF8.nextNonSpace(start,string_length);
      if (start==end)
        break;
      
      Zone * zone=get_smallest_zone(zone_type, start, end);
      if (zone && zone_type==zone->ztype)
      {
        zone_list.append(zone);
        start=end;
        end=string_start+string_length;
      } else
      {
        zone_type--;
        zone_list.empty();
        break;
      }
    }
    if (zone_list.size()) break;
  }
  return zone_list;
}

GList<GRect>
DjVuTXT::find_text_with_rect(
  const GRect &box, GUTF8String &text, const int padding) const
{
  GList<GRect> retval;
  int text_start=0;
  int text_end=0;
  page_zone.get_text_with_rect(box,text_start,text_end);
  if(text_start != text_end)
  {
    GList<Zone *> zones;
    page_zone.find_zones(zones,text_start,text_end);
    GPosition pos=zones;
    if(pos)
    {
      do
      {
        if(padding >= 0)
        {
          zones[pos]->get_smallest(retval,padding);
        }else
        {
          zones[pos]->get_smallest(retval);
        }
      } while(++pos);
    }
  }
  text=textUTF8.substr(text_start,text_end-text_start);
  return retval;
}


GList<DjVuTXT::Zone *>
DjVuTXT::find_text_in_rect(GRect target_rect, GUTF8String &text) const
   // returns a list of zones of type WORD in the nearest/selected paragraph 
{
   GList<Zone *> zone_list;
   GList<Zone *> lines;

   get_zones((int)PARAGRAPH, &page_zone, zone_list);
   // it's possible that no paragraph structure exists for reasons that  
   // 1) ocr engine is not capable 2) file was modified by user. In such case, 
   // we can only make a rough guess, i.e., select all the lines intersected with
   // target_rect
   if (zone_list.isempty())
   {
      get_zones((int)LINE, &page_zone, zone_list);
      GPosition pos;
      for(pos=zone_list; pos; ++pos)
      {
	 GRect rect=zone_list[pos]->rect;
	 int h0=rect.height()/2;
	 if(rect.intersect(rect,target_rect) && rect.height()>h0)
	    lines.append(zone_list[pos]);
      }
   } else 
   {
      GPosition pos, pos_sel=zone_list;
      float ar=0;
      for(pos=zone_list; pos; ++pos)
      {
	 GRect rect=zone_list[pos]->rect;
	 int area=rect.area();
	 if (rect.intersect(rect, target_rect))
	 {
	    float ftmp=rect.area()/(float)area;
	    if ( !ar || ar<ftmp )
	    {
	       ar=ftmp;
	       pos_sel=pos;
	    }
	 }
      }
      Zone *parag;
      if ( ar>0 ) parag=zone_list[pos_sel];
      zone_list.empty();
      if ( ar>0 ) 
      {
	 get_zones((int)LINE, parag, zone_list);
	 if ( !zone_list.isempty() )
	 {
	    for(GPosition pos=zone_list; pos; ++pos)
	    {
	       GRect rect=zone_list[pos]->rect;
	       int h0=rect.height()/2;
	       if(rect.intersect(rect,target_rect) && rect.height()>h0)
		  lines.append(zone_list[pos]);
	    }
	 }
      }
   }

   zone_list.empty();
   if (!lines.isempty()) 
   {
      int i=1, lsize=lines.size();

      GList<Zone *> words;
      for (GPosition pos=lines; pos; ++pos, ++i)
      {
	 words.empty();
	 get_zones((int)WORD, lines[pos], words);

	 if ( lsize==1 )
	 {
	    for(GPosition p=words;p;++p)
	    {
	       GRect rect=words[p]->rect;
	       if(rect.intersect(rect,target_rect))
	       //if (target_rect.contains(words[p]->rect))
		  zone_list.append(words[p]);
	    }
	 } else
	 {
	    if (i==1)
	    {
	       bool start=true;
	       for(GPosition p=words; p; ++p)
	       {
		  if ( start )
		  {
		     GRect rect=words[p]->rect;
		     if(rect.intersect(rect,target_rect))
			//if (target_rect.contains(words[p]->rect))
		     {
			start=false;
			zone_list.append(words[p]);
		     }
		  } else 
		     zone_list.append(words[p]);
	       }
	    } else if (i==lsize)
	    {
	       bool end=true;
	       for(GPosition p=words.lastpos();p;--p)
	       {
		  if ( end )
		  {
		     GRect rect=words[p]->rect;
		     if(rect.intersect(rect,target_rect))
			//if(target_rect.contains(words[p]->rect) )
		     {
			end=false;
			zone_list.append(words[p]);
		     }
		  } else 
		     zone_list.append(words[p]);
	       }
	    }

	    if (i!=1 && i!=lsize )
	    {
	       for(GPosition p=words;p;++p)
		  zone_list.append(words[p]);
	    }
	 }
      }
   } 

   return zone_list;
}


static inline bool
is_blank(char ch)
{
   return
       isspace(ch) ||
       ch==DjVuTXT::end_of_column ||
       ch==DjVuTXT::end_of_region ||
       ch==DjVuTXT::end_of_paragraph ||
       ch==DjVuTXT::end_of_line;
}

static inline bool
is_separator(char ch, bool whole_word)
   // Returns TRUE, if the 'ch' is a separator.
   // If 'whole_word' is TRUE, the separators also include
   // all punctuation characters. Otherwise, they include only
   // white characters.
{
   return is_blank(ch) ||
	  whole_word && strchr("-!,.;:?\"'", ch);
}

static inline bool
chars_equal(char ch1, char ch2, bool match_case)
{
   if (is_blank(ch1) && is_blank(ch2))
      return true;

   return match_case ? (ch1==ch2) : (toupper(ch1)==toupper(ch2));
}

static bool
equal(const char * txt, const char * str, bool match_case, int * length)
      // Returns TRUE, if 'txt' contains the same words as the 'str'
      // in the same order (but possibly with different number of
      // separators between words). The 'separators' in this
      // function are blank and 'end_of_...' characters.
      //
      // If search has been successful, the function will make 'length'
      // contain the index of the first character in the 'txt' beyond the
      // found string
      //
      // NOTE, that it may be different from strlen(str) because of different
      // number of spaces between words in 'str' and 'txt'
{
   const char * str_ptr=str;
   const char * txt_ptr=txt;
   while(*str_ptr)
   {
      while(*str_ptr && is_blank(*str_ptr)) str_ptr++;
      while(*str_ptr && *txt_ptr && !is_blank(*str_ptr))
	 if (chars_equal(*txt_ptr, *str_ptr, match_case))
	 {
	    txt_ptr++; str_ptr++;
	 } else break;
      if (!*str_ptr)
	 break;
      if (!*txt_ptr || !is_blank(*str_ptr) || !is_blank(*txt_ptr))
	 return false;
      while(*txt_ptr && is_blank(*txt_ptr)) txt_ptr++;
   }
   if (length) *length=txt_ptr-txt;
   return true;
}

GList<DjVuTXT::Zone *>
DjVuTXT::search_string(GUTF8String string, int & from, bool search_fwd,
		       bool match_case, bool whole_word) const
{
  GUTF8String local_string;
  
  const char * ptr;
  
  // Get rid of the leading separators
  for(ptr=string;*ptr;ptr++)
    if (!is_separator(*ptr, whole_word))
      break;
  local_string=ptr;
  
  // Get rid of the terminating separators
  while(local_string.length() &&
        is_separator(local_string[(int)(local_string.length())-1], whole_word))
    local_string.setat(local_string.length()-1, 0);
  
  string=local_string;
  
  if (whole_word)
  {
    // Make sure that the string does not contain any
    // separators in the middle
    for(const char * ptr=string;*ptr;ptr++)
      if (is_separator(*ptr, true))
        G_THROW( ERR_MSG("DjVuText.one_word") );
  }
  
  int string_length=strlen(string);
  bool found=false;
  
  if (string_length==0 || textUTF8.length()==0 ||
    string_length>(int) textUTF8.length())
  {
    if (search_fwd) from=textUTF8.length();
    else from=-1;
    return GList<Zone *>();
  }
  
  int real_str_len;
  if (search_fwd)
  {
    if (from<0) from=0;
    while(from<(int) textUTF8.length())
    {
      if (equal((const char *) textUTF8+from, string, match_case, &real_str_len))
      {
        if (!whole_word ||
          (from==0 || is_separator(textUTF8[from-1], true)) &&
          (from+real_str_len==(int) textUTF8.length() ||
          is_separator(textUTF8[from+real_str_len], true)))
        {
          found=true;
          break;
        }
      }
      from++;
    }      
  }
  else
  {
    if (from>(int) textUTF8.length()-1) from=(int) textUTF8.length()-1;
    while(from>=0)
    {
      if (equal((const char *) textUTF8+from, string, match_case, &real_str_len))
      {
        if (!whole_word ||
          (from==0 || is_separator(textUTF8[from-1], true)) &&
          (from+real_str_len==(int) textUTF8.length() ||
          is_separator(textUTF8[from+real_str_len], true)))
        {
          found=true;
          break;
        }
      }
      from--;
    }
  }
  
  if (found) return find_zones(from, real_str_len);
  else return GList<Zone *>();
}

unsigned int 
DjVuTXT::get_memory_usage() const
{
  return sizeof(*this) + textUTF8.length() + page_zone.memuse() - sizeof(page_zone); 
}



//***************************************************************************
//******************************** DjVuText *********************************
//***************************************************************************

void
DjVuText::decode(const GP<ByteStream> &gbs)
{
  GUTF8String chkid;
  GP<IFFByteStream> giff=IFFByteStream::create(gbs);
  IFFByteStream &iff=*giff;
  while( iff.get_chunk(chkid) )
  {
    if (chkid == "TXTa")
    {
      if (txt)
        G_THROW( ERR_MSG("DjVuText.dupl_text") );
      txt = DjVuTXT::create();
      txt->decode(iff.get_bytestream());
    }
    else if (chkid == "TXTz")
    {
      if (txt)
        G_THROW( ERR_MSG("DjVuText.dupl_text") );
      txt = DjVuTXT::create();
      const GP<ByteStream> gbsiff=BSByteStream::create(iff.get_bytestream());
      txt->decode(gbsiff);
    }
    // Add decoding of other chunks here
    iff.close_chunk();
  }
}

void
DjVuText::encode(const GP<ByteStream> &gbs)
{
  if (txt)
  {
    const GP<IFFByteStream> giff=IFFByteStream::create(gbs);
    IFFByteStream &iff=*giff;
    iff.put_chunk("TXTz");
    {
      GP<ByteStream> gbsiff=BSByteStream::create(iff.get_bytestream(),50);
      txt->encode(gbsiff);
    }
    iff.close_chunk();
  }
  // Add encoding of other chunks here
}


GP<DjVuText>
DjVuText::copy(void) const
{
   GP<DjVuText> text= new DjVuText;
      // Copy any primitives (if any)
   *text=*this;
      // Copy each substructure
   if (txt)
     text->txt = txt->copy();
   return text;
}

static GUTF8String
indent ( int spaces)
{
  GUTF8String ret;
  for( int i = 0 ; i < spaces ; i++ )
    ret += ' ';
  return ret;
}

static const char *tags[8]=
{ 0,
  "HIDDENTEXT",
  "PAGECOLUMN",
  "REGION",
  "PARAGRAPH",
  "LINE",
  "WORD",
  "CHARACTER" };
static const int tags_size=sizeof(tags)/sizeof(const char *);

static GUTF8String
start_tag(const DjVuTXT::ZoneType zone)
{
  GUTF8String retval;
  if((tags_size > (int)zone)&&((int)zone > 0))
  {
    switch (zone)
    {
      case DjVuTXT::CHARACTER:
        retval="<"+GUTF8String(tags[zone])+">";
        break;
      case DjVuTXT::WORD:
        retval=indent(2*(int)zone+2)+"<"+tags[zone]+">";
        break;
      default:
        retval=indent(2*(int)zone+2)+"<"+tags[zone]+">\n";
        break;
    }
  }
  return retval;
}

static GUTF8String
start_tag(const DjVuTXT::ZoneType zone, const GUTF8String &attributes)
{
  GUTF8String retval;
  if((tags_size > (int)zone)&&((int)zone > 0))
  {
    switch (zone)
    {
      case DjVuTXT::CHARACTER:
        retval="<"+GUTF8String(tags[zone])+" "+attributes+">";
        break;
      case DjVuTXT::WORD:
        retval=indent(2*(int)zone+2)+"<"+tags[zone]+" "+attributes+">";
        break;
      default:
        retval=indent(2*(int)zone+2)+"<"+tags[zone]+" "+attributes+">\n";
        break;
    }
  }
  return retval;
}

static inline GUTF8String
start_tag(const int layer)
{
  return start_tag((const DjVuTXT::ZoneType)layer);
}


static GUTF8String
end_tag(const DjVuTXT::ZoneType zone)
{
  GUTF8String retval;
  if((tags_size > (int)zone)&&((int)zone >= 0))
  {
    switch (zone)
    {
      case DjVuTXT::CHARACTER:
        retval="</"+GUTF8String(tags[zone])+">";
        break;
      case DjVuTXT::WORD:
        retval="</"+GUTF8String(tags[zone])+">\n";
        break;
      default:
        retval=indent(2*(int)zone+2)+"</"+tags[zone]+">\n";
        break;
    }
  }
  return retval;
}

static inline GUTF8String
end_tag(const int layer)
{
  return end_tag((const DjVuTXT::ZoneType)layer);
}

static GUTF8String
tolayer(int &layer, const DjVuTXT::ZoneType next_layer)
{
  GUTF8String retval;
  for( ;layer < (int)next_layer;layer++ )
  {
    retval+=start_tag(layer);
  }
  while (layer > (int)next_layer )
  {
    retval+=end_tag(--layer);
  }
  return retval;
}

static void
writeText( ByteStream & str_out,
            const GUTF8String &textUTF8,
            const DjVuTXT::Zone &zone,
            const int WindowHeight );

static void
writeText( ByteStream & str_out,
           const GUTF8String &textUTF8,
           const DjVuTXT::ZoneType zlayer,
           const GList<DjVuTXT::Zone> &children,
           const int WindowHeight )
{
//  assert( txt->has_valid_zones() );
//  DEBUG_MSG( "--zonetype=" << txt->page_zone.ztype << "\n" );

  //  Beginning tags for missing layers
  int layer=(int)zlayer;
  //  Output the next layer
  for(GPosition pos=children ; pos ; ++pos )
  {
    str_out.writestring(tolayer(layer,children[pos].ztype));
    writeText( str_out,
                textUTF8,
                children[pos],
                WindowHeight );
  }
  str_out.writestring(tolayer(layer,zlayer));
}

static void
writeText( ByteStream & str_out,
            const GUTF8String &textUTF8,
            const DjVuTXT::Zone &zone,
            const int WindowHeight )
{
//  DEBUG_MSG( "--zonetype=" << zone.ztype << "\n" );

  const GUTF8String xindent(indent( 2 * zone.ztype + 2 ));
  GPosition pos=zone.children;
  // Build attribute string
  if( ! pos )
  {
    GUTF8String coords;
    coords.format("coords=\"%d,%d,%d,%d\"",
      zone.rect.xmin, WindowHeight - 1 - zone.rect.ymin,
      zone.rect.xmax, WindowHeight - 1 - zone.rect.ymax);
    const int start=zone.text_start;
    const int end=textUTF8.firstEndSpace(start,zone.text_length);
    str_out.writestring(start_tag(zone.ztype,coords));
    str_out.writestring(textUTF8.substr(start,end-start).toEscaped());
    str_out.writestring(end_tag(zone.ztype));
  } else
  {
    writeText(str_out,textUTF8,zone.ztype,zone.children,WindowHeight);
  }
}

void
DjVuTXT::writeText(ByteStream &str_out,const int height) const
{
  if(has_valid_zones())
  {
    ::writeText(str_out,textUTF8,DjVuTXT::PAGE,page_zone.children,height);
  }else
  {
    str_out.writestring(start_tag(DjVuTXT::PAGE));
    str_out.writestring(end_tag(DjVuTXT::PAGE));
  }
}

void
DjVuText::writeText(ByteStream &str_out,const int height) const
{
  if(txt)
  {
    txt->writeText(str_out,height);
  }else
  {
    str_out.writestring("<"+GUTF8String(tags[DjVuTXT::PAGE])+"/>\n");
  }
   
}
GUTF8String
DjVuTXT::get_xmlText(const int height) const
{
  GP<ByteStream> gbs(ByteStream::create());
  ByteStream &bs=*gbs;
  writeText(bs,height);
  bs.seek(0L);
  return bs.getAsUTF8();
}

GUTF8String
DjVuText::get_xmlText(const int height) const
{
  GUTF8String retval;
  if(txt)
  {
    retval=txt->get_xmlText(height);
  }else
  {
    retval="<"+GUTF8String(tags[DjVuTXT::PAGE])+"/>\n";
  }
  return retval;
}

