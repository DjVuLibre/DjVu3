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
//C- $Id: DjVuPort.cpp,v 1.1.2.1 1999-04-12 16:48:22 eaf Exp $

#ifdef __GNUC__
#pragma implementation
#endif

#include "DjVuPort.h"
#include "GOS.h"

static DjVuPortcaster * pcaster;

DjVuPortcaster *
get_portcaster(void)
{
   if (!pcaster) pcaster=new DjVuPortcaster();
   return pcaster;
}

DjVuPortcaster::DjVuPortcaster(void) : DjVuPort(1)
{
}

DjVuPortcaster::~DjVuPortcaster(void)
{
   GCriticalSectionLock lock(&map_lock);
   
   for(GPosition pos=route_map;pos;++pos)
      delete (GList<void *> *) route_map[pos];
}

bool
DjVuPortcaster::is_port_alive(const DjVuPort * port)
{
   GCriticalSectionLock lock(&map_lock);
   return cont_map.contains(port);
}

void
DjVuPortcaster::add_port(const DjVuPort * port)
{
   GCriticalSectionLock lock(&map_lock);
   
   cont_map[port]=0;
}

void
DjVuPortcaster::del_port(const DjVuPort * port)
{
   GCriticalSectionLock lock(&map_lock);

      // Update "contents map"
   if (cont_map.contains(port)) cont_map.del(port);

      // Update "route map"
   if (route_map.contains(port))
   {
      delete (GList<void *> *) route_map[port];
      route_map.del(port);
   };
   for(GPosition pos=route_map;pos;)
   {
      GList<void *> & list=*(GList<void *> *) route_map[pos];
      GPosition list_pos;
      if (list.search((void *) port, list_pos)) list.del(list_pos);
      if (!list.size())
      {
	 delete &list;
	 GPosition tmp_pos=pos;
	 ++pos;
	 route_map.del(tmp_pos);
      } else ++pos;
   };
}

void
DjVuPortcaster::add_route(const DjVuPort * src, DjVuPort * dst)
      // Adds route src->dst
{
   GCriticalSectionLock lock(&map_lock);

   if (cont_map.contains(src) && cont_map.contains(dst))
   {
      if (!route_map.contains(src)) route_map[src]=new GList<void *>();
      GList<void *> & list=*(GList<void *> *) route_map[src];
      if (!list.contains(dst)) list.append(dst);
   }
}

void
DjVuPortcaster::del_route(const DjVuPort * src, DjVuPort * dst)
      // Deletes route src->dst
{
   GCriticalSectionLock lock(&map_lock);
   
   if (route_map.contains(src))
   {
      GList<void *> & list=*(GList<void *> *) route_map[src];
      GPosition pos;
      if (list.search(dst, pos)) list.del(pos);
      if (!list.size())
      {
	 delete &list;
	 route_map.del(src);
      };
   };
}

void
DjVuPortcaster::copy_routes(DjVuPort * dst, const DjVuPort * src)
      // For every route src->x or x->src, it creates a new one:
      // dst->x or x->dst respectively. It's useful when you create a copy
      // of a port and you want the copy to stay connected.
{
   GCriticalSectionLock lock(&map_lock);
   
   if (!cont_map.contains(src) || !cont_map.contains(dst)) return;

   for(GPosition pos=route_map;pos;++pos)
   {
      GList<void *> & list=*(GList<void *> *) route_map[pos];
      if (route_map.key(pos)==src)
	 for(GPosition pos=list;pos;++pos)
	    add_route(dst, (DjVuPort *) list[pos]);
      for(GPosition pos=list;pos;++pos)
	 if (list[pos]==src)
	    add_route((DjVuPort *) route_map.key(pos), dst);
   }
}

void
DjVuPortcaster::add_to_closure(GMap<const void *, void *> & set,
			       const DjVuPort * dst, int distance)
{
      // Assuming that the map's already locked
      // GCriticalSectionLock lock(&map_lock);
   
   set[dst]=(void *) distance;

   if (route_map.contains(dst))
   {
      GList<void *> & list=*(GList<void *> *) route_map[dst];
      for(GPosition pos=list;pos;++pos)
      {
	 DjVuPort * new_dst=(DjVuPort *) list[pos];
	 if (!set.contains(new_dst)) add_to_closure(set, new_dst, distance+1);
      };
   };
}

