//C-  -*- C++ -*-
//C-
//C-  Copyright © 2000-2001, LizardTech, Inc. All Rights Reserved.
//C-              Unauthorized use prohibited.
//C-
// 
// $Id: djvutoxml.cpp,v 1.6 2001-06-21 21:38:14 bcr Exp $
// $Name:  $

#include "DjVuDocument.h"
#include "GOS.h"
#include "DjVuMessage.h"
#include "ByteStream.h"
#include "DjVuAnno.h"
#include "DjVuText.h"
#include "DjVuImage.h"
#include "debug.h"

#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <assert.h>
#include <ctype.h>

class DjVuXMLExtractor
{
public:
  DjVuXMLExtractor(void);
  char const * version;
  int notext;		// You can turn off outputting text
  int noanno;		//  or annotations here!
  void writeXMLprolog ( ByteStream & str_out );
  void doPage( ByteStream & str_out,
    const GP<DjVuDocument> & doc, int external_page_num );
  void writeDocBody( ByteStream & str_out,
              const GP<DjVuDocument> & doc, 
              int page_num );
  void writeMainElement( ByteStream & str_out,
                  const GP<DjVuDocument> & doc, 
                  int page_num );
};

DjVuXMLExtractor::DjVuXMLExtractor(void)
: version("1.0"), notext(-1), noanno(-1) {}

static void
usage(void)
{
  DjVuPrintErrorUTF8("Usage: %s [--version=1.[01]] [--with[out]-anno] [--with[out]-text] <inputfile> <outputfile>\n",(const char *)GOS::basename(DjVuMessage::programname()));
}

//------------------------ prototypes ------------------------

static void
writeDocHead( ByteStream & str_out,
              const GP<DjVuDocument> & doc );

static void
writeText( ByteStream & str_out,
           GP<DjVuTXT> txt,
           const int internal_page_num,
           const int WindowHeight );

static void
OutputZone( ByteStream & str_out,
            const GUTF8String &textUTF8,
            DjVuTXT::Zone zone,
            const int WindowHeight );

static GUTF8String
indent ( int spaces);

static void
BeginningTag( ByteStream & str_out,
              int ztype );

static void
EndingTag( ByteStream & str_out,
           int ztype );

