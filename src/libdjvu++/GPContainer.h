//C-  -*- C++ -*-
//C-
//C-  Copyright (c) 1988 AT&T	
//C-  All Rights Reserved 
//C-
//C-  THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
//C-  The copyright notice above does not evidence any
//C-  actual or intended publication of such source code.
//C-
//C-  $Id: GPContainer.h,v 1.1 1999-01-22 00:40:19 leonb Exp $


#ifndef _GPCONTAINER_H_
#define _GPCONTAINER_H_

#include "GContainer.h"
#include "GSmartPointer.h"

#if defined(EXTERNAL_TEMPLATES) && defined(__GNUC__)
#pragma interface
#endif


/** @name GContainer.h

    @memo 
    Template class for generic containers.
    @author 
    Leon Bottou <leonb@research.att.com> -- initial implementation.\\
    Andrei Erofeev <eaf@geocities.com> -- bug fixes.
    @version 
    #$Id: GPContainer.h,v 1.1 1999-01-22 00:40:19 leonb Exp $# */
//@{

/** Dynamic array.
  */

template <class TYPE>
class GPArray : public GArray<GPBase> {
public:
  // -- CONSTRUCTORS
  GPArray();
  GPArray(int hibound);
  GPArray(int lobound, int hibound);
  GPArray(const GContainer<GPBase> &gc);
  GPArray(const GPArray<TYPE> &gc);
  // -- DESTRUCTOR
  virtual ~GPArray();
  // -- ACCESS
  GP<TYPE>& operator[](int n);
  const GP<TYPE>& operator[](int n) const;
  GP<TYPE>& operator[](GPosition pos);
  const GP<TYPE>& operator[](GPosition pos) const;
  // -- CONVERSION
  operator GP<TYPE>* ();
  operator const GP<TYPE>* () const;
  // -- ALTERATION
  void ins(int n, const GP<TYPE> &val, unsigned int howmany=1);
  GPArray<TYPE>& operator= (const GPArray &ga);
};

template<class TYPE>
GPArray<TYPE>::GPArray() {}

template<class TYPE>
GPArray<TYPE>::GPArray(int hibound) :
      GArray<GPBase>(hibound) {}

template<class TYPE>
GPArray<TYPE>::GPArray(int lobound, int hibound) :
      GArray<GPBase>(lobound, hibound) {}

template<class TYPE>
GPArray<TYPE>::GPArray(const GContainer<GPBase> &gc) :
      GArray<GPBase>(gc) {}

template<class TYPE>
GPArray<TYPE>::GPArray(const GPArray<TYPE> &gc) :
      GArray<GPBase>(gc) {}

template<class TYPE>
GPArray<TYPE>::~GPArray() {}

template<class TYPE>
inline GP<TYPE> & GPArray<TYPE>::operator[](int n)
{
   return (GP<TYPE> &) GArray<GPBase>::operator[](n);
}

template<class TYPE>
inline const GP<TYPE> & GPArray<TYPE>::operator[](int n) const
{
   return (const GP<TYPE> &) GArray<GPBase>::operator[](n);
}

template<class TYPE>
inline GP<TYPE> & GPArray<TYPE>::operator[](GPosition pos)
{
   return (GP<TYPE> &) GArray<GPBase>::operator[](pos);
}

template<class TYPE>
inline const GP<TYPE> & GPArray<TYPE>::operator[](GPosition pos) const
{
   return (const GP<TYPE> &) GArray<GPBase>::operator[](pos);
}

template<class TYPE>
inline GPArray<TYPE>::operator GP<TYPE>* ()
{
   return (GP<TYPE> *) GArray<GPBase>::operator GPBase*();
}

template<class TYPE>
inline GPArray<TYPE>::operator const GP<TYPE>* () const
{
   return (const GP<TYPE> *) GArray<GPBase>::operator const GPBase*();
}

template<class TYPE>
inline void GPArray<TYPE>::ins(int n, const GP<TYPE> & val, unsigned int howmany)
{
   GArray<GPBase>::ins(n, val, howmany);
}

template<class TYPE>
inline GPArray<TYPE>& GPArray<TYPE>::operator= (const GPArray &ga)
{
   GArray<GPBase>::operator=(ga);
   return *this;
}

/** Doubly linked list.
  */