void
DjVuPortcaster::compute_closure(GMap<const void *, void *> & set,
				const DjVuPort * src)
{
   GCriticalSectionLock lock(&map_lock);
   
   set.empty();
   if (route_map.contains(src))
   {
      GList<void *> & list=*(GList<void *> *) route_map[src];
      for(GPosition pos=list;pos;++pos)
	 add_to_closure(set, (DjVuPort *) list[pos], 0);
   };
}

void
DjVuPortcaster::sort_closure(const GMap<const void *, void *> & set,
			     GList<const void *> & list)
      // Will copy 'set' to 'list' putting elements closest to the source
      // the first (the distance is encoded as map's value)
{
   list.empty();

   int max_dist=0;
   GPosition pos;
   for(pos=set;pos;++pos)
      if (max_dist<(int) set[pos])
	 max_dist=(int) set[pos];

      // Not using GArray<> to avoid instantiation of another template
   GList<const void *> * lists=new GList<const void *>[max_dist+1];
   TRY {
      for(pos=set;pos;++pos)
	 lists[(int) set[pos]].append(set.key(pos));

      for(int dist=0;dist<=max_dist;dist++)
	 for(pos=lists[dist];pos;++pos)
	    list.append(lists[dist][pos]);
      
      delete lists; lists=0;
   } CATCH(exc) {
      delete lists; lists=0;
      RETHROW;
   } ENDCATCH;
}

GP<DataRange>
DjVuPortcaster::request_data(const DjVuPort * source, const GURL & url)
{
   GMap<const void *, void *> set;
   GList<const void *> list;
   compute_closure(set, source);
   sort_closure(set, list);

   GP<DataRange> data;
   for(GPosition pos=list;pos;++pos)
   {
      DjVuPort * port=(DjVuPort *) list[pos];
      if (is_port_alive(port))
	 if ((data=port->request_data(source, url)))
	    break;
   }
   return data;
}

bool
DjVuPortcaster::notify_error(const DjVuPort * source, const char * msg)
{
   GMap<const void *, void *> set;
   GList<const void *> list;
   compute_closure(set, source);
   sort_closure(set, list);

   for(GPosition pos=list;pos;++pos)
   {
      DjVuPort * port=(DjVuPort *) list[pos];
      if (is_port_alive(port))
	 if (port->notify_error(source, msg))
	    return 1;
   }
   return 0;
}

bool
DjVuPortcaster::notify_status(const DjVuPort * source, const char * msg)
{
   GMap<const void *, void *> set;
   GList<const void *> list;
   compute_closure(set, source);
   sort_closure(set, list);

   for(GPosition pos=list;pos;++pos)
   {
      DjVuPort * port=(DjVuPort *) list[pos];
      if (is_port_alive(port))
	 if (port->notify_status(source, msg))
	    return 1;
   }
   return 0;
}

void
DjVuPortcaster::notify_redisplay(const DjVuPort * source)
{
   GMap<const void *, void *> set;
   compute_closure(set, source);

   for(GPosition pos=set;pos;++pos)
   {
      DjVuPort * port=(DjVuPort *) set.key(pos);
      if (is_port_alive(port))
	 port->notify_redisplay(source);
   }
}

void
DjVuPortcaster::notify_relayout(const DjVuPort * source)
{
   GMap<const void *, void *> set;
   compute_closure(set, source);

   for(GPosition pos=set;pos;++pos)
   {
      DjVuPort * port=(DjVuPort *) set.key(pos);
      if (is_port_alive(port))
	 port->notify_relayout(source);
   }
}

void
DjVuPortcaster::notify_chunk_done(const DjVuPort * source, const char * name)
{
   GMap<const void *, void *> set;
   compute_closure(set, source);

   for(GPosition pos=set;pos;++pos)
   {
      DjVuPort * port=(DjVuPort *) set.key(pos);
      if (is_port_alive(port))
	 port->notify_chunk_done(source, name);
   }
}

