//C-  Copyright � 2000-2001, LizardTech, Inc. All Rights Reserved.
//C-              Unauthorized use prohibited.
//
// $Id: djvuxmlparser.cpp,v 1.1 2001-04-24 19:50:38 bcr Exp $
// $Name:  $

#include "XMLParser.h"
#include "XMLTags.h"
#include "GOS.h"
#include "GURL.h"
#include "DjVuDocument.h"
#include "ByteStream.h"
#include <stdio.h>
#include <ctype.h>

int 
main(int argc,char *argv[],char *[])
{
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

// this is a poor way of doing this
// if we had a global list of the mod files we would 
// only have to decode them once and save them once

    GP<lt_XMLParser> anno = lt_XMLParser::create_anno();
    GP<lt_XMLParser> text = lt_XMLParser::create_text();
    for(int i=1;i<argc;i++)
    {
      GP<lt_XMLTags> tag = lt_XMLTags::create();
      tag->init(GURL::Filename::Native(argv[i]));
      anno->parse(*tag);
      // if we try to change the text here we will 
      // lose the anno changes when we save
      anno->save();
      text->parse(*tag);
      text->save();
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

