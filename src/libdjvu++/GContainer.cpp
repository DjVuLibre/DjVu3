//C-  -*- C++ -*-
//C- DjVu� Reference Library (v. 3.0)
//C- 
//C- Copyright � 1999-2001 LizardTech, Inc. All Rights Reserved.
//C- The DjVu Reference Library is protected by U.S. Pat. No.
//C- 6,058,214 and patents pending.
//C- 
//C- This software is subject to, and may be distributed under, the
//C- GNU General Public License, Version 2. The license should have
//C- accompanied the software or you may obtain a copy of the license
//C- from the Free Software Foundation at http://www.fsf.org .
//C- 
//C- The computer code originally released by LizardTech under this
//C- license and unmodified by other parties is deemed the "LizardTech
//C- Original Code."
//C- 
//C- With respect to the LizardTech Original Code ONLY, and subject
//C- to any third party intellectual property claims, LizardTech
//C- grants recipient a worldwide, royalty-free, non-exclusive license
//C- under patent claims now or hereafter owned or controlled by
//C- LizardTech that are infringed by making, using, or selling
//C- LizardTech Original Code, but solely to the extent that any such
//C- patent(s) is/are reasonably necessary to enable you to make, have
//C- made, practice, sell, or otherwise dispose of LizardTech Original
//C- Code (or portions thereof) and not to any greater extent that may
//C- be necessary to utilize further modifications or combinations.
//C- 
//C- The LizardTech Original Code is provided "AS IS" WITHOUT WARRANTY
//C- OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED
//C- TO ANY WARRANTY OF NON-INFRINGEMENT, OR ANY IMPLIED WARRANTY OF
//C- MERCHANTIBILITY OR FITNESS FOR A PARTICULAR PURPOSE.
// 
// $Id: GContainer.cpp,v 1.31 2001-06-13 18:26:19 bcr Exp $
// $Name:  $

#ifdef __GNUC__
#pragma implementation
#endif

#include "GContainer.h"

// ------------------------------------------------------------
// DYNAMIC ARRAYS
// ------------------------------------------------------------


GArrayBase::GArrayBase(const GArrayBase &ref)
  : traits(ref.traits),
    gdata(data,0,1),
    minlo(ref.minlo), maxhi(ref.maxhi),
    lobound(ref.lobound), hibound(ref.hibound)
{
  if (maxhi >= minlo)
    gdata.resize(traits.size * (maxhi - minlo + 1),1);
  if (hibound >= lobound)
    traits.copy(traits.lea(data, lobound-minlo), 
                traits.lea(ref.data, lobound-minlo),
                hibound - lobound + 1, 0);
}


GArrayBase::GArrayBase(const GCONT Traits &traits)
  : traits(traits),
    gdata(data,0,1),
    minlo(0), maxhi(-1),
    lobound(0), hibound(-1)
{
}


GArrayBase::GArrayBase(const GCONT Traits &traits, int lobound, int hibound)
  : traits(traits),
    gdata(data,0,1),
    minlo(0), maxhi(-1),
    lobound(0), hibound(-1)
{
  resize(lobound, hibound);
}


GArrayBase::~GArrayBase()
{
  G_TRY { empty(); } G_CATCH_ALL { } G_ENDCATCH;
}


GArrayBase &
GArrayBase::operator= (const GArrayBase &ga)
{
  if (this == &ga)
    return *this;
  empty();
  if (ga.hibound >= ga.lobound)
    {
      resize(ga.lobound, ga.hibound);
      traits.copy( traits.lea(data, lobound-minlo),
                   traits.lea(ga.data, ga.lobound-ga.minlo),
                   hibound - lobound + 1, 0 );
    }
  return *this;
}


