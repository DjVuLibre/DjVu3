//C-  -*- C++ -*-
//C-
//C-  Copyright (c) 1998 AT&T	
//C-  All Rights Reserved 
//C-
//C-  THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
//C-  The copyright notice above does not evidence any
//C-  actual or intended publication of such source code.
//C-
//C-  $Id: GContainer.h,v 1.4 1999-02-19 19:03:43 leonb Exp $


#ifndef _GCONTAINER_H_
#define _GCONTAINER_H_

#include "DjVuGlobal.h"
#include <string.h>
#include <new.h>
#include "GException.h"

#if defined(EXTERNAL_TEMPLATES) && defined(__GNUC__)
#pragma interface
#endif


/** @name GContainer.h

    Files #"GContainer.h"# and #"GContainer.cpp"# implement three main
    template class for generic containers.  Class \Ref{GArray} implements an
    array of objects with variable bounds. Class \Ref{GList} implements a
    doubly linked list of objects.  Class \Ref{GMap} implements a hashed
    associative map.  The container templates are not thread-safe. Thread
    safety can be implemented using the facilities provided in
    \Ref{GThreads.h}.  Class \Ref{GPosition} is a universal iterator for all
    container objects.

    {\bf Historical comments} --- I chose to implement my own containers because
    the STL classes were not universally available and the compilers were
    rarely able to deal with such a template galore.  The following template
    classes are much simpler and (at some point) were usable with G++ 2.7.2 or
    CFront based compilers.
    
    @memo 
    Template class for generic containers.
    @author 
    Leon Bottou <leonb@research.att.com> -- initial implementation.\\
    Andrei Erofeev <eaf@research.att.com> -- bug fixes.
    @version 
    #$Id: GContainer.h,v 1.4 1999-02-19 19:03:43 leonb Exp $# */
//@{

class GContainerBase;
class GPosition;


/** Generic iterator class.
    This class represents a position in an arbitrary container 
    It provides a generic way to iterate over the objects contained
    in a container.  This is achieved with the following code:
    \begin{verbatim}
    GArray<int>    a;
    GList<GString> l;
    for (GPosition pos=a.firstpos(); pos; ++pos) 
      l.append(a[pos]); 
    for (GPosition pos=l.lastpos(); pos; --pos)
      printf("%d\n", l[pos]);
    \end{verbatim}
    A #GPosition# object remains meaningful as long as you do not modify the
    contents of the underlying container.  You should not use a #GPosition#
    that was initialized before modifying the contents of container object.
    Undetermined results may happed (as they say...) */

class GPosition {
public:
  /** Constructs a null #GPosition# object. */
  GPosition();
  /** Constructs a #GPosition# object for a container.
      The constructed object denotes the first position in the container.
      @param gc reference to a \Ref{GArray}, \Ref{GList} or \Ref{GMap}. */
  GPosition(const GContainerBase &gc);
  /** Tests if a position is valid.
      Returns true if #this# denotes a valid position in a container. */
  operator int();
  /** Tests if a position is valid.
      Returns true if #this# denotes an invalid position. */
  int operator !();
  /** Moves to the next position.
      The #GPosition# object then points to the next position in the 
      corresponding container object. It is marked as invalid if no such
      position exists. */
  GPosition& operator ++();
  /** Moves to the previous position.
      The #GPosition# object then points to the previous position in the 
      corresponding container object. It is marked as invalid if no such
      position exists. */
  GPosition& operator --();
public:
  // These are public because we cannot write 
  // friend class GContainer<T>;
  // friend class GArray<T>;
  // friend class GList<T>;
  // friend class GMap<T>;
  // friend class GContainerBase;
  const GContainerBase *cont;
  int   pi;
  void *pv;
};



/** Abstract base class for all containers.
    This abstract class contains virtual functions for iterating over a
    container regardless of its nature (#GArray#, #GList#, or #GMap#) and of
    the type of its elements.  In particular, functions #nextpos# and
    #prevpos# are called when using the pre-increment or pre-decrement
    operators on a \Ref{GPosition} object.
*/

class GContainerBase {
public:
  /** Virtual destructor. */
  virtual ~GContainerBase();
  /** Returns the number of contained elements. */
  virtual int size() const = 0;
  /** Returns a #GPosition# for the first element in the container.
      The #GPosition# object is marked as invalid when the container is empty. */
  virtual GPosition firstpos() const = 0;
  /** Returns a #GPosition# for the last element in the container.
      The #GPosition# object is marked as invalid when the container is empty. */
  virtual GPosition lastpos() const = 0;
  /** Moves position #pos# to the next element in the container.
      Argument #pos# is set to an invalid position when 
      ({\em a}) there is no next element in the container, or
      ({\em b}) the input value of #pos# is not a valid position for this container.
      This function is called by #GPosition::operator++#. */
  virtual void nextpos(GPosition &pos) const = 0;
  /** Moves position #pos# to the previous element in the container.
      Argument #pos# is set to an invalid position when 
      ({\em a}) there is no previous element in the container, or
      ({\em b}) the input value of #pos# is not a valid position for this container. 
      This function is called by #GPosition::operator--#. */
  virtual void prevpos(GPosition &pos) const = 0;
  // Backward compatible iteration (purposely not documented)
  virtual void first(GPosition &pos) const;
  virtual void last(GPosition &pos) const;
};


/** Abstract base template class for all containers.
    This abstract class provides virtual functions for accessing
    the elements of a container regardless of its nature
    (#GArray#, #GList#, or #GMap#).
 */

template <class TYPE>
class GContainer : public GContainerBase {
public:
  // Generic access
  /** Returns a constant pointer to the object located in this container at
      position #pos#.  This function returns the null pointer if position
      #pos# is not a valid position for this container. */
  virtual const TYPE* get(const GPosition &pos) const = 0;
  /** Returns a pointer to a object located in this container at
      position #pos#.  This function returns the null pointer if position
      #pos# is not a valid position for this container. */ 
  virtual TYPE* get(const GPosition &pos) = 0;
  // Backward compatible iteration (purposely not documented)
  const TYPE* next(GPosition &pos) const;
  const TYPE* prev(GPosition &pos) const;
  TYPE* next(GPosition &pos);
  TYPE* prev(GPosition &pos);
};


/** Dynamic array.  
    Template class #GArray<TYPE># implements an array of
    elements of type #TYPE#.  Each element is identified by an integer
    subscript.  The valid subscripts range is defined by dynamically
    adjustable lower- and upper-bounds.  Besides accessing and setting
    elements, member functions are provided to insert or delete elements at
    specified positions.
    
    This template class must be able to access
    \begin{itemize}
    \item a null constructor #TYPE::TYPE()#, 
    \item a copy constructor #TYPE::TYPE(const TYPE &)#,
    \item and a copy operator #TYPE & operator=(const TYPE &)#.
    \end{itemize} */

