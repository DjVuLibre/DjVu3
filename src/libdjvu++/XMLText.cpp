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
// $Id: XMLText.cpp,v 1.1 2001-04-24 17:54:09 jhayes Exp $
// $Name:  $

#ifdef __GNUC__
#pragma implementation
#endif

#include "XMLText.h"
#include "UnicodeByteStream.h"
#include "GOS.h"
#include "GURL.h"
#include "DjVuDocument.h"
#include "DjVuText.h"
#include "DjVuFile.h"
#include "debug.h"
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>

// used to build the zone tree
GRect make_next_layer(DjVuTXT::Zone *parent, const lt_XMLTags &tag, ByteStream *bs, int height)
{
  // the plugin thinks there are only Pages, Lines and Words
  // so we don't make Paragraphs, Regions and Columns zones
  // if we did the plugin is not able to search the text but 
  // DjVuToText writes out all the text anyway
  DjVuTXT::Zone *child = parent;
  if(tag.name == "WORD" || tag.name == "LINE")
  {
    child = parent->append_child();
    child->ztype = (tag.name == "WORD") ? DjVuTXT::WORD: DjVuTXT::LINE;
    child->text_start = bs->tell();
  }
  
  if(tag.name == "WORD")
  {
    GList<int> rectArgs;
    lt_XMLEdit::intList(tag.args["coords"], rectArgs);
    GPosition i = rectArgs;
    
    child->rect.xmin = rectArgs[i];
    child->rect.ymin = height - 1 - rectArgs[++i];
    child->rect.xmax = rectArgs[++i];
    child->rect.ymax = height - 1 - rectArgs[++i];
    child->text_length = tag.raw.length();
    bs->write((const char*)tag.raw, child->text_length);
    return child->rect;
  }
  else
  {
    GRect ret;
    int set = 0;
    for(GPosition i = tag.content; i; ++i)
    {
      GP<lt_XMLTags> t = tag.content[i].tag;
      child->rect = make_next_layer(child, *t, bs, height);
      
      if(t->name == "LINE")
      {
        bs->write("\n", 1);
      }
      child->text_length = bs->tell() - child->text_start;
      
      // find the size of the rect that holds the zone
      if(!set)
      {  // on first pass
        ret.xmin = child->rect.xmin;
        ret.ymin = child->rect.ymin;
        ret.xmax = child->rect.xmax;
        ret.ymax = child->rect.ymax;
        set = 1;
      }
      else
      {  // grow the rect if needed
        if(ret.xmin > child->rect.xmin) ret.xmin = child->rect.xmin;
        if(ret.ymin > child->rect.ymin) ret.ymin = child->rect.ymin;
        if(ret.xmax < child->rect.xmax) ret.xmax = child->rect.xmax;
        if(ret.ymax < child->rect.ymax) ret.ymax = child->rect.ymax;
      }
    }
    return ret; // returns the rect that holds the zone
  }
}


// used in debugging to see if the zone tree is built right       
void step_down(DjVuTXT::Zone *parent)
{
  if(parent)
  {
    for(GPosition i = parent->children; i; ++i)
      step_down(&parent->children[i]);
  }
}

void 
lt_XMLText::ChangeText(const lt_XMLTags &tags, const GURL &url,const GUTF8String &id, const GUTF8String &width,const GUTF8String &height)
{
  const GUTF8String url_string((const char *)url);
  DjVuDocument &doc = *(m_docs[url_string]);
  GUTF8String xid;
  
  if(id.is_int())
  {
    const int page = id.toInt();
    xid = page > 0 ? GUTF8String(page - 1) : GUTF8String("0");
  }
  else if(!id.length() || (doc.get_doc_type() == DjVuDocument::SINGLE_PAGE))
  {
    xid = "0";
  }
  else
  {
    xid=id;
  }
  
  GP<DjVuFile> dfile = doc.get_djvu_file(xid,true);
  if(!dfile)
  {
    G_THROW("Failed to get specified page");
  }
  
  dfile->start_decode();
  dfile->wait_for_finish();
  
  GP<DjVuText> text = DjVuText::create();
  GP<DjVuTXT> txt = text->txt = DjVuTXT::create();
  
  // to store the new text
  GP<ByteStream> textbs = ByteStream::create(); 
  
  txt->page_zone.text_start = 0;
  txt->page_zone.rect = make_next_layer(&txt->page_zone, tags, textbs, atoi(height));
  textbs->write("\0", 1);
  long len = textbs->tell();
  txt->page_zone.text_length = len;
  textbs->seek(0,SEEK_SET);
  textbs->read(txt->textUTF8.getbuf(len), len);
  
  dfile->set_modified(true);
  dfile->reset();
  GP<ByteStream> mbs = ByteStream::create();
  dfile->text = mbs;
  text->encode(dfile->text);
  m_files.append(dfile);
}

void
lt_XMLText::parse(const lt_XMLTags &tags)
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
          
          GURL url;
          const GURL::UTF8 simpleURL(args[datapos]);
          if (simpleURL.is_empty())
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