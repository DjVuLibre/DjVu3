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
//C- $Id: GCache.h,v 1.6 1999-06-11 22:07:29 leonb Exp $

#ifndef _GCACHE_H
#define _GCACHE_H

#ifdef __GNUC__
#pragma interface
#endif

#include "GSmartPointer.h"
#include "GContainer.h"
#include "Arrays.h"
#include "GThreads.h"
#include "debug.h"

#include <sys/types.h>
#include <time.h>

/** @name GCache.h
    Files #"GCache.h"# and #"GCache.cpp"# implement a simple caching class,
    which is based on \Ref{GMap} and is used by DjVu plugin to cache
    pages, which have already been decoded.

    See \Ref{GCache} for details.
    
    @memo Simple template data caching class.
    @author Andrei Erofeev <eaf@geocities.com>
    @version #$Id: GCache.h,v 1.6 1999-06-11 22:07:29 leonb Exp $#
*/

//@{

// We employ this GCacheItemBase to be able to qsort cache items basing
// on their sizes.
template<class Key, class Value>
class GCache;

class GCacheItemBase
{
public:
  GCacheItemBase(void);
  virtual ~GCacheItemBase(void);
  time_t	get_time(void) const;
public:
  // Do not use: These members are public because
  // friend template classes do not work well enough.
  time_t	time;
  GPosition	cache_pos;
  static int	qsort_func(const void * el1, const void * el2);
};

inline
GCacheItemBase::GCacheItemBase(void) : time(::time(0)) {}

inline
GCacheItemBase::~GCacheItemBase(void) {}

inline time_t
GCacheItemBase::get_time(void) const
{
   return time;
}

template<class Value>
class GCacheItem : public GCacheItemBase
{
public:
   GCacheItem(void);
   GCacheItem(const GP<Value> & xvalue);
   virtual ~GCacheItem(void);
   
   GP<Value>	get_value(void) const;
   unsigned int	get_size(void) const;
private:
   GP<Value>	value;
};

template<class Value> inline
GCacheItem<Value>::GCacheItem(void) {};

template<class Value> inline
GCacheItem<Value>::GCacheItem(const GP<Value> & xvalue) :
      value(xvalue) {}

template<class Value> inline
GCacheItem<Value>::~GCacheItem(void) {}

template<class Value> inline GP<Value>
GCacheItem<Value>::get_value(void) const
{
   return value;
}

template<class Value> inline unsigned int
GCacheItem<Value>::get_size(void) const
{
   return value->get_memory_usage();
}

/** #GCache# is a simple template class for caching data of any type.
    It is based on \Ref{GMap} class and like \Ref{GMap} it stores inside
    pairs #<key, value>#. In addition to \Ref{GMap} it keeps track of
    the age of components and is able to delete them when the total size
    of the map exceeds some predefined limit.

    Whenever a new item is added, #GCache# recomputes the total size
    of the cached items, and if it is too big, #GCache# will try to get
    rid of the oldest items until the size is less than predefined
    #max_size#. */
template<class Key, class Value>
class GCache
{
public:
      /** Constructs the #GCache#
	  @param max_size Maximum allowed size of the cache in bytes. */
   GCache(int max_size=5*2*1024*1024);
   virtual ~GCache(void);

      /** Attempts to find the value corresponding to #key# and return it.
	  #ZERO# will be returned if #GCache# does not know anything about the
	  #key#. If the cache is disabled, #ZERO# will be returned as well. */
   GP<Value> 	get_item(const Key & key);
      /** Deletes the item corresponding to the key #key#. Does nothing
	  if the cache is disabled. */
   void		del_item(const Key & key);
      /** Adds (possibly replaces) pair #<key, value># to the cache.
	  If the total size of all elements in the cache is greater than
	  the maximum size, the function will be deleting the oldest items
	  until the size is OK. Does nothing if the cache is disabled. */
   void		add_item(const Key & key, const GP<Value> & value);
      /** Clears the cache. All items are deleted. */
   void		clear(void);
      /** Sets new maximum size. If the total size of all items in the cache
	  is greater than #max_size#, the cache will be deleting the oldest
	  items until the size is OK */
   void		set_max_size(int max_size);
      /** Enables or disables the cache. See \Ref{is_enabled}() for details
	  @param en - If {\em en} is TRUE, the cache will be enabled.
	         Otherwise it will be disabled.
	*/
   void		enable(bool en);
      /** Returns #TRUE# if the cache is enabled, #FALSE# otherwise.
	  When a cache is disabled, \Ref{add_item}(), \Ref{get_item}() and
	  \Ref{del_item}() do nothing. But the {\em maximum size} is preserved
	  inside the class so that next time the cache is enabled, it will
	  be configured the same way. Clearly this "enable/disable" thing is
	  for convenience only. One could easily simulate this behaviour by
	  setting the {\em maximum size} of the cache to #ZERO#. */
   bool		is_enabled(void) const;
private:
   GCriticalSection		cache_lock;
   GMap<Key, GCacheItem<Value> >cache;
   bool		enabled;
   int		max_size;
   int		cur_size;

