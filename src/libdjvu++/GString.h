//C-  -*- C++ -*-
//C- DjVu® Reference Library (v. 3.0)
//C- 
//C- Copyright © 1999-2001 LizardTech, Inc. All Rights Reserved.
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
// $Id: GString.h,v 1.40 2001-04-13 00:41:16 bcr Exp $
// $Name:  $

#ifndef _GSTRING_H_
#define _GSTRING_H_

/** @name GString.h
    
    Files #"GString.h"# and #"GString.cpp"# implement a general purpose string
    class \Ref{GString}. This implementation relies on smart pointers (see
    \Ref{GSmartPointer.h}).
    
    {\bf Historical Comments} --- At some point during the DjVu research era,
    it became clear that C++ compilers rarely provided portable libraries. We
    then decided to avoid fancy classes (like #iostream# or #string#) and to
    rely only on the good old C library.  A good string class however is very
    useful.  We had already randomly picked letter 'G' to prefix class names
    and we logically derived the new class name.  Native English speakers kept
    laughing in hiding.  This is ironic because we completely forgot this
    letter 'G' when creating more challenging things like the ZP Coder or the
    IW44 wavelets.
    
    @memo
    General purpose string class.
    @author
    L\'eon Bottou <leonb@research.att.com> -- initial implementation.
    @version
    #$Id: GString.h,v 1.40 2001-04-13 00:41:16 bcr Exp $# */
//@{

#ifdef __GNUC__
#pragma interface
#endif

#include "DjVuGlobal.h"
#include "GContainer.h"

//include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#ifdef WIN32
#include <windows.h>
#endif


// Internal string representation.
class GStringRep : public GPEnabled
{
public:
  class Native;
  class UTF8;
  friend Native;
  friend UTF8;
  friend class GString;
  friend unsigned int hash(const GString &ref);

public:
  // default constructor
  GStringRep(void);
  // virtual destructor
  virtual ~GStringRep();

    // Other virtual methods.
      // Create an empty string.
  virtual GP<GStringRep> blank(const unsigned int sz = 0) const;
      // Append a string.
  virtual GP<GStringRep> append(const GP<GStringRep> &s2) const;
      // Test if isUTF8.
  virtual bool isUTF8(void) const { return false; }
      // Test if Native.
  virtual bool isNative(void) const { return false; }
      // Convert to Native.
  virtual GP<GStringRep> toNative(const bool noconvert=false) const;
      // Convert to UTF8.
  virtual GP<GStringRep> toUTF8(const bool noconvert=false) const;
      // Convert to same as current class.
  virtual GP<GStringRep> toThis(
    const GP<GStringRep> &rep,const GP<GStringRep> &locale=0) const;
      // Compare with #s2#.
  virtual int cmp(const GP<GStringRep> &s2) const;

    // Create an empty string.
  template <class TYPE> static GP<GStringRep> create(
    const unsigned int sz,TYPE *);
  static GP<GStringRep> create(const unsigned int sz = 0)
  { return create(sz,(GStringRep *)0); }

    // Creates with a strdup string.
  GP<GStringRep> strdup(const char *s) const;

    // Creates by appending to the current string
  GP<GStringRep> append(const char *s2) const;

  static GP<GStringRep> create(const char *s)
  { GStringRep dummy; return dummy.strdup(s); }

    // Creates with a concat operation.
  GP<GStringRep> concat(const GP<GStringRep> &s1,const GP<GStringRep> &s2) const;
  GP<GStringRep> concat(const char *s1,const GP<GStringRep> &s2) const;
  GP<GStringRep> concat(const GP<GStringRep> &s1,const char *s2) const;
  GP<GStringRep> concat(const char *s1,const char *s2) const;

  static GP<GStringRep> create(
    const GP<GStringRep> &s1,const GP<GStringRep> &s2)
  { GStringRep dummy; return dummy.concat(s1,s2); }
  static GP<GStringRep> create( const GP<GStringRep> &s1,const char *s2)
  { GStringRep dummy; return dummy.concat(s1,s2); }
  static GP<GStringRep> create( const char *s1, GP<GStringRep> &s2)
  { GStringRep dummy; return dummy.concat(s1,s2); }
  static GP<GStringRep> create(const char *s1,const char *s2)
  { GStringRep dummy; return dummy.concat(s1,s2); }

   /* Creates with a strdup and substr.  Negative values have strlen(s)+1
      added to them.
   */
  GP<GStringRep> substr(
    const char *s,const int start,const int length=(-1)) const;

  static GP<GStringRep> create(
    const char *s,const int start,const int length=(-1))
  { GStringRep dummy; return dummy.substr(s,start,length); }

  static GP<GStringRep> UTF8ToNative( const char *s );
  static GP<GStringRep> NativeToUTF8( const char *s );

