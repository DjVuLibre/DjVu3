//C-  -*- C++ -*-
//C- DjVu® Reference Library (v. 3.0)
//C- 
//C- Copyright © 1999-2000 LizardTech, Inc. All Rights Reserved.
//C- 
//C- This software (the "Original Code") is subject to, and may be
//C- distributed under, the GNU General Public License, Version 2.
//C- The license should have accompanied the Original Code or you
//C- may obtain a copy of the license from the Free Software
//C- Foundation at http://www.fsf.org .
//C- 
//C- With respect to the Original Code, and subject to any third
//C- party intellectual property claims, LizardTech grants recipient
//C- a worldwide, royalty-free, non-exclusive license under patent
//C- claims infringed by making, using, or selling Original Code
//C- which are now or hereafter owned or controlled by LizardTech,
//C- but solely to the extent that any such patent is reasonably
//C- necessary to enable you to make, have made, practice, sell, or 
//C- otherwise dispose of Original Code (or portions thereof) and
//C- not to any greater extent that may be necessary to utilize
//C- further modifications or combinations.
//C- 
//C- The Original Code is provided "AS IS" WITHOUT WARRANTY OF ANY
//C- KIND, EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO
//C- ANY WARRANTY OF NON-INFRINGEMENT, OR ANY IMPLIED WARRANTY OF 
//C- MERCHANTIBILITY OF FITNESS FOR A PARTICULAR PURPOSE.
// 
// $Id: DjVuNavDir.cpp,v 1.13 2000-11-03 02:08:37 bcr Exp $
// $Name:  $

#ifdef __GNUC__
#pragma implementation
#endif

#include "DjVuNavDir.h"
#include "debug.h"
#include "GException.h"
#include "GOS.h"
#include <ctype.h>

DjVuNavDir::DjVuNavDir(const char * dirURL)
{
   if (!dirURL) G_THROW("DjVuNavDir.zero_dir");
   baseURL=GURL(dirURL).base();
}

DjVuNavDir::DjVuNavDir(ByteStream & str, const char * dirURL)
{
   if (!dirURL) G_THROW("DjVuNavDir.zero_dir");
   
   baseURL=GURL(dirURL).base();
   
   decode(str);
}

void
DjVuNavDir::decode(ByteStream & str)
{
   GCriticalSectionLock lk(&lock);
   
   GList<GString> tmp_page2name;
   int eof=0;
   while(!eof)
   {
      char buffer[1024];
      char * ptr;
      for(ptr=buffer;ptr-buffer<1024;ptr++)
	 if ((eof=!str.read(ptr, 1)) || *ptr=='\n') break;
      if (ptr-buffer==1024) G_THROW("DjVuNavDir.long_line");
      *ptr=0;
      if (!strlen(buffer)) continue;

      if (!tmp_page2name.contains(buffer))
	 tmp_page2name.append(buffer);
   };

   // Now copying lists to arrays for faster access later
   int pages=tmp_page2name.size();
   page2name.resize(pages-1);

   int cnt;
   GPosition pos;
   for(pos=tmp_page2name, cnt=0;pos;++pos, cnt++)
      page2name[cnt]=tmp_page2name[pos];
   
   // Now creating reverse mapping (strings => numbers)
   for(cnt=0;cnt<pages;cnt++)
   {
      name2page[page2name[cnt]]=cnt;
      url2page[baseURL+GOS::encode_reserved(page2name[cnt])]=cnt;
   }
}

#ifndef NEED_DECODER_ONLY
void
DjVuNavDir::encode(ByteStream & str)
{
   GCriticalSectionLock lk(&lock);

   for(int i=0;i<page2name.size();i++)
   {
      GString & name=page2name[i];
      str.writall((const char*)name, name.length());
      str.writall("\n", 1);
   };
}
#endif NEED_DECODER_ONLY

int
DjVuNavDir::get_pages_num(void) const
{
   GCriticalSectionLock lk((GCriticalSection *)&lock);
   
   return page2name.size();
}

int
DjVuNavDir::name_to_page(const char * name) const
{
   GCriticalSectionLock lk((GCriticalSection *)&lock);

   if (!name2page.contains(name)) return -1;
   return name2page[name];
}

int
DjVuNavDir::url_to_page(const GURL & url) const
{
   GCriticalSectionLock lk((GCriticalSection *)&lock);

   if (!url2page.contains(url)) return -1;
   return url2page[url];
}

GString
DjVuNavDir::page_to_name(int page) const
{
   GCriticalSectionLock lk((GCriticalSection *)&lock);
   
   if (page<0) G_THROW("DjVuNavDir.neg_page");
   if (page>=page2name.size())
      G_THROW("DjVuNavDir.large_page");
   return page2name[page];
}

GURL
DjVuNavDir::page_to_url(int page) const
{
   GCriticalSectionLock lk((GCriticalSection *)&lock);
   
   return baseURL+GOS::encode_reserved(page_to_name(page));
}

void
DjVuNavDir::insert_page(int where, const char * name)
{
   GCriticalSectionLock lk((GCriticalSection *)&lock);

   int pages=page2name.size();
   if (where<0) where=pages;
   
   page2name.resize(pages);
   for(int i=pages;i>where;i--)
      page2name[i]=page2name[i-1];
   page2name[where]=name;
   name2page[name]=where;
   url2page[baseURL+GOS::encode_reserved(name)]=where;
}

#ifndef NEED_DECODER_ONLY
void
DjVuNavDir::delete_page(int page_num)
{
   GCriticalSectionLock lk((GCriticalSection *)&lock);

   int pages=page2name.size();
   
   if (page_num<0 || page_num>=pages)
      G_THROW("DjVuNavDir.bad_page");

   for(int i=page_num;i<pages-1;i++)
      page2name[i]=page2name[i+1];
   page2name.resize(--pages-1);
}
#endif