void
GArrayBase::steal(GArrayBase &ga)
{
  if (this != &ga)
    {
      empty();
      lobound = ga.lobound;
      hibound = ga.hibound;
      minlo = ga.minlo;
      maxhi = ga.maxhi;
      data = ga.data;
      ga.data = 0;
      ga.lobound = ga.minlo = 0;
      ga.hibound = ga.maxhi = -1;
    }
}


void 
GArrayBase::empty()
{
  resize(0, -1);
}


void 
GArrayBase::touch(int n)
{
  int nlo = (n<lobound ? n : lobound);
  int nhi = (n>hibound ? n : hibound);
  if (hibound < lobound)
    nlo = nhi = n;
  resize(nlo, nhi);
}


void 
GArrayBase::resize(int lo, int hi)
{
  // Validation
  int nsize = hi - lo + 1;
  if (nsize < 0)
    G_THROW( ERR_MSG("GContainer.bad_args") );
  // Destruction
  if (nsize == 0)
    {
      if (hibound >= lobound)
        traits.fini( traits.lea(data, lobound-minlo), hibound-lobound+1 );
      if (data)
        gdata.resize(0,1);
      lobound = minlo = 0;
      hibound = maxhi = -1;
      return;
    }
  // Simple extension
  if (lo >= minlo && hi <= maxhi)
    {
      if (lobound > lo)
        traits.init( traits.lea(data,lo-minlo), lobound-lo );
      else if (lo > lobound)
        traits.fini( traits.lea(data,lobound-minlo), lo-lobound );
      if (hi > hibound)
        traits.init( traits.lea(data,hibound-minlo+1), hi-hibound );
      else if (hibound > hi)
        traits.fini( traits.lea(data,hi-minlo+1), hibound-hi );        
      lobound = lo;
      hibound = hi;
      return;
    }
  // General case
  int nminlo = minlo;
  int nmaxhi = maxhi;
  if (nminlo > nmaxhi)
    nminlo = nmaxhi = lo;
  while (nminlo > lo) {
    int incr = nmaxhi - nminlo;
    nminlo -= (incr < 8 ? 8 : (incr > 32768 ? 32768 : incr));
  }
  while (nmaxhi < hi) {
    int incr = nmaxhi - nminlo;
    nmaxhi += (incr < 8 ? 8 : (incr > 32768 ? 32768 : incr));
  }
  // allocate and move
  int beg = lo;
  int end = hi;
  int bytesize = traits.size * (nmaxhi-nminlo+1);
  void *ndata;
  GPBufferBase gndata(ndata,bytesize,1);
#if GCONTAINER_ZERO_FILL
  memset(ndata, 0, bytesize);  // slower but cleaner
#endif
  if (lo < lobound)
    { traits.init( traits.lea(ndata,lo-nminlo), lobound-lo ); beg=lobound; }
  else if (lobound < lo)
    { traits.fini( traits.lea(data,lobound-minlo), lo-lobound); }
  if (hibound < hi)
    { traits.init( traits.lea(ndata,hibound-nminlo+1), hi-hibound ); end=hibound; }
  else if (hi < hibound)
    { traits.fini( traits.lea(data, hi-minlo+1), hibound-hi ); }
  if (end >= beg)
    { traits.copy( traits.lea(ndata, beg-nminlo), 
                   traits.lea(data, beg-minlo),
                   end-beg+1, 1 ); }
  // free and replace
  void *tmp=data;
  data=ndata;
  ndata=tmp;
  minlo = nminlo;
  maxhi = nmaxhi;
  lobound = lo;
  hibound = hi;
}


void 
GArrayBase::shift(int disp)
{
  lobound += disp;
  hibound += disp;
  minlo += disp;
  maxhi += disp;
}


void 
GArrayBase::del(int n, int howmany)
{
  if (howmany < 0)
    G_THROW( ERR_MSG("GContainer.bad_howmany") );
  if (howmany == 0)
    return;
  if ( n < lobound || n+(int)howmany-1 > hibound)
    G_THROW( ERR_MSG("GContainer.bad_sub2") );
  traits.fini( traits.lea(data, n-minlo), howmany );
  if ( n+howmany-1 < hibound)
    traits.copy( traits.lea(data, n-minlo),
                 traits.lea(data, n-minlo+howmany),
                 hibound - (n+howmany-1), 1 );
  hibound = hibound - howmany;
}


