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
// $Id: DjVuFileCache.h,v 1.9 2000-11-02 01:08:34 bcr Exp $
// $Name:  $


#ifndef _DJVUFILECACHE_H
#define _DJVUFILECACHE_H

#ifdef __GNUC__
#pragma interface
#endif

#include "DjVuFile.h"
#include "Arrays.h"
#include "debug.h"
#if defined(macintosh) //MCW can't compile
#else
#ifndef UNDER_CE
#include <sys/types.h>
#include <time.h>
#endif 

#endif

/** @name DjVuFileCache.h
    Files #"DjVuFileCache.h"# and #"DjVuFileCache.cpp"# implement a simple
    caching mechanism for keeping a given number of \Ref{DjVuFile} instances
    alive. The cache estimates the size of its elements and gets rid of
    the oldest ones when necessary.

    See \Ref{DjVuFileCache} for details.
    
    @memo Simple DjVuFile caching class.
    @author Andrei Erofeev <eaf@geocities.com>
    @version #$Id: DjVuFileCache.h,v 1.9 2000-11-02 01:08:34 bcr Exp $#
*/

//@{

/** #DjVuFileCache# is a simple list of \Ref{DjVuFile} instances. It keeps
    track of the total size of all elements and can get rid of the oldest
    one once the total size becomes over some threshold. Its main purpose
    is to keep the added \Ref{DjVuFile} instances alive until their size
    exceeds some given threshold (set by \Ref{set_maximum_size}() function).
    The user is supposed to use \Ref{DjVuPortcaster::name_to_port}() to
    find a file corresponding to a given name. The cache provides no
    naming services */
#ifdef UNDER_CE
class DjVuFileCache
{
public:
   DjVuFileCache(int) {}
   virtual ~DjVuFileCache(void) {}
   void del_file(const DjVuFile *) {}
   void add_file(const GP<DjVuFile> &) {}
   void clear(void) {}
   void set_max_size(int) {}
   int get_max_size(void) const {return 0;}
   void enable(bool en) {}
   bool is_enabled(void) const {return false;}
} ;
#else
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
      /** Returns the maximum allowed size of the cache. */
   int		get_max_size(void) const;
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
	  for convenience only. One could easily simulate this behavior by
	  setting the {\em maximum size} of the cache to #ZERO#. */
   bool		is_enabled(void) const;
public:
   class Item;
protected:
   GCriticalSection	class_lock;
   
      /** This function is called right after the given file has been added
	  to the cache for management. */
   virtual void	file_added(const GP<DjVuFile> & file);
      /** This function is called when the given file is no longer
	  managed by the cache. */
   virtual void	file_deleted(const GP<DjVuFile> & file);
      /** This function is called when after the cache decides to get rid
	  of the file. */
   virtual void	file_cleared(const GP<DjVuFile> & file);

   GPList<Item>	get_items(void);
private:
   GPList<Item>	list;
   bool		enabled;
   int		max_size;
   int		cur_size;

   int		calculate_size(void);
   void		clear_to_size(int size);
};

class DjVuFileCache::Item : public GPEnabled
{
public:
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

   Item(void);
   Item(const GP<DjVuFile> & xfile);
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

inline int
DjVuFileCache::get_max_size(void) const
{
   return max_size;
}
#endif
#endif
