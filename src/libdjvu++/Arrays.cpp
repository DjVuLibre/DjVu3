//C-  -*- C++ -*-
//C- DjVu� Reference Library (v. 3.0)
//C- 
//C- Copyright � 1999-2000 LizardTech, Inc. All Rights Reserved.
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
// $Id: Arrays.cpp,v 1.11 2000-11-02 01:08:34 bcr Exp $
// $Name:  $


#ifdef __GNUC__
#pragma implementation
#endif

#include "Arrays.h"

ArrayRep::ArrayRep(int xelsize,
		   void (* xdestroy)(void *, int, int),
		   void (* xinit1)(void *, int, int),
		   void (* xinit2)(void *, int, int, const void *, int, int),
		   void (* xcopy)(void *, int, int, const void *, int, int),
		   void (* xinsert)(void *, int, int, const void *, int)) :
      data(0), minlo(0), maxhi(-1), lobound(0), hibound(-1),
      elsize(xelsize), destroy(xdestroy), init1(xinit1),
      init2(xinit2), copy(xcopy), insert(xinsert)
{
}

ArrayRep::ArrayRep(int xelsize,
		   void (* xdestroy)(void *, int, int),
		   void (* xinit1)(void *, int, int),
		   void (* xinit2)(void *, int, int, const void *, int, int),
		   void (* xcopy)(void *, int, int, const void *, int, int),
		   void (* xinsert)(void *, int, int, const void *, int),
		   int hi) : data(0), minlo(0), maxhi(-1),
   lobound(0), hibound(-1), elsize(xelsize), destroy(xdestroy), init1(xinit1),
   init2(xinit2), copy(xcopy), insert(xinsert)
{
   resize(0, hi);
}

ArrayRep::ArrayRep(int xelsize,
		   void (* xdestroy)(void *, int, int),
		   void (* xinit1)(void *, int, int),
		   void (* xinit2)(void *, int, int, const void *, int, int),
		   void (* xcopy)(void *, int, int, const void *, int, int),
		   void (* xinsert)(void *, int, int, const void *, int),
		   int lo, int hi) : data(0), minlo(0), maxhi(-1),
   lobound(0), hibound(-1), elsize(xelsize), destroy(xdestroy), init1(xinit1),
   init2(xinit2), copy(xcopy), insert(xinsert)
{
   resize(lo,hi);
}

ArrayRep::ArrayRep(const ArrayRep & arr) : data(0), minlo(0), maxhi(-1),
   lobound(0), hibound(-1), elsize(arr.elsize), destroy(arr.destroy),
   init1(arr.init1), init2(arr.init2), copy(arr.copy), insert(arr.insert)
{
   resize(arr.lobound, arr.hibound);
   arr.copy(data, lobound-minlo, hibound-minlo,
	    arr.data, arr.lobound-arr.minlo, arr.hibound-arr.minlo);
}

ArrayRep::~ArrayRep()
{
   destroy(data, lobound-minlo, hibound-minlo);
   operator delete(data);
   data=0;
}

ArrayRep & 
ArrayRep::operator= (const ArrayRep & rep)
{
   if (&rep == this) return *this;
   empty();
   resize(rep.lobound, rep.hibound);
   copy(data, lobound-minlo, hibound-minlo,
	rep.data, rep.lobound-rep.minlo, rep.hibound-rep.minlo);
   return *this;
}

void
ArrayRep::resize(int lo, int hi)
{
  int nsize = hi - lo + 1;
  // Validation
  if (nsize < 0)
    G_THROW("arrays.resize");     //  Invalid low and high bounds in DArray resize
  // Destruction
  if (nsize == 0)
    {
      destroy(data, lobound-minlo, hibound-minlo);
      operator delete(data);
      data = 0;
      lobound = minlo = lo; 
      hibound = maxhi = hi; 
      return;
    }
  // Simple extension
  if (lo >= minlo && hi <= maxhi)
    {
      init1(data, lo-minlo, lobound-1-minlo);
      destroy(data, lobound-minlo, lo-1-minlo);
      init1(data, hibound+1-minlo, hi-minlo);
      destroy(data, hi+1-minlo, hibound-minlo);
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
  // allocate
  int bytesize=elsize*(nmaxhi-nminlo+1);
  void * ndata=operator new(bytesize);
  memset(ndata, 0, bytesize);
  // initialize
  init1(ndata, lo-nminlo, lobound-1-nminlo);
  init2(ndata, lobound-nminlo, hibound-nminlo,
        data, lobound-minlo, hibound-minlo);
  init1(ndata, hibound+1-nminlo, hi-nminlo);
  destroy(data, lobound-minlo, hibound-minlo);
  // free and replace
  operator delete (data);
  data = ndata;
  minlo = nminlo;
  maxhi = nmaxhi;
  lobound = lo;
  hibound = hi;
}

void
ArrayRep::shift(int disp)
{
   lobound += disp;
   hibound += disp;
   minlo += disp;
   maxhi += disp;
}

void
ArrayRep::del(int n, unsigned int howmany)
{
   if (howmany == 0)
      return;
   if ((int)(n + howmany) > hibound +1)
      G_THROW("arrays.ill_arg");      //  Illegal arguments in DArray::del
   copy(data, n-minlo, hibound-howmany-minlo,
	data, n+howmany-minlo, hibound-minlo);
   destroy(data, hibound+1-howmany-minlo, hibound-minlo);
   hibound = hibound - howmany;
}

void
ArrayRep::ins(int n, const void * what, unsigned int howmany)
{
   int nhi = hibound + howmany;
   if (howmany == 0) return;
   if (maxhi < nhi)
   {
      int nmaxhi = maxhi;
      while (nmaxhi < nhi)
	 nmaxhi += (nmaxhi < 8 ? 8 : (nmaxhi > 32768 ? 32768 : nmaxhi));
      int bytesize = elsize*(nmaxhi-minlo+1);
      void *ndata = operator new (bytesize);
      memset(ndata, 0, bytesize);
      copy(ndata, lobound-minlo, hibound-minlo,
	   data, lobound-minlo, hibound-minlo);
      destroy(data, lobound-minlo, hibound-minlo);
      operator delete (data);
      data = ndata;
      maxhi = nmaxhi;
   }

   insert(data, hibound+1-minlo, n-minlo, what, howmany);
   hibound=nhi;
}




// ---------------------------------------
// BEGIN HACK
// ---------------------------------------
// Included here to avoid dependency
// from ByteStream.o to Arrays.o

#ifndef DO_NOT_MOVE_GET_DATA_TO_ARRAYS_CPP
#include "ByteStream.h"
TArray<char>
MemoryByteStream::get_data(void)
{
   TArray<char> data(0, size()-1);
   readat((char*)data, size(), 0);
   return data;
}
#endif

// ---------------------------------------
// END HACK
// ---------------------------------------