static inline void *
nextptr(void *p, int elsize)
{
  return (void*)(((char*)p) + elsize);
}


static inline void *
prevptr(void *p, int elsize)
{
  return (void*)(((char*)p) - elsize);  
}


void 
GArrayBase::ins(int n, const void *src, int howmany)
{
  if (howmany < 0)
    G_THROW( ERR_MSG("GContainer.bad_howmany") );
  if (howmany == 0)
    return;
  // Make enough room
  if (hibound+howmany > maxhi)
    {
      int nmaxhi = maxhi;
      while (nmaxhi < hibound+howmany)
        nmaxhi += (nmaxhi < 8 ? 8 : (nmaxhi > 32768 ? 32768 : nmaxhi));
      int bytesize = traits.size * (nmaxhi-minlo+1);
      void *ndata = operator new (bytesize);
      GPBufferBase gndata(ndata,bytesize,1);
      memset(ndata, 0, bytesize);  // slower but cleaner
      if (hibound >= lobound)
        traits.copy( traits.lea(ndata, lobound-minlo),
                     traits.lea(data, lobound-minlo),
                     hibound-lobound+1, 1 );
      maxhi = nmaxhi;
      void *tmp=data;
      data = ndata;
      ndata=tmp;
    }
  // Shift data
  int elsize = traits.size;
  void *pdst = traits.lea(data, hibound+howmany-minlo);
  void *psrc = traits.lea(data, hibound-minlo);
  void *pend = traits.lea(data, n-minlo);
  while ((char*)psrc >= (char*)pend)
    {
      traits.copy( pdst, psrc, 1, 1 );
      pdst = prevptr(pdst, elsize);
      psrc = prevptr(psrc, elsize);
    }
  hibound += howmany;
  // Initialize new data
  if (! src)
    {
      traits.init( traits.lea(data, n-minlo), howmany );
      hibound += howmany;
      return;
    }
  // Initialize new data with copy constructor
  pdst = traits.lea(data, n-minlo);
  pend = traits.lea(data, n+howmany-minlo);
  while ((char*)pdst < (char*)pend)
    {
      traits.copy( pdst, src, 1, 0);
      pdst = nextptr(pdst, elsize);
    }
}



// ------------------------------------------------------------
// GPOSITION
// ------------------------------------------------------------





// ------------------------------------------------------------
// DOUBLY LINKED LISTS
// ------------------------------------------------------------


GListBase::GListBase(const Traits& traits)
  : traits(traits)
{
  nelem = 0;
  head.next = head.prev = 0;
}


GListBase::GListBase(const GListBase &ref)
  : traits(ref.traits)
{
  nelem = 0;
  head.next = head.prev = 0;
  GListBase::operator= (ref);
}

#include <stdio.h>
GListBase::~GListBase()
{
  G_TRY
  {
    empty();
  }
  G_CATCH_ALL
  {
    DjVuPrintErrorUTF8("%s","Exception\n");
  }
  G_ENDCATCH;
}


void 
GListBase::append(Node *n)
{
  // Link
  n->next = 0;
  n->prev = head.prev;
  head.prev = n;
  if (n->prev)
    n->prev->next = n;
  else
    head.next = n;
  // Finish
  nelem += 1;
}


void 
GListBase::prepend(Node *n)
{
  // Link
  n->next = head.next;
  n->prev = 0;
  head.next = n;
  if (n->next)
    n->next->prev = n;
  else
    head.prev = n;
  // Finish
  nelem += 1;
}


