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
// $Id: GString.h,v 1.64 2001-04-24 16:54:49 bcr Exp $
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
    #$Id: GString.h,v 1.64 2001-04-24 16:54:49 bcr Exp $# */
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
#include <wchar.h>

#if defined(__linux__) || defined(WIN32)
#define HAS_MBSTATE 1
#endif
#ifndef HAS_MBSTATE
typedef int mbstate_t;
#endif // HAS_MBSTATE

// Internal string representation.
class GStringRep : public GPEnabled
{
public:
  class Native;
  class UTF8;
  class ChangeLocale;
  friend Native;
  friend UTF8;
  friend class GBaseString;
  friend unsigned int hash(const GBaseString &ref);

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
  virtual int cmp(const GP<GStringRep> &s2,const int len=(-1)) const;

  // Convert strings to numbers.
  virtual int toInt() const;
  virtual long int toLong(
    GP<GStringRep>& endptr, bool &isLong, const int base=10) const;
  virtual unsigned long int toULong(
    GP<GStringRep>& endptr, bool &isULong, const int base=10) const;
  virtual double toDouble(
    GP<GStringRep>& endptr, bool &isDouble) const;

  // return next non space position
  virtual int nextNonSpace( const int from=0 ) const;

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
  static GP<GStringRep> create( const char *s1, const GP<GStringRep> &s2)
  { GStringRep dummy; return dummy.concat(s1,s2); }
  static GP<GStringRep> create(const char *s1,const char *s2)
  { GStringRep dummy; return dummy.concat(s1,s2); }

   /* Creates with a strdup and substr.  Negative values have strlen(s)+1
      added to them.
   */
  GP<GStringRep> substr(
    const char *s,const int start,const int length=(-1)) const;

  static GP<GStringRep> create(
    const char *s,const int start,const int length)
  { GStringRep dummy; return dummy.substr(s,start,length); }

  /** Initializes a string with a formatted string (as in #vprintf#).  The
      string is re-initialized with the characters generated according to the
      specified format #fmt# and using the optional arguments.  See the ANSI-C
      function #vprintf()# for more information. The current implementation
      will cause a segmentation violation if the resulting string is longer
      than 32768 characters. */
  GP<GStringRep> format(va_list &args) const;
  static GP<GStringRep> create_format(const char fmt[],...);
  static GP<GStringRep> create(const char fmt[],va_list &args);

  // -- SEARCHING

  static GP<GStringRep> UTF8ToNative( const char *s );
  static GP<GStringRep> NativeToUTF8( const char *s );

  // Creates an uppercase version of the current string.
  GP<GStringRep> upcase(void) const;
  // Creates a lowercase version of the current string.
  GP<GStringRep> downcase(void) const;

  /** Returns the next UCS4 character, and updates the pointer s. */
  static unsigned long UTF8toUCS4(
    unsigned char const *&s, void const * const endptr );

  /** Returns the number of bytes in next UCS4 character, 
      and sets #w# to the next UCS4 chacter.  */
  static int UTF8toUCS4(
    unsigned long &w, unsigned char const s[], void const * const endptr )
  { unsigned char const *r=s;w=UTF8toUCS4(r,endptr);return (int)((size_t)r-(size_t)s); }

  /** Returns the next UCS4 word from the UTF16 string. */
  static int UTF16toUCS4(
     unsigned long &w, unsigned short const * const s,void const * const eptr);

  static int UCS4toUTF16(
    unsigned long w, unsigned short &w1, unsigned short &w2);

  int cmp(const char *s2, const int len=(-1)) const; 
  static int cmp(
    const GP<GStringRep> &s1, const GP<GStringRep> &s2, const int len=(-1)) ;
  static int cmp(
    const GP<GStringRep> &s1, const char *s2, const int len=(-1));
  static int cmp(
    const char *s1, const GP<GStringRep> &s2, const int len=(-1));
  static int cmp(
    const char *s1, const char *s2, const int len=(-1));

  // Lookup the next character, and return the position of the next character.
  int getUCS4(unsigned long &w, const int from) const;

  virtual unsigned char *UCS4toString(
    const unsigned long w,unsigned char *ptr, mbstate_t *ps=0) const;

  static unsigned char *UCS4toUTF8(
    const unsigned long w,unsigned char *ptr);

  static unsigned char *UCS4toNative(
    const unsigned long w,unsigned char *ptr, mbstate_t *ps);

  int search(char c, int from=0) const;

  int search(char const *str, int from=0) const;

  int rsearch(char c, int from=0) const;

  int rsearch(char const *str, int from=0) const;

protected:
  // Return the next character and increment the source pointer.
  virtual unsigned long getValidUCS4(const char *&source) const;

