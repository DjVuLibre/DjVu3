//C-  -*- C++ -*-
//C-
//C-  Copyright (c) 1988 AT&T	
//C-  All Rights Reserved 
//C-
//C-  THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
//C-  The copyright notice above does not evidence any
//C-  actual or intended publication of such source code.
//C-
//C-  $Id: GSmartPointer.h,v 1.1 1999-01-22 00:40:19 leonb Exp $

#ifndef _GSMARTPOINTER_H_
#define _GSMARTPOINTER_H_

/** @name GSmartPointer.h

    Files #"GSmartPointer.h"# and #"GSmartPointer.cpp"# define a smart-pointer
    class which automatically performs thread-safe reference counting.  Class
    \Ref{GP} implements smart-pointers by overloading the usual pointer
    assignment and dereferencing operators. The overloaded operators maintain
    the reference counters and destroy the pointed objects as soon as their
    reference counter reaches zero.  Transparent type conversions are provided
    between smart-pointers and regular pointers.  Objects references by
    smart-pointers must be derived from class \Ref{GPEnabled}.

    @memo 
    Thread-Safe reference counting smart-pointers.
    @author 
    Leon Bottou <leonb@research.att.com> -- initial implementation\\
    Andrei Erofeev <eaf@research.att.com> -- bug fix.
    @version 
    #$Id: GSmartPointer.h,v 1.1 1999-01-22 00:40:19 leonb Exp $# 
    @args
*/
//@{

#if defined(EXTERNAL_TEMPLATES) && defined(__GNUC__)
#pragma interface
#endif

#include "DjVuGlobal.h"



/** Base class for reference counted objects.  
    This is the base class for all reference counted objects.
    Any instance of a subclass of #GPEnabled# can be used with 
    smart-pointers (see \Ref{GP}).  
 */
class GPEnabled
{
public:
  /// Null constructor.
  GPEnabled();
  /// Virtual destructor.
  virtual ~GPEnabled();
  /// Copy operator
  GPEnabled & operator=(const GPEnabled & obj);
protected:
  /// The reference counter
  volatile int count;
  /** Called when object must be destroyed.  
      Virtual function #destroy# is called when the reference counter is
      decreased from one to zero. The default implementation just calls
      #delete#. This default implementation should be enough for most
      purposes.  See the implementation of \Ref{GString} for an example of
      overriding #destroy#. */
  virtual void destroy();
private:
  friend class GPBase;
  void unref();
  void ref();
};



/** Base class for all smart-pointers.
    This class implements common mechanisms for all
    smart-pointers (see \Ref{GP}). There should be no need
    to use this class directly.  Its sole purpose consists
    in reducing the template expansion overhead.
*/

class GPBase
{
public:
  /** Null Constructor. */
  GPBase();
  /** Copy Constructor.
      Increments the reference count. 
      @param sptr reference to a #GPBase# object. */
  GPBase(const GPBase &sptr);
  /** Construct a GPBase from a pointer.
      Increments the reference count.
      @param nptr pointer to a #GPEnabled# object. */
  GPBase(GPEnabled *nptr);
  /** Destructor.
      Decrements the reference count. */
  ~GPBase();
  /** Accesses the actual pointer. */
  GPEnabled* get() const;
  /** Assignment. 
      Increments the counter of the new value of the pointer.
      Decrements the counter of the previous value of the pointer.
      @param nptr new #GPEnabled# pointer assigned to this object. */
  GPBase& assign(GPEnabled *nptr);
  /** Assignment operator. */
  GPBase & operator=(const GPBase & obj);
  /** Comparison operator. */
  int operator==(const GPBase & g2) const;
protected:
  /** Actual pointer */
  GPEnabled *ptr;
};