//------------------------ implementation ------------------------
int
main(int argc, char * argv[], char *env[])
{
  setlocale(LC_ALL,"");
  djvu_programname(argv[0]);
  DArray<GUTF8String> dargv(0,argc-1);
  for(int i=0;i<argc;++i)
    dargv[i]=GNativeString(argv[i]);

  GUTF8String name;
  DjVuXMLExtractor extract;
  if(argc>0)
  {
    const char * prog=(const char *)dargv[0];
    name=GOS::basename(dargv[0]);
    for(;argc>1;--argc, dargv.shift(-1))
    {
      const GUTF8String &arg=dargv[1];
      if(arg == "--version=1.1")
      {
        extract.version="1.1";
        if(extract.notext<0)
          extract.notext=0;
        if(extract.noanno<0)
          extract.noanno=0;
      }else if(arg == "--version=1.0")
      {
        extract.version="1.0";
        extract.notext=1;
        if(extract.noanno<0)
          extract.noanno=0;
      }else if(arg == "--with-text")
      {
        extract.version="1.1";
        extract.notext=0;
        if(extract.noanno<0)
          extract.noanno=1;
      }else if(arg == "--without-text")
      {
        extract.notext=1;
        if(extract.noanno<0)
          extract.noanno=0;
      }else if(arg == "--with-anno")
      {
        extract.noanno=0;
        if(extract.notext<0)
          extract.notext=1;
      }else if(arg == "--without-anno")
      {
        extract.noanno=1;
        if(extract.notext<0)
        {
          extract.version="1.1";
          extract.notext=0;
        }
      }else
      {
        break;
      }
    }
    dargv[0]=prog;
  }
  if(extract.noanno<0 && extract.notext<0)
  {
    extract.noanno=0;
    extract.notext=0;
  }
#ifdef DEBUG_SET_LEVEL
  {
    static GUTF8String debug=GOS::getenv("DEBUG");
    if (debug.length())
    {
      int level=debug.is_int()?debug.toInt():0;
      if (level<1) level=1;
      if (level>32) level=32;
      DEBUG_SET_LEVEL(level);
    }
  }
#endif
    
  G_TRY
  {
      GUTF8String name_in, name_out;
      int page_num=-1;
      
      for(int i=1;i<argc;i++)
      {
        GUTF8String arg(dargv[i]);
        if (arg == "-" || arg[0] != '-' || arg[1] != '-')
        {
          if (!name_in.length())
          {
            if (arg == "-")
            {
              DjVuMessage::perror( ERR_MSG("DjVuToXML.std_read") );
              usage();
              exit(1);
            }
            name_in=arg;
          } else if (!name_out.length())
          {
            name_out=arg;
          }else
          {
            usage();
            exit(1);
          }
        } else if (arg == "--page")
        {
          if (i+1>=argc)
          {
            DjVuMessage::perror( ERR_MSG("DjVuToXML.no_num") );
            usage();
            exit(1);
          }
          i++;
          page_num=dargv[i].toInt() - 1;
          if (page_num<0)
          {
            DjVuMessage::perror( ERR_MSG("DjVuToXML.negative_num") );
            usage();
            exit(1);
          }
        } else if (arg == "--help")
        {
          usage();
          exit(1);
        } else
        {
          DjVuMessage::perror( ERR_MSG("DjVuToXML.unrecog_opt") "\t"+arg);
        }
      }
      
      if (!name_in.length())
      {
        DjVuMessage::perror( ERR_MSG("DjVuToXML.no_name") );
        usage();
        exit(1);
      }
      if (!name_out.length())
      {
        name_out="-";
      }
      
      GP<DjVuDocument> doc=DjVuDocument::create_wait(GURL::Filename::UTF8(name_in));

      GP<ByteStream> gstr_out=ByteStream::create(GURL::Filename::UTF8(name_out), "w");
      ByteStream &str_out=*gstr_out;
      extract.writeXMLprolog( str_out);
      extract.writeMainElement( str_out, doc, page_num );

  }
  G_CATCH(exc)
  {
    exc.perror();
    exit(1);
  }
  G_ENDCATCH;
  return 0;
}


void
DjVuXMLExtractor::writeXMLprolog ( ByteStream & str_out )
{
  // Write XMLDecl
  static char const XMLDecl[] = "<?xml version=\"1.0\" ?>\n";
  str_out.writall(XMLDecl, sizeof(XMLDecl)-1);

  // Write doctypedecl
  static char const doctypedecl[] = "<!DOCTYPE DjVuXML PUBLIC \"-//W3C//DTD DjVuXML %s//EN\" \"pubtext/DjVuXML-s.dtd\">\n";
  GUTF8String s;
  s.format(doctypedecl,version);
  str_out.writestring(s);
}


void
DjVuXMLExtractor::writeMainElement(
  ByteStream & str_out, const GP<DjVuDocument> & doc, int page_num )
{
  static char const Stag[] = "<DjVuXML>\n";
  static char const Etag[] = "</DjVuXML>\n";

  str_out.write( Stag, sizeof(Stag)-1 );
  writeDocHead( str_out, doc );
  writeDocBody( str_out, doc, page_num );
  str_out.write( Etag, sizeof(Etag)-1 );
}


static void
writeDocHead( ByteStream & str_out,
              const GP<DjVuDocument> & doc )
{
  static char const Stag[] = "<HEAD>\n";
  static char const Etag[] = "</HEAD>\n";

  str_out.write( Stag, sizeof(Stag)-1 );

  // content
  GUTF8String url = "<TITLE>" + doc->get_init_url().name() + "</TITLE>\n";
  str_out.write( (const char *)url, url.length() );

  str_out.write( Etag, sizeof(Etag)-1 );
}