  GP<GStringRep> tocase(
    int (*xiswcase)(wint_t wc), wint_t (*xtowcase)(wint_t wc)) const;
private:
  int  size;
  char *data;
};

inline GP<GStringRep>
GStringRep::blank(const unsigned int sz) const
{ 
  return create(sz);
}

inline GP<GStringRep>
GStringRep::toThis(
  const GP<GStringRep> &rep,const GP<GStringRep> &locale) const
{
   return (locale?(locale->toThis(rep)):rep);
}

inline GP<GStringRep> 
GStringRep::create(const char fmt[],va_list &args)
{ 
  GP<GStringRep> s=create(fmt);
  return (s?(s->format(args)):s);
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
  virtual int cmp(const GP<GStringRep> &s2, const int len=(-1)) const;

  // Convert strings to numbers.
  virtual int toInt() const;
  virtual long int toLong(
    GP<GStringRep>& endptr, bool &isLong, const int base) const;
  virtual unsigned long int toULong(
    GP<GStringRep>& endptr, bool &isULong, const int base) const;
  virtual double toDouble(
    GP<GStringRep>& endptr, bool &isDouble) const;

  // return position of next non space.
  virtual int nextNonSpace( const int from=0 ) const;

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
  static GP<GStringRep> create( const char *s1, const GP<GStringRep> &s2)
  { GStringRep::Native dummy; return dummy.concat(s1,s2); }
  static GP<GStringRep> create(const char *s1,const char *s2)
  { GStringRep::Native dummy; return dummy.concat(s1,s2); }

    // Create with a strdup and substr operation.
  static GP<GStringRep> create(
    const char *s,const int start,const int length=(-1))
  { GStringRep::Native dummy; return dummy.substr(s,start,length); }

    // Create with an sprintf()
  static GP<GStringRep> create_format(const char fmt[],...);
  static GP<GStringRep> create(const char fmt[],va_list &args);

  virtual unsigned char *UCS4toString(
    const unsigned long w,unsigned char *ptr, mbstate_t *ps=0) const;

  friend class GBaseString;
protected:
  // Return the next character and increment the source pointer.
  virtual unsigned long getValidUCS4(const char *&source) const;
};

inline GP<GStringRep> 
GStringRep::Native::blank(const unsigned int sz) const
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

inline GP<GStringRep> 
GStringRep::Native::create(const char fmt[],va_list &args)
{ 
  GP<GStringRep> s=create(fmt);
  return (s?(s->format(args)):s);
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
  virtual int cmp(const GP<GStringRep> &s2,const int len=(-1)) const;

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
  static GP<GStringRep> create( const char *s1, const GP<GStringRep> &s2)
  { GStringRep::UTF8 dummy; return dummy.concat(s1,s2); }
  static GP<GStringRep> create( const char *s1,const char *s2)
  { GStringRep::UTF8 dummy; return dummy.concat(s1,s2); }

  static GP<GStringRep> create(
    const char *s,const int start,const int length=(-1))
  { GStringRep::UTF8 dummy; return dummy.substr(s,start,length); }

  static GP<GStringRep> create_format(const char fmt[],...);
  static GP<GStringRep> create(const char fmt[],va_list args);

  // return position of next non space.
  virtual int nextNonSpace( const int from=0 ) const;

  virtual unsigned char *UCS4toString(
    const unsigned long w,unsigned char *ptr, mbstate_t *ps=0) const;

  friend class GBaseString;
protected:
  // Return the next character and increment the source pointer.
  virtual unsigned long getValidUCS4(const char *&source) const;
};

inline GP<GStringRep> 
GStringRep::UTF8::blank(const unsigned int sz) const
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