void
DjVuPortcaster::notify_file_done(const DjVuPort * source)
{
   GMap<const void *, void *> set;
   compute_closure(set, source);

   for(GPosition pos=set;pos;++pos)
   {
      DjVuPort * port=(DjVuPort *) set.key(pos);
      if (is_port_alive(port))
	 port->notify_file_done(source);
   }
}

void
DjVuPortcaster::notify_file_stopped(const DjVuPort * source)
{
   GMap<const void *, void *> set;
   compute_closure(set, source);

   for(GPosition pos=set;pos;++pos)
   {
      DjVuPort * port=(DjVuPort *) set.key(pos);
      if (is_port_alive(port))
	 port->notify_file_stopped(source);
   }
}

void
DjVuPortcaster::notify_file_failed(const DjVuPort * source)
{
   GMap<const void *, void *> set;
   compute_closure(set, source);

   for(GPosition pos=set;pos;++pos)
   {
      DjVuPort * port=(DjVuPort *) set.key(pos);
      if (is_port_alive(port))
	 port->notify_file_failed(source);
   }
}

void
DjVuPortcaster::notify_decode_progress(const DjVuPort * source, float done)
{
   GMap<const void *, void *> set;
   compute_closure(set, source);

   for(GPosition pos=set;pos;++pos)
   {
      DjVuPort * port=(DjVuPort *) set.key(pos);
      if (is_port_alive(port))
	 port->notify_decode_progress(source, done);
   }
}

void
DjVuPortcaster::notify_file_data_received(const DjVuPort * source)
{
   GMap<const void *, void *> set;
   compute_closure(set, source);

   for(GPosition pos=set;pos;++pos)
   {
      DjVuPort * port=(DjVuPort *) set.key(pos);
      if (is_port_alive(port))
	 port->notify_file_data_received(source);
   }
}

void
DjVuPortcaster::notify_all_data_received(const DjVuPort * source)
{
   GMap<const void *, void *> set;
   compute_closure(set, source);

   for(GPosition pos=set;pos;++pos)
   {
      DjVuPort * port=(DjVuPort *) set.key(pos);
      if (is_port_alive(port))
	 port->notify_all_data_received(source);
   }
}

//****************************************************************************
//******************************* DjVuPort ***********************************
//****************************************************************************

GP<DataRange>
DjVuPort::request_data(const DjVuPort *, const GURL &) { return 0; }

bool
DjVuPort::notify_error(const DjVuPort *, const char *) { return 0; }

bool
DjVuPort::notify_status(const DjVuPort *, const char *) { return 0; }

void
DjVuPort::notify_redisplay(const DjVuPort *) {}

void
DjVuPort::notify_relayout(const DjVuPort *) {}

void
DjVuPort::notify_chunk_done(const DjVuPort *, const char *) {}

void
DjVuPort::notify_file_done(const DjVuPort *) {}

void
DjVuPort::notify_file_stopped(const DjVuPort *) {}

void
DjVuPort::notify_file_failed(const DjVuPort *) {}

void
DjVuPort::notify_decode_progress(const DjVuPort *, float) {}

void
DjVuPort::notify_file_data_received(const DjVuPort *) {}

void
DjVuPort::notify_all_data_received(const DjVuPort *) {}

//****************************************************************************
//*************************** DjVuSimplePort *********************************
//****************************************************************************

GP<DataRange>
DjVuSimplePort::request_data(const DjVuPort * source, const GURL & url)
{
   StdioByteStream str(GOS::url_to_filename(url), "r");
   GP<DataPool> pool=new DataPool();

   char buffer[1024];
   int length;
   while((length=str.read(buffer, 1024)))
      pool->add_data(buffer, length);
   
   pool->set_eof();
   return new DataRange(pool);
}

bool
DjVuSimplePort::notify_error(const DjVuPort * source, const char * msg)
{
   fprintf(stderr, "%s\n", msg);
   return 1;
}

bool
DjVuSimplePort::notify_status(const DjVuPort * source, const char * msg)
{
   fprintf(stderr, "%s\n", msg);
   return 1;
}