void 
GListBase::insert_after(GPosition pos, Node *n)
{
  // Prepare
  if (pos.ptr)
    {
      if (pos.cont != (void*)this)
        pos.throw_invalid((void*)this);
      Node *p = pos.ptr;
      n->prev = p;
      n->next = p->next;
    }
  else
    {
      n->prev = 0;
      n->next = head.next;
    }
  // Link
  if (n->prev)
    n->prev->next = n;
  else
    head.next = n;
  if (n->next)
    n->next->prev = n;
  else
    head.prev = n;
  // Finish
  nelem += 1;
}


void 
GListBase::insert_before(GPosition pos, Node *n)
{
  // Prepare
  if (pos.ptr)
    {
      if (pos.cont != (void*)this)
        pos.throw_invalid((void*)this);
      Node *p = pos.ptr;
      n->prev = p->prev;
      n->next = p;
    }
  else
    {
      n->prev = head.prev;
      n->next = 0;
    }
  // Link
  if (n->prev)
    n->prev->next = n;
  else
    head.next = n;
  if (n->next)
    n->next->prev = n;
  else
    head.prev = n;
  // Finish
  nelem += 1;
}


void
GListBase::insert_before(GPosition pos, GListBase &fromlist, GPosition &frompos)
{
  // Check
  if (!frompos.ptr || frompos.cont != (void*)&fromlist)
    frompos.throw_invalid((void*)&fromlist);
  if (pos.ptr && pos.cont != (void*)this)
    pos.throw_invalid((void*)this);
  // Update frompos
  Node *n = frompos.ptr;
  frompos.ptr = n->next;
  if (pos.ptr == n) return;
  // Unlink
  if (n->next)
    n->next->prev = n->prev;
  else
    fromlist.head.prev = n->prev;
  if (n->prev)
    n->prev->next = n->next;
  else
    fromlist.head.next = n->next;
  fromlist.nelem -= 1;
  // Prepare insertion
  if (pos.ptr)
    {
      Node *p = pos.ptr;
      n->prev = p->prev;
      n->next = p;
    }
  else
    {
      n->prev = head.prev;
      n->next = 0;
    }
  // Link
  if (n->prev)
    n->prev->next = n;
  else
    head.next = n;
  if (n->next)
    n->next->prev = n;
  else
    head.prev = n;
  nelem += 1;
}


void 
GListBase::del(GPosition &pos)
{
  // Check
  if (!pos.ptr || pos.cont != (void*)this) return;
  // Unlink
  Node *n = pos.ptr;
  if (n->next)
    n->next->prev = n->prev;
  else
    head.prev = n->prev;
  if (n->prev)
    n->prev->next = n->next;
  else
    head.next = n->next;
  // Finish
  nelem -= 1;
  traits.fini( (void*)n, 1);
  operator delete ( (void*)n );
  pos.ptr = 0;
}


GPosition 
GListBase::nth(unsigned int n) const
{
  Node *p = 0;
  if ((int)n < nelem)
    for (p=head.next; p; p=p->next)
      if ( n-- == 0)
        break;
  return GPosition(p, (void*)this);
}


void 
GListBase::empty()
{
  Node *n=head.next;
  while (n)
    {
      Node *p = n->next;
      traits.fini( (void*)n, 1 );
      operator delete ( (void*)n );
      n = p;
    }
  head.next = head.prev = 0;
  nelem = 0;
}


GListBase & 
GListBase::operator= (const GListBase & ref)
{
  if (this == &ref)
    return *this;
  empty();
  for(Node *n = ref.head.next; n; n=n->next)
    {
      Node *m = (Node*) operator new (traits.size);
      traits.copy( (void*)m, (void*)n, 1, 0);
      append(m);
    }
  return *this;
}





// ------------------------------------------------------------
// ASSOCIATIVE MAPS
// ------------------------------------------------------------




GSetBase::GSetBase(const Traits &traits)
  : traits(traits), nelems(0), nbuckets(0), 
    gtable(table), first(0)
{
  rehash(17);
}


