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
//C- $Id: DjVuPort.cpp,v 1.25 1999-11-20 07:55:32 bcr Exp $

#ifdef __GNUC__
#pragma implementation
#endif

#include "DjVuPort.h"
#include "GOS.h"
#include "DjVuImage.h"
#include "DjVuDocument.h"
#include "DjVuFile.h"


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

DjVuPort::~DjVuPort(void)
{
  get_portcaster()->del_port(this);
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
  if (pos && cont_map[pos])
    return GP<DjVuPort>((DjVuPort*)port);
  return 0;
}

void
DjVuPortcaster::add_alias(const DjVuPort * port, const char * alias)
{
   GCriticalSectionLock lock(&map_lock);
   a2p_map[alias]=port;
}

void
DjVuPortcaster::clear_aliases(const DjVuPort * port)
{
   GCriticalSectionLock lock(&map_lock);
   for(GPosition pos=a2p_map;pos;)
      if (a2p_map[pos]==port)
      {
	 GPosition this_pos=pos;
	 ++pos;
	 a2p_map.del(this_pos);
      } else ++pos;
}

GP<DjVuPort>
DjVuPortcaster::alias_to_port(const char * alias)
{
   GCriticalSectionLock lock(&map_lock);
   GPosition pos;
   if (a2p_map.contains(alias, pos))
   {
      DjVuPort * port=(DjVuPort *) a2p_map[pos];
      if (is_port_alive(port)) return port;
      else a2p_map.del(pos);
   }
   return 0;
}

GPList<DjVuPort>
DjVuPortcaster::prefix_to_ports(const char * prefix)
{
   GPList<DjVuPort> list;
   if (prefix)
   {
      int length=strlen(prefix);
      if (length)
      {
	 GCriticalSectionLock lock(&map_lock);
	 for(GPosition pos=a2p_map;pos;++pos)
	    if (!strncmp(prefix, a2p_map.key(pos), length))
	    {
	       GP<DjVuPort> port=(DjVuPort *) a2p_map[pos];
	       if (port) list.append(port);
	    }
      }
   }
   return list;
}

void
DjVuPortcaster::del_port(const DjVuPort * port)
{
   GCriticalSectionLock lock(&map_lock);

   GPosition pos;
   
      // Update "contents map"
   if (cont_map.contains(port, pos)) cont_map.del(pos);

      // Update "route map"
   if (route_map.contains(port, pos))
   {
      delete (GList<void *> *) route_map[pos];
      route_map.del(pos);
   }
   for(pos=route_map;pos;)
   {
//bcr: Mapping GMap to GList at the binary level.  This is CRAZY.

      GList<void *> & list=*(GList<void *> *) route_map[pos];
      GPosition list_pos;
      if (list.search((void *) port, list_pos)) list.del(list_pos);
      if (!list.size())
      {

//bcr: This is probably were the error really occures.  But this
//bcr: is so cryptic, since all the operators are overloaded both
//bcr: here and on the declaration line above.  The value is being
//bcr: deleted, while the list itself still exists.  So later when
//bcr: we try to access it, BOOM!  
	 delete &list;
//bcr: Assuming that is the problem, something like:	&list=0;
//bcr: will fix the problem.
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
           {
             GP<DjVuPort> p = (DjVuPort*) lists[dist][pos];
             if (p) list.append(p);
           }
     }
   else
     {
       // Gather ports without order
       for(pos=set;pos;++pos)
         {
           GP<DjVuPort> p = (DjVuPort*) set.key(pos);
           if (p) list.append(p);
         }
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
      url=list[pos]->id_to_url(source, id);
      if (!url.is_empty()) break;
   }
   return url;
}

GPBase
DjVuPortcaster::id_to_file(const DjVuPort * source, const char * id)
{
   GPList<DjVuPort> list;
   compute_closure(source, list, true);
   GPBase file;
   for(GPosition pos=list;pos;++pos)
      if ((file=list[pos]->id_to_file(source, id)).get()) break;
   return file;
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
DjVuPortcaster::notify_redisplay(const DjVuImage * source)
{
   GPList<DjVuPort> list;
   compute_closure(source, list);
   for(GPosition pos=list; pos; ++pos)
     list[pos]->notify_redisplay(source);
}

void
DjVuPortcaster::notify_relayout(const DjVuImage * source)
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
DjVuPortcaster::notify_file_flags_changed(const DjVuFile * source,
					  long set_mask, long clr_mask)
{
   GPList<DjVuPort> list;
   compute_closure(source, list);
   for(GPosition pos=list; pos; ++pos)
     list[pos]->notify_file_flags_changed(source, set_mask, clr_mask);
}

void
DjVuPortcaster::notify_doc_flags_changed(const DjVuDocument * source,
					 long set_mask, long clr_mask)
{
   GPList<DjVuPort> list;
   compute_closure(source, list);
   for(GPosition pos=list; pos; ++pos)
     list[pos]->notify_doc_flags_changed(source, set_mask, clr_mask);
}

void
DjVuPortcaster::notify_decode_progress(const DjVuPort * source, float done)
{
   GPList<DjVuPort> list;
   compute_closure(source, list);
   for(GPosition pos=list; pos; ++pos)
     list[pos]->notify_decode_progress(source, done);
}

//****************************************************************************
//******************************* DjVuPort ***********************************
//****************************************************************************

GURL
DjVuPort::id_to_url(const DjVuPort *, const char *) { return GURL(); }

GPBase
DjVuPort::id_to_file(const DjVuPort *, const char *) { return 0; }

GP<DataPool>
DjVuPort::request_data(const DjVuPort *, const GURL &) { return 0; }

bool
DjVuPort::notify_error(const DjVuPort *, const char *) { return 0; }

bool
DjVuPort::notify_status(const DjVuPort *, const char *) { return 0; }

void
DjVuPort::notify_redisplay(const DjVuImage *) {}

void
DjVuPort::notify_relayout(const DjVuImage *) {}

void
DjVuPort::notify_chunk_done(const DjVuPort *, const char *) {}

void
DjVuPort::notify_file_flags_changed(const DjVuFile *, long, long) {}

void
DjVuPort::notify_doc_flags_changed(const DjVuDocument *, long, long) {}

void
DjVuPort::notify_decode_progress(const DjVuPort *, float) {}

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
	 return new DataPool(fname);
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
