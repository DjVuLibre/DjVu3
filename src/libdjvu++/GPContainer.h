//C-  -*- C++ -*-
//C-
//C-  Copyright (c) 1998 AT&T	
//C-  All Rights Reserved 
//C-
//C-  THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
//C-  The copyright notice above does not evidence any
//C-  actual or intended publication of such source code.
//C-
//C-  $Id: GPContainer.h,v 1.2.2.3 1999-05-25 16:43:04 eaf Exp $


#ifndef _GPCONTAINER_H_
#define _GPCONTAINER_H_

#include "Arrays.h"
#include "GSmartPointer.h"
#include "GContainer.h"

#if defined(EXTERNAL_TEMPLATES) && defined(__GNUC__)
#pragma interface
#endif


/** @name GPContainer.h
    Files #"GPContainer.h"# and #"GPContainer.cpp"# contain implementation
    of \Ref{GPList}<T> and \Ref{DPArray}<T> classes, which are basically
    \Ref{GList}<\Ref{GP}<T>> and \Ref{DArray}<\Ref{GP}<T>>. Internally
    they are \Ref{GList}<\Ref{GPBase}> and \Ref{DArray}<\Ref{GPBase}>
    which results in the fact, that \Ref{GList} and \Ref{DArray} are
    instantiated only once. This is possible because \Ref{GP} and
    \Ref{GPBase} have the same size and the same internal
    structure. \Ref{GP} just adds template interface to \Ref{GPBase}. The
    result is that we reduce the size of the executable by having only
    one instantiation of the list and array.
    
    @memo List and array for \Ref{GP}<> pointers.
    @author 
    Leon Bottou <leonb@research.att.com>, Andrei Erofeev <eaf@geocities.com>.
    @version 
    #$Id: GPContainer.h,v 1.2.2.3 1999-05-25 16:43:04 eaf Exp $# */
//@{

/** Dynamic array.

    The only thing we have to say here is that #DPArray<TYPE># is the same
    as #DArray<GP<TYPE>>#. Use #DPArray# if you have many arrays of \Ref{GP}
    pointers in your program. This will reduce its size.
  */

template <class TYPE>
class DPArray : public DArray<GPBase> {
public:
  // -- CONSTRUCTORS
  DPArray();
  DPArray(int hibound);
  DPArray(int lobound, int hibound);
  DPArray(const DPArray<TYPE> &gc);
  // -- DESTRUCTOR
  virtual ~DPArray();
  // -- ACCESS
  GP<TYPE>& operator[](int n);
  const GP<TYPE>& operator[](int n) const;
  // -- CONVERSION
  operator GP<TYPE>* ();
  operator const GP<TYPE>* () const;
  // -- ALTERATION
  void ins(int n, const GP<TYPE> &val, unsigned int howmany=1);
  DPArray<TYPE>& operator= (const DPArray &ga);
};

template<class TYPE>
DPArray<TYPE>::DPArray() {}

template<class TYPE>
DPArray<TYPE>::DPArray(int hibound) :
      DArray<GPBase>(hibound) {}

template<class TYPE>
DPArray<TYPE>::DPArray(int lobound, int hibound) :
      DArray<GPBase>(lobound, hibound) {}

template<class TYPE>
DPArray<TYPE>::DPArray(const DPArray<TYPE> &gc) :
      DArray<GPBase>(gc) {}

template<class TYPE>
DPArray<TYPE>::~DPArray() {}

template<class TYPE>
inline GP<TYPE> &
DPArray<TYPE>::operator[](int n)
{
   return (GP<TYPE> &) DArray<GPBase>::operator[](n);
}

template<class TYPE>
inline const GP<TYPE> &
DPArray<TYPE>::operator[](int n) const
{
   return (const GP<TYPE> &) DArray<GPBase>::operator[](n);
}

template<class TYPE>
inline DPArray<TYPE>::operator GP<TYPE>* ()
{
   return (GP<TYPE> *) DArray<GPBase>::operator GPBase*();
}

template<class TYPE>
inline DPArray<TYPE>::operator const GP<TYPE>* () const
{
   return (const GP<TYPE> *) DArray<GPBase>::operator const GPBase*();
}

template<class TYPE>
inline void
DPArray<TYPE>::ins(int n, const GP<TYPE> & val, unsigned int howmany)
{
   DArray<GPBase>::ins(n, val, howmany);
}

template<class TYPE>
inline DPArray<TYPE> &
DPArray<TYPE>::operator= (const DPArray &ga)
{
   DArray<GPBase>::operator=(ga);
   return *this;
}

