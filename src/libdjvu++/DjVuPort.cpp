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
//C- $Id: DjVuPort.cpp,v 1.13 1999-09-07 15:59:53 leonb Exp $

#ifdef __GNUC__
#pragma implementation
#endif

#include "DjVuPort.h"
#include "GOS.h"



//****************************************************************************
//******************************* Globals ************************************
//****************************************************************************


static DjVuPortcaster *pcaster;

DjVuPortcaster *
DjVuPort::get_portcaster(void)
{
   if (!pcaster) pcaster = new DjVuPortcaster();
   return pcaster;
}


//****************************************************************************
//******************************* DjVuPort ***********************************
//****************************************************************************

void *
DjVuPort::operator new (size_t sz)
{
  void *addr = ::operator new (sz);
  DjVuPortcaster *pcaster = get_portcaster();
  GCriticalSectionLock lock(& pcaster->map_lock );
  pcaster->cont_map[addr] = 0;
  return addr;
}


DjVuPort::DjVuPort()
{
  DjVuPortcaster *pcaster = get_portcaster();
  GCriticalSectionLock lock(& pcaster->map_lock );
  GPosition p = pcaster->cont_map.contains(this);
  if (!p) THROW("DjVuPort was not allocated on the heap");
  pcaster->cont_map[p] = (void*)this;
}

DjVuPort::DjVuPort(const DjVuPort & port)
{
  DjVuPortcaster *pcaster = get_portcaster();
  GCriticalSectionLock lock(& pcaster->map_lock );
  GPosition p = pcaster->cont_map.contains(this);
  if (!p) THROW("DjVuPort was not allocated on the heap");
  pcaster->cont_map[p] = (void*)this;
  pcaster->copy_routes(this, &port);
}

DjVuPort &
DjVuPort::operator=(const DjVuPort & port)
{
   if (this != &port)
      get_portcaster()->copy_routes(this, &port);
   return *this;
}

void
DjVuPort::destroy(void)
{
  int ok_to_destroy = 0;
  {
    GCriticalSectionLock lock(& pcaster->map_lock );
    // Avoid destroying when count is not zero.  This can happens if
    // is_port_alive() has been called by another thread between the time the
    // count was decremented to zero and the time destroy is called.
    if (get_count() == 0)
      {
        get_portcaster()->del_port(this);
        ok_to_destroy = 1;
      }
  }
  if (ok_to_destroy)
    GPEnabled::destroy();
}


//****************************************************************************
//**************************** DjVuPortcaster ********************************
//****************************************************************************



DjVuPortcaster::DjVuPortcaster(void)
{
}

DjVuPortcaster::~DjVuPortcaster(void)
{
   GCriticalSectionLock lock(&map_lock);
   for(GPosition pos=route_map;pos;++pos)
      delete (GList<void *> *) route_map[pos];
}

GP<DjVuPort>
DjVuPortcaster::is_port_alive(DjVuPort *port)
{
   GCriticalSectionLock lock(&map_lock);
   GPosition pos = cont_map.contains(port);
   if (pos && cont_map[pos] == port)
     return GP<DjVuPort>( (DjVuPort*) port );
   return 0;
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
   }
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
   }
}

void
DjVuPortcaster::add_route(const DjVuPort * src, DjVuPort * dst)
      // Adds route src->dst
{
   GCriticalSectionLock lock(&map_lock);
   if (!src->get_count())
     THROW("Source port is not secured by a smart pointer.");
   if (!dst->get_count())
     THROW("Destination port is not secured by a smart pointer.");
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
      }
   }
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
      if (route_map.key(pos) == src)
	 for(GPosition pos=list;pos;++pos)
	    add_route(dst, (DjVuPort *) list[pos]);
      for(GPosition pos=list;pos;++pos)
	 if ((DjVuPort*)(list[pos]) == src)
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
          if (!set.contains(new_dst)) 
            add_to_closure(set, new_dst, distance+1);
        }
   }
}

void
DjVuPortcaster::compute_closure(const DjVuPort * src, GPList<DjVuPort> &list, bool sorted)
{
   GCriticalSectionLock lock(&map_lock);
   GMap<const void*, void*> set;
   if (route_map.contains(src))
   {
      GList<void *> & list=*(GList<void *> *) route_map[src];
      for(GPosition pos=list;pos;++pos)
      {
	 DjVuPort * dst=(DjVuPort *) list[pos];
	 if (dst==src) add_to_closure(set, src, 0);
	 else add_to_closure(set, dst, 1);
      }
   }
   // Compute list
   GPosition pos;
   if (sorted)
     {
       // Sort in depth order
       int max_dist=0;
       for(pos=set;pos;++pos)
         if (max_dist < (int) set[pos])
           max_dist = (int) set[pos];
       GArray<GList<const void*> > lists(0,max_dist);
       for(pos=set;pos;++pos)
         lists[(int) set[pos]].append(set.key(pos));
       for(int dist=0;dist<=max_dist;dist++)
         for(pos=lists[dist];pos;++pos)
           list.append( (DjVuPort*) lists[dist][pos] );
     }
   else
     {
       // Gather ports without order
       for(pos=set;pos;++pos)
         list.append( (DjVuPort*) set.key(pos) );
     }
}

GURL
DjVuPortcaster::id_to_url(const DjVuPort * source, const char * id)
{
   GPList<DjVuPort> list;
   compute_closure(source, list, true);
   GURL url;
   for(GPosition pos=list;pos;++pos)
     {
       url = list[pos]->id_to_url(source, id);
       if (!url.is_empty()) break;
     }
   return url;
}