   int		calculate_size(void);
   void		clear_to_size(int size);
};

//@}

template<class Key, class Value> inline
GCache<Key, Value>::GCache(int xmax_size) :
      enabled(1), max_size(xmax_size), cur_size(0) {}

template<class Key, class Value> inline
GCache<Key, Value>::~GCache(void) {}

template<class Key, class Value> int
GCache<Key, Value>::calculate_size(void)
{
   GCriticalSectionLock lock(&cache_lock);
   
   int size=0;
   for(GPosition pos=cache;pos;++pos)
      size+=cache[pos].get_size();
   return size;
}

template<class Key, class Value> inline GP<Value>
GCache<Key, Value>::get_item(const Key & k)
{
   GCriticalSectionLock lock(&cache_lock);
   
   GPosition pos;
   if (cache.contains(k, pos)) return cache[pos].get_value();
   else return 0;
}

template<class Key, class Value> void
GCache<Key, Value>::del_item(const Key & k)
{
   DEBUG_MSG("GCache::del_item(): Removing an item from cache\n");
   DEBUG_MAKE_INDENT(3);

   GCriticalSectionLock lock(&cache_lock);
   
   GPosition pos;
   if (cache.contains(k, pos))
   {
      cur_size-=cache[pos].get_size();
      cache.del(pos);
   };
   if (cur_size<=0) cur_size=calculate_size();
   DEBUG_MSG("current cache size=" << cur_size << "\n");
}

template<class Key, class Value> void
GCache<Key, Value>::clear_to_size(int size)
{
   DEBUG_MSG("GCache::clear_to_size(): dropping cache size to " << size << "\n");
   DEBUG_MAKE_INDENT(3);

   GCriticalSectionLock lock(&cache_lock);
   
   if (size==0)
   {
      cache.empty();
      cur_size=0;
   } else
      if (cache.size()>50)
      {
	    // More than 50 elements in the cache: use qsort to
	    // sort them before picking up the oldest
	 TArray<void *> item_arr(cache.size()-1);
	 GPosition pos;
	 int i;
	 for(pos=cache, i=0;pos;++pos, i++)
	 {
	    GCacheItemBase * item=&cache[pos];
	    item->cache_pos=pos;
	    item_arr[i]=item;
	 };

	 qsort((void**)item_arr, item_arr.size(), sizeof(item_arr[0]),
	       GCacheItemBase::qsort_func);

	 for(i=0;i<item_arr.size() && cur_size>(int) size;i++)
	 {
	    GCacheItem<Value> * item=(GCacheItem<Value> *) item_arr[i];
	    cur_size-=item->get_size();
	    cache.del(item->cache_pos);
	    if (cur_size<=0) cur_size=calculate_size();
	 };
      } else
      {
	    // Less than 50 elements: no reason to presort
	 while(cur_size>(int) size)
	 {
	    if (!cache.size())
	    {
		  // Oops. Discrepancy due to an item changed its size
	       cur_size=0;
	       break;
	    };

	       // Remove the oldest cache item
	    GPosition oldest_pos=cache;
	    GPosition pos=cache;
	    for(++pos;pos;++pos)
	       if (cache[pos].get_time()<cache[oldest_pos].get_time())
		  oldest_pos=pos;
	    cur_size-=cache[oldest_pos].get_size();
	    cache.del(oldest_pos);

	       // cur_size *may* become negative because items may change their
	       // size after they've been added to the cache
	    if (cur_size<=0) cur_size=calculate_size();
	 };
      };
   
   DEBUG_MSG("done: current cache size=" << cur_size << "\n");
}

template<class Key, class Value> void
GCache<Key, Value>::add_item(const Key & k, const GP<Value> & v)
{
   DEBUG_MSG("GCache::add_item(): trying to add a new item\n");
   DEBUG_MAKE_INDENT(3);

   GCriticalSectionLock lock(&cache_lock);

   int _max_size=enabled ? max_size : 0;
   if (max_size<0) _max_size=max_size;

   int add_size=v->get_memory_usage();
   
   if (_max_size>=0 && add_size>_max_size)
   {
      DEBUG_MSG("but this item is way too large => doing nothing\n");
      return;
   };

   del_item(k);
   if (_max_size>=0) clear_to_size(_max_size-add_size);
   
   cache[k]=v;
   cur_size+=add_size;
}

template<class Key, class Value> void
GCache<Key, Value>::clear(void)
{
   clear_to_size(0);
}

template<class Key, class Value> void
GCache<Key, Value>::set_max_size(int xmax_size)
{
   DEBUG_MSG("GCache::set_max_size(): resizing to " << xmax_size << "\n");
   DEBUG_MAKE_INDENT(3);

   GCriticalSectionLock lock(&cache_lock);
   
   max_size=xmax_size;
   cur_size=calculate_size();

   if (max_size>=0) clear_to_size(enabled ? max_size : 0);
}

template<class Key, class Value> void
GCache<Key, Value>::enable(bool en)
{
   enabled=en;
   set_max_size(max_size);
}

template<class Key, class Value> bool
GCache<Key, Value>::is_enabled(void) const
{
   return enabled;
}

#endif
