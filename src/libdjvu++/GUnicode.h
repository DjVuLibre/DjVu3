//C-  Copyright © 2000-2001, LizardTech, Inc. All Rights Reserved.
//C-              Unauthorized use prohibited.
//
// $Id: GUnicode.h,v 1.1 2001-01-17 00:14:55 bcr Exp $
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
    inline void init(const Remainder &r1);
    void init(const Remainder &r1,const Remainder &r2);
    void init(void const * const ,size_t const,EncodeType=OTHER);
    ~Remainder();
    EncodeType encodetype;
    void *data;
    GPBufferBase gdata;
    size_t size;
  };
  enum EncodeType {UCS4,UCS4BE,UCS4LE,UCS4_2143,UCS4_3412,UTF16,UTF16BE,UTF16LE,UTF8,EBCDIC,OTHER};
protected:
  unsigned int len;
  GString gs;
  Remainder remainder;
  unsigned long *UnicodePtr;
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
  UnicodeRep(void);
  UnicodeRep(const UnicodeRep &);
  UnicodeRep(const Remainder &r); 
  UnicodeRep(const Remainder &r1,const Remainder &r2); 
  ~UnicodeRep();
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
{return UnicodePtr;}

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