  // Creates an uppercase version of the current string.
  GP<GStringRep> upcase(void) const;
  // Creates a lowercase version of the current string.
  GP<GStringRep> downcase(void) const;

  static unsigned long UTF8toUCS4(
    unsigned char const *&s, void const * const eptr );

  static unsigned char *UCS4toUTF8(
    const unsigned long w,unsigned char *ptr);


  int cmp(const char *s2) const; 
  static int cmp(const GP<GStringRep> &s1, const GP<GStringRep> &s2) ;
  static int cmp(const GP<GStringRep> &s1, const char *s2);
  static int cmp(const char *s1, const GP<GStringRep> &s2);

private:
  int  size;
  char *data;
};

inline GP<GStringRep>
GStringRep::blank(const unsigned int sz = 0) const
{ 
  return create(sz);
}

inline GP<GStringRep>
GStringRep::toThis(
  const GP<GStringRep> &rep,const GP<GStringRep> &locale=0) const
{
   return (locale?(locale->toThis(rep)):rep);
}

class GStringRep::Native : public GStringRep
{
public:
  // default constructor
  Native(void);
  // virtual destructor
  virtual ~Native();

    // Other virtual methods.
      // Create an empty string.
  virtual GP<GStringRep> blank(const unsigned int sz = 0) const;
      // Append a string.
  virtual GP<GStringRep> append(const GP<GStringRep> &s2) const;
      // Test if Native.
  virtual bool isNative(void) const;
      // Convert to Native.
  virtual GP<GStringRep> toNative(const bool nothrow=false) const;
      // Convert to same as current class.
  virtual GP<GStringRep> toThis(
     const GP<GStringRep> &rep,const GP<GStringRep> &) const;
      // Compare with #s2#.
  virtual int cmp(const GP<GStringRep> &s2) const;


    // Create an empty string
  static GP<GStringRep> create(const unsigned int sz = 0)
  { return GStringRep::create(sz,(GStringRep::Native *)0); }

    // Create a strdup string.
  static GP<GStringRep> create(const char *s)
  { GStringRep::Native dummy; return dummy.strdup(s); }

  // Creates by appending to the current string

   // Creates with a concat operation.
  static GP<GStringRep> create(
    const GP<GStringRep> &s1,const GP<GStringRep> &s2)
  { GStringRep::Native dummy; return dummy.concat(s1,s2); }
  static GP<GStringRep> create( const GP<GStringRep> &s1,const char *s2)
  { GStringRep::Native dummy; return dummy.concat(s1,s2); }
  static GP<GStringRep> create( const char *s1, GP<GStringRep> &s2)
  { GStringRep::Native dummy; return dummy.concat(s1,s2); }
  static GP<GStringRep> create(const char *s1,const char *s2)
  { GStringRep::Native dummy; return dummy.concat(s1,s2); }

    // Create with a strdup and substr operation.
  static GP<GStringRep> create(
    const char *s,const int start,const int length=(-1))
  { GStringRep::Native dummy; return dummy.substr(s,start,length); }


  friend class GString;
};

inline GP<GStringRep> 
GStringRep::Native::blank(const unsigned int sz = 0) const
{
   return GStringRep::create(sz,(GStringRep::Native *)0);
}

inline bool
GStringRep::Native::isNative(void) const
{
  return true;
}

inline GP<GStringRep>
GStringRep::Native::toThis(
     const GP<GStringRep> &rep,const GP<GStringRep> &) const
{
  return rep?(rep->toNative(true)):rep;
}

class GStringRep::UTF8 : public GStringRep
{
public:
  // default constructor
  UTF8(void);
  // virtual destructor
  virtual ~UTF8();

    // Other virtual methods.
  virtual GP<GStringRep> blank(const unsigned int sz = 0) const;
  virtual GP<GStringRep> append(const GP<GStringRep> &s2) const;
      // Test if Native.
  virtual bool isUTF8(void) const;
      // Convert to UTF8.
  virtual GP<GStringRep> toUTF8(const bool nothrow=false) const;
      // Convert to same as current class.
  virtual GP<GStringRep> toThis(
    const GP<GStringRep> &rep,const GP<GStringRep> &) const;
      // Compare with #s2#.
  virtual int cmp(const GP<GStringRep> &s2) const;

    // Create an empty string.
  static GP<GStringRep> create(const unsigned int sz = 0)
  { return GStringRep::create(sz,(GStringRep::UTF8 *)0); }

    // Create a strdup string.
  static GP<GStringRep> create(const char *s)
  { GStringRep::UTF8 dummy; return dummy.strdup(s); }

