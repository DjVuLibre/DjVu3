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
// $Id: GUnicode.h,v 1.4.2.1 2001-03-22 02:04:16 bcr Exp $
// $Name:  $

#ifndef __GUNICODE_
#define __GUNICODE_

#ifdef __GNUC__
#pragma interface
#endif


#include "GString.h"

class UnicodeRep : public GPEnabled
{
public:
  class Remainder
  {
  public:
    enum EncodeType {UCS4,UCS4BE,UCS4LE,UCS4_2143,UCS4_3412,UTF16,UTF16BE,UTF16LE,UTF8,EBCDIC,OTHER};
    Remainder(const Remainder &);
    Remainder(const Remainder &,const Remainder &);
    Remainder(EncodeType=OTHER);
    ~Remainder();
    inline void init(const Remainder &r1);
    void init(const Remainder &r1,const Remainder &r2);
    void init(void const * const ,size_t,EncodeType=OTHER);
    EncodeType encodetype;
    void *data;
    GPBufferBase gdata;
    size_t size;
  };
  enum EncodeType {UCS4,UCS4BE,UCS4LE,UCS4_2143,UCS4_3412,UTF16,UTF16BE,UTF16LE,UTF8,EBCDIC,OTHER};

protected:
  UnicodeRep(void);
  UnicodeRep(const UnicodeRep &);
  UnicodeRep(const Remainder &r); 
  UnicodeRep(const Remainder &r1,const Remainder &r2); 

public:
  /// Default creator.
  static GP<UnicodeRep> create(void)
  { return new UnicodeRep(); }

  /// Copy creator.
  static GP<UnicodeRep> create(const UnicodeRep &r)
  { return new UnicodeRep(r); }

  /// Create from a remainder.
  static GP<UnicodeRep> create(const Remainder &r)
  { return new UnicodeRep(r); }

  /// Create from two remainders.
  static GP<UnicodeRep> create(const Remainder &r1,const Remainder &r2)
  { return new UnicodeRep(r1,r2); }

  /// Non-virtual destructor.
  ~UnicodeRep();

protected:
  unsigned int len;
  GString gs;
  Remainder remainder;
  unsigned long *UnicodePtr;
  GPBuffer<unsigned long> gUnicodePtr;
public:
  static unsigned long const nill;
private:
  unsigned long UTF8toWideChar(unsigned char const *&,void const * const);
  void initUTF8(void);
  unsigned long UTF16toWideChar(unsigned short const *&s,void const * const);
  unsigned long UTF16BEtoWideChar(unsigned char const *&s,void const * const);
  unsigned long UTF16LEtoWideChar(unsigned char const *&s,void const * const);
  unsigned long UTS4BEtoWideChar(unsigned char const *&s,void const * const);
  inline unsigned long UCS4BEtoWideChar(unsigned char const *&s,void const * const);
  inline unsigned long UCS4LEtoWideChar(unsigned char const *&s,void const * const);
  inline unsigned long UCS4_3412toWideChar(unsigned char const *&s,void const * const);
  inline unsigned long UCS4_2143toWideChar(unsigned char const *&s,void const * const);
  static GP<UnicodeRep> concat(const UnicodeRep &,const UnicodeRep &);
public:
  void init (void const * const,unsigned int const,const EncodeType=EBCDIC);
  inline void init (GString &,unsigned int const=0);
  inline void init (const GString &,unsigned int const=0);
  inline operator unsigned long const * () const;
  inline operator char const * () const;
  inline GString get_GString() const;
  inline void const * get_remainder (void) const;
  inline size_t get_remainder_size (void) const;
  inline EncodeType get_remainderType (void) const;
  inline unsigned int length(void) const;
  inline bool operator == (const UnicodeRep &s) const;
  inline bool operator != (const UnicodeRep &s) const;
  inline bool operator == (const GString &s) const;
  inline bool operator != (const GString &s) const;
  static GP<UnicodeRep> concat(const GP<UnicodeRep> &,const GP<UnicodeRep> &);
};

inline void
UnicodeRep::init (GString &str,unsigned int const i)
{
  init((char const *)str,i,UTF8);
  if(gs == str)
  {
    gs=str;
  }
}

inline void
UnicodeRep::init (const GString &str,unsigned int const i)
{
  init((char const *)str,i,UTF8);
  if(gs == str)
  {
    gs=str;
  }
}

inline
UnicodeRep::operator unsigned long const * () const
{return UnicodePtr?UnicodePtr:&nill;}

inline
UnicodeRep::operator char const * () const
{return gs;}

inline GString
UnicodeRep::get_GString (void) const
{return gs;}

inline const void *
UnicodeRep::get_remainder (void) const
{return remainder.data;}

inline size_t
UnicodeRep::get_remainder_size (void) const
{return remainder.size;}

inline UnicodeRep::EncodeType
UnicodeRep::get_remainderType (void) const
{return (UnicodeRep::EncodeType)remainder.encodetype;}

inline unsigned int
UnicodeRep::length(void) const
{return len;}

inline bool 
UnicodeRep::operator == (const UnicodeRep &s) const
{
	return (gs == s.gs)?true:false;
}