/** Reference counting pointer.

    Class #GP<TYPE># represents a smart-pointer to an object of type #TYPE#.
    Type #TYPE# must be a subclass of #GPEnabled#.  This class overloads the
    usual pointer assignment and dereferencing operators. The overloaded
    operators maintain the reference counters and destroy the pointed objects
    as soon as their reference counter reaches zero.  Transparent type
    conversions are provided between smart-pointers and regular pointers.

    Using a smart-pointer is a convenience and not an obligation.
    There is no need to use a smart-pointer to access a #GPEnabled# object.
    As long as you never use a smart-pointer to access a #GPEnabled# object,
    the object's reference counter stays null.  Since there is no reference
    counter transition from one to zero, the object remains allocated.  You can
    therefore choose to only use regular pointers to access objects allocated
    on the stack (automatic variables) or objects allocated dynamically.  In
    the latter case you must destroy the dynamically allocated object with
    operator #delete#.

    The first time you use a smart-pointer to access #GPEnabled# object, the
    reference counter is incremented to one  Object destruction will then
    happen automatically when the reference counters is decremented back to
    zero (i.e. when the last smart-pointer referencing this object stops doing so).
    This will happen regardless of how many regular pointers reference this object.
    In other words, if you start using smart-pointers with a #GPEnabled#
    object, you engage automatic mode for this object.  You should only do
    this with objects dynamically allocated with operator #new#.  You should
    never destroy the object yourself, but let the smart-pointers control the
    life of the object.
    
    {\bf Performance considerations} --- Thread safe reference counting incurs a
    significant overhead. smart-pointer are best used with sizeable objects
    for which the cost of maintaining the counters represent a small fraction
    of the processing time.  It is always possible to cache a smart-pointer
    into a regular pointer.  The cached pointer will remain valid until the
    smart-pointer is destroyed or the smart-pointer value is changed.

    {\bf Safety considerations} --- As explained above, a #GPEnabled# object
    switches to automatic mode as soon as it becomes referenced by a
    smart-pointer.  There is no way to switch the object back to manual mode.
    Suppose that you have decided to only use regular pointers with a
    particular #GPEnabled# object.  You therefore plan to destroy the object
    explicitly when you no longer need it.  When you pass a regular pointer to
    this object as argument to a function, you really need to be certain that
    the function implementation will not assign this pointer to a
    smart-pointer. Doing so would indeed destroy the object as soon as the
    function returns.  The bad news is that the fact that a function assigns a
    pointer argument to a smart-pointer does not necessarily appear in the
    function prototype.  Such a behavior must be {\em documented} with the
    function public interface.  As a convention, I usually write such
    functions with smart-pointer arguments instead of a regular pointer
    arguments.  This is not enough to catch the error at compile time, but
    this is a simple way to document such a behavior.  I still believe that
    this is a small problem in regard to the benefits of the smart-pointer.
    But one has to be aware of its existence.  
*/

template <class TYPE>
class GP : protected GPBase
{
public:
  /** Constructs a null smart-pointer. */
  GP();
  /** Constructs a copy of a smart-pointer.
      @param sptr smart-pointer to copy. */
  GP(const GP<TYPE> &sptr);
  /** Constructs a smart-pointer from a regular pointer.
      The pointed object must be dynamically allocated (with operator #new#).
      You should no longer explicitly destroy the object referenced by #sptr#
      since the object life is now controlled by smart-pointers.  
      @param nptr regular pointer to a {\em dynamically allocated object}. */
  GP(TYPE *nptr);
  /** Converts a smart-pointer into a regular pointer.  
      This is useful for caching the value of a smart-pointer for performances
      purposes.  The cached pointer will remain valid until the smart-pointer
      is destroyed or until the smart-pointer value is changed. */
  operator TYPE* () const;
  /** Assigns a regular pointer to a smart-pointer lvalue.
      The pointed object must be dynamically allocated (with operator #new#).
      You should no longer explicitly destroy the object referenced by #sptr#
      since the object life is now controlled by smart-pointers.  
      @param nptr regular pointer to a {\em dynamically allocated object}. */
  GP<TYPE>& operator= (TYPE *nptr);
  /** Assigns a smart-pointer to a smart-pointer lvalue.
      @param sptr smart-pointer copied into this smart-pointer. */
  GP<TYPE>& operator= (const GP<TYPE> &sptr);
  /** Indirection operator.
      This operator provides a convenient access to the members
      of a smart-pointed object. Operator #-># works with smart-pointers
      exactly as with regular pointers. */
  TYPE* operator->() const;
  /** Dereferencement operator.
      This operator provides a convenient access to the smart-pointed object. 
      Operator #*# works with smart-pointers exactly as with regular pointers. */
  TYPE& operator*() const;
  /** Comparison operator. 
      Returns true if the smart-pointer points to the object referenced 
      by #nptr#.  The automatic conversion from smart-pointers
      to regular pointers allows you to compare two smart-pointers as well.
      @param nptr pointer to compare with. */
  int operator== (TYPE *nptr) const;
  /** Comparison operator. 
      Returns true if the smart-pointer does not point to the object referenced 
      by #nptr#.  The automatic conversion from smart-pointers
      to regular pointers allows you to compare two smart-pointers as well.
      @param nptr pointer to compare with. */
  int operator!= (TYPE *nptr) const;
  /** Test operator.
      Returns true if the smart-pointer is null.  The automatic conversion 
      from smart-pointers to regular pointers allows you to perform
      the opposite test as well. You can use both following constructs:
      \begin{verbatim}
      if (gp) { ... }
      while (! gp) { ... }
      \end{verbatim} */
  int operator! () const;
};

