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
// $Id: GString.h,v 1.26 2001-03-12 23:50:23 fcrary Exp $
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
    #$Id: GString.h,v 1.26 2001-03-12 23:50:23 fcrary Exp $# */
//@{

#ifdef __GNUC__
#pragma interface
#endif

#include "DjVuGlobal.h"
#include "GContainer.h"

//include <string.h>
//include <stdlib.h>
#include <stdarg.h>
#ifdef WIN32
#include <windows.h>
#endif


// Internal string representation.

class GStringRep : public GPEnabled
{
  friend class GString;
  friend unsigned int hash(const GString &ref);
public:
  static GStringRep *xnew(unsigned int sz = 0);
private:
  int  size;
  char data[1];
};



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

    
class GString : protected GP<GStringRep> 
{
public: 
  // -- CONSTRUCTORS
  /** Null constructor. Constructs an empty string. */
  GString( void )
    { } 
  /// Copy constructor. Constructs a string by copying the string #gs#.
  GString(const GString &gs)
    : GP<GStringRep>(gs) { } 
  /// Constructs a string from a character.
  GString(const char dat);
  /// Constructs a string from a null terminated character array.
  GString(const char *dat);
  /** Constructs a string from a character array.  Elements of the character
      array #dat# are added into the string until the string length reaches
      #len# or until encountering a null character (whichever comes first). */
  GString(const char *dat, unsigned int len);
  /** Construct a string by copying a sub-string. The string will be
      initialized with at most #len# characters from string #gs# starting at
      position #from#.  The length of the constructed string may be smaller
      than #len# if the specified range is too large. */
  GString(const GString &gs, int from, unsigned int len);
  /** Constructs a string with a human-readable representation of integer
      #number#.  The format is similar to format #"%d"# in function
      #printf#. */
  GString(const int number);
  /** Constructs a string with a human-readable representation of floating
      point number #number#. The format is similar to format #"%f"# in
      function #printf#.  */
  GString(const double number);
  
  // -- COPY OPERATOR
  /** Copy operator. Resets this string with the value of the string #gs#.
      This operation is efficient because string memory is allocated using a
      "copy-on-write" strategy. Both strings will share the same segment of
      memory until one of the strings is modified. */
  GString& operator= (const GString &gs)
    { GP<GStringRep>::operator= (gs); return *this; }
  /** Copy a null terminated character array. Resets this string with the
      character string contained in the null terminated character array
      #str#. */
  GString& operator= (const char *str);

  // -- ACCESS
  /** Converts a string into a constant null terminated character array.  This
      conversion operator is very efficient because it simply returns a
      pointer to the internal string data. The returned pointer remains valid
      as long as the string is unmodified. */
  operator const char* ( void ) const  
    { return ptr ? (*this)->data : nullstr; }
  /// Returns the string length.
  unsigned int length( void ) const 
    { return ptr ? (*this)->size : 0; }
  /** Returns true if and only if the string contains zero characters.  This
      operator is useful for conditional expression in control structures.
      \begin{verbatim}
         if (! str) { ... }
         while (!! str) { ... }  -- Note the double operator!
      \end{verbatim}
      Class #GString# does not to support syntax "#if# #(str)# #{}#" because
      the required conversion operator introduces dangerous ambiguities with
      certain compilers. */
  int operator! ( void ) const 
    { return !ptr; } ;

  // -- INDEXING
  /** Returns the character at position #n#. An exception \Ref{GException} is
      thrown if number #n# is not in range #-len# to #len-1#, where #len# is
      the length of the string.  The first character of a string is numbered
      zero.  Negative positions represent characters relative to the end of
      the string. */
  char operator[] (int n) const
    { if (n<0 && ptr) n += (*this)->size;
      if (n<0 || !ptr || n>=(int)(*this)->size) throw_illegal_subscript();
      return (*this)->data[n]; }
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
    { return GString((GString&)(*this), from, len); }

  /** Returns an upper case copy of this string.  The returned string
      contains a copy of the current string with all letters turned into 
      upper case letters. */
  GString upcase( void ) const;

