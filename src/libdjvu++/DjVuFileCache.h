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
//C- $Id: DjVuFileCache.h,v 1.1 1999-09-10 21:52:36 eaf Exp $

#ifndef _DJVUFILECACHE_H
#define _DJVUFILECACHE_H

#ifdef __GNUC__
#pragma interface
#endif

#include "DjVuFile.h"
#include "Arrays.h"
#include "debug.h"

#include <sys/types.h>
#include <time.h>

/** @name DjVuFileCache.h
    Files #"DjVuFileCache.h"# and #"DjVuFileCache.cpp"# implement a simple
    caching mechanism for keeping a given number of \Ref{DjVuFile} instances
    alive. The cache estimates the size of its elements and gets rid of
    the oldest ones when necessary.

    See \Ref{DjVuFileCache} for details.
    
    @memo Simple DjVuFile caching class.
    @author Andrei Erofeev <eaf@geocities.com>
    @version #$Id: DjVuFileCache.h,v 1.1 1999-09-10 21:52:36 eaf Exp $#
*/

//@{

/** #DjVuFileCache# is a simple list of \Ref{DjVuFile} instances. It keeps
    track of the total size of all elements and can get rid of the oldest
    one once the total size becomes over some thresold. */
class DjVuFileCache
{
public:
      /** Constructs the #DjVuFileCache#
	  @param max_size Maximum allowed size of the cache in bytes. */
   DjVuFileCache(int max_size=5*2*1024*1024);
   virtual ~DjVuFileCache(void);

      /** Removes file #file# from the cache */
   void		del_file(const DjVuFile * file);
      /** Adds the given file to the cache. It it's already there, its
	  timestamp will be refreshed. */
   void		add_file(const GP<DjVuFile> & file);
      /** Clears the cache. All items will be deleted. */
   void		clear(void);
      /** Sets new maximum size. If the total size of all items in the cache
	  is greater than #max_size#, the cache will be deleting the oldest
	  items until the size is OK. */
   void		set_max_size(int max_size);
      /** Enables or disables the cache. See \Ref{is_enabled}() for details
	  @param en - If {\em en} is TRUE, the cache will be enabled.
	         Otherwise it will be disabled.
	*/
   void		enable(bool en);
      /** Returns #TRUE# if the cache is enabled, #FALSE# otherwise.
	  When a cache is disabled, \Ref{add_file}(), and
	  \Ref{del_file}() do nothing. But the {\em maximum size} is preserved
	  inside the class so that next time the cache is enabled, it will
	  be configured the same way. Clearly this "enable/disable" thing is
	  for convenience only. One could easily simulate this behaviour by
	  setting the {\em maximum size} of the cache to #ZERO#. */
   bool		is_enabled(void) const;
protected:
      /** This function is called right after the given file has been appended
	  to the list */
   virtual void	file_added(const GP<DjVuFile> & file);
      /** This function is called right after the given file has been removed
	  from the list. */
   virtual void	file_deleted(const GP<DjVuFile> & file);
private:
   class Item : public GPEnabled
   {
   public:
      Item(void);
      Item(const GP<DjVuFile> & xfile);
      virtual ~Item(void);
   
      GP<DjVuFile>	get_file(void) const;
      unsigned int	get_size(void) const;
      time_t		get_time(void) const;
      void		refresh(void);

   public:
      GP<DjVuFile>	file;
      time_t		time;
      GPosition		list_pos;
      static int	qsort_func(const void * el1, const void * el2);
   };
   GCriticalSection	list_lock;
   GPList<Item>		list;
   bool		enabled;
   int		max_size;
   int		cur_size;

   int		calculate_size(void);
   void		clear_to_size(int size);
};

//@}

inline
DjVuFileCache::Item::Item(void) : time(::time(0)) {};

inline
DjVuFileCache::Item::Item(const GP<DjVuFile> & xfile) :
      file(xfile), time(::time(0)) {}

inline
DjVuFileCache::Item::~Item(void) {}

inline GP<DjVuFile>
DjVuFileCache::Item::get_file(void) const
{
   return file;
}

inline unsigned int
DjVuFileCache::Item::get_size(void) const
{
   return file->get_memory_usage();
}

inline time_t
DjVuFileCache::Item::get_time(void) const
{
   return time;
}

inline void
DjVuFileCache::Item::refresh(void)
{
   time=::time(0);
}

inline
DjVuFileCache::DjVuFileCache(int xmax_size) :
      enabled(true), max_size(xmax_size), cur_size(0) {}

inline
DjVuFileCache::~DjVuFileCache(void) {}

inline void
DjVuFileCache::clear(void)
{
   clear_to_size(0);
}

inline bool
DjVuFileCache::is_enabled(void) const
{
   return enabled;
}

#endif
