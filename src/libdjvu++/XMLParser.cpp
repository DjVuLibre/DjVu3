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
// $Id: XMLParser.cpp,v 1.7 2001-05-16 18:32:59 bcr Exp $
// $Name:  $

#ifdef __GNUC__
#pragma implementation
#endif

#include "XMLParser.h"
#include "XMLTags.h"
#include "ByteStream.h"
#include "GOS.h"
#include "DjVuDocument.h"
#include "DjVuText.h"
#include "DjVuAnno.h"
#include "DjVuFile.h"
#include "debug.h"
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>

class lt_XMLParser::Anno : public lt_XMLParser
{
public:
  Anno(void);
  /// Parse the specified tags.
  virtual void parse(const lt_XMLTags &tags);
  /// write to disk.
protected:
  void ChangeAnno(const lt_XMLTags &map,const GURL &url,const GUTF8String &id,
    const GUTF8String &width,const GUTF8String &height);

};

// this class writes in a new text layer base on the xml input. 
// 
//  note: <HIDDENTEXT> object without any text will remove the text
//  from that page.

class lt_XMLParser::Text : public lt_XMLParser
{
public:
  Text(void);
  /// Parse the specified tags.
  virtual void parse(const lt_XMLTags &tags);
  /// write to disk.
protected:
  void ChangeText(const lt_XMLTags &text,const GURL &url,const GUTF8String &id,
    const GUTF8String &width,const GUTF8String &height);

};

lt_XMLParser::lt_XMLParser(void) {}
lt_XMLParser::Anno::Anno(void) {}
lt_XMLParser::Text::Text(void) {}

GP<lt_XMLParser>
lt_XMLParser::create_anno(void)
{
  return new Anno();
}

GP<lt_XMLParser>
lt_XMLParser::create_text(void)
{
  return new Text();
}

void 
lt_XMLParser::intList(char const *coords, GList<int> &retval)
{
  char *ptr=0;
  if(coords && *coords)
  {
    for(unsigned long i=strtoul(coords,&ptr,10);ptr&&ptr!=coords;i=strtoul(coords,&ptr,10))
    {
      retval.append(i);
      for(coords=ptr;isspace(*coords);++coords);
      if(*coords == ',')
      {
        ++coords;
      }
      if(!*coords)
        break;
    }
  }
}

void 
lt_XMLParser::empty(void)
{
  m_files.empty();
  m_docs.empty();
}

void 
lt_XMLParser::save(void)
{
  for(GPosition pos=m_docs;pos;++pos)
  {
    DjVuDocument &doc=*(m_docs[pos]);
    GURL url=doc.get_init_url();
//    GUTF8String name=GOS::url_to_filename(url);
    DEBUG_MSG("Saving "<<(const char *)url<<" with new annotations.\n");
    const bool bundle=doc.is_bundled()||(doc.get_doc_type()==DjVuDocument::SINGLE_PAGE);
    doc.save_as(url,bundle);
  }
  empty();
}

void
lt_XMLParser::parse(GP<ByteStream> &bs)
{
  GP<lt_XMLTags> tags=lt_XMLTags::create();
  tags->init(bs);
  parse(*tags);
}
  
static inline const GMap<GUTF8String,GMapArea::BorderType> &
BorderTypeMap(void)
{
  static GMap<GUTF8String,GMapArea::BorderType> typeMap;
  typeMap["none"]=GMapArea::NO_BORDER;
  typeMap["xor"]=GMapArea::XOR_BORDER;
  typeMap["solid"]=GMapArea::SOLID_BORDER;
  typeMap["default"]=GMapArea::SOLID_BORDER;
  typeMap["shadowout"]=GMapArea::SHADOW_OUT_BORDER;
  typeMap["shadowin"]=GMapArea::SHADOW_IN_BORDER;
  typeMap["etchedin"]=GMapArea::SHADOW_EIN_BORDER;
  typeMap["etchedout"]=GMapArea::SHADOW_EOUT_BORDER;
  return typeMap;
}

