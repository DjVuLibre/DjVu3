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
// $Id: GUnicode.cpp,v 1.8 2001-04-11 16:59:51 bcr Exp $
// $Name:  $

#ifdef __GNUC__
#pragma implementation
#endif

#include "GUnicode.h"

unsigned long const UnicodeRep::nill=0;

static void const * 
checkmarks(void const * const xbuf,size_t &bufsize,UnicodeRep::EncodeType &rep)
{
  unsigned char const *buf=(unsigned char const *)xbuf;
  if(bufsize >= 2 || (!bufsize && rep != UnicodeRep::OTHER))
  {
    const unsigned int s=(((unsigned int)buf[0])<<8)+(unsigned int)buf[1];
    switch(s)
    {
      case 0:
        if((bufsize>=4)||(!bufsize && rep == UnicodeRep::UCS4BE)||(!bufsize && rep == UnicodeRep::UCS4_2143))
        {
          const unsigned int s=(((unsigned int)buf[2])<<8)+(unsigned int)buf[3];
          if(s == 0xfeff)
          { 
            rep=UnicodeRep::UCS4BE;
            buf+=4;
          }else if(s == 0xfffe)
          {
            rep=UnicodeRep::UCS4_2143;
            buf+=4;
          }
        }
        break;
      case 0xfffe:
        if(((bufsize>=4)||(!bufsize && rep == UnicodeRep::UCS4LE)) && !(unsigned char *)buf[2] && !(unsigned char *)buf[3])
        {
          rep=UnicodeRep::UCS4LE;
          buf+=4;
        }else
        {
          rep=UnicodeRep::UTF16LE;
          buf+=2;
        }
        break;
      case 0xfeff:
        if(((bufsize>=4)||(!bufsize && rep == UnicodeRep::UCS4_3412))&&!(unsigned char *)buf[2] && !(unsigned char *)buf[3])
        {
          rep=UnicodeRep::UCS4_3412;
          buf+=4;
        }else
        {
          rep=UnicodeRep::UTF16LE;
          buf+=2;
        }
        break;
      case 0xefbb:
        if(((bufsize>=3)||(!bufsize && UnicodeRep::UTF8 == rep))&&(buf[2] == 0xbf))
        {
          rep=UnicodeRep::UTF8;
          buf+=3;
        }
        break;
      default:
        break;
    }
  }
  if(buf != xbuf)
  {
    if(bufsize)
    {
      const size_t s=(size_t)xbuf-(size_t)buf;
      if(bufsize> s)
      {
        bufsize-=s;
      }else
      {
        bufsize=0;
        buf=(const unsigned char *)&UnicodeRep::nill;
      }
    }
  }
  return buf;
}


UnicodeRep::UnicodeRep(void)
: len(0), remainder(UnicodeRep::Remainder::OTHER), gUnicodePtr(UnicodePtr)
{
}

UnicodeRep::UnicodeRep(const UnicodeRep &uni)
: len(uni.len), gs(uni.gs), remainder(uni.remainder), gUnicodePtr(UnicodePtr)
{
  if(uni.len)
  {
    gUnicodePtr.resize(uni.len);
    memcpy(UnicodePtr,uni.UnicodePtr,uni.len*sizeof(unsigned long));
  }else
  {
    gUnicodePtr.resize(0);
  }
}

UnicodeRep::UnicodeRep(const UnicodeRep::Remainder &r)
: gUnicodePtr(UnicodePtr)
{
	init(r.data,r.size,(UnicodeRep::EncodeType)r.encodetype);
}

UnicodeRep::UnicodeRep(
  const UnicodeRep::Remainder &r1,const UnicodeRep::Remainder &r2)
: gUnicodePtr(UnicodePtr)
{
  Remainder r(r1,r2);
  init(r.data,r.size,(UnicodeRep::EncodeType)r.encodetype);
}

UnicodeRep::~UnicodeRep()
{
//  if(UnicodePtr != nill)
//  {
//    delete [] UnicodePtr;
//  }
}

GUnicode::GUnicode(void)
{
  GP<UnicodeRep>::operator=(UnicodeRep::create());
}

GUnicode::GUnicode(GString &str)
{
  GP<UnicodeRep>::operator=(UnicodeRep::create());
  (*this)->init(str,str.length());
}

GUnicode::GUnicode(GString &str, unsigned int const i)
{
  GP<UnicodeRep>::operator=(UnicodeRep::create());
  unsigned int const  j=str.length();
  (*this)->init(str,(i>j)?j:i);
}
 