   // Creates with a concat operation.
  static GP<GStringRep> create(
    const GP<GStringRep> &s1,const GP<GStringRep> &s2)
  { GStringRep::UTF8 dummy; return dummy.concat(s1,s2); }
  static GP<GStringRep> create( const GP<GStringRep> &s1,const char *s2)
  { GStringRep::UTF8 dummy; return dummy.concat(s1,s2); }
  static GP<GStringRep> create( const char *s1, GP<GStringRep> &s2)
  { GStringRep::UTF8 dummy; return dummy.concat(s1,s2); }
  static GP<GStringRep> create( const char *s1,const char *s2)
  { GStringRep::UTF8 dummy; return dummy.concat(s1,s2); }

  static GP<GStringRep> create(
    const char *s,const int start,const int length=(-1))
  { GStringRep::UTF8 dummy; return dummy.substr(s,start,length); }
  // Creates by appending to the current string

  friend class GString;
};

inline GP<GStringRep> 
GStringRep::UTF8::blank(const unsigned int sz = 0) const
{
   return GStringRep::create(sz,(GStringRep::UTF8 *)0);
}

inline bool
GStringRep::UTF8::isUTF8(void) const
{
  return true;
}

inline GP<GStringRep> 
GStringRep::UTF8::toThis(
    const GP<GStringRep> &rep,const GP<GStringRep> &) const
{
  return rep?(rep->toUTF8(true)):rep;
}

/** General purpose character string.
    Each instance of class #GString# represents a character string.
    Overloaded operators provide a value semantic to #GString# objects.
    Conversion operators and constructors transparently convert between
    #GString# objects and #const char*# pointers.

    Functions taking strings as arguments should declare their arguments as
    "#const char*#".  Such functions will work equally well with #GString#
    objects since there is a fast conversion operator from #GString# to
    "#const char*#".  Functions returning strings should return #GString#
    objects because the class will automatically manage the necessary memory.
    
    Characters in the string can be identified by their position.  The first
    character of a string is numbered zero. Negative positions represent
    characters relative to the end of the string (i.e. position #-1# accesses
    the last character of the string, position #-2# represents the second last
    character, etc.)  */

class GUTF8String;
class GNativeString;
    
class GString : protected GP<GStringRep> 
{
public: 
  friend GUTF8String;
  friend GNativeString;

  // Sets the gstr pointer;
  void init(void);

  // -- CONSTRUCTORS
  /** Null constructor. Constructs an empty string. */
  GString( void ) { init(); } 
  /// Construct from parent class.
  GString( const GP<GStringRep> &rep);
  /// Copy constructor. Constructs a string by copying the string #gs#.
  GString(const GString &gs)
    : GP<GStringRep>(gs) { init(); } 
// private: // -- Eventually this will be private.
  /// Constructs a string from a character.
  GString(const char dat)
    : GP<GStringRep>(GStringRep::create(&dat,0,1)) { init(); }
  /// Constructs a string from a null terminated character array.
  GString(const char *dat)
    : GP<GStringRep>(GStringRep::create(dat)) { init(); }
  /// Constructs a string from a null terminated character array.
  GString(const unsigned char *dat)
    : GP<GStringRep>(GStringRep::create((const char *)dat)) { init(); }
  /** Constructs a string from a character array.  Elements of the character
      array #dat# are added into the string until the string length reaches
      #len# or until encountering a null character (whichever comes first). */
  GString(const char *dat, unsigned int len)
    : GP<GStringRep>(GStringRep::create(dat,0,((int)len<0)?(-1):(int)len))
    { init(); }

  /** Construct a string by copying a sub-string. The string will be
      initialized with at most #len# characters from string #gs# starting at
      position #from#.  The length of the constructed string may be smaller
      than #len# if the specified range is too large. */
  GString(const GString &gs, int from, unsigned int len)
    : GP<GStringRep>(GStringRep::create(gs,from,((int)len<0)?(-1):(int)len))
    { init(); }

  /** Constructs a string with a human-readable representation of integer
      #number#.  The format is similar to format #"%d"# in function
      #printf#. */
  GString(const int number)
  { format("%d",number); }

  /** Constructs a string with a human-readable representation of floating
      point number #number#. The format is similar to format #"%f"# in
      function #printf#.  */
  GString(const double number)
  { format("%f",number); }
public:
  /** Constructs a string with a formatted string (as in #vprintf#).  The
      string is re-initialized with the characters generated according to the
      specified format #fmt# and using the optional arguments.  See the ANSI-C
      function #vprintf()# for more information. The current implementation
      will cause a segmentation violation if the resulting string is longer
      than 32768 characters. */
  GString(const char fmt[], va_list args);
  // -- SEARCHING
  
  // -- COPY OPERATOR
  GString& operator= (const GP<GStringRep> &rep);

