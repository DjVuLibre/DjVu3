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
//C- $Id: DjVuNavDir.h,v 1.1.2.2 1999-04-23 21:22:47 eaf Exp $

#ifndef _DJVUNAVDIR_H
#define _DJVUNAVDIR_H

#ifdef __GNUC__
#pragma interface
#endif

#include "ByteStream.h"
#include "GContainer.h"
#include "GSmartPointer.h"
#include "GString.h"
#include "GThreads.h"
#include "GURL.h"
#include "Arrays.h"

//*****************************************************************************
//********************* Note: this class is thread-safe ***********************
//*****************************************************************************

class DjVuNavDir : public GPEnabled
{
private:
   GCriticalSection		lock;
   GURL				baseURL;
   DArray<GString>		page2name;
   GMap<GString, int>		name2page;
   GMap<GURL, int>		url2page;
public:
   int		get_memory_usage(void) const { return 1024; };
   
   void		decode(ByteStream & str);
   void		encode(ByteStream & str);

   void		insert_page(int where, const char * name);
   void		delete_page(int page_num);
   
   int		get_pages_num(void) const;
   int		url_to_page(const GURL & url) const;
   int		name_to_page(const char * name) const;
   GURL		page_to_url(int page) const;
   GString	page_to_name(int page) const;
   
   DjVuNavDir(const char * dirURL=0);
   DjVuNavDir(ByteStream & str, const char * dirURL=0);
   virtual ~DjVuNavDir(void) {};
};

#endif