template <class TYPE>
class GPList : public GList<GPBase>
{
public:
      // -- CONSTRUCTORS
   GPList();
   GPList(const GContainer<GPBase> &gc);
   GPList(const GPList<TYPE> &gc);
      // -- DESTRUCTOR
   ~GPList();
      // -- ACCESS
   GP<TYPE> & operator[](GPosition pos);
   const GP<TYPE> & operator[](GPosition pos) const;
      // -- TEST
   int operator==(const GPList<TYPE> &l2) const;
      // -- SEARCHING
   int contains(const GP<TYPE> & elt) const;
   int search(const GP<TYPE> & elt, GPosition &pos) const;
      // -- ALTERATION
   void append(const GP<TYPE> & elt);
   void prepend(const GP<TYPE> & elt);
   void insert_after(GPosition pos, const GP<TYPE> & elt);
   void insert_before(GPosition pos, const GP<TYPE> & elt);
      // -- GContainer stuff
   const GP<TYPE> * next(GPosition & pos) const;
   GP<TYPE> * next(GPosition & pos);
   const GP<TYPE> * prev(GPosition & pos) const;
   GP<TYPE> * prev(GPosition & pos);
      // -- ASSIGMENT
   GPList<TYPE>& operator= (const GPList<TYPE>& gl);
};

template<class TYPE>
GPList<TYPE>::GPList() {}

template<class TYPE>
GPList<TYPE>::GPList(const GContainer<GPBase> &gc) :
      GList<GPBase>(gc) {}

template<class TYPE>
GPList<TYPE>::GPList(const GPList<TYPE> &gc) :
      GList<GPBase>(gc) {}

template<class TYPE>
GPList<TYPE>::~GPList() {}

template<class TYPE>
inline GP<TYPE> & GPList<TYPE>::operator[](GPosition pos)
{
   return (GP<TYPE> &) GList<GPBase>::operator[](pos);
}

template<class TYPE>
inline const GP<TYPE> & GPList<TYPE>::operator[](GPosition pos) const
{
   return (const GP<TYPE> &) GList<GPBase>::operator[](pos);
}

template<class TYPE>
inline int GPList<TYPE>::operator==(const GPList<TYPE> &l2) const
{
   return GList<GPBase>::operator==(l2);
}

template<class TYPE>
inline int GPList<TYPE>::contains(const GP<TYPE> & elt) const
{
   return GList<GPBase>::contains((const GPBase &) elt);
}

template<class TYPE>
inline int GPList<TYPE>::search(const GP<TYPE> & elt, GPosition &pos) const
{
   return GList<GPBase>::search((const GPBase &) elt, pos);
}

template<class TYPE>
inline void GPList<TYPE>::append(const GP<TYPE> & elt)
{
   GList<GPBase>::append((const GPBase &) elt);
}

template<class TYPE>
inline void GPList<TYPE>::prepend(const GP<TYPE> & elt)
{
   GList<GPBase>::prepend((const GPBase &) elt);
}

template<class TYPE>
inline void GPList<TYPE>::insert_after(GPosition pos, const GP<TYPE> & elt)
{
   GList<GPBase>::insert_after(pos, (const GPBase &) elt);
}

template<class TYPE>
inline void GPList<TYPE>::insert_before(GPosition pos, const GP<TYPE> & elt)
{
   GList<GPBase>::insert_before(pos, (const GPBase &) elt);
}

template<class TYPE>
inline const GP<TYPE> * GPList<TYPE>::next(GPosition & pos) const
{
   return (const GP<TYPE> *) GList<GPBase>::next(pos);
}

template<class TYPE>
inline GP<TYPE> * GPList<TYPE>::next(GPosition & pos)
{
   return (GP<TYPE> *) GList<GPBase>::next(pos);
}

template<class TYPE>
inline const GP<TYPE> * GPList<TYPE>::prev(GPosition & pos) const
{
   return (const GP<TYPE> *) GList<GPBase>::prev(pos);
}

template<class TYPE>
inline GP<TYPE> * GPList<TYPE>::prev(GPosition & pos)
{
   return (GP<TYPE> *) GList<GPBase>::prev(pos);
}

template<class TYPE>
inline GPList<TYPE> & GPList<TYPE>::operator= (const GPList<TYPE>& gl)
{
   GList<GPBase>::operator=(gl);
   return *this;
}

#endif