static unsigned long int 
convertToColor(const char s[])
{
  unsigned long int i=0;
  if(s[0] == '#')
  {
    i=strtoul(s+1,0,16);
  }else if(s[0])
  {
    GUTF8String mesg;
    mesg.format( ERR_MSG("XMLAnno.bad_color") "\t%s",s);
    G_THROW(mesg);
  }
  return i;
}

void
lt_XMLParser::Anno::ChangeAnno(const lt_XMLTags &map,const GURL &url,const GUTF8String &id,const GUTF8String &width,const GUTF8String &height)
{
  const GUTF8String url_string((const char *)url);
  DjVuDocument &doc=*(m_docs[url_string]);
  int pagenum=(-1);
  if(id.is_int())
  {
    const int page=id.toInt(); //atoi((char const *)id); 
    if(page>0)
      pagenum=page-1;
  }
  GP<DjVuFile> dfile=((pagenum>=0)||!id.length())
    ?doc.get_djvu_file(pagenum,false):doc.get_djvu_file(id,false);
  if(!dfile)
  {
    G_THROW( ERR_MSG("XMLAnno.bad_page") );
  }
  dfile->start_decode();
  dfile->wait_for_finish();
  GP<DjVuInfo> info=(dfile->info);
  GP<DjVuAnno> ganno=DjVuAnno::create();
  DjVuAnno &anno=*ganno;
  if(dfile->contains_anno())
  {
    GP<ByteStream> annobs=dfile->get_merged_anno();
    if(annobs)
    {
      anno.decode(annobs);
    }
//  dfile->remove_anno();
  }
  GPosition map_pos;
  if(!info || !(map_pos=map.contains("AREA")))
  {
    if(anno.ant)
    {
      anno.ant->map_areas.empty();
    }  
  }else
  {
    const int h=info->height;
    const int w=info->width;
    double ws=1.0;
    double hs=1.0;
    if(width.is_int())
    {
      const int w=info->width;
      const int ww=width.toInt(); //atoi((const char *)width);
      if(ww)
      {
        ws=((double)w)/((double)ww); 
      }
    }
    if(height.is_int())
    {
      const int hh=height.toInt();//atoi((const char *)height);
      if(hh)
      {
        hs=((double)h)/((double)hh); 
      }
    }
    if(!anno.ant)
    {
      anno.ant=DjVuANT::create();
    }
    GPList<GMapArea> &map_areas=anno.ant->map_areas;
    map_areas.empty();
    GPList<lt_XMLTags> gareas=map[map_pos];
    for(GPosition pos=gareas;pos;++pos)
    {
      if(gareas[pos])
      {
        lt_XMLTags &areas=*(gareas[pos]);
        GMap<GUTF8String,GUTF8String> args=areas.args;
        GList<int> coords;
        // ******************************************************
        // Parse the coords attribute:  first read the raw data into
        // a list, then scale the x, y data into another list.  For
        // circles, you also get a radius element with (looks like an x
        // with no matching y).
        // ******************************************************
        {
          GPosition coords_pos=args.contains("coords");
          if(coords_pos)
          {
            GList<int> raw_coords;
            intList(args[coords_pos],raw_coords);
            for(GPosition raw_pos=raw_coords;raw_pos;++raw_pos)
            {
              const int r=raw_coords[raw_pos];
              const int x=(int)(ws*(double)r+0.5);
              coords.append(x);
              int y=h-1;
              if(! ++raw_pos)
              {
                y-=(int)(hs*(double)r+0.5);
              }else
              {
                y-=(int)(hs*(double)raw_coords[raw_pos]+0.5);
              }
              coords.append(y);
//            DjVuPrintMessage("Coords (%d,%d)\n",x,y);
            }
          }
        }
        GUTF8String shape;
        {
          GPosition shape_pos=args.contains("shape");
          if(shape_pos)
          {
            shape=args[shape_pos];
          }
        }
        GP<GMapArea> a;
        if(shape == "default")
        {
          GRect rect(0,0,w,h);
          a=GMapRect::create(rect);
        }else if(!shape.length() || shape == "rect")
        {
          int xx[4];
          int i=0;
          for(GPosition rect_pos=coords;(rect_pos)&&(i<4);++rect_pos,++i)
          {
            xx[i]=coords[rect_pos];
          }
          if(i!=4)
          {
            G_THROW( ERR_MSG("XMLAnno.bad_rect") );
          }
          int xmin,xmax; 
          if(xx[0]>xx[2])
          {
            xmax=xx[0];
            xmin=xx[2];
          }else
          {
            xmin=xx[0];
            xmax=xx[2];
          }
          int ymin,ymax; 
          if(xx[1]>xx[3])
          {
            ymax=xx[1];
            ymin=xx[3];
          }else
          {
            ymin=xx[1];
            ymax=xx[3];
          }
          GRect rect(xmin,ymin,xmax-xmin,ymax-ymin);
          a=GMapRect::create(rect);
        }else if(shape == "circle")
        {
          int xx[4];
          int i=0;
          GPosition rect_pos=coords.lastpos();
          if(rect_pos)
          {
            coords.append(coords[rect_pos]);
            for(rect_pos=coords;(rect_pos)&&(i<4);++rect_pos)
            {
              xx[i++]=coords[rect_pos];
            }
          }
          if(i!=4)
          {
            G_THROW( ERR_MSG("XMLAnno.bad_circle") );
          }
          int x=xx[0],y=xx[1],rx=xx[2],ry=(h-xx[3])-1;
          GRect rect(x-rx,y-ry,2*rx,2*ry);
          a=GMapOval::create(rect);
        }else if(shape == "oval")
        {
          int xx[4];
          int i=0;
          for(GPosition rect_pos=coords;(rect_pos)&&(i<4);++rect_pos,++i)
          {
            xx[i]=coords[rect_pos];
          }
          if(i!=4)
          {
            G_THROW( ERR_MSG("XMLAnno.bad_oval") );
          }
          int xmin,xmax; 
          if(xx[0]>xx[2])
          {
            xmax=xx[0];
            xmin=xx[2];
          }else
          {
            xmin=xx[0];
            xmax=xx[2];
          }
          int ymin,ymax; 
          if(xx[1]>xx[3])
          {
            ymax=xx[1];
            ymin=xx[3];
          }else
          {
            ymin=xx[1];
            ymax=xx[3];
          }
          GRect rect(xmin,ymin,xmax-xmin,ymax-ymin);
          a=GMapOval::create(rect);
        }else if(shape == "poly")
        {
          GP<GMapPoly> p=GMapPoly::create();
          for(GPosition poly_pos=coords;poly_pos;++poly_pos)
          {
            int x=coords[poly_pos];
            if(! ++poly_pos)
              break;
            int y=coords[poly_pos];
            p->add_vertex(x,y);
          }
          p->close_poly();
          a=p;
        }else
        {
          GUTF8String mesg( ERR_MSG("XMLAnno.unknown_shape") "\t");
          mesg+=shape;
          G_THROW(mesg);
        }
        if(a)
        {
          GPosition pos;
          if((pos=args.contains("href")))
          {
            a->url=args[pos];
          }
          if((pos=args.contains("target")))
          {
            a->target=args[pos];
          }
          if((pos=args.contains("alt")))
          {
            a->comment=args[pos];
          }
          if((pos=args.contains("bordertype")))
          {
            GUTF8String b=args[pos];
            static const GMap<GUTF8String,GMapArea::BorderType> typeMap=BorderTypeMap();
            if((pos=typeMap.contains(b)))
            {
              a->border_type=typeMap[pos];
            }else
            {
              GUTF8String mesg( ERR_MSG("XMLAnno.unknown_border") "\t");
              mesg+=b;
              G_THROW(mesg);
            }
          }
          a->border_always_visible=!!args.contains("visible");
          if((pos=args.contains("bordercolor")))
          {
            a->border_color=convertToColor(args[pos]);
          }
          if((pos=args.contains("highlight")))
          {
            a->hilite_color=convertToColor(args[pos]);
          }
          if((pos=args.contains("border")))
          {
             a->border_width=args[pos].toInt(); //atoi(args[pos]);
          }
          map_areas.append(a);
        }
      }
    }
  }
  dfile->set_modified(true);
  dfile->reset();
  dfile->anno=ByteStream::create();
  anno.encode(dfile->anno);
  m_files.append(dfile);
}

  
void
lt_XMLParser::Anno::parse(const lt_XMLTags &tags)
{
  GPList<lt_XMLTags> Body=tags.getTags("BODY");
  GPosition pos=Body;
 
  if(!pos || (pos != Body.lastpos()))
  {
    G_THROW( ERR_MSG("XMLAnno.extra_body") );
  }
  GP<lt_XMLTags> & GBody =Body[pos];
  if(!GBody)
  {
    G_THROW( ERR_MSG("XMLAnno.no_body") );
  }
  GMap<GUTF8String,GP<lt_XMLTags> > Maps;
  lt_XMLTags::getMaps("MAP","name",Body,Maps);

  GPList<lt_XMLTags> Objects=GBody->getTags("OBJECT");
  lt_XMLTags::getMaps("MAP","name",Objects,Maps);

  for(GPosition Objpos=Objects;Objpos;++Objpos)
  {
    lt_XMLTags const * const GObject=Objects[Objpos];
    if(GObject)
    {
      // Map of attributes to value (e.g. "width" --> "500")
      const GMap<GUTF8String,GUTF8String> &args=GObject->args;
      GURL codebase;
      {
        DEBUG_MSG("Setting up codebase... m_codebase = " << m_codebase << "\n");
        GPosition codebasePos=args.contains("codebase");
        // If user specified a codebase attribute, assume it is correct (absolute URL):
        //  the GURL constructor will throw an exception if it isn't
        if(codebasePos)
        {
          codebase=GURL::UTF8(args[codebasePos]);
        }else if (m_codebase.is_dir())
        {
          codebase=m_codebase;
        }else
        {
          codebase=GURL::Filename::UTF8(GOS::cwd());
        }
        DEBUG_MSG("codebase = " << codebase << "\n");
      }
      // the data attribute specifies the input file.  This can be
      //  either an absolute URL (starts with file:/) or a relative
      //  URL (for now, just a path and file name).  If it's absolute,
      //  our GURL will adequately wrap it.  If it's relative, we need
      //  to use the codebase attribute to form an absolute URL first.
      GPosition datapos=args.contains("data");
      if(datapos)
      {
        bool isDjVuType=false;
        GPosition typePos=args.contains("type");
        if(typePos)
        {
          if(args[typePos] != "image/x.djvu")
          {
            DjVuPrintError("%s","Ignoring image/x.djvu OBJECT tag.\n");
            continue;
          }
          isDjVuType=true;
        }
        GURL url=GURL::UTF8(args[datapos],(args[datapos][0] == '/')?codebase.base():codebase);
        GUTF8String width;
        {
          GPosition widthPos=args.contains("width");
          if(widthPos)
          {
            width=args[widthPos];
          }
        }
        GUTF8String height;
        {
          GPosition heightPos=args.contains("height");
          if(heightPos)
          {
            height=args[heightPos];
          }
        }
        GUTF8String page;
        {
          GPosition paramPos=GObject->contains("PARAM");
          if(paramPos)
          {
            GPList<lt_XMLTags> Params=(*(GObject))[paramPos];
            for(GPosition loc=Params;loc;++loc)
            {
              if(Params[loc]->args.contains("name") && Params[loc]->args.contains("value"))
              {
                GUTF8String name=(Params[loc]->args["name"]);
                if(name.downcase() == "flags")
                {
                  GMap<GUTF8String,GUTF8String> args;
                  lt_XMLTags::ParseValues((const char *)(Params[loc]->args["value"]),args,true);
                  if(args.contains("page"))
                  {
                    page=args["page"];
                  }
                }
              }
            }
          }
        }
        GP<lt_XMLTags> map;
        {
          GPosition usemappos=GObject->args.contains("usemap");
          if(usemappos)
          {
            GUTF8String mapname=GObject->args[usemappos];
            GPosition mappos=Maps.contains(mapname);
            if(!mappos)
            {
              GUTF8String mesg( ERR_MSG("XMLAnno.map_find") "\t");
              mesg+=mapname;
              G_THROW(mesg);
            }else
            {
              map=Maps[mappos];
            }
          }
        }
        {
          GUTF8String url_string((char const *)url);
          GPosition docspos=m_docs.contains(url_string);
          if(! docspos)
          {
            GP<DjVuDocument> doc=DjVuDocument::create_wait(url);
            if(! doc->wait_for_complete_init())
            {
              GUTF8String mesg( ERR_MSG("XMLAnno.fail_init") "\t");
              mesg+=GUTF8String((const char *)url);
              G_THROW(mesg);
            }
            m_docs[url_string]=doc;
          }
        }
        if(map)
        {
          ChangeAnno(*map,url,page,width,height);
        }
      }
    }
  }
}