GSetBase::GSetBase(const GSetBase &ref)
  : traits(ref.traits), 
    nelems(0), nbuckets(0), gtable(table), first(0)
{
  GSetBase::operator= (ref);
}


GSetBase::~GSetBase()
{
  G_TRY { empty(); } G_CATCH_ALL { } G_ENDCATCH;
//  delete [] table;
}


GCONT HNode *
GSetBase::hashnode(unsigned int hashcode) const
{
  int bucket = hashcode % nbuckets;
  return table[bucket];
}

GCONT HNode *
GSetBase::installnode(HNode *n)
{
  // Rehash if table is more than 60% full
  if (nelems*3 > nbuckets*2)
    rehash( 2*nbuckets - 1 );
  // Create and insert
  insertnode(n);
  return n;
}

void 
GSetBase::insertnode(HNode *n)
{
  int bucket = n->hashcode % nbuckets;
  n->prev = n->hprev = table[bucket];
  if (n->prev) 
    {
      // bucket was not empty
      n->next = n->prev->next;
      n->prev->next = n;
      if (n->next)
        n->next->prev = n;
    }
  else
    {
      // bucket was empty.
      n->next = first;
      first = n;
      if (n->next)
        n->next->prev = n;
    }
  // finish
  table[bucket] = n;
  nelems += 1;
}


void   
GSetBase::deletenode(GCONT HNode *n)
{
  if (n == 0) 
    return;
  int bucket = n->hashcode % nbuckets;
  // Regular links
  if (n->next)
    n->next->prev = n->prev;
  if (n->prev)
    n->prev->next = n->next;
  else
    first = (HNode*)(n->next);
  // HPrev links
  if (table[bucket] == n)
    table[bucket] = n->hprev;
  else
    ((HNode*)(n->next))->hprev = n->hprev;
  // Delete entry
  traits.fini( (void*)n, 1 );
  operator delete ( (void*)n );
  nelems -= 1;
}


void   
GSetBase::rehash(int newbuckets)
{
  // Save chain of nodes
  Node *n = first;
  // Simulate an empty map
  nelems = 0;
  first = 0;
  // Allocate a new empty bucket table
// delete [] table;
  gtable.resize(0);
  nbuckets = newbuckets;
  typedef HNode *HNodePtr;
// table = new HNodePtr[nbuckets];
  gtable.resize(nbuckets);
//  DjVuPrintErrorUTF8("GSetBase::rehash table=%x\n",table);
  gtable.clear();
//  for (int i=0; i<nbuckets; i++)
//    table[i] = 0;
  // Insert saved nodes
  while (n)
    {
      Node *p = n->next;
      insertnode((HNode*)n);
      n = p;
    }
}


GSetBase& 
GSetBase::operator=(const GSetBase &ref)
{
  if (this == &ref) 
    return *this;
  empty();
  rehash(ref.nbuckets);
  for (Node *n = ref.first; n; n=n->next)
    {
      HNode *m = (HNode*) operator new (traits.size);
      traits.copy( (void*)m, (void*)n, 1, 0);
      insertnode(m);
    }
  return *this;
}


GPosition 
GSetBase::firstpos() const
{
  return GPosition(first, (void*)this);
}


void 
GSetBase::del(GPosition &pos)
{
  if (pos.ptr && pos.cont==(void*)this)
    {
      deletenode((HNode*)pos.ptr);
      pos.ptr = 0;
    }
}

void 
GSetBase::empty()
{
  HNode *n = first;
  while (n)
    {
      HNode *p = (HNode*)(n->next);
      traits.fini( (void*)n, 1 );
      operator delete ( (void*)n );
      n = p;
    }
  first = 0;
  nelems = 0;
  gtable.clear();
//  for (int i=0; i<nbuckets; i++)
//    table[i] = 0;
}

