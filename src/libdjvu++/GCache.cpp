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
//C- $Id: GCache.cpp,v 1.1.2.1 1999-04-12 16:48:22 eaf Exp $

#ifdef __GNUC__
#pragma implementation
#endif

#include "GCache.h"
#include "DjVuFile.h"

int
GCacheItemBase::qsort_func(const void * el1, const void * el2)
{
   const GCacheItemBase * item1=*(GCacheItemBase **) el1;
   const GCacheItemBase * item2=*(GCacheItemBase **) el2;
   time_t time1=item1->get_time();
   time_t time2=item2->get_time();

   return time1<time2 ? -1 : time1>time2 ? 1 : 0;
}

static GCache<GString, DjVuFile> cache;

void f(void)
{
   cache.add_item("haha", new DjVuFile("olala", 0, 0));
   cache.del_item("haha");
   cache.get_item("geg");
   cache.set_max_size(0);
}
