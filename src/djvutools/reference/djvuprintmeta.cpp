//C-  -*- C++ -*-
//C-
//C-  Copyright © 2000-2001, LizardTech, Inc. All Rights Reserved.
//C-              Unauthorized use prohibited.
//C-
// 
// $Id: djvuprintmeta.cpp,v 1.2 2001-09-21 20:21:30 leonb Exp $
// $Name:  $

#include "DjVuDocument.h"
#include "GOS.h"
#include "DjVuMessage.h"
#include "ByteStream.h"
#include "DjVuAnno.h"
#include "DjVuText.h"
#include "DjVuImage.h"
#include "XMLTags.h"
#include "IFFByteStream.h"
#include "BSByteStream.h"

#include "debug.h"
#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <assert.h>
#include <ctype.h>

//------------------------ implementation ------------------------
int
main(int argc, char * argv[], char *env[])
{
  setlocale(LC_ALL,"");
  djvu_programname(argv[0]);
  if(argc !=4 )
  {
    fprintf(stderr,"Usage: %s <DjVuFile> <page#> <FIELDNAME>\n",argv[0]);
    fprintf(stderr,"Prints all metadata of the specified page and fieldname\n");
    fprintf(stderr,"example: %s foo.djvu 1 AUTHOR\n",argv[0]);
    exit(1);
  }
  G_TRY
  {
    GArray<GUTF8String> dargv(0,argc-1);
    for(int i=0;i<argc;++i)
      dargv[i]=GNativeString(argv[i]);

    GUTF8String name;
    if(argc>0)
    {
      const char * prog=(const char *)dargv[0];
      name=GOS::basename(dargv[0]);
      dargv[0]=prog;
    }
   
    GP<ByteStream> xbs(ByteStream::create());
    {
      const GP<DjVuDocument> doc(DjVuDocument::create_wait(GURL::Filename::UTF8(dargv[1])));
      const GP<DjVuFile> file(doc->get_djvu_file(dargv[2].toInt()-1));
      const GP<IFFByteStream> giff(IFFByteStream::create(file->get_meta()));
      for(GUTF8String chkid;giff->get_chunk(chkid);)
      {
        GP<ByteStream> gbs(giff->get_bytestream());
        if(chkid == "METa")
        {
          xbs->copy(*gbs);
        }else if(chkid == "METz")
        {
          gbs=BSByteStream::create(gbs);
          xbs->copy(*gbs);
        }
        giff->close_chunk();
      } 
    }
    if(xbs->size())
    {
      xbs->seek(0L);
      const GP<lt_XMLTags> gmetatags(lt_XMLTags::create(xbs));
      if(gmetatags)
      {
        lt_XMLTags &metatags=*gmetatags;
        GPosition pos=metatags.contains(dargv[3]);
        if(pos)
        {
          GPList<lt_XMLTags> gtags=metatags[pos];
          for(GPosition pos2=gtags;pos2;++pos2)
          {
            printf("%s='%s'\n",(const char *)dargv[3],(const char *)gtags[pos2]->get_raw());
          }
        }
      }
    }
  }  
  G_CATCH(exc)
  {
    exc.perror();
    exit(1);
  }
  G_ENDCATCH;
  exit(0);
#ifdef WIN32
  return 0;
#endif
}