inline GP<GStringRep> 
GStringRep::UTF8::create(const char fmt[],va_list args)
{ 
  GP<GStringRep> s=create(fmt);
  return (s?(s->format(args)):s);
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
    
class GBaseString : protected GP<GStringRep> 
{
public: 
  friend GUTF8String;
  friend GNativeString;
protected:
  // Sets the gstr pointer;
  void init(void);

  GBaseString &init(const GP<GStringRep> &rep)
  { GP<GStringRep>::operator=(rep); init(); return *this;}

  // -- CONSTRUCTORS
  /** Null constructor. Constructs an empty string. */
  GBaseString( void );

public:
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
      Class #GBaseString# does not to support syntax "#if# #(str)# #{}#" because
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
  // -- SEARCHING
  /** Searches character #c# in the string, starting at position #from# and
      scanning forward until reaching the end of the string.  This function
      returns the position of the matching character.  It returns #-1# if
      character #c# cannot be found. */
  int search(char c, int from=0) const
  { return ptr?((*this)->search(c,from)):(-1); }

  /** Searches sub-string #str# in the string, starting at position #from# and
      scanning forward until reaching the end of the string.  This function
      returns the position of the first matching character of the sub-string.
      It returns #-1# if string #str# cannot be found. */
  int search(const char *str, int from=0) const
  { return ptr?((*this)->search(str,from)):(-1); }

  /** Searches character #c# in the string, starting at position #from# and
      scanning backwards until reaching the beginning of the string.  This
      function returns the position of the matching character.  It returns
      #-1# if character #c# cannot be found. */
  int rsearch(char c, int from=-1) const
  { return ptr?((*this)->rsearch(c,from)):(-1); }
  /** Searches sub-string #str# in the string, starting at position #from# and
      scanning backwards until reaching the beginning of the string.  This
      function returns the position of the first matching character of the
      sub-string. It returns #-1# if string #str# cannot be found. */
  int rsearch(const char *str, int from=-1) const
  { return ptr?((*this)->rsearch(str,from)):(-1); }
  /** Searches for any of the specified characters in the accept string.
      It returns #-1# if the none of the characters and be found, otherwise
      the position of the first match. */
  int contains(const char accept[], const int from=-1) const;

  /** Concatenates strings. Returns a string composed by concatenating
      the characters of strings #s1# and #s2#. */
  GUTF8String operator+(const GUTF8String &s2) const;
  GNativeString operator+(const GNativeString &s2) const;

  /** Returns an integer.  Implements i18n atoi.  */
  int toInt(void) const
  { return ptr?(*this)->toInt():0; }

  /** Returns a long intenger.  Implments i18n strtol.  */
  long int toLong(
    GBaseString& endptr, bool &isLong, const int base=10) const
  { return ptr?(*this)->toLong(endptr, isLong, base):0; }

  /** Returns a unsigned long integer.  Implements i18n strtoul. */
  unsigned long int toULong(
    GBaseString& endptr, bool &isULong, const int base=10) const
  { return ptr?(*this)->toLong(endptr, isULong, base):0; }

  /** Returns a double.  Implements the i18n strtod.  */
  double toDouble( GBaseString& endptr, bool& isDouble ) const
  { return ptr?(*this)->toDouble(endptr,isDouble):(double)0; }

  // -- HASHING

  // -- COMPARISONS
    /// Returns an #int#.  Compares string with #s2# and returns sorting order.
  int cmp(const GBaseString &s2, const int len=(-1)) const
    { return GStringRep::cmp(*this,s2,len); }
    /// Returns an #int#.  Compares string with #s2# and returns sorting order.
  int cmp(const char *s2, const int len=(-1)) const
    { return GStringRep::cmp(*this,s2,len); }
    /// Returns an #int#.  Compares string with #s2# and returns sorting order.
  int cmp(const char s2) const
    { return GStringRep::cmp(*this,&s2,1); }
    /// Returns an #int#.  Compares #s2# with #s2# and returns sorting order.
  static int cmp(const char *s1, const char *s2, const int len=(-1))
    { return GStringRep::cmp(s1,s2,len); }
  /** Returns a boolean.  Compares string with #s2# and a given length
      of #len# */
//  bool ncmp(const GBaseString& s2, const int len) const
//    { return !cmp(s2,len); }
//  bool ncmp(const char *s2, const int len) const
//    { return !cmp(s2,len); }
  /** Returns a boolean. The Standard C strncmp takes two string and
      compares the first N characters.  static bool GBaseString::ncmp will
      compare #s1# with #s2# 
      with the #len# characters starting from the beginning of the string.*/
//  static bool ncmp(const char *s1, const char *s2, const int len)
//    { return !GStringRep::cmp(s1,s2,len); }

  /** String comparison. Returns true if and only if character strings #s1#
      and #s2# are equal (as with #strcmp#.)
    */
  bool operator==(const GBaseString &s2) const
    { return !cmp(s2); }
  bool operator==(const char *s2) const
    { return !cmp(s2); }
  bool operator==(const char s2)  const
    { return !cmp(s2); }
  friend bool operator==(const char    *s1, const GBaseString &s2) 
    { return !s2.cmp(s1); }
  friend bool operator==(const char s1, const GBaseString &s2) 
    { return !s2.cmp(s1); }

  /** String comparison. Returns true if and only if character strings #s1#
      and #s2# are not equal (as with #strcmp#.)
    */
  bool operator!=(const GBaseString &s2) const
    { return !!cmp(s2); }
  bool operator!=(const char *s2) const
    { return !!cmp(s2); }
  bool operator!=(const char s2) const
    { return !!cmp(s2); }
  friend bool operator!=(const char *s1, const GBaseString &s2)
    { return !!s2.cmp(s1); }
  friend bool operator!=(const char s1, const GBaseString &s2)
    { return !!s2.cmp(s1); }

  /** String comparison. Returns true if and only if character strings #s1# is
      lexicographically greater than or equal to string #s2# (as with #strcmp#.)
   */
  bool operator>=(const GBaseString &s2) const
    { return (cmp(s2)>=0); }
  bool operator>=(const char *s2) const
    { return (cmp(s2)>=0); }
  bool operator>=(const char s2) const
    { return (cmp(s2)>=0); }
  friend bool operator>=(const char    *s1, const GBaseString &s2)
    { return (s2.cmp(s1)<=0); }
  friend bool operator>=(const char s1, const GBaseString &s2)
    { return (s2.cmp(s1)<=0); }

  /** String comparison. Returns true if and only if character strings #s1# is
      lexicographically less than string #s2# (as with #strcmp#.)
   */
  bool operator<(const GBaseString &s2) const
    { return (cmp(s2)<0); }
  bool operator<(const char *s2) const
    { return (cmp(s2)<0); }
  bool operator<(const char s2) const
    { return (cmp(s2)<0); }
  friend bool operator<(const char *s1, const GBaseString &s2)
    { return (s2.cmp(s1)>0); }
  friend bool operator<(const char s1, const GBaseString &s2)
    { return (s2.cmp(s1)>0); }

  /** String comparison. Returns true if and only if character strings #s1# is
      lexicographically greater than string #s2# (as with #strcmp#.)
   */
  bool operator> (const GBaseString &s2) const
    { return (cmp(s2)>0); }
  bool operator> (const char *s2) const
    { return (cmp(s2)>0); }
  bool operator> (const char s2) const
    { return (cmp(s2)>0); }
  friend bool operator> (const char    *s1, const GBaseString &s2)
    { return (s2.cmp(s1)<0); }
  friend bool operator> (const char s1, const GBaseString &s2)
    { return (s2.cmp(s1)<0); }

  /** String comparison. Returns true if and only if character strings #s1# is
      lexicographically less than or equal to string #s2# (as with #strcmp#.)
   */
  bool operator<=(const GBaseString &s2) const
    { return (cmp(s2)<=0); }
  bool operator<=(const char *s2) const
    { return (cmp(s2)<=0); }
  bool operator<=(const char s2) const
    { return (cmp(s2)<=0); }
  friend bool operator<=(const char    *s1, const GBaseString &s2)
    { return !(s1>s2); }
  friend bool operator<=(const char    s1, const GBaseString &s2)
    { return !(s1>s2); }

   /** Returns an integer.  Implements a functional i18n atoi. Note that if
       you pass a GBaseString that is not in Native format the results may be
       disparaging. */

  /** Returns a hash code for the string.  This hashing function helps when
      creating associative maps with string keys (see \Ref{GMap}).  This hash
      code may be reduced to an arbitrary range by computing its remainder
      modulo the upper bound of the range. */
  friend unsigned int hash(const GBaseString &ref);
  // -- HELPERS
  friend GStringRep;

  /// Returns next non space position.
  int nextNonSpace( const int from=0 ) const
  { return ptr?(*this)->nextNonSpace(from):0; }
protected:
  const char *gstr;
  static void throw_illegal_subscript() no_return;
  static const char *nullstr;
  GNativeString UTF8ToNative(const bool currentlocale=false) const;
  GUTF8String NativeToUTF8(void) const;
};

class GUTF8String : public GBaseString
{
public:
  void init(void)
  { GBaseString::init(); }

  GUTF8String &init(const GP<GStringRep> &rep)
  { GP<GStringRep>::operator=(rep?rep->toUTF8(true):rep); init(); return *this; }

  // -- CONSTRUCTORS
  /** Null constructor. Constructs an empty string. */
  GUTF8String(void);
  /// Constructs a string from a character.
  GUTF8String(const char dat);
  /// Constructs a string from a null terminated character array.
  GUTF8String(const char *str);
  /// Constructs a string from a null terminated character array.
  GUTF8String(const unsigned char *str);
  /** Constructs a string from a character array.  Elements of the character
      array #dat# are added into the string until the string length reaches
      #len# or until encountering a null character (whichever comes first). */
  GUTF8String(const char *dat, unsigned int len);
  /// Construct from base class.
  GUTF8String(const GP<GStringRep> &str);
  GUTF8String(const GBaseString &str); 
  GUTF8String(const GUTF8String &str);
  GUTF8String(const GNativeString &str);
  /** Constructs a string from a character array.  Elements of the character
      array #dat# are added into the string until the string length reaches
      #len# or until encountering a null character (whichever comes first). */
  GUTF8String(const GBaseString &gs, int from, unsigned int len);

  /** Copy a null terminated character array. Resets this string with the
      character string contained in the null terminated character array
      #str#. */
  GUTF8String& operator= (const char str);
  GUTF8String& operator= (const char *str);
  GUTF8String& operator= (const GP<GStringRep> &str);
  GUTF8String& operator= (const GBaseString &str);
  GUTF8String& operator= (const GUTF8String &str);
  GUTF8String& operator= (const GNativeString &str);

  /** Constructs a string with a formatted string (as in #vprintf#).  The
      string is re-initialized with the characters generated according to the
      specified format #fmt# and using the optional arguments.  See the ANSI-C
      function #vprintf()# for more information. The current implementation
      will cause a segmentation violation if the resulting string is longer
      than 32768 characters. */
  GUTF8String(const GUTF8String &fmt, va_list &args);

  /// Constructs a string from a character.
  /** Constructs a string with a human-readable representation of integer
      #number#.  The format is similar to format #"%d"# in function
      #printf#. */
  GUTF8String(const int number);

  /** Constructs a string with a human-readable representation of floating
      point number #number#. The format is similar to format #"%f"# in
      function #printf#.  */
  GUTF8String(const double number);


  /** Initializes a string with a formatted string (as in #printf#).  The
      string is re-initialized with the characters generated according to the
      specified format #fmt# and using the optional arguments.  See the ANSI-C
      function #printf()# for more information. The current implementation
      will cause a segmentation violation if the resulting string is longer
      than 32768 characters. */
  GUTF8String &format(const char *fmt, ... );
  /** Initializes a string with a formatted string (as in #vprintf#).  The
      string is re-initialized with the characters generated according to the
      specified format #fmt# and using the optional arguments.  See the ANSI-C
      function #vprintf()# for more information. The current implementation
      will cause a segmentation violation if the resulting string is longer
      than 32768 characters. */
  GUTF8String &format(const GUTF8String &fmt, va_list &args)
  { return (*this = (fmt.ptr?GUTF8String(fmt,args):fmt)); }

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

  /** Returns a long intenger.  Implments i18n strtol.  */
  long int toLong(
    GUTF8String& endptr, bool &isLong, const int base=10) const
  { return ptr?(*this)->toLong(endptr, isLong, base):0; }

  static long int toLong(
    const GUTF8String& src, GUTF8String& endptr, bool &isLong,
    const int base=10)
  { return src.toLong(endptr,isLong,base); }

  /** Returns a unsigned long integer.  Implements i18n strtoul. */
  unsigned long int toULong(
    GUTF8String& endptr, bool &isULong, const int base=10) const
  { return ptr?(*this)->toLong(endptr, isULong, base):0; }

  static unsigned long int toULong(
    const GUTF8String& src, GUTF8String& endptr, bool &isULong,
    const int base=10)
  { return src.toLong(endptr,isULong,base); }

  /** Returns a double.  Implements the i18n strtod.  */
  double toDouble( GUTF8String& endptr, bool& isDouble ) const
  { return ptr?(*this)->toDouble(endptr,isDouble):(double)0; }

  static double toDouble(
    const GUTF8String& src, GUTF8String& endptr, bool& isDouble)
  { return src.toDouble(endptr,isDouble); }


   /** Returns an integer.  Gives the position of the next non white space */
   int nextNonSpace( int from=0  ) const
      { return ptr?(*this)->nextNonSpace( from ):0; }

  // -- CONCATENATION
  /// Appends character #ch# to the string.
  GUTF8String& operator+= (char ch)
  {
    return init(GStringRep::UTF8::create((const char*)*this,GStringRep::UTF8::create(&ch,0,1)));
  }

  /// Appends the null terminated character array #str# to the string.
  GUTF8String& operator+= (const char *str)
  {
    return init(GStringRep::UTF8::create(*this,str));
  }
  /// Appends the specified GBaseString to the string.
  GUTF8String& operator+= (const GBaseString &str)
  { 
    return init(GStringRep::UTF8::create(*this,str));
  }

  /** Returns a sub-string.  The sub-string is composed by copying #len#
      characters starting at position #from# in this string.  The length of
      the resulting string may be smaller than #len# if the specified range is
      too large. */
  GUTF8String substr(int from, unsigned int len=1) const
    { return GUTF8String(*this, from, len); }

  /** Returns an upper case copy of this string.  The returned string
      contains a copy of the current string with all letters turned into 
      upper case letters. */
  GUTF8String upcase( void ) const;
  /** Returns an lower case copy of this string.  The returned string
      contains a copy of the current string with all letters turned into 
      lower case letters. */
  GUTF8String downcase( void ) const;

  /** Concatenates strings. Returns a string composed by concatenating
      the characters of strings #s1# and #s2#.
  */
  GUTF8String operator+(const GBaseString &s2) const 
    { return GStringRep::UTF8::create(*this,s2); }
  GUTF8String operator+(const GUTF8String &s2) const
    { return GStringRep::UTF8::create(*this,s2); }
  GUTF8String operator+(const GNativeString &s2) const;
  GUTF8String operator+(const char    *s2) const
    { return GStringRep::UTF8::create(*this,s2); }
  friend GUTF8String operator+(const char    *s1, const GUTF8String &s2) 
    { return GStringRep::UTF8::create(s1,s2); }
};

class GNativeString : public GBaseString
{
public:
  void init(void)
  { GBaseString::init(); }

  GNativeString &init(const GP<GStringRep> &rep)
  {  GP<GStringRep>::operator=(rep?rep->toNative(true):rep); init(); return *this; }

  // -- CONSTRUCTORS
  /** Null constructor. Constructs an empty string. */
  GNativeString(void);
  /// Constructs a string from a character.
  GNativeString(const char dat);
  /// Constructs a string from a null terminated character array.
  GNativeString(const char *str);
  /// Constructs a string from a null terminated character array.
  GNativeString(const unsigned char *str);
  /** Constructs a string from a character array.  Elements of the character
      array #dat# are added into the string until the string length reaches
      #len# or until encountering a null character (whichever comes first). */
  GNativeString(const char *dat, unsigned int len);
  /// Construct from base class.
  GNativeString(const GP<GStringRep> &str);
  GNativeString(const GBaseString &str); 
  GNativeString(const GUTF8String &str);
  GNativeString(const GNativeString &str);
  /** Constructs a string from a character array.  Elements of the character
      array #dat# are added into the string until the string length reaches
      #len# or until encountering a null character (whichever comes first). */
  GNativeString(const GBaseString &gs, int from, unsigned int len);

  /** Constructs a string with a formatted string (as in #vprintf#).  The
      string is re-initialized with the characters generated according to the
      specified format #fmt# and using the optional arguments.  See the ANSI-C
      function #vprintf()# for more information. The current implementation
      will cause a segmentation violation if the resulting string is longer
      than 32768 characters. */
  GNativeString(const GNativeString &fmt, va_list &args);

  /** Constructs a string with a human-readable representation of integer
      #number#.  The format is similar to format #"%d"# in function
      #printf#. */
  GNativeString(const int number);

  /** Constructs a string with a human-readable representation of floating
      point number #number#. The format is similar to format #"%f"# in
      function #printf#.  */
  GNativeString(const double number);

  /** Copy a null terminated character array. Resets this string with the
      character string contained in the null terminated character array
      #str#. */
  GNativeString& operator= (const char str);
  GNativeString& operator= (const char *str);
  GNativeString& operator= (const GP<GStringRep> &str);
  GNativeString& operator= (const GBaseString &str);
  GNativeString& operator= (const GUTF8String &str);
  GNativeString& operator= (const GNativeString &str);

  // -- CONCATENATION
  /// Appends character #ch# to the string.
  GNativeString& operator+= (char ch)
  {
    return init(GStringRep::Native::create(*this,GStringRep::UTF8::create(&ch,0,1)));
  }
  /// Appends the null terminated character array #str# to the string.
  GNativeString& operator+= (const char *str)
  {
    return init(GStringRep::Native::create(*this,str));
  }
  /// Appends the specified GBaseString to the string.
  GNativeString& operator+= (const GBaseString &str)
  { 
    return init(GStringRep::Native::create(*this,str));
  }

   /** Return an integer.  Returns position of next non space value */
   int nextNonSpace( int from=0 )
      { return ptr?(*this)->nextNonSpace(from):0; }

  /** Returns a sub-string.  The sub-string is composed by copying #len#
      characters starting at position #from# in this string.  The length of
      the resulting string may be smaller than #len# if the specified range is
      too large. */
  GNativeString substr(int from, unsigned int len=1) const
    { return GNativeString(*this, from, len); }

  /** Returns an upper case copy of this string.  The returned string
      contains a copy of the current string with all letters turned into 
      upper case letters. */
  GNativeString upcase( void ) const;
  /** Returns an lower case copy of this string.  The returned string
      contains a copy of the current string with all letters turned into 
      lower case letters. */
  GNativeString downcase( void ) const;

  /** Returns a long intenger.  Implments i18n strtol.  */
  long int toLong(
    GNativeString& endptr, bool &isLong, const int base=10) const
  { return ptr?(*this)->toLong(endptr, isLong, base):0; }

  static long int toLong(
    const GNativeString& src, GNativeString& endptr, bool &isLong, const int base=10)
  { return src.toLong(endptr,isLong,base); }

  /** Returns a unsigned long integer.  Implements i18n strtoul. */
  unsigned long int toULong(
    GNativeString& endptr, bool &isULong, const int base=10) const
  { return ptr?(*this)->toLong(endptr, isULong, base):0; }

  static unsigned long int toULong(
    const GNativeString& src, GNativeString& endptr, bool &isULong, const int base=10)
  { return src.toLong(endptr,isULong,base); }

  /** Returns a double.  Implements the i18n strtod.  */
  double toDouble( GNativeString& endptr, bool& isDouble ) const
  { return ptr?(*this)->toDouble(endptr,isDouble):(double)0; }

  static double toDouble(
    const GNativeString& src,  GNativeString& endptr, bool& isDouble)
  { return src.toDouble(endptr,isDouble); }

  GNativeString operator+(const GBaseString &s2) const
    { return GStringRep::Native::create(*this,s2); }
  GNativeString operator+(const GNativeString &s2) const 
    { return GStringRep::Native::create(*this,s2); }
  GUTF8String operator+(const GUTF8String &s2) const;
  GNativeString operator+(const char    *s2) const
    { return GStringRep::Native::create(*this,s2); }
  friend GNativeString operator+(const char    *s1, const GNativeString &s2) 
    { return GStringRep::Native::create(s1,s2); }

  /** Initializes a string with a formatted string (as in #printf#).  The
      string is re-initialized with the characters generated according to the
      specified format #fmt# and using the optional arguments.  See the ANSI-C
      function #printf()# for more information. The current implementation
      will cause a segmentation violation if the resulting string is longer
      than 32768 characters. */
  GNativeString &format(const char *fmt, ... );
  /** Initializes a string with a formatted string (as in #vprintf#).  The
      string is re-initialized with the characters generated according to the
      specified format #fmt# and using the optional arguments.  See the ANSI-C
      function #vprintf()# for more information. The current implementation
      will cause a segmentation violation if the resulting string is longer
      than 32768 characters. */
  GNativeString &format(const GNativeString &fmt, va_list &args)
  { return (*this = (fmt.ptr?GNativeString(fmt,args):fmt)); }
};

//@}

inline
GBaseString::operator const char* ( void ) const  
{
  return ptr?(*this)->data:nullstr;
}

inline unsigned int
GBaseString::length( void ) const 
{
  return ptr ? (*this)->size : 0;
}

inline bool
GBaseString::operator! ( void ) const
{
  return !ptr;
}

inline GUTF8String 
GUTF8String::upcase( void ) const
{ 
  return (ptr?(*this)->upcase():(*this));
}

inline GUTF8String 
GUTF8String::downcase( void ) const
{ 
  return (ptr?(*this)->downcase():(*this));
}

inline GNativeString 
GNativeString::upcase( void ) const
{ 
  return (ptr?(*this)->upcase():(*this));
}

inline GNativeString 
GNativeString::downcase( void ) const
{ 
  return (ptr?(*this)->downcase():(*this));
}

inline
GUTF8String::GUTF8String(const GNativeString &str)
{ init(str.length()?(str->toUTF8(true)):(GP<GStringRep>)str); }

inline
GNativeString::GNativeString(const GUTF8String &str)
{ init(str.length()?(str->toNative(true)):(GP<GStringRep>)str); }

inline
GUTF8String::GUTF8String(const GP<GStringRep> &str)
{ init(str?(str->toUTF8(true)):str); }

inline
GNativeString::GNativeString(const GP<GStringRep> &str)
{ init(str?(str->toNative(true)):str); }

inline
GUTF8String::GUTF8String(const GBaseString &str)
{ init(str.length()?(str->toUTF8(true)):(GP<GStringRep>)str); }

inline
GNativeString::GNativeString(const GBaseString &str)
{ init(str.length()?(str->toNative(true)):(GP<GStringRep>)str); }

inline void
GBaseString::init(void)
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

inline GBaseString::GBaseString(void) { init(); }

inline GUTF8String::GUTF8String(void) { }
inline GUTF8String::GUTF8String(const char dat)
{ init(GStringRep::UTF8::create(&dat,0,1)); }
inline GUTF8String::GUTF8String(const char *str)
{ init(GStringRep::UTF8::create(str)); }
inline GUTF8String::GUTF8String(const unsigned char *str)
{ init(GStringRep::UTF8::create((const char *)str)); }
inline GUTF8String::GUTF8String(const char *dat, unsigned int len)
{ init(GStringRep::UTF8::create(dat,0,((int)len<0)?(-1):(int)len)); }
inline GUTF8String::GUTF8String(const GUTF8String &str)
{ init(str); }
inline GUTF8String::GUTF8String(const GBaseString &gs, int from, unsigned int len)
{ init(GStringRep::UTF8::create(gs,from,((int)len<0)?(-1):(int)len)); }
inline GUTF8String::GUTF8String(const int number)
{ init(GStringRep::UTF8::create_format("%d",number)); }
inline GUTF8String::GUTF8String(const double number)
{ init(GStringRep::UTF8::create_format("%f",number)); }
inline GUTF8String::GUTF8String(const GUTF8String &fmt, va_list &args)
{ init(fmt.ptr?fmt->format(args):fmt); }

inline GUTF8String& GUTF8String::operator= (const char str)
{ return init(GStringRep::UTF8::create(&str,0,1)); }
inline GUTF8String& GUTF8String::operator= (const char *str)
{ return init(GStringRep::UTF8::create(str)); }
inline GUTF8String& GUTF8String::operator= (const GP<GStringRep> &str)
{ return init(str); }
inline GUTF8String& GUTF8String::operator= (const GBaseString &str)
{ return init(str); }
inline GUTF8String& GUTF8String::operator= (const GUTF8String &str)
{ return init(str); }
inline GUTF8String& GUTF8String::operator= (const GNativeString &str)
{ return init(str); }

inline GNativeString::GNativeString(void) { }
inline GNativeString::GNativeString(const char dat)
{ init(GStringRep::Native::create(&dat,0,1)); }
inline GNativeString::GNativeString(const char *str)
{ init(GStringRep::Native::create(str)); }
inline GNativeString::GNativeString(const unsigned char *str)
{ init(GStringRep::Native::create((const char *)str)); }
inline GNativeString::GNativeString(const char *dat, unsigned int len)
{ init(GStringRep::Native::create(dat,0,((int)len<0)?(-1):(int)len)); }
//inline GNativeString::GNativeString(const GP<GStringRep> &str)
//{ init(str); }
//inline GNativeString::GNativeString(const GBaseString &str)
//{ init(str); }
//inline GNativeString::GNativeString(const GUTF8String &str)
//{ init(str); }
inline GNativeString::GNativeString(const GNativeString &str)
{ init(str); }
inline GNativeString::GNativeString(const GBaseString &gs, int from, unsigned int len)
{ init(GStringRep::Native::create(gs,from,((int)len<0)?(-1):(int)len)); }
inline GNativeString::GNativeString(const int number)
{ init(GStringRep::Native::create_format("%d",number)); }
inline GNativeString::GNativeString(const double number)
{ init(GStringRep::Native::create_format("%f",number)); }
inline GNativeString::GNativeString(const GNativeString &fmt, va_list &args)
{ init(fmt.ptr?fmt->format(args):fmt); }

inline GNativeString& GNativeString::operator= (const char str)
{ return init(GStringRep::Native::create(&str,0,1)); }
inline GNativeString& GNativeString::operator= (const char *str)
{ return init(GStringRep::Native::create(str)); }
inline GNativeString& GNativeString::operator= (const GP<GStringRep> &str)
{ return init(str); }
inline GNativeString& GNativeString::operator= (const GBaseString &str)
{ return init(str); }
inline GNativeString& GNativeString::operator= (const GUTF8String &str)
{ return init(str); }
inline GNativeString& GNativeString::operator= (const GNativeString &str)
{ return init(str); }

inline GUTF8String GBaseString::operator+(const GUTF8String &s2) const
  { return GStringRep::UTF8::create(*this,s2); }
inline GNativeString GBaseString::operator+(const GNativeString &s2) const
  { return GStringRep::Native::create(*this,s2); }
inline GUTF8String GNativeString::operator+(const GUTF8String &s2) const
  { return GStringRep::UTF8::create(ptr?(*this)->toUTF8(true):(*this),s2); }
inline GUTF8String GUTF8String::operator+(const GNativeString &s2) const
  { return GStringRep::UTF8::create(*this,s2.ptr?s2->toUTF8(true):s2); }

// ------------------- The end

#endif
