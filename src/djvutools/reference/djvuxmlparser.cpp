//C-  Copyright © 2000-2001, LizardTech, Inc. All Rights Reserved.
//C-              Unauthorized use prohibited.
//
// $Id: djvuxmlparser.cpp,v 1.7 2001-06-05 03:19:57 bcr Exp $
// $Name:  $

#include "XMLParser.h"
#include "XMLTags.h"
#include "GOS.h"
#include "GURL.h"
#include "DjVuDocument.h"
#include "ByteStream.h"
#include "DjVuMessage.h"
#include <stdio.h>
#include <ctype.h>
#include <locale.h>
#include <stdlib.h>

int 
main(int argc,char *argv[],char *[])
{
  setlocale(LC_ALL,"");
  djvu_programname(argv[0]);
  DArray<GUTF8String> dargv(0,argc-1);
  for(int i=0;i<argc;++i)
    dargv[i]=GNativeString(argv[i]);
  G_TRY
  {
    bool is_valid=(argc >= 2);
    if((is_valid=(argc>=2)))
    {
      int i=1;
      do {
        if(! GURL::Filename::Native(argv[i]).is_file())
        {
          is_valid=false;
          DjVuPrintError("Error: File '%s' does not exist.\n",argv[i]);
          exit(1);
        }
      } while (++i<argc);
    }
    if(! is_valid)
    {
      DjVuPrintError("Usage: %s <inputfiles>\n",argc?argv[0]:"-");
      exit(1);
    }

    for(int i=0;i<argc;++i)
    {
      const GP<lt_XMLParser> parser(lt_XMLParser::create());
      {
        const GP<lt_XMLTags> tag(
          lt_XMLTags::create(GURL::Filename::Native(dargv[i])));
        parser->parse(*tag);
      }
      parser->save();
    }
  }
  G_CATCH(ex)
  {
    ex.perror();
    exit(1);
  }
  G_ENDCATCH;
  exit(0);
#ifdef WIN32
  return 0;
#endif
}

