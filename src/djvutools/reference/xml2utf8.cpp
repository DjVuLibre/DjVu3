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
// $Id: xml2utf8.cpp,v 1.4 2001-06-05 03:19:57 bcr Exp $
// $Name:  $

/** @name nativetoutf8

    {\bf Synopsis}
    \begin{verbatim}
        nativetoutf8 [<inputfile>] [<outputfile>]
    \end{verbatim}

    @author
    Dr Bill C Riemers <bcr@lizardtech.com>
    @version
    #$Id: xml2utf8.cpp,v 1.4 2001-06-05 03:19:57 bcr Exp $# */
//@{
//@}




#include "GException.h"
#include "UnicodeByteStream.h"
#include "ByteStream.h"
#include "DjVuMessage.h"
#include "GURL.h"

#include <locale.h>
#include <stdlib.h>
#include <stdio.h>

int 
main(int argc, char **argv)
{
  setlocale(LC_ALL,"");
  djvu_programname(argv[0]);
  DArray<GUTF8String> dargv(0,argc-1);
  for(int i=0;i<argc;++i)
    dargv[i]=GNativeString(argv[i]);
  GURL::Filename::UTF8 inurl((argc<2)?GUTF8String("-"):dargv[1]);
  GURL::Filename::UTF8 outurl((argc<3)?GUTF8String("-"):dargv[argc-1]);
  G_TRY
  {
    GP<ByteStream> bs(ByteStream::create(inurl,"r")); 
    {
      GP<XMLByteStream> uni=XMLByteStream::create(bs);
      bs=ByteStream::create();
      GUTF8String ustr;
      while((ustr=uni->gets()).length())
      {
        bs->writestring(ustr);
      }
    }
    bs->seek(0L);
    GP<ByteStream> outbs=ByteStream::create(outurl,"w");
    outbs->write24(0xEFBBBF);
    outbs->copy(*bs);
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