  /** Returns an lower case copy of this string.  The returned string
      contains a copy of the current string with all letters turned into 
      lower case letters. */
  GString downcase( void ) const;

  /** Returns a copy of this string with characters used in XML with
      '<'  to "&lt;", '>'  to "&gt;",  '&' to "&amp;" '\'' to "&apos;",
      and  '\"' to  "&quot;".   Characters 0x01 through 0x1f are also
      escaped. */
  GString toEscaped(void ) const;

  /** Converts strings containing HTML/XML escaped characters into their
      unescaped forms. Numeric representations of characters (e.g., "&#38;"
      or "&#x26;" for "*") are the only forms converted by this function. */
  GString fromEscaped( void ) const;

  /** Converts strings containing HTML/XML escaped characters (e.g.,
      "&lt;" for "<") into their unescaped forms. The conversion is partially
      defined by the ConvMap argument which specifies the conversion strings
      to be recognized. [The function BasicMap() is available to produce a list
      which inverts the actions of toEscaped().] Numeric representations of
      characters (e.g., "&#38;" or "&#x26;" for "*") are always converted. */
  GString fromEscaped( const GMap<GString,GString> ConvMap ) const;

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
      specified format #fmt# and using the optional arguments.  See the Ansi-C
      function #printf()# for more information. The current implementation
      will cause a segmentation violation if the resulting string is longer
      than 32768 characters. */
  void format(const char *fmt, ... );
  /** Initializes a string with a formatted string (as in #vprintf#).  The
      string is re-initialized with the characters generated according to the
      specified format #fmt# and using the optional arguments.  See the Ansi-C
      function #vprintf()# for more information. The current implementation
      will cause a segmentation violation if the resulting string is longer
      than 32768 characters. */
  void format(const char *fmt, va_list args);
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

  // -- CONCATENATION
  /// Appends character #ch# to the string.
  GString& operator+= (char ch);
  /// Appends the null terminated character array #str# to the string.
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
      are equal (as with #strcmp#.) */
  friend int operator==(const GString &s1, const GString &s2) 
    { return strcmp(s1,s2)==0; }
  friend int operator==(const GString &s1, const char    *s2) 
    { return strcmp(s1,s2)==0; }
  friend int operator==(const char    *s1, const GString &s2) 
    { return strcmp(s1,s2)==0; }
  /** String comparison. Returns true if and only if character strings #s1# and #s2#
      are not equal (as with #strcmp#.) */
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
      lexicographically less than or equal to string #s2# (as with #strcmp#.) */
  friend int operator<=(const GString &s1, const GString &s2)
    { return strcmp(s1,s2)<=0; }
  friend int operator<=(const GString &s1, const char    *s2) 
    { return strcmp(s1,s2)<=0; }
  friend int operator<=(const char    *s1, const GString &s2)
    { return strcmp(s1,s2)<=0; }
  /** String comparison. Returns true if and only if character strings #s1# is
      lexicographically less than string #s2# (as with #strcmp#.) */
  friend int operator< (const GString &s1, const GString &s2)
    { return strcmp(s1,s2)< 0; }
  friend int operator< (const GString &s1, const char    *s2)
    { return strcmp(s1,s2)< 0; }
  friend int operator< (const char    *s1, const GString &s2) 
    { return strcmp(s1,s2)< 0; }

  // -- HASHING
  /** Returns a hash code for the string.  This hashing function helps when
      creating associative maps with string keys (see \Ref{GMap}).  This hash
      code may be reduced to an arbitrary range by computing its remainder
      modulo the upper bound of the range. */
  friend unsigned int hash(const GString &ref);
  // -- HELPERS

protected:
  GString(GStringRep* rep)
    : GP<GStringRep>(rep) { }
  GString& operator= (GStringRep *rep)
    {  GP<GStringRep>::operator= (rep); return *this; }
  static GString concat (const char *str1, const char *str2);
  static void throw_illegal_subscript() no_return;
  static const char *nullstr;
};

//@}


// ------------------- The end

#endif