#ifdef max
#undef max
#endif
template<class TYPE>
static inline TYPE max(TYPE a,TYPE b) { return (a>b)?a:b; }
#ifdef min
#undef min
#endif
template<class TYPE>
static inline TYPE min(TYPE a,TYPE b) { return (a<b)?a:b; }

// used to build the zone tree
void
make_next_layer(
  DjVuTXT::Zone &parent,
  const lt_XMLTags &tag, ByteStream &bs,
  const int height)
{
  // the plugin thinks there are only Pages, Lines and Words
  // so we don't make Paragraphs, Regions and Columns zones
  // if we did the plugin is not able to search the text but 
  // DjVuToText writes out all the text anyway
  DjVuTXT::Zone *self_ptr;
  char sepchar;
  if(tag.name == "WORD")
  {
    self_ptr=parent.append_child();
    self_ptr->ztype = DjVuTXT::WORD;
    sepchar=' ';
  }else if(tag.name == "LINE")
  {
    self_ptr=parent.append_child();
    self_ptr->ztype = DjVuTXT::LINE;
    sepchar=DjVuTXT::end_of_line;
  }else if(tag.name == "PARAGRAPH")
  {
    self_ptr=parent.append_child();
    self_ptr->ztype = DjVuTXT::PARAGRAPH;
    sepchar=DjVuTXT::end_of_paragraph;
  }else if(tag.name == "REGION")
  {
    self_ptr=parent.append_child();
    self_ptr->ztype = DjVuTXT::REGION;
    sepchar=DjVuTXT::end_of_region;
  }else if(tag.name == "PAGECOLUMN")
  {
    self_ptr=parent.append_child();
    self_ptr->ztype = DjVuTXT::COLUMN;
    sepchar=DjVuTXT::end_of_column;
  }else
  {
    self_ptr = &parent;
    self_ptr->ztype = DjVuTXT::PAGE;
    sepchar=0;
  }
  DjVuTXT::Zone &self = *self_ptr;
  self.text_start = bs.tell();
  int &xmin=self.rect.xmin;
  int &ymin=self.rect.ymin;
  int &xmax=self.rect.xmax;
  int &ymax=self.rect.ymax;
  xmin=parent.rect.xmax;
  ymin=parent.rect.ymax;
  xmax=parent.rect.xmin;
  ymax=parent.rect.ymin;
  GPosition pos=tag.args.contains("coords");
  if(pos)
  {
    GList<int> rectArgs;
    lt_XMLParser::intList(tag.args[pos], rectArgs);
    if((pos=rectArgs))
    {
      xmin=rectArgs[pos];
      if(++pos)
      {
        ymin=(height-1)-rectArgs[pos];
        if(++pos)
        {
          xmax=rectArgs[pos];
          if(++pos)
          {
            ymax=(height-1)-rectArgs[pos];
          }else
          {
            ymax=parent.rect.xmax;
          }
        }
      }
      if(pos)
      {
        if(xmin>xmax)
        {
          const int t=xmin;
          xmin=xmax;
          xmax=t;
        }
        if(ymin>ymax)
        {
          const int t=ymin;
          ymin=ymax;
          ymax=t;
        }
      }else
      {
        xmin=parent.rect.xmax;
        ymin=parent.rect.ymax;
        xmax=parent.rect.xmin;
        ymax=parent.rect.ymin;
      }
    }
  }
  if(self.ztype == DjVuTXT::WORD)
  {
    bs.writestring(tag.raw.substr(0,tag.raw.firstEndSpace()));
//    DjVuPrintMessage("<<<%s>>>\n",
//      (const char *)tag.raw.substr(0,tag.raw.firstEndSpace()));
    if(sepchar)
      bs.write8(sepchar);
    self.text_length = bs.tell() - self.text_start;
  }else
  {
    for(GPosition pos = tag.content; pos; ++pos)
    {
      GP<lt_XMLTags> t = tag.content[pos].tag;
      make_next_layer(self, *t, bs, height);
      if(sepchar)
        bs.write8(sepchar);
      self.text_length = bs.tell() - self.text_start;
    }
  }
  parent.rect.xmin=min(xmin,parent.rect.xmin);
  parent.rect.ymin=min(ymin,parent.rect.ymin);
  parent.rect.xmax=max(xmax,parent.rect.xmax);
  parent.rect.ymax=max(ymax,parent.rect.ymax);
  if(xmin>xmax)
  {
    const int t=xmin;
    xmin=xmax;
    xmax=t;
  }
  if(ymin>ymax)
  {
    const int t=ymin;
    ymin=ymax;
    ymax=t;
  }
//  DjVuPrintMessage("(%d,%d)(%d,%d)<<<\\%o>>>\n",
//    xmin,ymin,xmax,ymax, sepchar);
}