GUnicode::GUnicode(const GString &str)
{
  GP<UnicodeRep>::operator=(UnicodeRep::create());
  (*this)->init(str,str.length());
}

GUnicode::GUnicode(const GString &str, unsigned int const i)
{
  GP<UnicodeRep>::operator=(UnicodeRep::create());
  unsigned int const  j=str.length();
  (*this)->init(str,(i>j)?j:i);
}
 
GUnicode::GUnicode(unsigned long const * const wide)
{
  GP<UnicodeRep>::operator=(UnicodeRep::create());
  (*this)->init(wide,0,UnicodeRep::UCS4);
}

GUnicode::GUnicode(unsigned long const * const wide,unsigned int const i)
{
  GP<UnicodeRep>::operator=(UnicodeRep::create());
  (*this)->init(i?wide:0,i*sizeof(unsigned long),UnicodeRep::UCS4);
}

GUnicode::GUnicode(
  unsigned char const * const str,unsigned int const i,const EncodeType t)
{
  GP<UnicodeRep>::operator=(UnicodeRep::create());
  (*this)->init(i?str:0,i,(UnicodeRep::EncodeType &)t);
}

GUnicode::GUnicode(
  unsigned short const * const str,unsigned int const i,const EncodeType t)
{
  GP<UnicodeRep>::operator=(UnicodeRep::create());
  (*this)->init(i?str:0,i*sizeof(unsigned short),(UnicodeRep::EncodeType &)t);
}

GUnicode::GUnicode(
  unsigned long const * const str,unsigned int const i,const EncodeType t)
{
  GP<UnicodeRep>::operator=(UnicodeRep::create());
  (*this)->init(i?str:0,i*sizeof(unsigned long),(UnicodeRep::EncodeType &)t);
}

GP<UnicodeRep> 
UnicodeRep::concat(const UnicodeRep &uni1,const UnicodeRep &uni2)
{
  GP<UnicodeRep> retval;
  unsigned int const len1=uni1.length();
  unsigned int const len2=uni2.length();
  if(len2)
  {
    if(len1)
    {
      UnicodeRep *ptr=(retval=UnicodeRep::create());
      ptr->remainder.init(uni2.remainder);
      ptr->len=len1+len2;
      ptr->gUnicodePtr.resize(ptr->len+1);
      memcpy(ptr->UnicodePtr,uni1.UnicodePtr,len1*sizeof(unsigned long));
      memcpy(ptr->UnicodePtr+len1,uni2.UnicodePtr,len2*sizeof(unsigned long));
      ptr->UnicodePtr[ptr->len]=0;
      ptr->gs=uni1.gs+uni2.gs;
    }else
    {
      UnicodeRep *ptr=new UnicodeRep(uni2);
      retval=ptr;
    }
  }else if(uni1.remainder.size)
  {
    if(uni2.remainder.size)
    {
      UnicodeRep r(uni1.remainder,uni2.remainder);
      UnicodeRep *ptr=new UnicodeRep(uni1);
      retval=ptr;
      if(r.length())
      {
        ptr->remainder.init(0,0);
        retval=concat(*retval,r);
      }else
      {    
        retval->remainder.init(r.remainder);
      }
    }else
    {
      retval=new UnicodeRep(uni1);
    }
  }else
  {
    UnicodeRep *ptr=new UnicodeRep(uni1);
    retval=ptr;
    if(uni2.remainder.size)
    {
      ptr->remainder.init(uni2.remainder);
    }
  }
  return retval;
}

GP<UnicodeRep> 
UnicodeRep::concat(const GP<UnicodeRep> &guni1,const GP<UnicodeRep> &guni2)
{
  return ((!guni1)
    ?(guni2)
    :((!guni2)
      ?(guni1)
      :((guni2->length())
        ?((guni1->length())?concat(*guni1,*guni2):guni2)
        :((guni2->remainder.size)?concat(*guni1,*guni2):guni1))));
}