  /** Copy operator. Resets this string with the value of the string #gs#.
      This operation is efficient because string memory is allocated using a
      "copy-on-write" strategy. Both strings will share the same segment of
      memory until one of the strings is modified. */
  GString& operator= (const GString &gs);

  /** Copy a null terminated character array, in the native mbs format.
      Resets this string with the character string contained in the null
      terminated character array #str#. */
  GString& assignNative(const char *str)
  { return ((*this)=GStringRep::Native::create(str)); }

  /** Copy a null terminated character array, in the native mbs format.
      Resets this string with the character string contained in the null
      terminated character array #str#. */
  GString& assignUTF8(const char *str)
  { return ((*this)=GStringRep::UTF8::create(str)); }

  // -- ACCESS
  /** Converts a string into a constant null terminated character array.  This
      conversion operator is very efficient because it simply returns a
      pointer to the internal string data. The returned pointer remains valid
      as long as the string is unmodified. */
  operator const char* ( void ) const  ;
  /// Returns the string length.
  unsigned int length( void ) const;
  /** Returns true if and only if the string contains zero characters.  This
      operator is useful for conditional expression in control structures.
      \begin{verbatim}
         if (! str) { ... }
         while (!! str) { ... }  -- Note the double operator!
      \end{verbatim}
      Class #GString# does not to support syntax "#if# #(str)# #{}#" because
      the required conversion operator introduces dangerous ambiguities with
      certain compilers. */
  bool operator! ( void ) const;

  // -- INDEXING
  /** Returns the character at position #n#. An exception \Ref{GException} is
      thrown if number #n# is not in range #-len# to #len-1#, where #len# is
      the length of the string.  The first character of a string is numbered
      zero.  Negative positions represent characters relative to the end of
      the string. */
  char operator[] (int n) const
    { if (n<0 && ptr) n += (*this)->size;
      if (n<0 || !ptr || n>=(int)(*this)->size) throw_illegal_subscript();
      return ((*this)->data)[n]; }
  /** Set the character at position #n# to value #ch#.  An exception
      \Ref{GException} is thrown if number #n# is not in range #-len# to
      #len#, where #len# is the length of the string.  If character #ch# is
      zero, the string is truncated at position #n#.  The first character of a
      string is numbered zero. Negative positions represent characters
      relative to the end of the string. If position #n# is equal to the
      length of the string, this function appends character #ch# to the end of
      the string. */
  void setat(int n, char ch);
  /// Returns #TRUE# if the string contains an integer number.
  bool is_int(void) const;
  /// Returns #TRUE# if the string contains a float number.
  bool is_float(void) const;
  // -- DERIVED STRINGS
  /** Returns a sub-string.  The sub-string is composed by copying #len#
      characters starting at position #from# in this string.  The length of
      the resulting string may be smaller than #len# if the specified range is
      too large. */
  GString substr(int from, unsigned int len=1) const
    { return GString(*this, from, len); }
  /** Returns an upper case copy of this string.  The returned string
      contains a copy of the current string with all letters turned into 
      upper case letters. */
  GString upcase( void ) const;
  /** Returns an lower case copy of this string.  The returned string
      contains a copy of the current string with all letters turned into 
      lower case letters. */
  GString downcase( void ) const;

  /** Converts strings between native & UTF8 **/
  GNativeString getUTF82Native( char* tocode=NULL ) const;/*MBCS*/
  GUTF8String getNative2UTF8( const char* fromcode="" ) const;/*MBCS*/

  // -- ALTERING
  /// Reinitializes a string with the null string.
  void empty( void );
  /** Provides a direct access to the string buffer.  Returns a pointer for
      directly accessing the string buffer.  This pointer valid remains valid
      as long as the string is not modified by other means.  Positive values
      for argument #n# represent the length of the returned buffer.  The
      returned string buffer will be large enough to hold at least #n#
      characters plus a null character.  If #n# is positive but smaller than
      the string length, the string will be truncated to #n# characters. */
  char *getbuf(int n = -1);
  /** Initializes a string with a formatted string (as in #printf#).  The
      string is re-initialized with the characters generated according to the
      specified format #fmt# and using the optional arguments.  See the ANSI-C
      function #printf()# for more information. The current implementation
      will cause a segmentation violation if the resulting string is longer
      than 32768 characters. */
  GString &format(const char *fmt, ... );
  /** Initializes a string with a formatted string (as in #vprintf#).  The
      string is re-initialized with the characters generated according to the
      specified format #fmt# and using the optional arguments.  See the ANSI-C
      function #vprintf()# for more information. The current implementation
      will cause a segmentation violation if the resulting string is longer
      than 32768 characters. */
  GString &format(const char *fmt, va_list args);
  // -- SEARCHING
  /** Searches character #c# in the string, starting at position #from# and
      scanning forward until reaching the end of the string.  This function
      returns the position of the matching character.  It returns #-1# if
      character #c# cannot be found. */
  int search(char c, int from=0) const;
  /** Searches sub-string #str# in the string, starting at position #from# and
      scanning forward until reaching the end of the string.  This function
      returns the position of the first matching character of the sub-string.
      It returns #-1# if string #str# cannot be found. */
  int search(const char *str, int from=0) const;
  /** Searches character #c# in the string, starting at position #from# and
      scanning backwards until reaching the beginning of the string.  This
      function returns the position of the matching character.  It returns
      #-1# if character #c# cannot be found. */
  int rsearch(char c, int from=-1) const;
  /** Searches sub-string #str# in the string, starting at position #from# and
      scanning backwards until reaching the beginning of the string.  This
      function returns the position of the first matching character of the
      sub-string. It returns #-1# if string #str# cannot be found. */
  int rsearch(const char *str, int from=-1) const;
  /** Searches for any of the specified characters in the accept string.
      It returns #-1# if the none of the characters and be found, otherwise
      the position of the first match. */
  int contains(const char accept[], const int from=-1) const;