#if 0
// used in debugging to see if the zone tree is built right       
void step_down(DjVuTXT::Zone &parent)
{
  if(parent)
  {
    for(GPosition i = parent.children; i; ++i)
      step_down(*parent.children[i]);
  }
}
#endif

void 
lt_XMLParser::Text::ChangeText(const lt_XMLTags &tags, const GURL &url,
  const GUTF8String &id, const GUTF8String &width,const GUTF8String &height)
{
  const GUTF8String url_string((const char *)url);
  DjVuDocument &doc = *(m_docs[url_string]);
  int pagenum=(-1); 
  if(id.is_int())
  {
    const int page=id.toInt(); //atoi((char const *)id); 
    if(page>0)
      pagenum=page-1;
  }
  GP<DjVuFile> dfile=((pagenum>=0)||!id.length())
    ?doc.get_djvu_file(pagenum,false):doc.get_djvu_file(id,false);
  if(!dfile)
  {
    G_THROW( ERR_MSG("XMLAnno.bad_page") );
  }
  
  GP<DjVuText> text = DjVuText::create();
  GP<DjVuTXT> txt = text->txt = DjVuTXT::create();
  
  // to store the new text
  GP<ByteStream> textbs = ByteStream::create(); 
  
  txt->page_zone.text_start = 0;
  DjVuTXT::Zone &parent=txt->page_zone;
  parent.rect.xmin=0;
  parent.rect.ymin=0;
  parent.rect.xmax=width.toInt();
  parent.rect.ymax=height.toInt();
  make_next_layer(parent,
    tags, *textbs, parent.rect.ymax);
  textbs->write8(0);
  long len = textbs->tell();
  txt->page_zone.text_length = len;
  textbs->seek(0,SEEK_SET);
  textbs->read(txt->textUTF8.getbuf(len), len);
  
  dfile->start_decode();
  dfile->wait_for_finish();
  dfile->change_text(txt,true);
  m_files.append(dfile);
}

