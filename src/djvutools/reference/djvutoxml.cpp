//C-  -*- C++ -*-
//C-
//C-  Copyright © 2000-2001, LizardTech, Inc. All Rights Reserved.
//C-              Unauthorized use prohibited.
//C-
// 
// $Id: djvutoxml.cpp,v 1.3 2001-05-09 00:38:26 bcr Exp $
// $Name:  $

#include "DjVuDocument.h"
#include "GOS.h"
#include "DjVuMessage.h"
#include "ByteStream.h"
#include "GMapAreas.h"
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

static char const * progname="";
static char const * version="1.0";
static int notext=0;		// You can turn off outputting text
static int noanno=-1;		//  or annotations here!

static void
usage(void)
{
  DjVuPrintError("Usage: %s [--version=1.[01]] [--with[out]-anno] [--with[out]-text] <inputfile> <outputfile>\n",progname);
}

//------------------------ prototypes ------------------------

static void
writeXMLprolog ( ByteStream & str_out );

static void
writeMainElement( ByteStream & str_out,
                  const GP<DjVuDocument> & doc, 
                  int page_num );

static void
doPage( ByteStream & str_out,
        const GP<DjVuDocument> & doc, 
        int external_page_num);

static void
writeDocHead( ByteStream & str_out,
              const GP<DjVuDocument> & doc );

static void
writeDocBody( ByteStream & str_out,
              const GP<DjVuDocument> & doc, 
              int page_num );

static void
writeArea( ByteStream & str_out,
           GMapArea & area,
           int WindowHeight );

static void
writeText( ByteStream & str_out,
           GP<DjVuTXT> txt,
           const int internal_page_num,
           const int WindowHeight );

static void
OutputZone( ByteStream & str_out,
            GUTF8String textUTF8,
            DjVuTXT::Zone zone,
            int prev_ztype,
            int WindowHeight );

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
  DjVuMessage::use_locale();
  DArray<GUTF8String> dargv(0,argc-1);
  for(int i=0;i<argc;++i)
    dargv[i]=GNativeString(argv[i]);

  GUTF8String name;
  if(argc>0)
  {
    const char * prog=(const char *)dargv[0];
    name=GOS::basename(dargv[0]);
    progname=name;
    for(;argc>1;--argc, dargv.shift(-1))
    {
      const char * const arg=dargv[1];
      if((arg == GUTF8String("--version=1.1")))
      {
        version="1.1";
        if(notext<0)
          notext=0;
        if(noanno<0)
          noanno=0;
      }else if((arg == GUTF8String("--version=1.0")))
      {
        version="1.0";
        notext=1;
        if(noanno<0)
          noanno=0;
      }else if((arg == GUTF8String("--with-text")))
      {
        version="1.1";
        notext=0;
        if(noanno<0)
          noanno=1;
      }else if((arg == GUTF8String("--without-text")))
      {
        notext=1;
        if(noanno<0)
          noanno=0;
      }else if((arg == GUTF8String("--with-anno")))
      {
        noanno=0;
        if(notext<0)
          notext=1;
      }else if((arg == GUTF8String("--without-anno")))
      {
        noanno=1;
        if(notext<0)
        {
          version="1.1";
          notext=0;
        }
      }else
      {
        break;
      }
    }
    dargv[0]=prog;
  }
  if(noanno<0 && notext<0)
  {
    noanno=0;
    notext=1;
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
      writeXMLprolog( str_out);
      writeMainElement( str_out, doc, page_num );

  }
  G_CATCH(exc)
  {
    exc.perror();
    exit(1);
  }
  G_ENDCATCH;
  return 0;
}


static void
writeXMLprolog ( ByteStream & str_out )
{
  // Write XMLDecl
  static char const XMLDecl[] = "<?xml version=\"%s\" ?>\n";
  GUTF8String s;
  s.format(XMLDecl,version);
  str_out.write( (const char *)s, s.length() );

  // Write doctypedecl
  static char const doctypedecl[] = "<!DOCTYPE DjVuXML PUBLIC \"-//W3C//DTD DjVuXML %s//EN\" \"pubtext/DjVuXML-s.dtd\">\n";
  s.format(doctypedecl,version);
  str_out.write( (const char *)s,s.length());
}


static void
writeMainElement( ByteStream & str_out,
                  const GP<DjVuDocument> & doc, 
                  int page_num )
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


