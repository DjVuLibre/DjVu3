//C-  -*- C++ -*-
//C-
//C-  Copyright © 2000-2001, LizardTech, Inc. All Rights Reserved.
//C-              Unauthorized use prohibited.
//C-
// 
// $Id: djvutoxml.cpp,v 1.7 2001-06-25 18:24:45 bcr Exp $
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

static void
usage(void)
{
  DjVuPrintErrorUTF8("Usage: %s [--with[out]-anno] [--with[out]-text] <inputfile> <outputfile>\n",(const char *)GOS::basename(DjVuMessage::programname()));
}

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
  int notext=(-1);		// You can turn off outputting text
  int noanno=(-1);		//  or annotations here!
  if(argc>0)
  {
    const char * prog=(const char *)dargv[0];
    name=GOS::basename(dargv[0]);
    for(;argc>1;--argc, dargv.shift(-1))
    {
      const GUTF8String &arg=dargv[1];
      if(arg == "--with-text")
      {
        notext=0;
        if(noanno<0)
          noanno=1;
      }else if(arg == "--without-text")
      {
        notext=1;
        if(noanno<0)
          noanno=0;
      }else if(arg == "--with-anno")
      {
        noanno=0;
        if(notext<0)
          notext=1;
      }else if(arg == "--without-anno")
      {
        noanno=1;
        if(notext<0)
        {
          notext=0;
        }
      }else
      {
        break;
      }
    }
    dargv[0]=prog;
  }
  int flags=0;
  if(noanno > 0)
    flags|=DjVuImage::NOMAP;
  if(notext > 0)
    flags|=DjVuImage::NOTEXT;
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
      doc->writeDjVuXML(gstr_out,flags);
//      ByteStream &str_out=*gstr_out;
//      extract.writeXMLprolog( str_out);
//      extract.writeMainElement( str_out, doc, page_num );

  }
  G_CATCH(exc)
  {
    exc.perror();
    exit(1);
  }
  G_ENDCATCH;
  return 0;
}