void
UnicodeRep::init(
  void const * const xbuf,unsigned int bufsize,EncodeType t)
{ 
  void const * const buf=checkmarks(xbuf,bufsize,t); 
  if(t != OTHER)
  {
    remainder.encodetype=(UnicodeRep::Remainder::EncodeType)t;
  }
  if(buf)
  {
    unsigned char const *eptr=(unsigned char *)buf;
    unsigned int j=0;
    switch(remainder.encodetype)
    {
      case UCS4:
      case UCS4BE:
      case UCS4LE:
      case UCS4_2143:
      case UCS4_3412:
        {
          if(bufsize)
          {
            unsigned int const  ii=bufsize/sizeof(unsigned long);
            for(j=0;(j<ii)&&*(unsigned long const *)eptr;j++,eptr+=sizeof(unsigned long)) EMPTY_LOOP;
          }else
          {
            while(*(unsigned long const *)eptr)
              eptr+=sizeof(unsigned long);
            j=((size_t)eptr-(size_t)buf)/sizeof(unsigned long);
          }
        }
        break;
      case UTF16:
      case UTF16BE:
      case UTF16LE:
        {
          if(bufsize)
          {
            unsigned int const  ii=bufsize/sizeof(unsigned short);
            for(j=0;(j<ii)&&*(unsigned short const *)eptr;j++,eptr+=sizeof(unsigned short)) EMPTY_LOOP;
          }else
          {
            while(*(unsigned short const *)eptr)
              eptr+=sizeof(unsigned short);
            j=((size_t)eptr-(size_t)buf)/sizeof(unsigned short);
          }
        }
        break;
      case UTF8:
      case EBCDIC:
        {
          if(bufsize)
          {
            for(j=0;(j<bufsize)&&*eptr;j++,eptr++) EMPTY_LOOP;
          }else
          {
            while(*eptr)
              eptr++;
            j=(size_t)eptr-(size_t)buf;
          }
        }
        break;
      default:
        break;
    }
    if (j)
    {
      unsigned char const *ptr=(unsigned char *)buf;
      gUnicodePtr.resize(j+1);
      switch(remainder.encodetype)
      {
        case UCS4:
          for(len=0;
            (ptr<eptr)&&(UnicodePtr[len]=*(unsigned long const *)ptr);
            len++,ptr+=sizeof(unsigned long const)) EMPTY_LOOP;
          UnicodePtr[len]=0;
          break;
        case UCS4BE:
          for(len=0;(UnicodePtr[len]=UCS4BEtoUCS4(ptr,eptr));len++) EMPTY_LOOP;
          break;
        case UCS4LE:
          for(len=0;(UnicodePtr[len]=UCS4LEtoUCS4(ptr,eptr));len++) EMPTY_LOOP;
          break;
        case UCS4_2143:
          for(len=0;(UnicodePtr[len]=UCS4_2143toUCS4(ptr,eptr));len++) EMPTY_LOOP;
          break;
        case UCS4_3412:
          for(len=0;(UnicodePtr[len]=UCS4_3412toUCS4(ptr,eptr));len++) EMPTY_LOOP;
          break;
        case UTF16:
          for(len=0;
            (UnicodePtr[len]=UTF16toUCS4((unsigned short const*&)ptr,eptr)) EMPTY_LOOP;
            len++) EMPTY_LOOP;
          break;
        case UTF16BE:
          for(len=0;(UnicodePtr[len]=UTF16BEtoUCS4(ptr,eptr));len++) EMPTY_LOOP;
          break;
        case UTF16LE:
          for(len=0;(UnicodePtr[len]=UTF16LEtoUCS4(ptr,eptr));len++) EMPTY_LOOP;
          break;
        case UTF8:
          for(len=0;(UnicodePtr[len]=UTF8toUCS4(ptr,eptr));len++) EMPTY_LOOP;
          break;
        case EBCDIC:
          for(len=0;(ptr<eptr)&&(UnicodePtr[len]=*ptr++);len++) EMPTY_LOOP;
          UnicodePtr[len]=0;
          break;
        default:
          UnicodePtr[(len=0)]=0;
          break;
      }
      if(len<j)
      {
        unsigned long *old_wide;
        GPBuffer<unsigned long> gold_wide(old_wide);
        gold_wide.swap(gUnicodePtr);
        gUnicodePtr.resize(len+1);
        for(j=0;j<len&&(UnicodePtr[j]=old_wide[j]);j++) EMPTY_LOOP;
//        delete [] old_wide;
        UnicodePtr[len]=0;
      }
    }
    if(t != UTF8)
    {
      initUTF8();
    }else
    {
      gs=GStringRep::UTF8::create((char const *)buf);
      if((size_t)eptr<(size_t)buf+bufsize)
      {
        gs.setat((size_t)eptr-(size_t)buf,0);
      }
    }
    if((size_t)eptr<(size_t)buf+bufsize)
    {
      remainder.init(eptr,bufsize+(size_t)buf-(size_t)eptr);
    }
  }else
  {
    gs=GStringRep::UTF8::create((size_t)0);
    remainder.init(0,0);
    len=0;
    gUnicodePtr.resize(0);
//    if(UnicodePtr != &nill)
//    {
//      delete [] UnicodePtr;
//    }
//    UnicodePtr=nill;
  }
}

