//C-  -*- C++ -*-
//C-
//C- Copyright (c) 1999 AT&T Corp.
//C- Copyright (c) 2000 LizardTech Inc.
//C- All rights reserved.
//C-
//C- This software may only be used by you under license from LizardTech
//C- Inc. A copy of LizardTech's Source Code Agreement is available at
//C- LizardTech's Internet website having the URL <http://www.djvu.com/open>.
//C- If you received this software without first entering into a license with
//C- LizardTech, you have an infringing copy of this software and cannot use it
//C- without violating LizardTech's intellectual property rights.
//C-
//C- $Id: DjVuFileCache.cpp,v 1.7 2000-09-18 17:10:10 bcr Exp $

#ifdef __GNUC__
#pragma implementation
#endif

#include "DjVuFileCache.h"

#ifndef UNDER_CE
int
DjVuFileCache::Item::qsort_func(const void * el1, const void * el2)
{
   const Item * item1=*(Item **) el1;
   const Item * item2=*(Item **) el2;
   time_t time1=item1->get_time();
   time_t time2=item2->get_time();
   return time1<time2 ? -1 : time1>time2 ? 1 : 0;
}

void
DjVuFileCache::set_max_size(int xmax_size)
{
   DEBUG_MSG("DjVuFileCache::set_max_size(): resizing to " << xmax_size << "\n");
   DEBUG_MAKE_INDENT(3);

   GCriticalSectionLock lock(&class_lock);
   
   max_size=xmax_size;
   cur_size=calculate_size();

   if (max_size>=0) clear_to_size(enabled ? max_size : 0);
}

void
DjVuFileCache::enable(bool en)
{
   enabled=en;
   set_max_size(max_size);
}

void
DjVuFileCache::add_file(const GP<DjVuFile> & file)
{
   DEBUG_MSG("DjVuFileCache::add_file(): trying to add a new item\n");
   DEBUG_MAKE_INDENT(3);

   GCriticalSectionLock lock(&class_lock);

      // See if the file is already cached
   GPosition pos;
   for(pos=list;pos;++pos)
      if (list[pos]->get_file()==file) break;
   
   if (pos) list[pos]->refresh();	// Refresh the timestamp
   else
   {
	 // Doesn't exist in the list yet
      int _max_size=enabled ? max_size : 0;
      if (max_size<0) _max_size=max_size;

      int add_size=file->get_memory_usage();
   
      if (_max_size>=0 && add_size>_max_size)
      {
	 DEBUG_MSG("but this item is way too large => doing nothing\n");
	 return;
      }

      if (_max_size>=0) clear_to_size(_max_size-add_size);

      list.append(new Item(file));
      cur_size+=add_size;
      file_added(file);
   }
}

void
DjVuFileCache::clear_to_size(int size)
{
   DEBUG_MSG("DjVuFileCache::clear_to_size(): dropping cache size to " << size << "\n");
   DEBUG_MAKE_INDENT(3);

   GCriticalSectionLock lock(&class_lock);
   
   if (size==0)
   {
      list.empty();
      cur_size=0;
   } else
      if (list.size()>20)
      {
	    // More than 20 elements in the cache: use qsort to
	    // sort them before picking up the oldest
	 GTArray<void *> item_arr(list.size()-1);
	 GPosition pos;
	 int i;
	 for(pos=list, i=0;pos;++pos, i++)
	 {
	    GP<Item> item=list[pos];
	    item->list_pos=pos;
	    item_arr[i]=item;
	 }

	 qsort(&item_arr[0], item_arr.size(), sizeof(item_arr[0]), Item::qsort_func);

	 for(i=0;i<item_arr.size() && cur_size>(int) size;i++)
	 {
	    Item * item=(Item *) item_arr[i];
	    cur_size-=item->get_size();
	    GP<DjVuFile> file=item->file;
	    list.del(item->list_pos);
	    file_cleared(file);
	    if (cur_size<=0) cur_size=calculate_size();
	 }
      } else
      {
	    // Less than 20 elements: no reason to presort
	 while(cur_size>(int) size)
	 {
	    if (!list.size())
	    {
		  // Oops. Discrepancy due to an item changed its size
	       cur_size=0;
	       break;
	    }

	       // Remove the oldest cache item
	    GPosition oldest_pos=list;
	    GPosition pos=list;
	    for(++pos;pos;++pos)
	       if (list[pos]->get_time()<list[oldest_pos]->get_time())
		  oldest_pos=pos;
	    cur_size-=list[oldest_pos]->get_size();
	    GP<DjVuFile> file=list[oldest_pos]->file;
	    list.del(oldest_pos);
	    file_cleared(file);

	       // cur_size *may* become negative because items may change their
	       // size after they've been added to the cache
	    if (cur_size<=0) cur_size=calculate_size();
	 }
      }
   
   DEBUG_MSG("done: current cache size=" << cur_size << "\n");
}

int
DjVuFileCache::calculate_size(void)
{
   GCriticalSectionLock lock(&class_lock);
   
   int size=0;
   for(GPosition pos=list;pos;++pos)
      size+=list[pos]->get_size();
   return size;
}

void
DjVuFileCache::del_file(const DjVuFile * file)
{
   DEBUG_MSG("DjVuFileCache::del_file(): Removing an item from cache\n");
   DEBUG_MAKE_INDENT(3);

   GCriticalSectionLock lock(&class_lock);

   for(GPosition pos=list;pos;++pos)
      if (list[pos]->get_file()==file)
      {
	 GP<DjVuFile> file=list[pos]->get_file();
	 cur_size-=list[pos]->get_size();
	 list.del(pos);
	 file_deleted(file);
	 break;
      }
   if (cur_size<0) cur_size=calculate_size();
   DEBUG_MSG("current cache size=" << cur_size << "\n");
}

GPList<DjVuFileCache::Item>
DjVuFileCache::get_items(void)
{
   GCriticalSectionLock lock(&class_lock);

   return list;
}

void
DjVuFileCache::file_added(const GP<DjVuFile> &) {}

void
DjVuFileCache::file_deleted(const GP<DjVuFile> &) {}

void
DjVuFileCache::file_cleared(const GP<DjVuFile> &) {}

#endif