static void
writeDocBody( ByteStream & str_out,
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

static void
startMap( ByteStream &str_out, const int external_page_num, const bool isEmpty)
{
  static char const Stag2a[] = "  <MAP name=\"Page%0d\">\n";
  static char const Stag2b[] = "  <MAP name=\"Page%0d\"/>\n";
  char const * const Stag2 = (isEmpty)?Stag2a:Stag2b;
  GUTF8String MapTag;
  MapTag.format( Stag2, external_page_num );
  str_out.write( (const char *)MapTag, MapTag.length() );
}

//  Generate output for a single page
//  If there are no maps, no output is generated. Otherwise, we generate an 
//  OBJECT and a MAP.
static void
doPage( ByteStream & str_out,
        const GP<DjVuDocument> & doc,
        int external_page_num )
{
  int internal_page_num = external_page_num - 1;
  GP<DjVuImage> dimg = doc->get_page(internal_page_num);
  if (!dimg)
    G_THROW( ERR_MSG("DjVuToText.decode_failed") );

  //  <OBJECT> tag
  //    First, puzzle out the URL
  GURL url = doc->page_to_url(internal_page_num);
  if ( doc->get_doc_type() == DjVuDocument::BUNDLED )
    url = url.base();       // bundled, so strip off the page number
  //    Now, build the tag
  GUTF8String ObjectTag = "  <OBJECT";
  ObjectTag = ObjectTag + " data=\"" + url + "\"";
  ObjectTag = ObjectTag + " type=\"" + (*dimg).get_mimetype() + "\"";
  ObjectTag = ObjectTag + " height=\"" + GUTF8String((*dimg).get_height()) + "\"";
  ObjectTag = ObjectTag + " width=\"" + GUTF8String((*dimg).get_width()) + "\"";
  if(!noanno)
  {
    ObjectTag = ObjectTag + " usemap=\"Page" + GUTF8String(external_page_num) + "\"";
  }
  ObjectTag = ObjectTag + ">\n";
  str_out.write( (const char *)ObjectTag, ObjectTag.length() );
  //  page number
  GUTF8String PageNum;
  PageNum.format( "    <PARAM name=\"FLAGS\" value=\"PAGE=%0d\" />\n", external_page_num );  //----strict switch?
  str_out.write( (const char *)PageNum, PageNum.length() );

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
    GP<ByteStream> anno_str = dimg->get_anno();
    if (anno_str)
    {
      GP<DjVuAnno> anno = DjVuAnno::create();
      anno->decode(anno_str);

      // Annotation areas
      GP<DjVuANT> ant = anno->ant;
      if (ant)
      {
        startMap(str_out,external_page_num,true);
        static char const Etag2[] = "  </MAP>\n";
        GPList<GMapArea> maps = ant->map_areas;
        DEBUG_MSG( "Number of map areas = " << maps.size() << "\n" );

        //  MAP tag
        for( GPosition area = maps.firstpos() ; area ; ++area )
        {
          writeArea( str_out, *(maps[area]), dimg->get_height() );
        }
        str_out.write( Etag2, sizeof(Etag2)-1 );
      }else
      {
        startMap(str_out,external_page_num,false);
      }
    }else
    {
      startMap(str_out,external_page_num,false);
    }
  }
}