void
UnicodeRep::initUTF8(void)
{
  unsigned char *buf;
  GPBuffer<unsigned char> gbuf(buf,length()*6+1);
  unsigned char *ptr=buf;
  for(unsigned long *wide=UnicodePtr;*wide;wide++)
  {
    ptr=GStringRep::UCS4toUTF8(wide[0],ptr);
  }
  gs=GStringRep::UTF8::create((char const *)buf);
}

static inline unsigned long
add_char(unsigned long const U, unsigned char const * const r)
{
  unsigned long const C=r[0];
  return ((C|0x3f) == 0xbf)?((U<<6)|(C&0x3f)):0;
}

unsigned long
UnicodeRep::UTF8toUCS4(
  unsigned char const *&s,void const * const eptr)
{
  return GStringRep::UTF8toUCS4(s,eptr);
}

unsigned long
UnicodeRep::UTF16toUCS4(unsigned short const *&s,void const * const eptr)
{
  unsigned long U=0;
  unsigned short const * const r=s+1;
  if(r <= eptr)
  {
    unsigned long const W1=s[0];
    if((W1<0xD800)&&(W1>0xDFFF))
    {
      if((U=W1))
      {
        s=r;
      }
    }else if(W1<=0xDBFF)
    {
      unsigned short const * const rr=r+1;
      if(rr <= eptr)
      {
        unsigned long const W2=s[1];
        if(((W2>=0xDC00)||(W2<=0xDFFF))&&((U=((W1&0x3ff)<<10)|(W2&0x3ff))))
        {
          s=rr;
        }else
        {
          U=(unsigned int)(-1)-W1;
          s=r;
        }
      }
    }
  }
  return U;
}

unsigned long
UnicodeRep::UTF16BEtoUCS4(unsigned char const *&s,void const * const eptr)
{
  unsigned long U=0;
  unsigned char const * const r=s+2;
  if(r <= eptr)
  {
    unsigned long const C1MSB=s[0];
    if((C1MSB<0xD8)&&(C1MSB>0xDF))
    {
      if((U=(((C1MSB&0x3)<<8)|((unsigned long)s[1]))))
      {
        s=r;
      }
    }else if(C1MSB<=0xDB)
    {
      unsigned char const * const rr=r+2;
      if(rr <= eptr)
      {
        unsigned long const C2MSB=s[2];
        if((C2MSB>=0xDC)||(C2MSB<=0xDF))
        {
          U=0x10000+((unsigned long)s[1]<<10)+(unsigned long)s[3]
            +(((C1MSB<<18)|(C2MSB<<8))&0xc0300);
          s=rr;
        }else
        {
          U=(unsigned int)(-1)-(((C1MSB&0x3)<<8)|((unsigned long)s[1]));
          s=r;
        }
      }
    }
  }
  return U;
}

unsigned long
UnicodeRep::UTF16LEtoUCS4(unsigned char const *&s,void const * const eptr)
{
  unsigned long U=0;
  unsigned long const C1MSB=s[1];
  unsigned char const * const r=s+2;
  if(r <= eptr)
  {
    if((C1MSB<0xD8)&&(C1MSB>0xDF))
    {
      if((U=(((C1MSB&0x3)<<8)|((unsigned long)s[1]))))
      {
        s=r;
      }
    }else if(C1MSB<=0xDB)
    {
      unsigned char const * const rr=r+2;
      if(rr <= eptr)
      {
        unsigned long const C2MSB=s[3];
        if((C2MSB>=0xDC)||(C2MSB<=0xDF))
        {
          U=0x10000+((unsigned long)s[0]<<10)+(unsigned long)s[2]
            +(((C1MSB<<18)|(C2MSB<<8))&0xc0300);
          s=rr;
        }else
        {
          U=(((C1MSB&0x3)<<8)|((unsigned long)s[1]));
          s=r;
        }
      }
    }
  }
  return U;
}

inline unsigned long
UnicodeRep::UCS4BEtoUCS4(unsigned char const *&s,void const * const eptr)
{
  unsigned long U=0;
  unsigned char const * const r=s+4;
  if(r<=eptr)
  {
    U=(((((((unsigned long)s[0]<<8)|(unsigned long)s[1])<<8)|(unsigned long)s[2])<<8)|(unsigned long)s[3]);
    if(U)
    {
      s=r;
    } 
  }
  return U;
}

