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
// $Id: XMLEdit.cpp,v 1.1 2001-04-24 17:54:09 jhayes Exp $
// $Name:  $

#ifdef __GNUC__
#pragma implementation
#endif

#include "XMLEdit.h"
#include "UnicodeByteStream.h"
#include "GOS.h"
#include "GURL.h"
#include "DjVuDocument.h"
#include "DjVuFile.h"
#include "debug.h"
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>

void 
lt_XMLEdit::intList(char const *coords, GList<int> &retval)
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
lt_XMLEdit::empty(void)
{
  m_files.empty();
  m_docs.empty();
}

void 
lt_XMLEdit::save(void)
{
  for(GPosition pos=m_docs;pos;++pos)
  {
    DjVuDocument &doc=*(m_docs[pos]);
    GURL url=doc.get_init_url();
//    GUTF8String name=GOS::url_to_filename(url);
//    DjVuPrintMessage("Saving file '%s' with new annotations.\n",(const char *)url);
    const bool bundle=doc.is_bundled()||(doc.get_doc_type()==DjVuDocument::SINGLE_PAGE);
    doc.save_as(url,bundle);
  }
  empty();
}

void
lt_XMLEdit::parse(const char fname[],const GURL &codebase)
{
  m_codebase=codebase;
  GP<lt_XMLTags> tags=lt_XMLTags::create();
  const GURL::UTF8 url(fname,codebase);
  tags->init(url);
  parse(*tags);
}

void
lt_XMLEdit::parse(GP<ByteStream> &bs)
{
  GP<lt_XMLTags> tags=lt_XMLTags::create();
  tags->init(bs);
  parse(*tags);
}
  