void
DjVuXMLExtractor::writeDocBody( ByteStream & str_out,
              const GP<DjVuDocument> & doc, 
              int page_num )
{
  static char const Stag[] = "<BODY>\n";
  static char const Etag[] = "</BODY>\n";

  str_out.write( Stag, sizeof(Stag)-1 );

  // Write area information
  if (page_num<0)
  {
    for(page_num=0;page_num<doc->get_pages_num();)
      doPage(str_out, doc, ++page_num);
  }else
  {
    doPage(str_out, doc, page_num);
  }
  str_out.write( Etag, sizeof(Etag)-1 );
}

//  Generate output for a single page
//  If there are no maps, no output is generated. Otherwise, we generate an 
//  OBJECT and a MAP.
void
DjVuXMLExtractor::doPage( ByteStream & str_out,
        const GP<DjVuDocument> & doc,
        int external_page_num )
{
  int internal_page_num = external_page_num - 1;
  GP<DjVuImage> dimg = doc->get_page(internal_page_num);
  if (!dimg)
    G_THROW( ERR_MSG("DjVuToText.decode_failed") );

  //  <OBJECT> tag
  //    First, puzzle out the URL
  GURL url = doc->get_init_url();
  const GUTF8String pagename=dimg->get_djvu_file()->get_url().fname();
  //    Now, build the tag
  GUTF8String ObjectTag = "  <OBJECT";
  ObjectTag = ObjectTag + " data=\"" + url.get_string() + "\"";
  ObjectTag = ObjectTag + " type=\"" + dimg->get_mimetype() + "\"";
  ObjectTag = ObjectTag + " height=\"" + GUTF8String(dimg->get_height()) + "\"";
  ObjectTag = ObjectTag + " width=\"" + GUTF8String(dimg->get_width()) + "\"";
  if(!noanno)
  {
    ObjectTag = ObjectTag + " usemap=\""+pagename.toEscaped()+"\"";
  }
  ObjectTag = ObjectTag + ">\n";
  str_out.write( (const char *)ObjectTag, ObjectTag.length() );
  str_out.writestring("    <PARAM name=\"PAGE\" value=\""+pagename.toEscaped()+"\" />\n");  //----strict switch?

  const GP<DjVuInfo> info(dimg->get_info());
  if(info)
  {
    str_out.writestring(info->get_paramtags());
  }
  const GP<DjVuAnno> anno(DjVuAnno::create());
  if(!noanno)
  {
    const GP<ByteStream> anno_str = dimg->get_anno();
    if(anno_str)
    {
      anno->decode(anno_str);
    }
  }
  str_out.writestring(anno->get_paramtags());
  //  TEXT element (if any)
  if(!notext)
  {
    GP<ByteStream> text_str = dimg->get_text();
    if (text_str)
    {
      GP<DjVuText> text = DjVuText::create();
      text->decode(text_str);
      GP<DjVuTXT> txt = text->txt;
      if( txt )
      {
        static char const Stag3[] = "    <HIDDENTEXT>\n";
        static char const Etag3[] = "    </HIDDENTEXT>\n";
        str_out.write( Stag3, sizeof(Stag3)-1 );
        writeText( str_out, txt, internal_page_num, dimg->get_height() );
        str_out.write( Etag3, sizeof(Etag3)-1 );
      }
    }
  }
  //  </OBJECT>
  static char const Etag1[] = "  </OBJECT>\n";
  str_out.write( Etag1, sizeof(Etag1)-1 );

  if(!noanno)
  {
    str_out.writestring(anno->get_xmlmap(pagename,dimg->get_height()));
  }
}