  // -- CONCATENATION
  /// Appends character #ch# to the string.
  GString& operator+= (char ch);
  /// Appends the null terminated character array #str# to the string.
  GString& operator+= (const char *str);
  /// Appends the specified GString to the string.
  GString& operator+= (const GString &str);

  /** Concatenates strings. Returns a string composed by concatenating
      the characters of strings #s1# and #s2#. */
  friend GString operator+(const GString &s1, const GString &s2) 
    { return GStringRep::create(s1,s2); }
  friend GString operator+(const GString &s1, const char    *s2) 
    { return GStringRep::create(s1,s2); }
  friend GString operator+(const char    *s1, const GString &s2) 
    { return GStringRep::create(s1,s2); }

  // -- COMPARISONS
  int cmp(const GString &s2) const
    { return GStringRep::cmp(*this,s2); }
  int cmp(const char *s2) const
    { return GStringRep::cmp(*this,s2); }
  int cmp(const char s2) const
    { return GStringRep::cmp(*this,GStringRep::create(&s2,1)); }

  /** String comparison. Returns true if and only if character strings #s1#
      and #s2# are equal (as with #strcmp#.)
    */
  friend bool operator==(const GString &s1, const GString &s2) 
    { return !s1.cmp(s2); }
  friend bool operator==(const GString &s1, const char *s2) 
    { return !s1.cmp(s2); }
  friend bool operator==(const GString &s1, const char s2) 
    { return !s1.cmp(s2); }
  friend bool operator==(const char    *s1, const GString &s2) 
    { return !s2.cmp(s1); }
  friend bool operator==(const char s1, const GString &s2) 
    { return !s2.cmp(s1); }

  /** String comparison. Returns true if and only if character strings #s1#
      and #s2# are not equal (as with #strcmp#.)
    */
  friend bool operator!=(const GString &s1, const GString &s2)
    { return !!s1.cmp(s2); }
  friend bool operator!=(const GString &s1, const char *s2)
    { return !!s1.cmp(s2); }
  friend bool operator!=(const GString &s1, const char s2)
    { return !!s1.cmp(s2); }
  friend bool operator!=(const char *s1, const GString &s2)
    { return !!s2.cmp(s1); }
  friend bool operator!=(const char s1, const GString &s2)
    { return !!s2.cmp(s1); }

  /** String comparison. Returns true if and only if character strings #s1# is
      lexicographically greater than or equal to string #s2# (as with #strcmp#.)
   */
  friend bool operator>=(const GString &s1, const GString &s2) 
    { return (s1.cmp(s2)>=0); }
  friend bool operator>=(const GString &s1, const char *s2) 
    { return (s1.cmp(s2)>=0); }
  friend bool operator>=(const GString &s1, const char s2) 
    { return (s1.cmp(s2)>=0); }
  friend bool operator>=(const char    *s1, const GString &s2)
    { return (s2.cmp(s1)<=0); }
  friend bool operator>=(const char s1, const GString &s2)
    { return (s2.cmp(s1)<=0); }

  /** String comparison. Returns true if and only if character strings #s1# is
      lexicographically less than string #s2# (as with #strcmp#.)
   */
  friend bool operator<(const GString &s1, const GString &s2)
    { return (s1.cmp(s2)<0); }
  friend bool operator<(const GString &s1, const char *s2)
    { return (s1.cmp(s2)<0); }
  friend bool operator<(const GString &s1, const char s2)
    { return (s1.cmp(s2)<0); }
  friend bool operator<(const char *s1, const GString &s2)
    { return (s2.cmp(s1)>0); }
  friend bool operator<(const char s1, const GString &s2)
    { return (s2.cmp(s1)>0); }