inline bool 
UnicodeRep::operator != (const UnicodeRep &s) const
{
	return (gs != s.gs)?true:false;
}

inline bool 
UnicodeRep::operator == (const GString &s) const
{
	return (gs == s)?true:false;
}

inline bool 
UnicodeRep::operator != (const GString &s) const
{
	return (gs != s)?true:false;
}

class GUnicode : public GP<UnicodeRep>
{
public:
  enum EncodeType {
    UCS4=UnicodeRep::UCS4,
    UCS4BE=UnicodeRep::UCS4BE,
    UCS4LE=UnicodeRep::UCS4LE,
    UCS4_2143=UnicodeRep::UCS4_2143,
    UCS4_3412=UnicodeRep::UCS4_3412,
    UTF16=UnicodeRep::UTF16,
    UTF16BE=UnicodeRep::UTF16BE,
    UTF16LE=UnicodeRep::UTF16LE,
    UTF8=UnicodeRep::UTF8,
    EBCDIC=UnicodeRep::EBCDIC,
    OTHER=UnicodeRep::OTHER};
  GUnicode(void);
  GUnicode(GString &str);
  GUnicode(GString &str,unsigned int const);
  GUnicode(const GString &str);
  GUnicode(const GString &str,unsigned int const);
  GUnicode( unsigned long const * const wide );
  GUnicode( unsigned long const * const wide, unsigned int const );
  GUnicode( unsigned char const * const,
    unsigned int const, const EncodeType );
  GUnicode( unsigned short const * const,
    unsigned int const, const EncodeType );
  GUnicode( unsigned long const * const,
    unsigned int const, const EncodeType );
  inline operator unsigned long const * () const;
  inline operator char const * () const;
  inline GString get_GString (void) const;
  inline void const *get_remainder (void) const;
  inline size_t get_remainder_size (void) const;
  inline EncodeType get_remainderType (void) const;
  inline unsigned int length(void) const;
  inline GUnicode & operator = (const GUnicode &guni);
  inline GUnicode & operator += (const GUnicode &guni);
  inline GUnicode & operator += (unsigned long const);
  inline GUnicode & operator += (unsigned long const *);
  inline GUnicode & operator += (const GString &);
  inline unsigned long operator [] (int n) const;
  inline bool operator ! () const;
  inline bool operator == (const GUnicode &) const;
  inline bool operator != (const GUnicode &) const;
  inline bool operator == (const GString &) const;
  inline bool operator != (const GString &) const;
  friend inline GUnicode operator + (const GUnicode &,const GUnicode &);
  GUnicode substr(unsigned int const from,unsigned int const to) const;
  GUnicode substr_nr(unsigned int const from,unsigned int const to) const;
  void setat(unsigned int const pos,unsigned long const w);
private:
  inline GUnicode(const GP<UnicodeRep> &gui) : GP<UnicodeRep>(gui) {}
};

inline
GUnicode
operator+(const GUnicode &gui1,const GUnicode &gui2)
{
  return UnicodeRep::concat(gui1,gui2);
}

inline 
GUnicode & GUnicode::operator = (const GUnicode &guni)
{
  GP<UnicodeRep>::operator = (guni);
  return *this;
}

inline 
GUnicode & GUnicode::operator += (const GUnicode &guni)
{
  GP<UnicodeRep>::operator = (UnicodeRep::concat(*this,guni));
  return *this;
}

inline 
GUnicode & GUnicode::operator += (unsigned long const w)
{
  return (operator += (GUnicode(&w,1)));
}

inline 
GUnicode & GUnicode::operator += (unsigned long const *w)
{
  return (operator += (GUnicode(w)));
}

inline 
GUnicode & GUnicode::operator += (const GString &gs)
{
  return (operator += (GUnicode(gs)));
}

inline unsigned long 
GUnicode::operator [] (int n) const
{
  return (operator unsigned long const *())[n];
}

inline bool
GUnicode::operator ! () const
{
  return !length();
}

inline bool 
GUnicode::operator == (const GUnicode &s) const
{
  return ((GP<UnicodeRep>::operator*()) == (*s));
}

inline bool 
GUnicode::operator != (const GUnicode &s) const
{
  return ((GP<UnicodeRep>::operator*()) != (*s));
}

inline bool 
GUnicode::operator == (const GString &s) const
{
  return ((GP<UnicodeRep>::operator*()) == s);
}

inline bool 
GUnicode::operator != (const GString &s) const
{
  return ((GP<UnicodeRep>::operator*()) != s);
}

inline
GUnicode::operator unsigned long const * () const
{return (unsigned long const *)(**this);}

inline
GUnicode::operator char const * () const
{return (char const *)(**this);}

inline GString
GUnicode::get_GString (void) const
{return GString((*this)->get_GString());}

inline const void *
GUnicode::get_remainder (void) const
{return ((*this)->get_remainder());}

inline size_t
GUnicode::get_remainder_size (void) const
{return ((*this)->get_remainder_size());}

inline GUnicode::EncodeType
GUnicode::get_remainderType () const
{return (EncodeType)(*this)->get_remainderType();}

inline unsigned int
GUnicode::length(void) const
{return (*this)->length();}

#endif /* __GUNICODE_ */