template <class TYPE>
class GArray : public GContainer<TYPE> {
public:
  // -- CONSTRUCTORS
  /** Constructs an empty array. The valid subscript range is initially
      empty. Member function #touch# and #resize# provide convenient ways
      to enlarge the subscript range. */
  GArray();
  /** Constructs an array with subscripts in range 0 to #hibound#. 
      The subscript range can be subsequently modified with member functions
      #touch# and #resize#.
      @param hibound upper bound of the initial subscript range. */
  GArray(int hibound);
  /** Constructs an array with subscripts in range #lobound# to #hibound#.  
      The subscript range can be subsequently modified with member functions
      #touch# and #resize#.
      @param lobound lower bound of the initial subscript range.
      @param hibound upper bound of the initial subscript range. */
  GArray(int lobound, int hibound);
  /** Constructs an array by copying the elements of container #gc#.
      The valid subscript will range from zero to the number of 
      elements in container #gc# minus one.      
      @param gc container from which elements will be copied. */
  GArray(const GContainer<TYPE> &gc);
  /** Copy constructor. The resulting array will have the same
      valid subscript range as array #gc#. All elements in #gc#
      will be copied into the constructed array. */
  GArray(const GArray<TYPE> &gc);
  // -- DESTRUCTOR
  virtual ~GArray();
  // -- VIRTUAL FUNCTIONS
  virtual int size() const;
  virtual GPosition firstpos() const;
  virtual GPosition lastpos() const;
  virtual void nextpos(GPosition &pos) const;
  virtual void prevpos(GPosition &pos) const;
  virtual const TYPE *get(const GPosition &pos) const;
  virtual TYPE* get(const GPosition &pos);
  // -- ACCESS
  /** Returns the lower bound of the valid subscript range. */
  int lbound() const;
  /** Returns the upper bound of the valid subscript range. */
  int hbound() const;
  /** Returns a reference to the array element for subscript #n#.  This
      reference can be used for both reading (as "#a[n]#") and writing (as
      "#a[n]=v#") an array element.  This operation will not extend the valid
      subscript range: an exception \Ref{GException} is thrown if argument #n#
      is not in the valid subscript range. */
  TYPE& operator[](int n);
  /** Returns a constant reference to the array element for subscript #n#.
      This reference can only be used for reading (as "#a[n]#") an array
      element.  This operation will not extend the valid subscript range: an
      exception \Ref{GException} is thrown if argument #n# is not in the valid
      subscript range.  This variant of #operator[]# is necessary when dealing
      with a #const GArray<TYPE>#. */
  const TYPE& operator[](int n) const;
  /** Returns a reference to the array element for position #n#.  This
      reference can be used for both reading and writing an array element.  An
      exception \Ref{GException} is thrown if argument #n# is not a valid
      #GPosition# for this container. */
  TYPE& operator[](GPosition pos);
  /** Returns a constant reference to the array element for position #n#.
      This reference can only be used for reading an array element.  An
      exception \Ref{GException} is thrown if argument #n# is not a valid
      #GPosition# for this container. This variant of #operator[]# is
      necessary when dealing with a #const GArray<TYPE>#. */
  const TYPE& operator[](GPosition pos) const;
  // -- CONVERSION
  /** Returns a pointer for reading or writing the array elements.  This
      pointer can be used to access the array elements with the same
      subscripts and the usual bracket syntax.  This pointer remains valid as
      long as the valid subscript range is unchanged. If you change the
      subscript range, you must stop using the pointers returned by prior
      invokation of this conversion operator. */
  operator TYPE* ();
  /** Returns a pointer for reading (but not modifying) the array elements.
      This pointer can be used to access the array elements with the same
      subscripts and the usual bracket syntax.  This pointer remains valid as
      long as the valid subscript range is unchanged. If you change the
      subscript range, you must stop using the pointers returned by prior
      invokation of this conversion operator. */
  operator const TYPE* () const;
  // -- ALTERATION
  /** Erases the array contents. All elements in the array are destroyed.  
      The valid subscript range is set to the empty range. */
  void empty();
  /** Extends the subscript range so that is contains #n#.
      This function does nothing if #n# is already int the valid subscript range.
      If the valid range was empty, both the lower bound and the upper bound
      are set to #n#.  Otherwise the valid subscript ranhe is extented
      to encompass #n#. This function is very handy when called before setting
      an array element:
      \begin{verbatim}
        int lineno=1;
        GArray<GString> a;
        while (! end_of_file()) { 
          a.touch[lineno]; 
          a[lineno++] = read_a_line(); 
        }
      \end{verbatim} 
      @param n 
  */
  void touch(int n);
  /** Resets the valid subscript range to #0#---#hibound#. 
      This function may destroy some array elements and may construct
      new array elements with the null constructor. Setting #hibound# to
      #-1# resets the valid subscript range to the empty range.
      @param hibound upper bound of the new subscript range. */      
  void resize(int hibound);
  /** Resets the valid subscript range to #lobound#---#hibound#. 
      This function may destroy some array elements and may construct
      new array elements with the null constructor. Setting #lobound# to #0# and
      #hibound# to #-1# resets the valid subscript range to the empty range.
      @param lobound lower bound of the new subscript range.
      @param hibound upper bound of the new subscript range. */
  void resize(int lobound, int hibound);
  /** Shifts the valid subscript range. Argument #disp# is added to both 
      bounds of the valid subscript range. Array elements previously
      located at subscript #x# will now be located at subscript #x+disp#. */
  void shift(int disp);
  /** Deletes array elements. The array elements corresponding to
      subscripts #n#...#n+howmany-1# are destroyed. All array elements
      previously located at subscripts greater or equal to #n+howmany#
      are moved to subscripts starting with #n#. The new subscript upper
      bound is reduced in order to account for this shift. 
      @param n subscript of the first element to delete.
      @param howmany number of elements to delete. */
  void del(int n, unsigned int howmany=1);
  /** Insert new elements into an array. This function inserts
      #howmany# elements at position #n# into the array. The initial value #val#
      is copied into the new elements. All array elements previously located at subscripts
      #n# and higher are moved to subscripts #n+howmany# and higher. The upper bound of the 
      valid subscript range is increased in order to account for this shift.
      @param n subscript of the first inserted element.
      @param val initial value of the new elements.
      @param howmany number of elements to insert. */
  void ins(int n, const TYPE &val, unsigned int howmany=1);
  /** Copy operator. All elements in array #*this# are destroyed.
      The valid subscript range is set to the valid subscript range of
      array #ga#. All elements of #ga# are copied into array #*this#. */
  GArray<TYPE>& operator= (const GArray &ga);
protected:
  // Implementation
  TYPE *data;
  int   minlo;
  int   maxhi;
  int   lobound;
  int   hibound;
};



/** Sortable array.  
    Template class #GSArray<TYPE># implements sorting routines for the array
    elements. These sorting routines are implemented in a subsclass in order
    to reduce the template instanciation overhead for class #GArray#.  Besides
    the #TYPE# constructors and operators required by class #GArray#, this
    template class must be able to access a less-or-equal comparison operator:
    #TYPE::operator<=(const TYPE&)#.  */

