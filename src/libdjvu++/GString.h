//C-  -*- C++ -*-
//C-
//C-  Copyright (c) 1998 AT&T	
//C-  All Rights Reserved 
//C-
//C-  THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
//C-  The copyright notice above does not evidence any
//C-  actual or intended publication of such source code.
//C-
//C-  $Id: GString.h,v 1.3 1999-02-01 18:32:33 leonb Exp $


#ifndef _GSTRING_H_
#define _GSTRING_H_

/** @name GString.h
    
    Files #"GString.h"# and #"GString.cpp"# implement a general purpose string
    class \Ref{GString}. This implementation relies on smart pointers (see
    \Ref{GSmartPointer.h}).
    
    {\bf Historical Comments} --- At some point during the DjVu research era,
    it became clear that C++ compilers rarely provided portable libraries. I
    then decided to avoid fancy classes (like #iostream# or #string#) and to
    rely only on the good old C library.  A good string class however is very
    useful.  We had already randomly picked letter 'G' to prefix class names
    and derived the new class name #GString#.  Native english speakers kept
    laughing in hiding and did not tell me anything until it was too late to
    change (how naive...).  This is ironic because I completely forgot this
    letter 'G' when creating more challenging things like #ZPCodec# or
    #IWCodec#.
    
    @memo
    General purpose string class.
    @author
    Leon Bottou <leonb@research.att.com> -- initial implementation.
    @version
    #$Id: GString.h,v 1.3 1999-02-01 18:32:33 leonb Exp $# */
//@{

#include "DjVuGlobal.h"
#include <string.h>
#include <stdlib.h>
#include "GException.h"
#include "GSmartPointer.h"

#ifdef __GNUC__
#pragma interface
#endif

// Internal string representation.

class GStringRep : public GPEnabled
{
  friend class GString;
  friend unsigned int hash(const GString &ref);
public:
  virtual ~GStringRep();
  static GStringRep *xnew(unsigned int sz = 0);
protected:
  virtual void destroy();
private:
  unsigned int size;
  char data[1];
};



/** General purpose character strings.
    Each instance of class #GString# represents a character string.
    Overloaded operators provide a value semantic to #GString# objects.
    Conversion operators and constructors transparently convert between
    #GString# objects and #const char*# pointers.

    Functions taking strings as arguments should declare their arguments as
    "#const char*#". Such functions will work equally well with #GString#
    objects since there is a fast conversion operator from #GString# to 
    "#const char*#". Functions returning strings however should return 
    #GString# objects in order to avoid managing the memory necessary 
    to represent the returned string.
    
    Characters in the string can be identified by their position. 
    The first character of a string
    is numbered zero. Negative values for argument #from# represent
    characters relative to the end of the string (i.e. position #-1#
    accesses the last character of the string, position #-2# represents
    the second last character, etc.)  */

    