inline unsigned long
UnicodeRep::UCS4LEtoUCS4(unsigned char const *&s,void const * const eptr)
{
  unsigned long U=0;
  unsigned char const * const r=s+4;
  if(r<=eptr)
  {
    U=(((((((unsigned long)s[3]<<8)|(unsigned long)s[2])<<8)|(unsigned long)s[1])<<8)|(unsigned long)s[0]);
    if(U)
    {
      s=r;
    }
  }
  return U;
}

inline unsigned long
UnicodeRep::UCS4_2143toUCS4(unsigned char const *&s,void const * const eptr)
{
  unsigned long U=0;
  unsigned char const * const r=s+4;
  if(r<=eptr)
  {
    U=(((((((unsigned long)s[1]<<8)|(unsigned long)s[0])<<8)|(unsigned long)s[3])<<8)|(unsigned long)s[2]);
    if(U)
    {
      s=r;
    }
  }
  return U;
}

inline unsigned long
UnicodeRep::UCS4_3412toUCS4(unsigned char const *&s,void const * const eptr)
{
  unsigned long U=0;
  unsigned char const * const r=s+4;
  if(r<=eptr)
  {
    U=(((((((unsigned long)s[2]<<8)|(unsigned long)s[3])<<8)|(unsigned long)s[0])<<8)|(unsigned long)s[1]);
    if(U)
    {
      s=r;
    }
  }
  return U;
}

GUnicode
GUnicode::substr_nr(unsigned int const from,unsigned int const to) const
{
  unsigned int const  len=length();
  unsigned int const  xto=(to>len)?len:to;
  GUnicode retval;
  if(from < xto)
  { 
    retval=GUnicode(((unsigned long const *)*this)+from,xto-from);
  }
  return retval;
}

GUnicode
GUnicode::substr(unsigned int const from,unsigned int const to) const
{
  unsigned int const  len=length();
  unsigned int const  xto=(to>len)?len:to;
  GUnicode retval;
  if(!from && xto == len)
  {
    retval=*this;
  }else if(from < xto)
  { 
    retval=GUnicode(((unsigned long const *)*this)+from,xto-from);
    if(xto == len)
    {
      unsigned int const s=get_remainder_size();
      if(s)
      {
        retval+=GUnicode((unsigned char const *)get_remainder(),s,get_remainderType());
      }
    }
  }
  return retval;
}

void
GUnicode::setat(unsigned int const pos,unsigned long const w)
{
  unsigned int const  len=length();
  if(!w)
  {
    operator=(substr(0,pos));
  }else if(pos<len)
  {
    operator=(((pos
              ?(substr(0,pos)+GUnicode(&w,1))
              :GUnicode(&w,1))+substr(pos+1,len+1)));
  }
}

UnicodeRep::Remainder::Remainder(UnicodeRep::Remainder::EncodeType t)
: encodetype(t),gdata(data,0,1),size(0) {}


UnicodeRep::Remainder::Remainder(const Remainder &r)
: encodetype(r.encodetype),gdata(data,r.size,1),size(r.size)
{
  if(size)
  {
    memcpy(data,r.data,size);
  }
}

UnicodeRep::Remainder::Remainder(const Remainder &r1,const Remainder &r2)
: encodetype(UnicodeRep::Remainder::OTHER),gdata(data,0,1),size(0)
{
  init(r1,r2);
}

UnicodeRep::Remainder::~Remainder()
{
}

inline void
UnicodeRep::Remainder::init(const Remainder &r)
{
  init(r.data,r.size,r.encodetype);
}

void 
UnicodeRep::Remainder::init(
  const Remainder &r1, const Remainder &r2)
{
  EncodeType t=
	  encodetype=(r2.encodetype == OTHER)?r1.encodetype:r2.encodetype;
  if(!r2.size)
  {
    init(r1.data,r1.size,t);
  }else if(r1.size
    &&((r1.encodetype == OTHER)||(r1.encodetype == t)))
  {
    gdata.resize(0,1);
    gdata.resize(size,1);
    size=r1.size+r2.size;
    memcpy(data,r1.data,r1.size);
    memcpy((unsigned char *)data+r2.size,r2.data,r2.size);
    if(t != OTHER)
    {
      encodetype=t;
    }
  }else
  {
    init(r2.data,r2.size,t);
  }
}

void
UnicodeRep::Remainder::init(
  void const * const d,size_t const s,UnicodeRep::Remainder::EncodeType t)
{
  gdata.resize(0,1);
  if(d && s)
  {
    gdata.resize(s,1);
    size=s;
    memcpy(data,d,s);
  }else
  {
    data=0;
    size=0;
  }
  if(t != OTHER)
  {
    encodetype=t;
  }
}