template<class TYPE>
class GSArray : public GArray<TYPE>
{
public:
  // -- CONSTRUCTORS
  /** Constructs an empty array. The valid subscript range is initially
      empty. Member function #touch# and #resize# provide convenient ways
      to enlarge the subscript range. */
  GSArray() {};
  /** Constructs an array with subscripts in range 0 to #hibound#. 
      The subscript range can be subsequently modified with member functions
      #touch# and #resize#.
      @param hibound upper bound of the initial subscript range. */
  GSArray(int hibound) : GArray<TYPE>(hibound) {};
  /** Constructs an array with subscripts in range #lobound# to #hibound#.  
      The subscript range can be subsequently modified with member functions
      #touch# and #resize#.
      @param lobound lower bound of the initial subscript range.
      @param hibound upper bound of the initial subscript range. */
  GSArray(int lobound, int hibound) : GArray<TYPE>(lobound, hibound) {};
  /** Constructs an array by copying the elements of container #gc#.
      The valid subscript will range from zero to the number of 
      elements in container #gc# minus one.      
      @param gc container from which elements will be copied. */
  GSArray(const GContainer<TYPE> &gc) : GArray<TYPE>(gc) {};
  /** Copy constructor. The resulting array will have the same
      valid subscript range as array #gc#. All elements in #gc#
      will be copied into the constructed array. */
  GSArray(const GArray<TYPE> &gc) : GArray<TYPE>(gc) {};
  // -- SORT
  /** Sort array elements.  Sort all array elements in ascending order.  Array
      elements are compared using the less-or-equal comparison operator for
      type #TYPE#. */
  void sort();
  /** Sort array elements in subscript range #lo# to #hi#.  Sort all array
      elements whose subscripts are in range #lo#..#hi# in ascending order.
      The other elements of the array are left untouched. Both arguments #lo#
      and #hi# must be in the valid subscript range.  Array elements are
      compared using the less-or-equal comparison operator for type #TYPE#.
      @param lo low bound for the subscripts of the elements to sort.
      @param hi high bound for the subscripts of the elements to sort. */
  void sort(int lo, int hi);
};


/** Doubly linked list.
    Template class #GList<TYPE># implements a doubly linked list of elements
    of type #TYPE#.  Member functions are provided to search the list for an element,
    to insert or delete elements at specified positions.
    
    This template class must be able to access
    \begin{itemize}
    \item a copy constructor #TYPE::TYPE(const TYPE &)#,
    \item and a comparison operator #TYPE::operator==(const TYPE &)#.
    \end{itemize} */

template <class TYPE>
class GList : public GContainer<TYPE>
{
public:
  // -- CONSTRUCTORS
  /** Null Constructor. Constructs a list with zero elements. */
  GList();
  /** Constructs a list by copying the elements of container #gc#.
      @param gc container from which elements will be copied. */
  GList(const GContainer<TYPE> &gc);
  /** Constructs a list by copying the elements of list #gc#.
      @param gc list from which elements will be copied. */
  GList(const GList<TYPE> &gc);
  // -- DESTRUCTOR
  /** Virtual destructor. */
  ~GList();
  // -- VIRTUAL FUNCTIONS
  virtual int size() const;
  virtual GPosition firstpos() const;
  virtual GPosition lastpos() const;
  virtual void nextpos(GPosition &pos) const;
  virtual void prevpos(GPosition &pos) const;
  virtual const TYPE *get(const GPosition &pos) const;
  virtual TYPE* get(const GPosition &pos);
  // -- ACCESS
  /** Tests whether a list is empty.  Returns a non zero value if the list
      contains zero elements. */
  int isempty() const;
  /** Returns a reference to the list element at position #pos#.  This
      reference can be used for both reading (as "#a[n]#") and modifying (as
      "#a[n]=v#") a list element. An exception \Ref{GException} is thrown if
      #pos# is not a valid position for this list. */
  TYPE& operator[](GPosition pos);
  /** Returns a constant reference to the list element at position #pos#.
      This reference only be used for reading a list element.  An exception
      \Ref{GException} is thrown if #pos# is not a valid position. This
      variant of #operator[]# is necessary when dealing with a 
      #const GList<TYPE>#. */
  const TYPE& operator[](GPosition pos) const;
  // -- TEST
  /** Compares two lists. Returns a non zero value if and only if both lists
      contain the same elements (as tested by #TYPE::operator==(const TYPE&)#
      in the same order. */
  int operator==(const GList<TYPE> &l2) const;
  // -- SEARCHING
  /** Initializes position #pos# for accessing the #n#-th list element.
      If the list contains less than #n# elements, function #nth# returns 
      zero and the position #pos# is left unchanged. Otherwise function #nth# 
      returns a non zero value.  Position #pos# can be subsequently used to access
      the list element with #operator[](const GPosition&)#. */
  int nth(unsigned int n, GPosition &pos) const;
  /** Tests whether the list contains a given element.  Returns a non zero
      value if and only if the list contains an element equal to #elt# as
      checked by #TYPE::operator==(const TYPE&)#. */
  int contains(const TYPE &elt) const;
  /** Searches the list for a given element. If position #pos# is a valid
      position for this list, the search starts at the specified position. If
      position #pos# is not a valid position, the search starts at the
      beginning of the list.  The list elements are sequentially compared with
      #elt# using #TYPE::operator==(const TYPE&)#.  As soon as a list element
      is equal to #elt#, function #search# initializes argument #pos# with the
      position of this list element and returns a non zero value.  If however
      the search reaches the end of the list, function #search# returns
      zero without altering #pos#. */
  int search(const TYPE &elt, GPosition &pos) const;
  // -- ALTERATION
  /** Erases the list contents.  All list elements are destroyed and
      unlinked. The list is left with zero elements. */
  void empty();
  /** Inserts an element after the last element of the list. 
      The new element is initialized with a copy of argument #elt#. */
  void append(const TYPE& elt);
  /** Inserts an element before the first element of the list. 
      The new element is initialized with a copy of argument #elt#. */
  void prepend(const TYPE& elt);
  /** Inserts an element after the list element at position #pos#.  An
      exception \Ref{GException} is thrown if #pos# is not a valid position
      for this list.  The new element is initialized with a copy of #elt#. */
  void insert_after(GPosition pos, const TYPE& elt);
  /** Inserts an element before the list element at position #pos#.  An
      exception \Ref{GException} is thrown if #pos# is not a valid position
      for this list.  The new element is initialized with a copy of #elt#. */
  void insert_before(GPosition pos, const TYPE& elt);
  /** Destroys the list element at position #pos#. This function does 
      nothing unless position #pos# is a valiud position for this list */
  void del(GPosition &pos);
  // -- ASSIGMENT
  /** Copy operator.  
      The list is reinitialized by copying all elements of list #gl#. */
  GList<TYPE>& operator= (const GList<TYPE>& gl);
protected:
  // Node implementation
  class GListNode {
  public:
    GListNode() {};
    GListNode(const TYPE& val) : data(val) {};
    GListNode *next;
    GListNode *prev;
    TYPE data;
  };
  friend class GListNode;
  // Implementation
  int nelem;
  GListNode head;
};