  /** String comparison. Returns true if and only if character strings #s1# is
      lexicographically greater than string #s2# (as with #strcmp#.)
   */
  friend bool operator> (const GString &s1, const GString &s2)
    { return (s1.cmp(s2)>0); }
  friend bool operator> (const GString &s1, const char *s2)
    { return (s1.cmp(s2)>0); }
  friend bool operator> (const GString &s1, const char s2)
    { return (s1.cmp(s2)>0); }
  friend bool operator> (const char    *s1, const GString &s2)
    { return (s2.cmp(s1)<0); }
  friend bool operator> (const char s1, const GString &s2)
    { return (s2.cmp(s1)<0); }

  /** String comparison. Returns true if and only if character strings #s1# is
      lexicographically less than or equal to string #s2# (as with #strcmp#.)
   */
  friend bool operator<=(const GString &s1, const GString &s2)
    { return !(s1>s2); }
  friend bool operator<=(const GString &s1, const char *s2)
    { return !(s1>s2); }
  friend bool operator<=(const GString &s1, const char s2)
    { return !(s1>s2); }
  friend bool operator<=(const char    *s1, const GString &s2)
    { return !(s1>s2); }
  friend bool operator<=(const char    s1, const GString &s2)
    { return !(s1>s2); }

   /** Returns a boolean.  Compares string with #s2# and a given length
       of #len#
   */
   inline bool ncmp(const GString& s2, const int len=1) const
      { return (substr(0,len) == s2.substr(0,len)); }
   /** Returns a boolean. The Standard C strncmp takes two string and compares the 
       first N characters.  static bool GString::ncmp will compare #s1# with #s2# 
       with the #len# characters starting from the beginning of the string.*/
   static bool ncmp(const GString &s1,const GString &s2, const int len=1)
      { return (s1.substr(0,len) == s2.substr(0,len)); }
   /** Returns an integer.  Implements a functional i18n atoi. Note that if you pass
       a GString that is not in Native format the results may be disparaging. */
   inline int nativeToInt() const
     { return atoi((const char*)(*this)); }
   static int nativeToInt( const GString& src )
     { return atoi((const char*)(src)); }
   /** Returns an integer.  Implements i18n atoi.  Takes a UTF8 string and converts that
       value into a native format string, which is then used to make the atoi call. */
   int toInt(void) const;
   static int toInt( const GString& src );
   /* # END code block added by CHRISP */

  // -- HASHING
  /** Returns a hash code for the string.  This hashing function helps when
      creating associative maps with string keys (see \Ref{GMap}).  This hash
      code may be reduced to an arbitrary range by computing its remainder
      modulo the upper bound of the range. */
  friend unsigned int hash(const GString &ref);
  // -- HELPERS
  friend GStringRep;
protected:
  const char *gstr;
  static void throw_illegal_subscript() no_return;
  static const char *nullstr;
  GNativeString UTF8ToNative(const bool currentlocale=false) const;
  GUTF8String NativeToUTF8(void) const;
};

class GUTF8String : public GString
{
public:
  GUTF8String(void) { init(); }
  GUTF8String(const char *str)
    : GString(GStringRep::UTF8::create(str)) { init(); }
  GUTF8String(const unsigned char *str)
    : GString(GStringRep::UTF8::create((const char *)str)) { init(); }
  GUTF8String(const GP<GStringRep> &str);
  GUTF8String(const GString &str); 
  GUTF8String(const GUTF8String &str)
    : GString((GP<GStringRep>)str) { init(); }
  GUTF8String(const GNativeString &str);
  /// Constructs a string from a character.
  GUTF8String(const char dat)
    : GString(GStringRep::UTF8::create(&dat,0,1)) { init(); }

  /** Constructs a string from a character array.  Elements of the character
      array #dat# are added into the string until the string length reaches
      #len# or until encountering a null character (whichever comes first). */
  GUTF8String(const char *dat, unsigned int len)
    : GString(GStringRep::UTF8::create(dat,0,((int)len<0)?(-1):(int)len))
  { init(); }

  /** Construct a string by copying a sub-string. The string will be
      initialized with at most #len# characters from string #gs# starting at
      position #from#.  The length of the constructed string may be smaller
      than #len# if the specified range is too large. */
  GUTF8String(const GString &gs, int from, unsigned int len)
    : GString(GStringRep::UTF8::create(gs,from,((int)len<0)?(-1):(int)len))
  { init(); }

  /** Constructs a string with a human-readable representation of integer
      #number#.  The format is similar to format #"%d"# in function
      #printf#. */
  GUTF8String(const int number)
  { format("%d",number); }