void
lt_XMLParser::Text::parse(const lt_XMLTags &tags)
{
  GPList<lt_XMLTags> Body=tags.getTags("BODY");
  GPosition pos=Body;
  
  if(!pos || (pos != Body.lastpos()))
  {
    G_THROW( ERR_MSG("XMLAnno.extra_body") );
  }
  
  GP<lt_XMLTags> & GBody = Body[pos];
  if(!GBody)
  {
    G_THROW( ERR_MSG("XMLAnno.no_body") );
  }
  
  GPList<lt_XMLTags> Objects = GBody->getTags("OBJECT");
  
  for(GPosition Objpos=Objects;Objpos;++Objpos)
  {
    lt_XMLTags const * const GObject=Objects[Objpos];
    if(GObject)
    {
      GPosition textPos = GObject->contains("HIDDENTEXT");
      if(textPos)
      {
        
        // Map of attributes to value (e.g. "width" --> "500")
        const GMap<GUTF8String,GUTF8String> &args=GObject->args;
        GURL codebase;
        {
          DEBUG_MSG("Setting up codebase... m_codebase = " << m_codebase << "\n");
          GPosition codebasePos=args.contains("codebase");
          // If user specified a codebase attribute, assume it is correct (absolute URL):
          //  the GURL constructor will throw an exception if it isn't
          if(codebasePos)
          {
            codebase=GURL::UTF8(args[codebasePos]);
          }else if (m_codebase.is_dir())
          {
            codebase=m_codebase;
          }else
          {
            codebase=GURL::Filename::UTF8(GOS::cwd());
          }
          DEBUG_MSG("codebase = " << codebase << "\n");
        }
        // the data attribute specifies the input file.  This can be
        //  either an absolute URL (starts with file:/) or a relative
        //  URL (for now, just a path and file name).  If it's absolute,
        //  our GURL will adequately wrap it.  If it's relative, we need
        //  to use the codebase attribute to form an absolute URL first.
        GPosition datapos=args.contains("data");
        if(datapos)
        {
          bool isDjVuType=false;
          GPosition typePos=args.contains("type");
          if(typePos)
          {
            if(args[typePos] != "image/x.djvu")
            {
              DjVuPrintError("%s","Ignoring image/x.djvu OBJECT tag.\n");
              continue;
            }
            isDjVuType=true;
          }
          
          GURL url=GURL::UTF8(args[datapos],(args[datapos][0] == '/')?codebase.base():codebase);
          GUTF8String width;
          {
            GPosition widthPos=args.contains("width");
            if(widthPos)
            {
              width=args[widthPos];
            }
          }
          
          GUTF8String height;
          {
            GPosition heightPos=args.contains("height");
            if(heightPos)
            {
              height=args[heightPos];
            }
          }
          
          GUTF8String page;
          {
            GPosition paramPos=GObject->contains("PARAM");
            if(paramPos)
            {
              GPList<lt_XMLTags> Params=(*(GObject))[paramPos];
              for(GPosition loc=Params;loc;++loc)
              {
                if(Params[loc]->args.contains("name") && Params[loc]->args.contains("value"))
                {
                  GUTF8String name=(Params[loc]->args["name"]);
                  if(name.downcase() == "flags")
                  {
                    GMap<GUTF8String,GUTF8String> args;
                    lt_XMLTags::ParseValues((const char *)(Params[loc]->args["value"]),args,true);
                    if(args.contains("page"))
                    {
                      page=args["page"];
                    }
                  }
                }
              }
            }
          }
          
          {
            GUTF8String url_string((char const *)url);
            GPosition docspos=m_docs.contains(url_string);
            if(! docspos)
            {
              GP<DjVuDocument> doc=DjVuDocument::create_wait(url);
              if(! doc->wait_for_complete_init())
              {
                GUTF8String mesg( ERR_MSG("XMLAnno.fail_init") "\t");
                mesg+=GUTF8String((const char *)url);
                G_THROW(mesg);
              }
              m_docs[url_string]=doc;
            }
          }
          
          // loop through the hidden text - there should only be one 
          // if there are more ??only the last one will be saved??
          GPList<lt_XMLTags> textTags = (*(GObject))[textPos];
          for(GPosition i = textTags; i; ++i)
          {
            ChangeText(*textTags[i], url, page, width, height);
          } // for(i)
        } // if(dataPos) 
      } // if(textPos)
    } // if(GObject)
  } // for(Object)
}