/** Hashed associative map.
    Template class #GMap<KTYPE,VTYPE># implements a associative map.  The
    associative map contains an arbitrary number of entries. Each entry is a
    pair containing one element of type #KTYPE# (named the "key") and one
    element of type #VTYPE# (named the "value").  All entries have different
    keys.  The entry associated to a particular value of the key can retrieved
    by function #contains#.  The key and the value can be accessed using
    function #key# and #operator[]#.

    This template class must be able to access
    \begin{itemize}
    \item a #VTYPE# null constructor #VTYPE::VTYPE()#, 
    \item a #VTYPE# copy operator #VTYPE::operator=(const VTYPE &)#,
    \item a #KTYPE# copy constructor #KTYPE::KTYPE(const KTYPE &)#,
    \item a #KTYPE# comparison operator #KTYPE::operator==(const KTYPE &)#,
    \item and a #KTYPE# hashing function #hash(const KTYPE&)#.
    \end{itemize} 

    The hashing function must return an #unsigned int# number. Multiple
    invocations of the hashing function with equal arguments (in the sense of
    #KTYPE::operator==#) must always return the same number.

    Associative maps support only forward iteration : position objects
    (\Ref{GPosition}) defined on associative maps cannot be decremented with
    #GPosition::operator--#.  */

template<class KTYPE, class VTYPE>
class GMap : public GContainer<VTYPE>
{
public:
  // CONSTRUCTORS
  /** Construct an associative map with no entries. 
      New map entries will be initialized with the null constructor
      #VTYPE::VTYPE()#. */
  GMap();
  /** Copy Constructor. Constructs an associative map by copying all the
      entries and the default value of map #ref#. */
  GMap(const GMap<KTYPE,VTYPE>&ref);
  // --  DESTRUCTOR
  /** Virtual destructor. */
  ~GMap();
  // -- VIRTUAL FUNCTIONS
  virtual int size() const;
  virtual GPosition firstpos() const;
  virtual GPosition lastpos() const;
  virtual void nextpos(GPosition &pos) const;
  virtual void prevpos(GPosition &pos) const;
  virtual const VTYPE *get(const GPosition &pos) const;
  virtual VTYPE* get(const GPosition &pos);
  // -- TESTS
  /** Tests whether the associative map is empty.  Returns a non zero value if
      and only if the map contains zero entries. */
  int isempty() const;
  /** Tests whether the map contains an entry for key #key#.
      Returns a non zero value if and only if the map contains an entry whose 
      key is equal to #key# according to #KTYPE::operator==(const KTYPE&)#. */
  int contains(const KTYPE &key) const;
  /** Searches a map entry for key #key#. Returns a non zero value if and only
      if the map contains an entry whose key is equal to #key# according to
      #KTYPE::operator==(const KTYPE&)#.  Position #pos# is then set to the 
      position of the map entry matching ket #key#. */
  int contains(const KTYPE &key, GPosition &pos) const;
  // -- ACCESS
  /** Erases the associative map contents.  All entries are destroyed and
      removed. The map is left with zero entries. */
  void empty();
  /** Returns a reference to the value of the map entry for key #key#.  This
      reference can be used for both reading (as "#a[n]#") and modifying (as
      "#a[n]=v#"). If there is no entry for key #key#, a new entry is created
      for that key with the null constructor #VTYPE::VTYPE()#. */
  VTYPE& operator[](const KTYPE &key);
  /** Returns a reference to the value of the map entry at position #pos#.  An
      exception \Ref{GException} is thrown if position #pos# is not valid. */
  VTYPE& operator[](const GPosition &pos);
  /** Returns a constant reference to the value of the map entry for key
      #key#.  This reference can only be used for reading (as "#a[n]#") the
      entry value.  An exception \Ref{GException} is thrown if no entry
      contains key #key#. This variant of #operator[]# is necessary when
      dealing with a #const GMAP<KTYPE,VTYPE>#. */
  const VTYPE& operator[](const KTYPE &key) const;
  /** Returns a constant reference to the value of the map entry at position
      #pos#.  An exception \Ref{GException} is thrown if position #pos# is not
      valid. This variant of #operator[]# is necessary when dealing with a
      #const GMAP<KTYPE,VTYPE>#. */
  const VTYPE& operator[](const GPosition &pos) const;
  /** Returns a constant reference to the key of the map entry at position
      #pos#.  An exception \Ref{GException} is thrown if position #pos# is not
      valid.  There is no direct way to change the key of a map entry. */
  const KTYPE &key(const GPosition &pos) const;
  /** Destroys the map entry for key #key#.  Nothing is done if there is no
      entry for key #key#. */
  void del(const KTYPE &key);
  /** Destroys the map entry for key #key#.  Nothing is done if position
      #pos# is not a valid position for this container. */
  void del(GPosition &pos);
  // -- ASSIGMENT
  /** Copy operator.  The associative map is reinitialized by copying all
      entries in map #ref#. */
  GMap<KTYPE,VTYPE>& operator=(const GMap<KTYPE,VTYPE>&ref);
protected:
  // implementation
  class GMapNode {
  public:
    GMapNode(const KTYPE& key) : key(key) {};
    GMapNode *next;
    GMapNode **pprev;
    unsigned int hash;
    KTYPE key;
    VTYPE val;
  };
  friend class GMapNode;
  // helper
  void *search(const KTYPE &key, int *pbucket) const;
  void *search_create(const KTYPE &key, int *pbucket);
  void rehash(int newsize);
  // members
  int nbuckets;
  int nelems;
  GMapNode **table;
};


/** @name Hash functions
    These functions let you use template class \Ref{GMap}
    with the corresponding elementary types. 
    @memo Hash functions for elementary types. */
//@{

/** Hashing function (unsigned int). */
static inline unsigned int 
hash(const unsigned int & x) 
{ 
  return x; 
}

/** Hashing function (int). */
static inline unsigned int 
hash(const int & x) 
{ 
  return (unsigned int)x;
}

/** Hashing function (long). */
static inline unsigned int
hash(const long & x) 
{ 
  return (unsigned int)x;
}

/** Hashing function (unsigned long). */
static inline unsigned int
hash(const unsigned long & x) 
{ 
  return (unsigned int)x;
}

/** Hashing function (float). */
static inline unsigned int
hash(const float & x) 
{ 
  // optimizer will get rid of unnecessary code  
  unsigned int *addr = (unsigned int*)&x;
  if (sizeof(float)<2*sizeof(unsigned int))
    return addr[0];
  else
    return addr[0]^addr[1];
}