  /** Constructs a string with a human-readable representation of floating
      point number #number#. The format is similar to format #"%f"# in
      function #printf#.  */
  GUTF8String(const double number)
  { format("%f",number); }

  /** Copy a null terminated character array. Resets this string with the
      character string contained in the null terminated character array
      #str#. */
  GUTF8String& operator= (GUTF8String &str)
  { 
    GP<GStringRep>(*this)=str;
    init();
    return *this;
  }

  template <class TYPE>
  GUTF8String& operator= (const TYPE &str)
  { return (*this=GUTF8String(str)); }


  /** Returns a copy of this string with characters used in XML with
      '<'  to "&lt;", '>'  to "&gt;",  '&' to "&amp;" '\'' to "&apos;",
      and  '\"' to  "&quot;".   Characters 0x01 through 0x1f are also
      escaped. */
  GUTF8String toEscaped(void ) const;

  /** Converts strings containing HTML/XML escaped characters into their
      unescaped forms. Numeric representations of characters (e.g., "&#38;"
      or "&#x26;" for "*") are the only forms converted by this function. */
  GUTF8String fromEscaped( void ) const;

  /** Converts strings containing HTML/XML escaped characters (e.g.,
      "&lt;" for "<") into their unescaped forms. The conversion is partially
      defined by the ConvMap argument which specifies the conversion strings
      to be recognized. Numeric representations of
      characters (e.g., "&#38;" or "&#x26;" for "*") are always converted. */
  GUTF8String fromEscaped( const GMap<GUTF8String,GUTF8String> ConvMap ) const;

   /** Returns a boolean. The Standard C strncmp takes two string and compares the 
       first N characters.  static bool GString::ncmp will compare #s1# with #s2# 
       with the #len# characters starting from the beginning of the string.*/
   static bool ncmp(const GUTF8String &s1,const GUTF8String &s2, const int len=1)
      { return (s1.substr(0,len) == s2.substr(0,len)); }

  // -- CONCATENATION
  /// Appends character #ch# to the string.
  GUTF8String& operator+= (char ch)
  {
    return (*this=GStringRep::UTF8::create(*this,GStringRep::UTF8::create(&ch,1)));
  }

  /// Appends the null terminated character array #str# to the string.
  GUTF8String& operator+= (const char *str)
  {
    return (*this=GStringRep::UTF8::create(*this,str));
  }
  /// Appends the specified GString to the string.
  GUTF8String& operator+= (const GString &str)
  { 
    return (*this=GStringRep::UTF8::create(*this,str));
  }

  /** Concatenates strings. Returns a string composed by concatenating
      the characters of strings #s1# and #s2#.
  */
  friend GUTF8String operator+(const GUTF8String &s1, const GUTF8String &s2) 
    { return GStringRep::UTF8::create(s1,s2); }
  friend GUTF8String operator+(const GUTF8String &s1, const char    *s2) 
    { return GStringRep::UTF8::create(s1,s2); }
  friend GUTF8String operator+(const char    *s1, const GUTF8String &s2) 
    { return GStringRep::UTF8::create(s1,s2); }
};

class GNativeString : public GString
{
public:
  GNativeString(void)
  { init(); }
  GNativeString(const char *str)
    : GString(GStringRep::Native::create(str))
  { init(); }
  GNativeString(const unsigned char *str)
    : GString(GStringRep::Native::create((const char *)str))
  { init(); }
  GNativeString(const GP<GStringRep> &str);
  GNativeString(const GString &str); 
  GNativeString(const GUTF8String &str);
  GNativeString(const GNativeString &str) 
    : GString((GP<GStringRep>)str) { init(); }
  /** Constructs a string from a character array.  Elements of the character
      array #dat# are added into the string until the string length reaches
      #len# or until encountering a null character (whichever comes first). */
  GNativeString(const char *dat, unsigned int len)
    : GString(GStringRep::Native::create(dat,0,((int)len<0)?(-1):(int)len))
  { init(); }
  GNativeString(const char dat)
    : GString(GStringRep::Native::create(&dat,0,1)) { init(); }

  /** Constructs a string with a human-readable representation of integer
      #number#.  The format is similar to format #"%d"# in function
      #printf#. */
  GNativeString(const int number) { format("%d",number); }

  /** Constructs a string with a human-readable representation of floating
      point number #number#. The format is similar to format #"%f"# in
      function #printf#.  */
  GNativeString(const double number) { format("%f",number); }

  /** Construct a string by copying a sub-string. The string will be
      initialized with at most #len# characters from string #gs# starting at
      position #from#.  The length of the constructed string may be smaller
      than #len# if the specified range is too large. */
  GNativeString(const GString &gs, int from, unsigned int len)
    : GString(GStringRep::Native::create(gs,from,((int)len<0)?(-1):(int)len))
  { init(); }