static void
writeText( ByteStream & str_out,
           GP<DjVuTXT> txt,
           const int internal_page_num,
           const int WindowHeight )
{
  assert( txt->has_valid_zones() );
//  DEBUG_MSG( "--zonetype=" << txt->page_zone.ztype << "\n" );

  GPosition i = txt->page_zone.children;

  //  Beginning tags for missing layers
  int layer;
  int next_layer = txt->page_zone.children[i].ztype;
  for( layer = DjVuTXT::COLUMN ; layer < next_layer ; layer++ )
    BeginningTag( str_out, layer );

  //  Output the next layer
  for( ; i ; ++i )
    OutputZone( str_out,
                txt->textUTF8,
                txt->page_zone.children[i], 
                WindowHeight );

  //  Ending tags for missing layers
  for( layer = next_layer-1 ; layer > DjVuTXT::PAGE ; layer-- )
    EndingTag( str_out, layer );
}
static const char *tags[8]=
{ "HIDDENTEXT",
  "PAGE",
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
  if((tags_size > (int)zone)&&((int)zone >= 0))
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

static void
OutputZone( ByteStream & str_out,
            const GUTF8String &textUTF8,
            DjVuTXT::Zone zone,
            const int WindowHeight )
{
//  DEBUG_MSG( "--zonetype=" << zone.ztype << "\n" );

  const GUTF8String xindent(indent( 2 * zone.ztype + 2 ));
  GPosition pos=zone.children;
  // Build attribute string
  if( ! pos )
  {
    const GUTF8String lf((zone.ztype < DjVuTXT::WORD)?"\n":"");
    GUTF8String tag= xindent;
    tag.format((const char *)(tag+"<%s coords=\"%d,%d,%d,%d\">"+lf),
      tags[zone.ztype],
      zone.rect.xmin, WindowHeight - 1 - zone.rect.ymin,
      zone.rect.xmax, WindowHeight - 1 - zone.rect.ymax);
    const int start=zone.text_start;
    const int end=textUTF8.firstEndSpace(start,zone.text_length);
    str_out.writestring(tag+textUTF8.substr(start,end-start).toEscaped()
      +lf+"</"+tags[zone.ztype]+">\n");
    str_out.writestring(end_tag(zone.ztype));
  } else
  {
    //  Beginning tags for this layer and any missing layers
    int layer;

    //  Output the next layer
    for(layer=zone.ztype; pos ; ++pos )
    {
      int next_layer=zone.children[pos].ztype;
      for( ;layer < next_layer;layer++ )
      {
        str_out.writestring(start_tag(layer));
      }
      while (layer > next_layer )
      {
        str_out.writestring(end_tag(--layer));
      }
      OutputZone( str_out,
                  textUTF8,
                  zone.children[pos], 
                  WindowHeight );
    }
    for(const int next_layer=zone.ztype;layer > next_layer;)
    {
      str_out.writestring(end_tag(--layer));
    }
  }
}


static GUTF8String
indent ( int spaces)
{
  GUTF8String ret;
  for( int i = 0 ; i < spaces ; i++ )
    ret += ' ';
  return ret;
}


static void
BeginningTag( ByteStream & str_out,
              int ztype )
{
  static GUTF8String tags[8] = { "empty", "page (unused)", "<PAGECOLUMN>\n", 
                             "<REGION>\n", "<PARAGRAPH>\n", "<LINE>\n", 
                             "word (unused)", "character (unused)" };
  GUTF8String out = indent( 2 * ztype + 2 );
  out += tags[ztype];
  str_out.write( (char const *)out, out.length() );
}


static void
EndingTag( ByteStream & str_out,
           int ztype )
{
  static GUTF8String tags[] = { "empty", "page (unused)", "</PAGECOLUMN>\n", 
                            "</REGION>\n", "</PARAGRAPH>\n", "</LINE>\n",
                            "word (unused)", "character (unused)" };
  GUTF8String out = indent( 2 * ztype + 2 );
  out += tags[ztype];
  str_out.write( (char const *)out, out.length() );
}