/** Hashing function (double). */
static inline unsigned int
hash(const double & x) 
{ 
  // optimizer will get rid of unnecessary code
  unsigned int *addr = (unsigned int*)&x;
  if (sizeof(double)<2*sizeof(unsigned int))
    return addr[0];
  else if (sizeof(double)<4*sizeof(unsigned int))
    return addr[0]^addr[1];
  else
    return addr[0]^addr[1]^addr[2]^addr[3];    
}

//@}
//@}


// ------------- GPOSITION INLINES


inline 
GPosition::GPosition() 
  : cont(0), pi(0), pv(0)
{ 
}

inline 
GPosition::GPosition(const GContainerBase &gc)
  : cont(0), pi(0), pv(0)
{ 
  *this = gc.firstpos(); 
}

inline 
GPosition::operator int() 
{ 
  return cont!=0; 
}

inline int 
GPosition::operator !() 
{ 
  return cont==0; 
}

inline GPosition& 
GPosition::operator ++() 
{
  if (cont) 
    cont->nextpos(*this); 
  return *this; 
}

inline GPosition& 
GPosition::operator --() 
{ 
  if (cont) 
    cont->nextpos(*this); 
  return *this; 
}

template <class TYPE> TYPE*
GContainer<TYPE>::next(GPosition &pos) 
{
  TYPE *ret = get(pos); 
  nextpos(pos); 
  return ret;
}

template <class TYPE> TYPE*
GContainer<TYPE>::prev(GPosition &pos) 
{
  TYPE *ret = get(pos); 
  prevpos(pos); 
  return ret;
}

template <class TYPE> const TYPE*
GContainer<TYPE>::next(GPosition &pos) const
{
  return ((GContainer<TYPE>*)this)->next(pos);
}

template <class TYPE> const TYPE*
GContainer<TYPE>::prev(GPosition &pos) const
{
  return ((GContainer<TYPE>*)this)->prev(pos);
}


// ----- GARRAY IMPLEMENTATION

template <class TYPE>
GArray<TYPE>::GArray ()
  : data(0), minlo(0), maxhi(-1), lobound(0), hibound(-1)
{
}

template <class TYPE>
GArray<TYPE>::GArray(int hi)
  : data(0), minlo(0), maxhi(-1), lobound(0), hibound(-1)
{
  resize(0,hi);
}

template <class TYPE>
GArray<TYPE>::GArray(int lo, int hi)
  : data(0), minlo(0), maxhi(-1), lobound(0), hibound(-1)
{
  resize(lo,hi);
}

template <class TYPE>
GArray<TYPE>::GArray(const GContainer<TYPE> &gc)
  : data(0), minlo(0), maxhi(-1), lobound(0), hibound(-1)
{
  resize(0, gc.size() - 1);
  int idata = -minlo;
  for (GPosition pos(gc); pos; ++pos,++idata)
    data[idata] = *gc.get(pos);
}

template <class TYPE>
GArray<TYPE>::GArray(const GArray<TYPE> &ga)
  : data(0), minlo(0), maxhi(-1), lobound(0), hibound(-1)
{
  resize(ga.lobound, ga.hibound);
  for (int i = lobound; i <= hibound; i++)
    data[i-minlo] = ga.data[i-ga.minlo];
}


template <class TYPE>
GArray<TYPE>::~GArray()
{
  for (int i=lobound; i<=hibound; i++)
    data[i-minlo].TYPE::~TYPE();
  operator delete((void*)data);
  data = 0;
}


template <class TYPE> inline int
GArray<TYPE>::size() const
{
  return hibound - lobound + 1;
}

template <class TYPE> GPosition
GArray<TYPE>::firstpos() const
{
  GPosition pos;
  pos.cont = this;
  pos.pi = lobound;
  if (hibound<lobound)
    pos.cont = 0;
  return pos;
}

template <class TYPE> GPosition
GArray<TYPE>::lastpos() const
{
  GPosition pos;
  pos.cont = this;
  pos.pi = hibound;
  if (hibound<lobound)
    pos.cont = 0;
  return pos;
}

template <class TYPE> void
GArray<TYPE>::nextpos(GPosition &pos) const
{
  if (pos.cont==this && ++pos.pi <= hibound) return;
  pos.cont = 0;
}

template <class TYPE> void
GArray<TYPE>::prevpos(GPosition &pos) const
{
  if (pos.cont==this && --pos.pi >= lobound) return;
  pos.cont = 0;
}

template <class TYPE> inline TYPE* 
GArray<TYPE>::get(const GPosition &pos)
{
  if (pos.cont==this && pos.pi>=lobound && pos.pi<=hibound) 
    return data + pos.pi - minlo;
  return 0;
}

template <class TYPE> inline const TYPE *
GArray<TYPE>::get(const GPosition &pos) const
{
  return ((GArray<TYPE>*)(this))->get(pos);
}


template <class TYPE> inline
GArray<TYPE>::operator TYPE* ()
{
  return &data[-minlo];
}

template <class TYPE> inline
GArray<TYPE>::operator const TYPE* () const
{
  return &data[-minlo];
}

template <class TYPE> inline TYPE& 
GArray<TYPE>::operator[](int n)
{
  if (n<lobound || n>hibound)
    THROW("Illegal GArray subscript");
  return data[n - minlo];
}

template <class TYPE> inline const TYPE& 
GArray<TYPE>::operator[](int n) const
{
  return ((GArray<TYPE>*)(this))->operator[](n);
}

template <class TYPE> inline TYPE& 
GArray<TYPE>::operator[](GPosition pos)
{
  if (! (pos.cont==this && pos.pi>=lobound && pos.pi<=hibound))
    THROW("Illegal GArray subscript");    
  return data[pos.pi - minlo];
}

template <class TYPE> inline const TYPE& 
GArray<TYPE>::operator[](GPosition pos) const
{
  return ((GArray<TYPE>*)(this))->operator[](pos);
}

template <class TYPE> inline int
GArray<TYPE>::lbound() const
{
  return lobound;
}

template <class TYPE> inline int
GArray<TYPE>::hbound() const
{
  return hibound;
}

template <class TYPE> inline void
GArray<TYPE>::empty()
{
  resize(0, -1);
}

template <class TYPE> inline void
GArray<TYPE>::resize(int hi)
{
  resize(0, hi);
}

template <class TYPE> inline void
GArray<TYPE>::touch(int n)
{
  if (hibound < lobound)
    {
      resize(n,n);
    }
  else
    {
      int nlo = lobound;
      int nhi = hibound;
      if (n < nlo) nlo = n;
      if (n > nhi) nhi = n;
      resize(nlo, nhi);
    }
}