static void
writeArea( ByteStream & str_out,
           GMapArea & area,
           int WindowHeight )
{
  GUTF8String msg("    <AREA");

  //  shape
  msg = msg + " shape=\"" + area.get_shape_name() + "\"";

  //  coordinates
  GUTF8String coords;

  switch( area.get_shape_type() )
  {
  case GMapArea::RECT:          // For rectangle and oval, the coordinates are reported
  case GMapArea::OVAL:          // in top left, then bottom right regardless of the order
                                // in which they are stored.
    coords += "," + GUTF8String( area.get_xmin() ) +
              "," + GUTF8String( WindowHeight - 1 -area.get_ymax() ) +
              "," + GUTF8String( area.get_xmax() ) +
              "," + GUTF8String( WindowHeight - 1 -area.get_ymin() );
    break;

  default:                      // For everything else (polygons), report the points in
                                // order they appear.
    {
      GList<int> CoordList;
      area.get_coords( CoordList );
      for (GPosition i = CoordList ; i ; ++i)
      {
        coords += "," + GUTF8String( CoordList[i] );
        ++i;
        coords += "," + GUTF8String( WindowHeight - 1 - CoordList[i] );
      }
    }
    break;
  }

  msg = msg + " coords=\"" + coords.substr(1,coords.length()-1) + "\"";


  //  URI
  if( !!area.url ) 
    msg = msg + " href=\"" + area.url +"\"";
  else
    msg = msg + " nohref=\"nohref\"";

  //  alt
  msg = msg + " alt=\"" + GUTF8String(area.comment).toEscaped() + "\"";

  //  target
  if( area.target.length() > 0 )
    msg = msg + " target=\"" + area.target + "\"";

  //  highlight
  if( area.hilite_color != GMapArea::NO_HILITE &&
      area.hilite_color != GMapArea::XOR_HILITE )
  {
    GUTF8String hilite;
    hilite.format( "#%06X", area.hilite_color );
    msg = msg + " highlight=\"" + hilite + "\"";
  }

  //  border type
  GUTF8String b_type;
  switch( area.border_type )
  {
  case GMapArea::NO_BORDER:
    b_type = "none";
    break;
  case GMapArea::XOR_BORDER:
    b_type = "xor";
    break;
  case GMapArea::SOLID_BORDER:
    b_type = "solid";
    break;
  case GMapArea::SHADOW_IN_BORDER:
    b_type = "shadowin";
    break;
  case GMapArea::SHADOW_OUT_BORDER:
    b_type = "shadowout";
    break;
  case GMapArea::SHADOW_EIN_BORDER:
    b_type = "etchedin";
    break;
  case GMapArea::SHADOW_EOUT_BORDER:
    b_type = "etchedout";
    break;
  }
  msg = msg + " bordertype=\"" + b_type + "\"";

  //  border color
  if( GUTF8String(area.border_type) != GMapArea::NO_BORDER )
  {
    GUTF8String color;
    color.format( "#%06X", area.border_color );
    msg = msg + " bordercolor=\"" + color + "\"";
  }

  //  border width
  if( GUTF8String(area.border_type) != GMapArea::NO_BORDER )
    msg = msg + " border=\"" + GUTF8String(area.border_width) + "\"";

  //  border visible
  if( area.border_always_visible )
    msg = msg + " visible=\"visible\"";

  //  close message
  msg += " />\n";                                                       //-------------strictness switch?
  str_out.write( (const char *)msg, msg.length() );
}


static void
writeText( ByteStream & str_out,
           GP<DjVuTXT> txt,
           const int internal_page_num,
           const int WindowHeight )
{
  assert( txt->has_valid_zones() );
  DEBUG_MSG( "--zonetype=" << txt->page_zone.ztype << "\n" );

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
                txt->page_zone.ztype,
                WindowHeight );

  //  Ending tags for missing layers
  for( layer = next_layer-1 ; layer > DjVuTXT::PAGE ; layer-- )
    EndingTag( str_out, layer );
}


static void
OutputZone( ByteStream & str_out,
            GUTF8String textUTF8,
            DjVuTXT::Zone zone,
            int prev_ztype,
            int WindowHeight )
{
  DEBUG_MSG( "--zonetype=" << zone.ztype << "\n" );

  // Build attribute string
  if( zone.ztype == DjVuTXT::WORD )
  {
    GUTF8String word = indent( 2*zone.ztype + 2 );
    word += "<WORD coords=\"";
    GUTF8String coords;
    coords.format( "%d,%d,%d,%d\">", zone.rect.xmin, 
                                    WindowHeight - 1 - zone.rect.ymin,
                                    zone.rect.xmax, 
                                    WindowHeight - 1 - zone.rect.ymax);
    word += coords;
    word += textUTF8.substr(zone.text_start,zone.text_length).toEscaped();
    char *sword=word.getbuf();
    for(int i=word.length()-1;(i>0)&&isspace(sword[i]);--i)
    {
      sword[i]=0;
    }
    word += "</WORD>\n";
    str_out.write( (char const *)word, word.length() );
  } else
  {
    GPosition i = zone.children;

    //  Beginning tags for this layer and any missing layers
    int layer;
    int next_layer = zone.children[i].ztype;
    for( layer = zone.ztype ; layer < next_layer ; layer++ )
      BeginningTag( str_out, layer );

    //  Output the next layer
    for( ; i ; ++i )
      OutputZone( str_out,
                  textUTF8,
                  zone.children[i], 
                  zone.ztype,
                  WindowHeight );

    //  Ending tags for this layer and any missing layers
    for( layer = next_layer-1 ; layer >= zone.ztype ; layer-- )
      EndingTag( str_out, layer );
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


