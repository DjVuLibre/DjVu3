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
//C- $Id: DjVuPort.h,v 1.1.2.2 1999-04-28 19:49:04 eaf Exp $
 
#ifndef _DJVUPORT_H
#define _DJVUPORT_H

#ifdef __GNUC__
#pragma interface
#endif

#include "GException.h"
#include "GURL.h"
#include "GThreads.h"
#include "GContainer.h"
#include "DataPool.h"

/** @name DjVuInterface.h
    @memo DjVu document interface.
    @author Andrei Erofeev
    @version #$Id: DjVuPort.h,v 1.1.2.2 1999-04-28 19:49:04 eaf Exp $#
*/

//@{

/** 
    */

class DjVuPort
{
public:
   DjVuPort(bool dont_add_to_pcaster=0);
   DjVuPort(const DjVuPort & port);
   virtual ~DjVuPort(void);

   DjVuPort & operator=(const DjVuPort & port);
   
      // For example of using inherits() go to DjVuDocument::notify_chunk_done()
   virtual bool		inherits(const char * class_name) const;
   
   virtual GP<DataRange>request_data(const DjVuPort * source, const GURL & url);
   virtual bool		notify_error(const DjVuPort * source, const char * msg);
   virtual bool		notify_status(const DjVuPort * source, const char * msg);
   virtual void		notify_redisplay(const DjVuPort * source);
   virtual void		notify_relayout(const DjVuPort * source);
   virtual void		notify_chunk_done(const DjVuPort * source, const char * name);
   virtual void		notify_file_done(const DjVuPort * source);
   virtual void		notify_file_stopped(const DjVuPort * source);
   virtual void		notify_file_failed(const DjVuPort * source);
   virtual void		notify_decode_progress(const DjVuPort * source, float done);
   virtual void		notify_file_data_received(const DjVuPort * source);
   virtual void		notify_all_data_received(const DjVuPort * source);
};

class DjVuSimplePort : public DjVuPort
{
public:
   virtual bool		inherits(const char * class_name) const;
   
   virtual GP<DataRange>request_data(const DjVuPort * source, const GURL & url);
   virtual bool		notify_error(const DjVuPort * source, const char * msg);
   virtual bool		notify_status(const DjVuPort * source, const char * msg);
};

class DjVuMemoryPort : public DjVuPort
{
public:
   virtual bool		inherits(const char * class_name) const;
   
   virtual GP<DataRange>request_data(const DjVuPort * source, const GURL & url);
   void		add_data(const GURL & url, const GP<DataPool> & pool);
private:
   class Pair : public GPEnabled
   {
   public:
      GURL		url;
      GP<DataPool>	pool;
   };
   GCriticalSection	lock;
   GPList<Pair>		list;
};

class DjVuPortcaster : public DjVuPort
{
public:
   DjVuPortcaster(void);
   virtual ~DjVuPortcaster(void);

   void		add_route(const DjVuPort * src, DjVuPort * dst);
   void		del_route(const DjVuPort * src, DjVuPort * dst);
   void		copy_routes(DjVuPort * dst, const DjVuPort * src);

   void		add_port(const DjVuPort * port);
   void		del_port(const DjVuPort * port);

   bool		is_port_alive(const DjVuPort * port);

   virtual bool		inherits(const char * class_name) const;
   
   virtual GP<DataRange>request_data(const DjVuPort * source, const GURL & url);
   virtual bool		notify_error(const DjVuPort * source, const char * msg);
   virtual bool		notify_status(const DjVuPort * source, const char * msg);
   virtual void		notify_redisplay(const DjVuPort * source);
   virtual void		notify_relayout(const DjVuPort * source);
   virtual void		notify_chunk_done(const DjVuPort * source, const char * name);
   virtual void		notify_file_done(const DjVuPort * source);
   virtual void		notify_file_stopped(const DjVuPort * source);
   virtual void		notify_file_failed(const DjVuPort * source);
   virtual void		notify_decode_progress(const DjVuPort * source, float done);
   virtual void		notify_file_data_received(const DjVuPort * source);
   virtual void		notify_all_data_received(const DjVuPort * source);
private:
      // We use these 'void *' to minimize number of template, which need
      // to be expanded. Ugly, but helps save space.
   GCriticalSection		map_lock;
   GMap<const void *, void *>	route_map;	// GMap<DjVuPort *, GList<DjVuPort *> *>
   GMap<const void *, void *>	cont_map;	// GMap<DjVuPort *, void *>

   void		add_to_closure(GMap<const void *, void *> & set,
			       const DjVuPort * dst, int distance);
   void		compute_closure(GMap<const void *, void *> & set,
				const DjVuPort * src);
   void		sort_closure(const GMap<const void *, void *> & set,
			     GList<const void *> & list);
};

extern DjVuPortcaster * get_portcaster(void);

inline
DjVuPort::DjVuPort(bool dont_add_to_pcaster)
{
   if (!dont_add_to_pcaster) get_portcaster()->add_port(this);
}

inline
DjVuPort::DjVuPort(const DjVuPort & port)
{
   get_portcaster()->copy_routes(this, &port);
}

inline
DjVuPort::~DjVuPort(void)
{
   get_portcaster()->del_port(this);
}

inline DjVuPort &
DjVuPort::operator=(const DjVuPort & port)
{
   if (this!=&port)
      get_portcaster()->copy_routes(this, &port);
   return *this;
}

inline bool
DjVuPort::inherits(const char * class_name) const
{
   return !strcmp("DjVuPort", class_name);
}

inline bool
DjVuSimplePort::inherits(const char * class_name) const
{
   return
      !strcmp("DjVuSimplePort", class_name) ||
      DjVuPort::inherits(class_name);
}

inline bool
DjVuMemoryPort::inherits(const char * class_name) const
{
   return
      !strcmp("DjVuMemoryPort", class_name) ||
      DjVuPort::inherits(class_name);
}

inline bool
DjVuPortcaster::inherits(const char * class_name) const
{
   return
      !strcmp("DjVuPortcaster", class_name) ||
      DjVuPort::inherits(class_name);
}

//@}

#endif