  /** Copy a null terminated character array. Resets this string with the
      character string contained in the null terminated character array
      #str#. */
  GNativeString& operator= (GNativeString &str)
  { 
    GP<GStringRep>(*this)=str;
    init();
    return *this;
  }

  template <class TYPE>
  GNativeString& operator= (const TYPE &str)
  { return (*this=GNativeString(str)); }

   /** Returns a boolean. The Standard C strncmp takes two string and compares the 
       first N characters.  static bool GString::ncmp will compare #s1# with #s2# 
       with the #len# characters starting from the beginning of the string.*/
   static bool ncmp(
     const GNativeString &s1,const GNativeString &s2, const int len=1)
     { return (s1.substr(0,len) == s2.substr(0,len)); }

  // -- CONCATENATION
  /// Appends character #ch# to the string.
  GNativeString& operator+= (char ch)
  {
    return (*this=GStringRep::Native::create(*this,GStringRep::UTF8::create(&ch,1)));
  }
  /// Appends the null terminated character array #str# to the string.
  GNativeString& operator+= (const char *str)
  {
    return (*this=GStringRep::Native::create(*this,str));
  }
  /// Appends the specified GString to the string.
  GNativeString& operator+= (const GString &str)
  { 
    return (*this=GStringRep::Native::create(*this,str));
  }

  friend GNativeString operator+(const GNativeString &s1, const GNativeString &s2) 
    { return GStringRep::Native::create(s1,s2); }
  friend GNativeString operator+(const GNativeString &s1, const char    *s2) 
    { return GStringRep::Native::create(s1,s2); }
  friend GNativeString operator+(const char    *s1, const GNativeString &s2) 
    { return GStringRep::Native::create(s1,s2); }
};

//@}

inline
GString::operator const char* ( void ) const  
{
  return ptr?(*this)->data:nullstr;
}

inline GString&
GString::operator= (const GString &gs)
{
  GP<GStringRep>::operator=(gs);
  gstr=(*this)->data;
  return *this;
}

inline unsigned int
GString::length( void ) const 
{
  return ptr ? (*this)->size : 0;
}

inline bool
GString::operator! ( void ) const
{
  return !ptr;
}

inline GString 
GString::upcase( void ) const
{ 
  return (ptr?GString((*this)->upcase()):(*this));
}

inline GString 
GString::downcase( void ) const
{ 
  return (ptr?GString((*this)->downcase()):(*this));
}

inline int 
GString::toInt(void) const
{ return getUTF82Native().nativeToInt(); }

inline int
GString::toInt( const GString& src )
{ return src.getUTF82Native().nativeToInt(); }
   /* # END code block added by CHRISP */

inline
GUTF8String::GUTF8String(const GNativeString &str)
: GString((str.length()?(str->toNative(true)):(GP<GStringRep>)str))
{ init(); }

inline
GNativeString::GNativeString(const GUTF8String &str)
: GString((str.length()?(str->toUTF8(true)):(GP<GStringRep>)str))
{ init(); }

inline
GUTF8String::GUTF8String(const GP<GStringRep> &str)
: GString(str?(str->toNative(true)):str)
{ init(); }

inline
GNativeString::GNativeString(const GP<GStringRep> &str)
: GString(str?(str->toNative(true)):str)
{ init(); }

inline
GUTF8String::GUTF8String(const GString &str)
: GString((str.length()?(str->toNative(true)):(GP<GStringRep>)str))
{ init(); }

inline
GNativeString::GNativeString(const GString &str)
: GString((str.length()?(str->toUTF8(true)):(GP<GStringRep>)str))
{ init(); }

inline void
GString::init(void)
{
  gstr=ptr?((*this)->data):nullstr;
}

inline GP<GStringRep> 
GStringRep::UTF8ToNative( const char *s )
{
  return GStringRep::UTF8::create(s)->toNative();
}

inline GP<GStringRep> 
GStringRep::NativeToUTF8( const char *s )
{
  return GStringRep::Native::create(s)->toUTF8();
}

inline int
GStringRep::cmp(const char *s1) const
{
  return s1?strcmp(data,s1):1;
}

inline int 
GStringRep::cmp(const GP<GStringRep> &s1, const GP<GStringRep> &s2)
{
  return (s1?(s1->cmp(s2)):(s2?(-1):0));
}

inline int 
GStringRep::cmp(const GP<GStringRep> &s1, const char *s2)
{
  return (s1?(s1->cmp(s2)):(s2?(-1):0));
}

inline int 
GStringRep::cmp(const char *s1, const GP<GStringRep> &s2)
{
  return (s2?(-s2->cmp(s1)):(s1?1:0));
}

// ------------------- The end

#endif