/** Doubly linked list.

    The only thing we have to say here is that #GPList<TYPE># is the same
    as #GPList<GP<TYPE>>#. Use #GPList# if you have many lists of \Ref{GP}
    pointers in your program. This will reduce its size.
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
inline GP<TYPE> &
GPList<TYPE>::operator[](GPosition pos)
{
   return (GP<TYPE> &) GList<GPBase>::operator[](pos);
}

template<class TYPE>
inline const GP<TYPE> &
GPList<TYPE>::operator[](GPosition pos) const
{
   return (const GP<TYPE> &) GList<GPBase>::operator[](pos);
}

template<class TYPE>
inline int
GPList<TYPE>::operator==(const GPList<TYPE> &l2) const
{
   return GList<GPBase>::operator==(l2);
}

template<class TYPE>
inline int
GPList<TYPE>::contains(const GP<TYPE> & elt) const
{
   return GList<GPBase>::contains((const GPBase &) elt);
}

template<class TYPE>
inline int
GPList<TYPE>::search(const GP<TYPE> & elt, GPosition &pos) const
{
   return GList<GPBase>::search((const GPBase &) elt, pos);
}

template<class TYPE>
inline void
GPList<TYPE>::append(const GP<TYPE> & elt)
{
   GList<GPBase>::append((const GPBase &) elt);
}

template<class TYPE>
inline void
GPList<TYPE>::prepend(const GP<TYPE> & elt)
{
   GList<GPBase>::prepend((const GPBase &) elt);
}

template<class TYPE>
inline void
GPList<TYPE>::insert_after(GPosition pos, const GP<TYPE> & elt)
{
   GList<GPBase>::insert_after(pos, (const GPBase &) elt);
}

template<class TYPE>
inline void
GPList<TYPE>::insert_before(GPosition pos, const GP<TYPE> & elt)
{
   GList<GPBase>::insert_before(pos, (const GPBase &) elt);
}

template<class TYPE>
inline const GP<TYPE> *
GPList<TYPE>::next(GPosition & pos) const
{
   return (const GP<TYPE> *) GList<GPBase>::next(pos);
}

template<class TYPE>
inline GP<TYPE> *
GPList<TYPE>::next(GPosition & pos)
{
   return (GP<TYPE> *) GList<GPBase>::next(pos);
}

template<class TYPE>
inline const GP<TYPE> *
GPList<TYPE>::prev(GPosition & pos) const
{
   return (const GP<TYPE> *) GList<GPBase>::prev(pos);
}

template<class TYPE>
inline GP<TYPE> *
GPList<TYPE>::prev(GPosition & pos)
{
   return (GP<TYPE> *) GList<GPBase>::prev(pos);
}

template<class TYPE>
inline GPList<TYPE> &
GPList<TYPE>::operator= (const GPList<TYPE>& gl)
{
   GList<GPBase>::operator=(gl);
   return *this;
}

#if 0
/* Hashed associative map.
  */

template<class KTYPE, class VTYPE>
class GPMap : public GMap<KTYPE, GPBase>
{
public:
      // CONSTRUCTORS
   GPMap();
   GPMap(const GPMap<KTYPE, VTYPE> & ref);
      // --  DESTRUCTOR
      /** Virtual destructor. */
   ~GPMap();
      // -- ACCESS
   GP<VTYPE> & operator[](const KTYPE & key);
   const GP<VTYPE> & operator[](const KTYPE & key) const;
      // -- ASSIGMENT
   GPMap<KTYPE, VTYPE> & operator=(const GPMap<KTYPE, VTYPE> & ref);
      // -- GContainer stuff
   const GP<VTYPE> * next(GPosition & pos) const;
   GP<VTYPE> * next(GPosition & pos);
   const GP<VTYPE> * prev(GPosition & pos) const;
   GP<VTYPE> * prev(GPosition & pos);
};

template<class KTYPE, class VTYPE>
GPMap<KTYPE, VTYPE>::GPMap() {}

template<class KTYPE, class VTYPE>
GPMap<KTYPE, VTYPE>::GPMap(const GPMap<KTYPE, VTYPE> &gc) :
      GMap<KTYPE, GPBase>(gc) {}

template<class KTYPE, class VTYPE>
GPMap<KTYPE, VTYPE>::~GPMap() {}

template<class KTYPE, class VTYPE>
inline GP<VTYPE> &
GPMap<KTYPE, VTYPE>::operator[](const KTYPE & key)
{
   return (GP<VTYPE> &) GMap<KTYPE, GPBase>::operator[](key);
}

template<class KTYPE, class VTYPE>
inline const GP<VTYPE> &
GPMap<KTYPE, VTYPE>::operator[](const KTYPE & key) const
{
   return (const GP<VTYPE> &) GMap<KTYPE, GPBase>::operator[](key);
}

template<class KTYPE, class VTYPE>
inline GPMap<KTYPE, VTYPE> &
GPMap<KTYPE, VTYPE>::operator=(const GPMap<KTYPE, VTYPE>& gm)
{
   GMap<KTYPE, GPBase>::operator=(gm);
   return *this;
}

template<class KTYPE, class VTYPE>
inline const GP<VTYPE> *
GPMap<KTYPE, VTYPE>::next(GPosition & pos) const
{
   return (const GP<VTYPE> *) GMap<KTYPE, GPBase>::next(pos);
}

template<class KTYPE, class VTYPE>
inline GP<VTYPE> *
GPMap<KTYPE, VTYPE>::next(GPosition & pos)
{
   return (GP<VTYPE> *) GMap<KTYPE, GPBase>::next(pos);
}

template<class KTYPE, class VTYPE>
inline const GP<VTYPE> *
GPMap<KTYPE, VTYPE>::prev(GPosition & pos) const
{
   return (const GP<VTYPE> *) GMap<KTYPE, GPBase>::prev(pos);
}

template<class KTYPE, class VTYPE>
inline GP<VTYPE> *
GPMap<KTYPE, VTYPE>::prev(GPosition & pos)
{
   return (GP<VTYPE> *) GMap<KTYPE, GPBase>::prev(pos);
}

#endif

//@}

#endif