GPBase
DjVuPortcaster::get_cached_file(const DjVuPort * source, const GURL & url)
{
  // NOTE: We traverse the list from the end!
   GPList<DjVuPort> list;
   compute_closure(source, list, true);
   GPBase file;
   for(GPosition pos=list;pos;++pos)
     if ((file = list[pos]->get_cached_file(source, url)).get())
       break;
   return file;
}

void
DjVuPortcaster::cache_djvu_file(const DjVuPort * source, class DjVuFile * file)
{
   GPList<DjVuPort> list;
   compute_closure(source, list);
   for(GPosition pos=list; pos; ++pos)
     list[pos]->cache_djvu_file(source, file);
}

GP<DataPool>
DjVuPortcaster::request_data(const DjVuPort * source, const GURL & url)
{
   GPList<DjVuPort> list;
   compute_closure(source, list, true);
   GP<DataPool> data;
   for(GPosition pos=list;pos;++pos)
     if ((data = list[pos]->request_data(source, url)))
       break;
   return data;
}

bool
DjVuPortcaster::notify_error(const DjVuPort * source, const char * msg)
{
   GPList<DjVuPort> list;
   compute_closure(source, list, true);
   for(GPosition pos=list;pos;++pos)
     if (list[pos]->notify_error(source, msg))
       return 1;
   return 0;
}

bool
DjVuPortcaster::notify_status(const DjVuPort * source, const char * msg)
{
   GPList<DjVuPort> list;
   compute_closure(source, list, true);
   for(GPosition pos=list;pos;++pos)
     if (list[pos]->notify_status(source, msg))
       return 1;
   return 0;
}

void
DjVuPortcaster::notify_redisplay(const DjVuPort * source)
{
   GPList<DjVuPort> list;
   compute_closure(source, list);
   for(GPosition pos=list; pos; ++pos)
     list[pos]->notify_redisplay(source);
}

void
DjVuPortcaster::notify_relayout(const DjVuPort * source)
{
   GPList<DjVuPort> list;
   compute_closure(source, list);
   for(GPosition pos=list; pos; ++pos)
     list[pos]->notify_relayout(source);
}

void
DjVuPortcaster::notify_chunk_done(const DjVuPort * source, const char * name)
{
   GPList<DjVuPort> list;
   compute_closure(source, list);
   for(GPosition pos=list; pos; ++pos)
     list[pos]->notify_chunk_done(source, name);
}

void
DjVuPortcaster::notify_file_done(const DjVuPort * source)
{
   GPList<DjVuPort> list;
   compute_closure(source, list);
   for(GPosition pos=list; pos; ++pos)
     list[pos]->notify_file_done(source);
}

void
DjVuPortcaster::notify_file_stopped(const DjVuPort * source)
{
   GPList<DjVuPort> list;
   compute_closure(source, list);
   for(GPosition pos=list; pos; ++pos)
     list[pos]->notify_file_stopped(source);
}

void
DjVuPortcaster::notify_file_failed(const DjVuPort * source)
{
   GPList<DjVuPort> list;
   compute_closure(source, list);
   for(GPosition pos=list; pos; ++pos)
     list[pos]->notify_file_failed(source);
}

void
DjVuPortcaster::notify_decode_progress(const DjVuPort * source, float done)
{
   GPList<DjVuPort> list;
   compute_closure(source, list);
   for(GPosition pos=list; pos; ++pos)
     list[pos]->notify_decode_progress(source, done);
}

void
DjVuPortcaster::notify_file_data_received(const DjVuPort * source)
{
   GPList<DjVuPort> list;
   compute_closure(source, list);
   for(GPosition pos=list; pos; ++pos)
     list[pos]->notify_file_data_received(source);
}

void
DjVuPortcaster::notify_all_data_received(const DjVuPort * source)
{
   GPList<DjVuPort> list;
   compute_closure(source, list);
   for(GPosition pos=list; pos; ++pos)
     list[pos]->notify_all_data_received(source);
}

//****************************************************************************
//******************************* DjVuPort ***********************************
//****************************************************************************

GURL
DjVuPort::id_to_url(const DjVuPort *, const char *) { return GURL(); }

GPBase
DjVuPort::get_cached_file(const DjVuPort *, const GURL &) { return 0; }

void
DjVuPort::cache_djvu_file(const DjVuPort *, class DjVuFile *) {}

GP<DataPool>
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


GP<DataPool>
DjVuSimplePort::request_data(const DjVuPort * source, const GURL & url)
{
   TRY {
      if (url.is_local_file_url())
      {
	 GString fname=GOS::url_to_filename(url);
	 if (GOS::basename(fname)=="-") fname="-";

	 GP<DataPool> pool=new DataPool();
	 StdioByteStream str(fname, "rb");
	 char buffer[1024];
	 int length;
	 while((length=str.read(buffer, 1024)))
	    pool->add_data(buffer, length);
	 pool->set_eof();
	 return pool;
      }
   } CATCH(exc) {
   } ENDCATCH;
   return 0;
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





//****************************************************************************
//*************************** DjVuMemoryPort *********************************
//****************************************************************************



GP<DataPool>
DjVuMemoryPort::request_data(const DjVuPort * source, const GURL & url)
{
  GCriticalSectionLock lk(&lock);
   for(GPosition pos=list;pos;++pos)
      if (list[pos]->url==url)
	 return list[pos]->pool;
   return 0;
}

void
DjVuMemoryPort::add_data(const GURL & url, const GP<DataPool> & pool)
{
   GP<Pair> pair;
   GCriticalSectionLock lk(&lock);
   for(GPosition pos=list;pos;++pos)
      if (list[pos]->url==url)
      {
	 pair=list[pos];
	 break;
      }
   if (!pair)
   {
      pair=new Pair();
      list.append(pair);
   }
   pair->pool=pool;
}