//@}

// INLINE FOR GPENABLED

inline
GPEnabled::GPEnabled()
  : count(0)
{
}

inline GPEnabled & 
GPEnabled::operator=(const GPEnabled & obj)
{ 
  /* The copy operator should do nothing
     because the count should not be changed.
     Subclasses of GPEnabled will call this version of the
     copy operator as part of the default 'memberwise copy'
     strategy. Thank you Andrei! */
  return *this; 
}


// INLINE FOR GPBASE

inline
GPBase::GPBase()
  : ptr(0)
{
}

inline
GPBase::GPBase(GPEnabled *nptr)
  : ptr(0)
{
  if (nptr)
    nptr->ref();
  ptr = nptr;
}

inline
GPBase::GPBase(const GPBase &sptr)
{
  if (sptr.ptr)
    sptr.ptr->ref();
  ptr = sptr.ptr;
}

inline
GPBase::~GPBase()
{
  GPEnabled *old = ptr;
  ptr = 0;
  if (old)
    old->unref();
}


inline GPEnabled* 
GPBase::get() const
{
  return ptr;
}

inline GPBase &
GPBase::operator=(const GPBase & obj)
{
   return assign(obj.get());
}

inline int 
GPBase::operator==(const GPBase & g2) const
{
   return get() == g2.get();
}




// INLINE FOR GP<TYPE>

template <class TYPE> inline
GP<TYPE>::GP()
{
}

template <class TYPE> inline
GP<TYPE>::GP(TYPE *nptr)
: GPBase((GPEnabled*)nptr)
{
}


template <class TYPE> inline
GP<TYPE>::GP(const GP<TYPE> &sptr)
: GPBase((GPEnabled*) sptr)
{
}


template <class TYPE> inline
GP<TYPE>::operator TYPE* () const
{
  return (TYPE*)(this->get());
}

template <class TYPE> inline TYPE*
GP<TYPE>::operator->() const
{
  return (TYPE*)(this->get());
}

template <class TYPE> inline TYPE&
GP<TYPE>::operator*() const
{
  return *(TYPE*)(this->get());
}

template <class TYPE> inline GP<TYPE>& 
GP<TYPE>::operator= (TYPE *nptr)
{
  return (GP<TYPE>&)( this->assign(nptr) );
}

template <class TYPE> inline GP<TYPE>& 
GP<TYPE>::operator= (const GP<TYPE> &sptr)
{
  return (GP<TYPE>&)( this->assign(sptr.get()) );
}

template <class TYPE> inline int
GP<TYPE>::operator== (TYPE *nptr) const
{
  return ( (TYPE*)ptr == nptr );
}

template <class TYPE> inline int
GP<TYPE>::operator!= (TYPE *nptr) const
{
  return ( (TYPE*)ptr != nptr );
}

template <class TYPE> inline int
GP<TYPE>::operator! () const
{
  return !ptr;
}

#endif