template <class TYPE> void
GArray<TYPE>::resize(int lo, int hi)
{
  int i;
  int nsize = hi - lo + 1;
  // Validation
  if (nsize < 0)
    THROW("Invalid low and high bounds in GArray resize");
  // Destruction
  if (nsize == 0)
    {
      if (data && lobound<=hibound)
        for (int i=lobound; i<=hibound; i++)
          data[i-minlo].TYPE::~TYPE();
      operator delete((void*)data);
      data = 0;
      lobound = minlo = lo; 
      hibound = maxhi = hi; 
      return;
    }
  // Simple extension
  if (lo >= minlo && hi <= maxhi)
    {
      for (i=lo; i<lobound; i++)
        new ((void*)(&data[i-minlo])) TYPE;
      for (i=lobound; i<lo; i++)
        data[i-minlo].TYPE::~TYPE();
      for (i=hi; i>hibound; i--)
        new ((void*)(&data[i-minlo])) TYPE;
      for (i=hibound; i>hi; i--)
        data[i-minlo].TYPE::~TYPE();
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
  int bytesize = sizeof(TYPE)*(nmaxhi-nminlo+1);
  void *addr = operator new (bytesize);
  memset(addr, 0, bytesize);
  TYPE *ndata = (TYPE*)addr;
  // copy
  for (i=lo; i<=hi; i++)
    if (i>=lobound && i<=hibound)
      new ((void*)(&ndata[i-nminlo])) TYPE (data[i-minlo]);
    else
      new ((void*)(&ndata[i-nminlo])) TYPE;
  // destroys
  for (i=lobound; i<=hibound; i++)
    data[i-minlo].TYPE::~TYPE();
  // free and replace
  operator delete ((void*)(data));
  data = ndata;
  minlo = nminlo;
  maxhi = nmaxhi;
  lobound = lo;
  hibound = hi;
}

template <class TYPE> void
GArray<TYPE>::shift(int disp)
{
  lobound += disp;
  hibound += disp;
  minlo += disp;
  maxhi += disp;
}

template <class TYPE> void
GArray<TYPE>::del(int n, unsigned int howmany)
{
  if (howmany == 0)
    return;
  if ((int)(n + howmany) > hibound +1)
    THROW("Illegal arguments in GArray::del");
  int i;
  for (i=n; i<=(int)(hibound-howmany); i++)
    data[i-minlo] = data[i-minlo+howmany];
  for (i=hibound+1-howmany; i<=hibound; i++)
    data[i-minlo].TYPE::~TYPE();
  hibound = hibound - howmany;
}


template <class TYPE> void
GArray<TYPE>::ins(int n, const TYPE &val, unsigned int howmany)
{
  int i;
  int nhi = hibound + howmany;
  if (howmany == 0)
    return;
  if (maxhi < nhi)
    {
      int nmaxhi = maxhi;
      while (nmaxhi < nhi)
        nmaxhi += (nmaxhi < 8 ? 8 : (nmaxhi > 32768 ? 32768 : nmaxhi));
      int bytesize = sizeof(TYPE)*(nmaxhi-minlo+1);
      void *addr = operator new (bytesize);
      memset(addr, 0, bytesize);
      TYPE *ndata = (TYPE*)addr;
      for (i=lobound; i<=hibound; i++)
        new ((void*)(&ndata[i-minlo])) TYPE (data[i-minlo]);
      for (i=lobound; i<=hibound; i++)
        data[i-minlo].TYPE::~TYPE();
      operator delete ((void*)(data));
      data = ndata;
      maxhi = nmaxhi;
    }
  for (i=hibound+howmany; i>hibound; i--)
    {
      if (i-n >= (int)howmany)
        new ((void*)(&data[i-minlo])) TYPE (data[i-minlo-howmany]);
      else
        new ((void*)(&data[i-minlo])) TYPE (val);
    }
  for (i=hibound; i>=n; i--)
    {
      if (i-n >= (int)howmany)
        data[i-minlo] = data[i-minlo-howmany];
      else
        data[i-minlo] = val;
    }
  hibound = nhi;
}

template <class TYPE> GArray<TYPE>& 
GArray<TYPE>::operator= (const GArray<TYPE> &ga)
{
  if (&ga == this) return *this;
  empty();
  resize(ga.lobound, ga.hibound);
  for (int i = lobound; i <= hibound; i++)
    data[i-minlo] = ga.data[i-ga.minlo];
  return *this;
}



// ----- GSARRAY IMPLEMENTATION

template <class TYPE> void
GSArray<TYPE>::sort()
{
  sort(lobound, hibound);
}

template <class TYPE> void
GSArray<TYPE>::sort(int lo, int hi)
{
  if (hi <= lo)
    return;
  // Test for insertion sort (optimize!)
  if (hi <= lo + 20)
    {
      for (int i=lo+1; i<=hi; i++)
        {
          int j = i;
          TYPE tmp = (*this)[i];
          while ((--j>=lo) && !((*this)[j]<=tmp))
            (*this)[j+1] = (*this)[j];
          (*this)[j+1] = tmp;
        }
      return;
    }
  // -- determine suitable quick-sort pivot
  TYPE tmp = (*this)[lo];
  TYPE pivot = (*this)[(lo+hi)/2];
  if (pivot <= tmp)
    { tmp = pivot; pivot=(*this)[lo]; }
  if ((*this)[hi] <= tmp)
    { pivot = tmp; }
  else if ((*this)[hi] <= pivot)
    { pivot = (*this)[hi]; }
  // -- partition set
  int h = hi;
  int l = lo;
  while (l < h)
    {
      while (! (pivot <= (*this)[l])) l++;
      while (! ((*this)[h] <= pivot)) h--;
      if (l < h)
        {
          tmp = (*this)[l];
          (*this)[l] = (*this)[h];
          (*this)[h] = tmp;
          l = l+1;
          h = h-1;
        }
    }
  // -- recursively restart
  sort(lo, h);
  sort(l, hi);
}



// --------- GLIST IMPLEMENTATION

template <class TYPE>
GList<TYPE>::GList()
  :  nelem(0)
{
  head.next = &head;
  head.prev = &head;
}

template <class TYPE>
GList<TYPE>::GList(const GContainer<TYPE> &gc)
  :  nelem(0)
{
  head.next = &head;
  head.prev = &head;
  for (GPosition pos(gc); pos; ++pos)
    append(* gc.get(pos));
}

template <class TYPE>
GList<TYPE>::GList(const GList<TYPE> &gc)
  :  nelem(0)
{
  head.next = &head;
  head.prev = &head;
  GPosition pos(gc);
  const TYPE *pdata;
  while ((pdata = gc.next(pos)))
    append(*pdata);
}

template <class TYPE>
GList<TYPE>::~GList()
{
  GListNode *node = head.next;
  while (node != &head)
    {
      GListNode *next = node->next;
      delete node;
      node = next;
    }
}


template <class TYPE> inline int 
GList<TYPE>::size() const
{
  return nelem;
}

template <class TYPE> GPosition 
GList<TYPE>::firstpos() const
{
  GPosition pos;
  GListNode *p = head.next;
  pos.pv = (void*)p;
  if (p != &head) 
    pos.cont = this;
  return pos;
}

template <class TYPE> GPosition 
GList<TYPE>::lastpos() const
{
  GPosition pos;
  GListNode *p = head.prev;
  pos.pv = (void*)p;
  if (p != &head) 
    pos.cont = this;
  return pos;
}

template <class TYPE> void 
GList<TYPE>::nextpos(GPosition &pos) const
{
  if (pos.cont==this) { 
    GListNode *p = ((GListNode*)pos.pv)->next;
    pos.pv = (void*)p;
    if (p != &head) 
      return;
  }
  pos.cont = 0;
}

template <class TYPE> void 
GList<TYPE>::prevpos(GPosition &pos) const
{
  if (pos.cont==this) { 
    GListNode *p = ((GListNode*)pos.pv)->prev;
    pos.pv = (void*)p;
    if (p != &head) 
      return;
  }
  pos.cont = 0;
}

template <class TYPE> inline TYPE *
GList<TYPE>::get(const GPosition &pos) 
{
  if (pos.cont!=this)
    return 0;
  GListNode *ref = (GListNode*)pos.pv;
  return &ref->data;
}

template <class TYPE> inline  const TYPE *
GList<TYPE>::get(const GPosition &pos) const
{
  return ((GList<TYPE>*)this)->get(pos);
}

template <class TYPE> inline int
GList<TYPE>::isempty() const
{
  return !nelem;
}

template <class TYPE> inline TYPE& 
GList<TYPE>::operator[](GPosition pos)
{
  TYPE *ref = get(pos);
  if (! ref)
    THROW("Illegal GList subscript");
  return *ref;
}

template <class TYPE> inline const TYPE& 
GList<TYPE>::operator[](GPosition pos) const
{
  return ((GList<TYPE>*)(this))->operator[](pos);
}

template <class TYPE> int
GList<TYPE>::operator==(const GList<TYPE> &l2) const
{
  if (nelem != l2.nelem()) 
    return 0;
  GListNode *p = head.next;
  GListNode *q = l2.head.next;
  while (p != &head)
    {
      if (p->data != q->data)
        return 0;
      p = p->next;
      q = q->next;
    }
  return 1;
}

template <class TYPE> int 
GList<TYPE>::nth(unsigned int n, GPosition &pos) const
{
  if ((int)n >= nelem)
    return 0;
  GListNode *p = head.next;
  while (n-- > 0)
    p = p->next;
  pos.cont = this;
  pos.pv = (void*)(p);
  return 1;
}

template <class TYPE> int 
GList<TYPE>::contains(const TYPE& elt) const
{
   GPosition pos;
   return search(elt, pos);
}

template <class TYPE> int 
GList<TYPE>::search(const TYPE& elt, GPosition &pos) const
{
  GListNode *p = (GListNode*)(pos.pv);
  if (pos.cont!=this || p==&head)
    p = head.next;
  for (; p != &head; p = p->next)
    if (elt == p->data)
      break;
  if (p == &head)
    return 0;
  pos.cont = this;
  pos.pv = (void*)p;
  return 1;
}

template <class TYPE> void 
GList<TYPE>::empty()
{
  GListNode *node = head.next;
  while (node != &head)
    {
      GListNode *next = node->next;
      delete node;
      node = next;
    }
  nelem = 0;
  head.next = &head;
  head.prev = &head;
}

template <class TYPE> void 
GList<TYPE>::append(const TYPE& elt)
{
  GListNode *node = new GListNode(elt);
  node->next = &head;
  node->prev = head.prev;
  node->next->prev = node;
  node->prev->next = node;
  nelem += 1;
}

template <class TYPE> void 
GList<TYPE>::prepend(const TYPE& elt)
{
  GListNode *node = new GListNode(elt);
  node->next = head.next;
  node->prev = &head;
  node->next->prev = node;
  node->prev->next = node;
  nelem += 1;
}

template <class TYPE> GList<TYPE>& 
GList<TYPE>::operator= (const GList<TYPE>& gl)
{
  if (&gl == this) 
    return *this;
  empty();
  GListNode *p = gl.head.next;
  while (p != &gl.head)
    {
      append( p->data );
      p = p->next;
    }
  return *this;
}

template <class TYPE> void 
GList<TYPE>::insert_after(GPosition pos, const TYPE& elt)
{
  GListNode *ref = (GListNode*)(pos.pv);
  if (pos.cont!=this || ref==&head)
    THROW("Illegal GList subscript");
  GListNode *node = new GListNode(elt);
  node->prev = ref;
  node->next = ref->next;
  node->next->prev = node;
  node->prev->next = node;
  nelem += 1;
}

template <class TYPE> void 
GList<TYPE>::insert_before(GPosition pos, const TYPE& elt)
{
  GListNode *ref = (GListNode*)(pos.pv);
  if (pos.cont!=this || ref==&head)
    THROW("Illegal GList subscript");
  GListNode *node = new GListNode(elt);
  node->prev = ref->prev;
  node->next = ref;
  node->next->prev = node;
  node->prev->next = node;
  nelem += 1;
}

template <class TYPE> void 
GList<TYPE>::del(GPosition &pos)
{
  GListNode *ref = (GListNode*)(pos.pv);
  if (pos.cont==this && ref!=&head)
    {
      GListNode *before = ref->prev;
      GListNode *after = ref->next;
      before->next = after;
      after->prev = before;
      delete ref;
      nelem -= 1;
    }
}



// ---------- GMAP IMPLEMENTATION

template <class KTYPE, class VTYPE> 
GMap<KTYPE,VTYPE>::GMap()
  : nbuckets(17), nelems(0)
{
  typedef GMapNode *PGMN;
  table = new PGMN [nbuckets];
  for (int i=0; i<nbuckets; i++) 
    table[i] = 0;
}

template <class KTYPE, class VTYPE> 
GMap<KTYPE,VTYPE>::GMap(const GMap<KTYPE,VTYPE>&ref)
  : nbuckets(ref.nbuckets), nelems(ref.nelems)
{
  typedef GMapNode *PGMN;
  table = new PGMN [nbuckets];
  for (int i=0; i<nbuckets; i++) 
    table[i] = 0;
  for (GPosition pos(ref); pos; ++pos)
    this->operator[](ref.key(pos)) = ref[pos];
}

template <class KTYPE, class VTYPE> 
GMap<KTYPE,VTYPE>::~GMap()
{
  empty();
  delete [] table;
}


template <class KTYPE, class VTYPE> inline int 
GMap<KTYPE,VTYPE>::size() const
{
  return nelems;
}

template <class KTYPE, class VTYPE> GPosition 
GMap<KTYPE,VTYPE>::firstpos() const
{
  GPosition pos;
  pos.pi = 0;
  pos.pv = table[0];
  while (pos.pv==0 && ++pos.pi < nbuckets)
    pos.pv = (void*)(table[pos.pi]);
  if (pos.pv) pos.cont=this;
  return pos;
}

template <class KTYPE, class VTYPE> GPosition 
GMap<KTYPE,VTYPE>::lastpos() const
{
  THROW("GMap does not support backward iteration");
  GPosition pos;
  return pos;
}

template <class KTYPE, class VTYPE> void 
GMap<KTYPE,VTYPE>::nextpos(GPosition &pos) const
{
  if (pos.cont == this)
    {
      GMapNode *n = (GMapNode*)(pos.pv);
      pos.pv = (void*)(n->next);
      while (!pos.pv && ++pos.pi < nbuckets)
        pos.pv = (void*)(table[pos.pi]);
      if (!pos.pv)
        pos.cont = 0;
    }
}

template <class KTYPE, class VTYPE> void 
GMap<KTYPE,VTYPE>::prevpos(GPosition &pos) const
{
  THROW("GMap does not support backward iteration");
}

template <class KTYPE, class VTYPE> inline VTYPE* 
GMap<KTYPE,VTYPE>::get(const GPosition &pos)
{
  GMapNode *n = (GMapNode*)(pos.pv);
  if (pos.cont==this && n!=0)
    return &n->val;
  return 0;
}

template <class KTYPE, class VTYPE> inline const VTYPE *
GMap<KTYPE,VTYPE>::get(const GPosition &pos) const
{
  return ((GMap<KTYPE,VTYPE>*)this)->get(pos);
}

template <class KTYPE, class VTYPE> inline const KTYPE &
GMap<KTYPE,VTYPE>::key(const GPosition &pos) const
{
  GMapNode *n = (GMapNode*)(pos.pv);
  if (! (pos.cont==this && n!=0))
    THROW("Illegal GMap subscript"); 
  return n->key;
}

template <class KTYPE, class VTYPE> inline int
GMap<KTYPE,VTYPE>::isempty() const
{
  return !nelems;
}

template <class KTYPE, class VTYPE> int
GMap<KTYPE,VTYPE>::contains(const KTYPE &key, GPosition &pos) const
{
  GMapNode *n = (GMapNode*)search(key, &pos.pi);
  pos.pv = (void*)n;
  pos.cont = (n ? this : 0);
  return (pos.cont != 0);
}

template <class KTYPE, class VTYPE> int
GMap<KTYPE,VTYPE>::contains(const KTYPE &key) const
{
  GMapNode *n = (GMapNode*)search(key, 0);
  return (n != 0);
}

template <class KTYPE, class VTYPE> void
GMap<KTYPE,VTYPE>::empty()
{
  GMapNode *p;
  nelems = 0;
  for (int i=0; i<nbuckets; i++) 
    while ((p = table[i])) {
      table[i] = p->next;
      delete p;
    }
}

template <class KTYPE, class VTYPE> inline VTYPE&
GMap<KTYPE,VTYPE>::operator[](const GPosition &pos)
{
  VTYPE *ret = get(pos);
  if (!ret) THROW("Illegal GMap subscript");
  return *ret;
}

template <class KTYPE, class VTYPE> const VTYPE&
GMap<KTYPE,VTYPE>::operator[](const GPosition &pos) const
{
  return ((GMap<KTYPE,VTYPE>*)this)->operator[](pos); 
}

template <class KTYPE, class VTYPE> VTYPE&
GMap<KTYPE,VTYPE>::operator[](const KTYPE &key)
{
  GMapNode *n = (GMapNode*)search_create(key,0);
  return n->val;
}

template <class KTYPE, class VTYPE> const VTYPE&
GMap<KTYPE,VTYPE>::operator[](const KTYPE &key) const
{
  GMapNode *n = (GMapNode*)search(key,0);
  if (!n) THROW("Cannot create key into const GMap");
  return n->val;
}

template <class KTYPE, class VTYPE> void
GMap<KTYPE,VTYPE>::del(GPosition &pos)
{
  GMapNode *n = (GMapNode*)(pos.pv);
  if (pos.cont==this && n)
    {
      if (n->next) n->next->pprev=n->pprev;
      *(n->pprev) = n->next;
      delete n;
      nelems -= 1;
      pos.cont = 0;
    }
}

template <class KTYPE, class VTYPE> void
GMap<KTYPE,VTYPE>::del(const KTYPE &key)
{
  GMapNode *n = (GMapNode*)search(key,0);
  if (n)
    {
      if (n->next) n->next->pprev=n->pprev;
      *(n->pprev) = n->next;
      delete n;
      nelems -= 1;
    }
}

template <class KTYPE, class VTYPE> GMap<KTYPE,VTYPE>& 
GMap<KTYPE,VTYPE>::operator=(const GMap<KTYPE,VTYPE>&ref)
{
  if (&ref == this) return *this;
  empty();
  for (GPosition pos=ref.firstpos(); pos; ++pos)
    (*this)[ref.key(pos)] = ref[pos];
}

template <class KTYPE, class VTYPE> void 
GMap<KTYPE,VTYPE>::rehash(int newsize)
{
  typedef GMapNode *PGMN;
  GMapNode **ntable = new PGMN [newsize];
  memset(ntable, 0, sizeof(PGMN)*newsize);
  for (int i=0; i<nbuckets; i++)
    {
      GMapNode *n = table[i];
      while (n)
        {
          GMapNode *p = n->next;
          int bucket = n->hash % newsize;
          n->pprev = &ntable[bucket];
          if ((n->next = ntable[bucket]))
            n->next->pprev = &(n->next);
          ntable[bucket] = n;
          n = p;
        }
    }
  delete [] table;
  table = ntable;
  nbuckets = newsize;
}

template <class KTYPE, class VTYPE> void *
GMap<KTYPE,VTYPE>::search(const KTYPE &key, int *pbucket) const 
{
  unsigned int hashcode = hash(key);
  int bucket = hashcode % nbuckets;
  GMapNode *n;
  for (n=table[bucket]; n; n=n->next)
    if (hashcode == n->hash)
      if (key == n->key)
        break;
  if (pbucket)
    *pbucket = bucket;
  return n;
}

template <class KTYPE, class VTYPE> void *
GMap<KTYPE,VTYPE>::search_create(const KTYPE &key, int *pbucket)
{
  unsigned int hashcode = hash(key);
  int bucket = hashcode % nbuckets;
  GMapNode *n;
  for (n=table[bucket]; n; n=n->next)
    if (hashcode == n->hash)
      if (key == n->key)
        break;
  if (n==0)
    {
      if (nelems*3 > nbuckets*2)
        rehash(2*nbuckets - 1);
      bucket = hashcode % nbuckets;
      n = new GMapNode(key);
      n->hash = hashcode;
      n->pprev = &table[bucket];
      if ((n->next = table[bucket]))
        n->next->pprev = &(n->next);
      table[bucket] = n;
      nelems += 1;
    }
  if (pbucket)
    *pbucket = bucket;
  return n;
}




// ------------ THE END

#endif

