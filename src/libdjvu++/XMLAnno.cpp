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
// $Id: XMLAnno.cpp,v 1.7.2.1 2001-03-22 02:04:16 bcr Exp $
// $Name:  $

#ifdef __GNUC__
#pragma implementation
#endif

#include "XMLAnno.h"
#include "UnicodeByteStream.h"
#include "GOS.h"
#include "GURL.h"
#include "DjVuDocument.h"
#include "GMapAreas.h"
#include "DjVuAnno.h"
#include "DjVuFile.h"
#include "debug.h"
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>

static void intList
(char const *coords, GList<int> &retval)
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

static inline const GMap<GString,GMapArea::BorderType> &
BorderTypeMap(void)
{
  static GMap<GString,GMapArea::BorderType> typeMap;
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
    GString mesg;
    mesg.format("Color %s is not supported",s);
    G_THROW(mesg);
  }
  return i;
}

void
lt_XMLAnno::ChangeAnno(const lt_XMLTags &map,const GURL &url,const GString &id,const GString &width,const GString &height)
{
  const GString url_string((const char *)url);
  DjVuDocument &doc=*(docs[url_string]);
  bool const is_int=id.is_int();
  GString xid;
  if(is_int)
  {
    const int page=atoi((char const *)id);
    if(page>0)
    {
      xid=page-1;
    }else
    {
      xid="0";
    }
  }else if(!id.length()
    ||( doc.get_doc_type()==DjVuDocument::SINGLE_PAGE ))
  {
    xid="0";
  }else
  {
    xid=id;
  }
  GP<DjVuFile> dfile=doc.get_djvu_file(xid,true);
  if(!dfile)
  {
    G_THROW("Failed to get specified page");
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
      const int ww=atoi((const char *)width);
      if(ww)
      {
        ws=((double)w)/((double)ww); 
      }
    }
    if(height.is_int())
    {
      const int hh=atoi((const char *)height);
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
        GMap<GString,GString> args=areas.args;
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
//            printf("Coords (%d,%d)\n",x,y);
            }
          }
        }
        GString shape;
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
            G_THROW("Too few coords for  rectangular region");
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
//          printf("xmin=%d ymin=%d xmax=%d,ymax=%d\n",xmin,ymin,xmax,ymax);
          GRect rect(xmin,ymin,xmax-xmin,ymax-ymin);
//          printf("xmin=%d ymin=%d width=%u height=%u\n",xmin,ymin,rect.width(),rect.height());
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
            G_THROW("Too few coords for circular region");
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
            G_THROW("Too few coords for oval region");
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
          GString mesg("Unknown shape ");
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
            GString b=args[pos];
            static const GMap<GString,GMapArea::BorderType> typeMap=BorderTypeMap();
            if((pos=typeMap.contains(b)))
            {
              a->border_type=typeMap[pos];
            }else
            {
              GString mesg("Unrecongized border type ");
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
            a->border_width=atoi(args[pos]);
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
  files.append(dfile);
}

void 
lt_XMLAnno::empty(void)
{
  files.empty();
  docs.empty();
}

void 
lt_XMLAnno::save(void)
{
  for(GPosition pos=docs;pos;++pos)
  {
    DjVuDocument &doc=*(docs[pos]);
    GURL url=doc.get_init_url();
    GString name=GOS::url_to_filename(url);
    printf("Saving file '%s' with new annotations.\n",(const char *)name);
    const bool bundle=doc.is_bundled()||(doc.get_doc_type()==DjVuDocument::SINGLE_PAGE);
    doc.save_as(name,bundle);
  }
  empty();
}

void
lt_XMLAnno::parse(const char xmlfile[],GString basedir)
{
  m_basedir=basedir;
  GP<lt_XMLTags> tags=lt_XMLTags::create();
  tags->init(xmlfile);
  parse(*tags);
}

void
lt_XMLAnno::parse(GP<ByteStream> &bs)
{
  GP<lt_XMLTags> tags=lt_XMLTags::create();
  tags->init(bs);
  parse(*tags);
}
  
void
lt_XMLAnno::parse(const lt_XMLTags &tags)
{
  GPList<lt_XMLTags> Body=tags.getTags("BODY");
  GPosition pos=Body;
 
  if(!pos || (pos != Body.lastpos()))
  {
    G_THROW("Exactly one <BODY></BODY> pair is needed");
  }
  GP<lt_XMLTags> & GBody =Body[pos];
  if(!GBody)
  {
    G_THROW("No <BODY></BODY> pair found");
  }
  GMap<GString,GP<lt_XMLTags> > Maps;
  lt_XMLTags::getMaps("MAP","name",Body,Maps);

  GPList<lt_XMLTags> Objects=GBody->getTags("OBJECT");
  lt_XMLTags::getMaps("MAP","name",Objects,Maps);

  for(GPosition Objpos=Objects;Objpos;++Objpos)
  {
    lt_XMLTags const * const GObject=Objects[Objpos];
    if(GObject)
    {
      // Map of attributes to value (e.g. "width" --> "500")
      const GMap<GString,GString> &args=GObject->args;
      GURL codebase;
      {
        DEBUG_MSG("Setting up codebase... m_basedir = " << m_basedir << "\n");
        GPosition codebasePos=args.contains("codebase");
        // If user specified a codebase attribute, assume it is correct (absolute URL):
        //  the GURL constructor will throw an exception if it isn't
        if(codebasePos)
        {
          codebase=GURL(args[codebasePos]);
        }else if (GOS::is_dir(m_basedir))
        {
          codebase=GOS::filename_to_url(m_basedir);
        }else
        {
          codebase=GOS::filename_to_url(GOS::cwd());
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
            fprintf(stderr,"Ignoring image/x.djvu OBJECT tag.\n");
            continue;
          }
          isDjVuType=true;
        }
        GURL url;
        GURL simpleURL(args[datapos]);
        if (simpleURL.is_valid())
        {
          url=simpleURL;
        }else if(args[datapos][0] == '/')
        {
          url=codebase.base()+args[datapos];
        }else            // relative URL
        {
          url=codebase+args[datapos];
          DEBUG_MSG("relative URL converted to absolute URL= " << url << "\n");
        }
        GString width;
        {
          GPosition widthPos=args.contains("width");
          if(widthPos)
          {
            width=args[widthPos];
          }
        }
        GString height;
        {
          GPosition heightPos=args.contains("height");
          if(heightPos)
          {
            height=args[heightPos];
          }
        }
        GString page;
        {
          GPosition paramPos=GObject->contains("PARAM");
          if(paramPos)
          {
            GPList<lt_XMLTags> Params=(*(GObject))[paramPos];
            for(GPosition loc=Params;loc;++loc)
            {
              if(Params[loc]->args.contains("name") && Params[loc]->args.contains("value"))
              {
                GString name=(Params[loc]->args["name"]);
                if(name.downcase() == "flags")
                {
                  GMap<GString,GString> args;
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
            GString mapname=GObject->args[usemappos];
            GPosition mappos=Maps.contains(mapname);
            if(!mappos)
            {
              GString mesg("Failed to find specify map ");
              mesg+=mapname;
              G_THROW(mesg);
            }else
            {
              map=Maps[mappos];
            }
          }
        }
        {
          GString url_string((char const *)url);
          GPosition docspos=docs.contains(url_string);
          if(! docspos)
          {
            GP<DjVuDocument> doc=DjVuDocument::create_wait(url);
            if(! doc->wait_for_complete_init())
            {
              GString mesg("Failed to initialize ");
              mesg+=GString((const char *)url);
              G_THROW(mesg);
            }
            docs[url_string]=doc;
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

