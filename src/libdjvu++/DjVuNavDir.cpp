//C-  -*- C++ -*-
//C-
//C- Copyright (c) 1999 AT&T Corp.  All rights reserved.
//C-
//C- This software may only be used by you under license from AT&T
//C- Corp. ("AT&T"). A copy of AT&T's Source Code Agreement is available at
//C- AT&T's Internet website having the URL <http://www.djvu.att.com/open>.
//C- If you received this software without first entering into a license with
//C- AT&T, you have an infringing copy of this software and cannot use it
//C- without violating AT&T's intellectual property rights.
//C-
//C- $Id: DjVuNavDir.cpp,v 1.7 2000-01-26 23:59:32 eaf Exp $

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
   if (!dirURL) THROW("ZERO directory URL passed to the DjVuNavDir constructor.");
   baseURL=GURL(dirURL).base();
}

DjVuNavDir::DjVuNavDir(ByteStream & str, const char * dirURL)
{
   if (!dirURL) THROW("ZERO directory URL passed to the DjVuNavDir constructor.");
   
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
      if (ptr-buffer==1024) THROW("Failed to read DjVu directory. Line is too long.");
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
   
   if (page<0) THROW("Page number may not be negative.");
   if (page>=page2name.size())
      THROW("Page number is too big.");
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
      THROW("Invalid page number passed as input.");

   for(int i=page_num;i<pages-1;i++)
      page2name[i]=page2name[i+1];
   page2name.resize(--pages-1);
}
#endif