class GString : protected GP<GStringRep> 
{
public: 
  // -- CONSTRUCTORS
  /** Null constructor. Constructs an empty string (with zero characters). */
  GString();
  /** Copy constructor. Constructs a string by copying #gs#. */
  GString(const GString &gs);
  /** Constructs a string from a zero terminated character array. */ 
  GString(const char *dat);
  /** Constructs a string from a character array.  Elements of the character
      array #dat# are added into the string until the string length reaches
      #len# or until encountering a null character (whichever comes first). */
  GString(const char *dat, unsigned int len);
  /** Construct a string by copying a sub-string. The string will be
      initialized with at most #len# characters from string #gs# starting at
      position #from#.  The length of the constructed string is smaller than
      #len# when the specified range is too large. */
  GString(const GString &gs, int from, unsigned int len);
  /** Constructs a string with a human-readable representation of integer
      #number#. */
  GString(const int number);
  /** Constructs a string with a human-readable representation of floating
      point number #number#. */
  GString(const double number);
  // -- COPY OPERATOR
  /** Copy operator. Resets the string with the value of string #gs#.  This
      operation is efficient : both strings will share the same segment of
      memory with a copy-on-demand scheme. */
  GString& operator= (const GString &gs);
  /** Copy a zero terminated character array. Resets the string with the
      character string contained in the zero terminated character array
      #str#. */
  GString& operator= (const char *str);
  // -- ACCESS
  /** Converts a string into a constant zero terminated character array.  This
      conversion operator is very efficient. The returned pointer remains
      valid as long as the string is unmodified. */
  operator const char* () const  { return (*this)->data; }
  /** Returns the string length. */
  unsigned int length() const { return (*this)->size; }
  /** Returns true if and only if the string contains zero characters.  This
      operator is useful for conditional expression in control structures.
      \begin{verbatim}
         if (! str) { ... }
         while (!! str) { ... }  -- Note the double operator!
      \end{verbatim}
      We chose not to support syntax "#if# #(str)# #{}#" because the required
      conversion operator introduces dangerous ambiguities with certain
      compilers. */
  int operator! () const { return ((*this)->data[0] == 0); }
  // -- INDEXING
  /** Returns the character at position #n#. An exception
      \Ref{GException} is thrown if number #n# is not in range #-len# to
      #len-1#, where #len# is the length of the string. */
  char operator[] (int n) const;
  /** Set the character at position #n# to value #ch#.  An exception
      \Ref{GException} is thrown if number #n# is not in range #-len# to
      #len#, where #len# is the length of the string.  If character #ch# is
      zero, the string is truncated at position #n#.  If position #n# is equal
      to the length of the string, the character #ch# is appended to the end
      of the string. */
  void setat(int n, char ch);
  // -- DERIVED STRINGS
  /** Returns a sub-string.  The sub-string is composed by copying #len#
      characters starting at position #from# in this string.  The length of
      the resulting string can be smaller than #len# when the specified range 
      is too large. */
  GString substr(int from, unsigned int len=1) const;
  /** Returns an upper case copy of this string.  The returned string
      contains a copy of the current string with all letters turned into 
      upper case letters. */
  GString upcase() const;
  /** Returns an lower case copy of this string.  The returned string
      contains a copy of the current string with all letters turned into 
      loweer case letters. */
  GString downcase() const;
  // -- ALTERING
  /** Reinitializes a string with the null string. */
  void empty();
  /** Provides a direct access to the string buffer.  Returns a pointer for
      directly accessing the string buffer.  This pointer valid remains valid
      as long as the string is not modified by other means.  Argument #n#
      represents the minimum length of the returned buffer.  If #n# is greater
      than the string length, the returned string buffer will be large enough to
      hold at least #n# characters plus a zero termination. If #n# is positive
      but smaller than the string length, the string will be truncated to #n#
      characters. */
  char *getbuf(int n = -1);
  /** Initializes a string with a formatted string (as in #printf#).  The
      string is re-initialized with the characters generated according to the
      specified format #fmt# and using the optional arguments.  See the Ansi-C
      function #printf()# for more information.  The current implementation
      will not work with strings longer than 4096 characters. */
  void format(const char *fmt, ... );
  // -- SEARCHING
  /* Searches character #c# in the strin scanning forward from position #from#
     until reaching the end of the string.  This function returns the position
     of the matching character.  It returns #-1# if character #c# cannot be
     found. */
  int search(char c, int from=0) const;
  /* Searches sub-string #str# in the string scanning forward from position
     #from# until reaching the end of the string.  This function returns the
     position of the first matching character of the sub-string.  It returns
     #-1# if character #c# cannot be found. */
  int search(const char *str, int from=0) const;
  /* Searches character #c# in the strin scanning backward from position
     #from# until reaching the beginning of the string.  This function returns
     the position of the matching character.  It returns #-1# if character #c#
     cannot be found. */
  int rsearch(char c, int from=-1) const;
  /* Searches sub-string #str# in the strin scanning backward from position
     #from# until reaching the beginning of the string.  This function returns
     the position of the first matching character of the sub-string. It
     returns #-1# if character #c# cannot be found. */
  int rsearch(const char *str, int from=-1) const;
  // -- CONCATENATION
  /** Appends character #ch# to the string. */
  GString& operator+= (char ch);
  /** Appends the zero terminated character array #str# to the string. */
  GString& operator+= (const char *str);
  /** Concatenates strings. Returns a string composed by concatenating
      the characters of strings #s1# and #s2#. */
  friend GString operator+(const GString &s1, const GString &s2) 
    { return GString::concat(s1,s2);}
  friend GString operator+(const GString &s1, const char    *s2) 
    { return GString::concat(s1,s2);}
  friend GString operator+(const char    *s1, const GString &s2) 
    { return GString::concat(s1,s2);}
  // -- COMPARISONS
  /** String comparison. Returns true if and only if character strings #s1# and #s2#
      are equal (in the sense of #strcmp#.) */
  friend int operator==(const GString &s1, const GString &s2) 
    { return strcmp(s1,s2)==0; }
  friend int operator==(const GString &s1, const char    *s2) 
    { return strcmp(s1,s2)==0; }
  friend int operator==(const char    *s1, const GString &s2) 
    { return strcmp(s1,s2)==0; }
  /** String comparison. Returns true if and only if character strings #s1# and #s2#
      are not equal (in the sense of #strcmp#.) */
  friend int operator!=(const GString &s1, const GString &s2)
    { return strcmp(s1,s2)!=0; }
  friend int operator!=(const GString &s1, const char    *s2)
    { return strcmp(s1,s2)!=0; }
  friend int operator!=(const char    *s1, const GString &s2) 
    { return strcmp(s1,s2)!=0; }
  /** String comparison. Returns true if and only if character strings #s1# is
      lexicographically greater than or equal to string #s2# (as with #strcmp#.) */
  friend int operator>=(const GString &s1, const GString &s2) 
    { return strcmp(s1,s2)>=0; }
  friend int operator>=(const GString &s1, const char    *s2) 
    { return strcmp(s1,s2)>=0; }
  friend int operator>=(const char    *s1, const GString &s2)
    { return strcmp(s1,s2)>=0; }
  /** String comparison. Returns true if and only if character strings #s1# is
      lexicographically greater than string #s2# (as with #strcmp#.) */
  friend int operator> (const GString &s1, const GString &s2)
    { return strcmp(s1,s2)> 0; }
  friend int operator> (const GString &s1, const char    *s2) 
    { return strcmp(s1,s2)> 0; }
  friend int operator> (const char    *s1, const GString &s2)
    { return strcmp(s1,s2)> 0; }
  /** String comparison. Returns true if and only if character strings #s1# is
      lexicographically lesser than or equal to string #s2# (as with #strcmp#.) */
  friend int operator<=(const GString &s1, const GString &s2)
    { return strcmp(s1,s2)<=0; }
  friend int operator<=(const GString &s1, const char    *s2) 
    { return strcmp(s1,s2)<=0; }
  friend int operator<=(const char    *s1, const GString &s2)
    { return strcmp(s1,s2)<=0; }
  /** String comparison. Returns true if and only if character strings #s1# is
      lexicographically lesser than string #s2# (as with #strcmp#.) */
  friend int operator< (const GString &s1, const GString &s2)
    { return strcmp(s1,s2)< 0; }
  friend int operator< (const GString &s1, const char    *s2)
    { return strcmp(s1,s2)< 0; }
  friend int operator< (const char    *s1, const GString &s2) 
    { return strcmp(s1,s2)< 0; }
  // -- HASHING
  /** Returns a hash code for the string.  This is useful for creating
      associative maps with string keys (see \Ref{GMap}). */
  friend unsigned int hash(const GString &ref);
  // -- HELPERS
protected:
  GString(GStringRep* rep);
  GString& operator= (GStringRep *rep);
  static GString concat (const char *str1, const char *str2);
};

//@}


// ------------------- Inline functions

inline
GString::GString(GStringRep *rep)
  : GP<GStringRep> ( rep )
{
}

inline
GString::GString(const GString &gs)
  : GP<GStringRep> ( gs )
{
}

inline GString& 
GString::operator= (GStringRep *rep)
{
  GP<GStringRep>::operator= (rep);
  return *this;
}

inline  GString& 
GString::operator= (const GString &gs)
{
  GP<GStringRep>::operator= (gs);
  return *this;
}

inline GString 
GString::substr(int from, unsigned int len) const
{
  return GString((GString&)*this, from, len);
}

inline char
GString::operator[] (int n) const
{
  if (n < 0)  
    n += (*this)->size;
  if (n < 0 || n>=(int)(*this)->size)
    THROW("Illegal bound in GString subscript");
  return (*this)->data[n];
}

inline 
GString & GString::operator+=(char ch)
{
   setat((*this)->size, ch);
   return *this;
}


// ------------------- The end

#endif



