//C-  Copyright © 2000-2001, LizardTech, Inc. All Rights Reserved.
//C-              Unauthorized use prohibited.
//
// $Id: GUnicode.cpp,v 1.2 2001-01-17 00:22:52 bcr Exp $
// $Name:  $

#ifdef __GNUC__
#pragma implementation
#endif

#include "GUnicode.h"

static unsigned long const nill=0;

UnicodeRep::UnicodeRep(void)
: len(0), remainder(UnicodeRep::Remainder::OTHER), UnicodePtr(const_cast<unsigned long *>(&nill))
{
}

UnicodeRep::UnicodeRep(const UnicodeRep &uni)
: len(uni.len), gs(uni.gs), remainder(uni.remainder)
{
  if(uni.len)
  {
    UnicodePtr=new unsigned long[uni.len];
    memcpy(UnicodePtr,uni.UnicodePtr,uni.len*sizeof(unsigned long));
  }else
  {
    UnicodePtr=const_cast<unsigned long *>(&nill);
  }
}

UnicodeRep::UnicodeRep(const UnicodeRep::Remainder &r)
{
	init(r.data,r.size,(UnicodeRep::EncodeType)r.encodetype);
}

UnicodeRep::UnicodeRep(
  const UnicodeRep::Remainder &r1,const UnicodeRep::Remainder &r2)
{
  Remainder r(r1,r2);
  init(r.data,r.size,(UnicodeRep::EncodeType)r.encodetype);
}

UnicodeRep::~UnicodeRep()
{
  if(UnicodePtr != &nill)
  {
    delete [] UnicodePtr;
  }
}


GUnicode::GUnicode(void)
{
  GP<UnicodeRep>::operator=(new UnicodeRep());
}

GUnicode::GUnicode(GString &str)
{
  GP<UnicodeRep>::operator=(new UnicodeRep());
  (*this)->init(str,str.length());
}

GUnicode::GUnicode(GString &str, unsigned int const i)
{
  GP<UnicodeRep>::operator=(new UnicodeRep());
  unsigned int const  j=str.length();
  (*this)->init(str,(i>j)?j:i);
}
 
GUnicode::GUnicode(const GString &str)
{
  GP<UnicodeRep>::operator=(new UnicodeRep());
  (*this)->init(str,str.length());
}

GUnicode::GUnicode(const GString &str, unsigned int const i)
{
  GP<UnicodeRep>::operator=(new UnicodeRep());
  unsigned int const  j=str.length();
  (*this)->init(str,(i>j)?j:i);
}
 
GUnicode::GUnicode(unsigned long const * const wide)
{
  GP<UnicodeRep>::operator=(new UnicodeRep());
  (*this)->init(wide,0,UnicodeRep::UCS4);
}

GUnicode::GUnicode(unsigned long const * const wide,unsigned int const i)
{
  GP<UnicodeRep>::operator=(new UnicodeRep());
  (*this)->init(i?wide:0,i*sizeof(unsigned long),UnicodeRep::UCS4);
}

GUnicode::GUnicode(
  unsigned char const * const str,unsigned int const i,const EncodeType t)
{
  GP<UnicodeRep>::operator=(new UnicodeRep());
  (*this)->init(i?str:0,i,(UnicodeRep::EncodeType &)t);
}

GUnicode::GUnicode(
  unsigned short const * const str,unsigned int const i,const EncodeType t)
{
  GP<UnicodeRep>::operator=(new UnicodeRep());
  (*this)->init(i?str:0,i*sizeof(unsigned short),(UnicodeRep::EncodeType &)t);
}

GUnicode::GUnicode(
  unsigned long const * const str,unsigned int const i,const EncodeType t)
{
  GP<UnicodeRep>::operator=(new UnicodeRep());
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
      UnicodeRep *ptr=new UnicodeRep();
      retval=ptr;
      ptr->remainder.init(uni2.remainder);
      ptr->len=len1+len2;
      ptr->UnicodePtr=new unsigned long[ptr->len+1];
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
  void const * const buf,unsigned int const i,const EncodeType t)
{ 
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
          if(i)
          {
            unsigned int const  ii=i/sizeof(unsigned long);
            for(j=0;(j<ii)&&*(unsigned long const *)eptr;j++,eptr+=sizeof(unsigned long));
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
          if(i)
          {
            unsigned int const  ii=i/sizeof(unsigned short);
            for(j=0;(j<ii)&&*(unsigned short const *)eptr;j++,eptr+=sizeof(unsigned short));
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
          if(i)
          {
            for(j=0;(j<i)&&*eptr;j++,eptr++);
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
      UnicodePtr=new unsigned long [j+1];
      switch(remainder.encodetype)
      {
        case UCS4:
          for(len=0;
            (ptr<eptr)&&(UnicodePtr[len]=*(unsigned long const *)ptr);
            len++,ptr+=sizeof(unsigned long const));
          UnicodePtr[len]=0;
          break;
        case UCS4BE:
          for(len=0;(UnicodePtr[len]=UCS4BEtoWideChar(ptr,eptr));len++);
          break;
        case UCS4LE:
          for(len=0;(UnicodePtr[len]=UCS4LEtoWideChar(ptr,eptr));len++);
          break;
        case UCS4_2143:
          for(len=0;(UnicodePtr[len]=UCS4_2143toWideChar(ptr,eptr));len++);
          break;
        case UCS4_3412:
          for(len=0;(UnicodePtr[len]=UCS4_3412toWideChar(ptr,eptr));len++);
          break;
        case UTF16:
          for(len=0;
            (UnicodePtr[len]=UTF16toWideChar((unsigned short const*&)ptr,eptr));
            len++);
          break;
        case UTF16BE:
          for(len=0;(UnicodePtr[len]=UTF16BEtoWideChar(ptr,eptr));len++);
          break;
        case UTF16LE:
          for(len=0;(UnicodePtr[len]=UTF16LEtoWideChar(ptr,eptr));len++);
          break;
        case UTF8:
          for(len=0;(UnicodePtr[len]=UTF8toWideChar(ptr,eptr));len++);
          break;
        case EBCDIC:
          for(len=0;(ptr<eptr)&&(UnicodePtr[len]=*ptr++);len++);
          UnicodePtr[len]=0;
          break;
        default:
          UnicodePtr[(len=0)]=0;
          break;
      }
      if(len<j)
      {
        unsigned long *old_wide=UnicodePtr;
        UnicodePtr=new unsigned long [len+1];
        for(j=0;j<len&&(UnicodePtr[j]=old_wide[j]);j++);
        delete [] old_wide;
        UnicodePtr[len]=0;
      }
    }
    if(t != UTF8)
    {
      initUTF8();
    }else
    {
      gs=(char const *)buf;
      if((size_t)eptr<(size_t)buf+i)
      {
        gs.setat((size_t)eptr-(size_t)buf,0);
      }
    }
    if((size_t)eptr<(size_t)buf+i)
    {
      remainder.init(eptr,i+(size_t)buf-(size_t)eptr);
    }
  }else
  {
    gs=GString();
    remainder.init(0,0);
    len=0;
    if(UnicodePtr != &nill)
    {
      delete [] UnicodePtr;
    }
    UnicodePtr=nill;
  }
}

void
UnicodeRep::initUTF8(void)
{
  GString temp;
  unsigned char *ptr=(unsigned char *)(temp.getbuf(length()*6)); 
  for(unsigned long *wide=UnicodePtr;*wide;wide++)
  {
    unsigned long const w=*wide;
    if(w <= 0x7f)
    {
      *ptr++ = (unsigned char)w;
    }else if(w <= 0x7ff)
    {
      *ptr++ = (unsigned char)((w>>6)|0xC0);
      *ptr++ = (unsigned char)((w|0x80)&0xBF);
    }else if(w <= 0xFFFF)
    {
      *ptr++ = (unsigned char)((w>>12)|0xE0);
      *ptr++ = (unsigned char)(((w>>6)|0x80)&0xBF);
      *ptr++ = (unsigned char)((w|0x80)&0xBF);
    }else if(w <= 0x1FFFFF)
    {
      *ptr++ = (unsigned char)((w>>18)|0xF0);
      *ptr++ = (unsigned char)(((w>>12)|0x80)&0xBF);
      *ptr++ = (unsigned char)(((w>>6)|0x80)&0xBF);
      *ptr++ = (unsigned char)((w|0x80)&0xBF);
    }else if(w <= 0x3FFFFFF)
    {
      *ptr++ = (unsigned char)((w>>24)|0xF8);
      *ptr++ = (unsigned char)(((w>>18)|0x80)&0xBF);
      *ptr++ = (unsigned char)(((w>>12)|0x80)&0xBF);
      *ptr++ = (unsigned char)(((w>>6)|0x80)&0xBF);
      *ptr++ = (unsigned char)((w|0x80)&0xBF);
    }else if(w <= 0x7FFFFFF)
    {
      *ptr++ = (unsigned char)((w>>30)|0xFC);
      *ptr++ = (unsigned char)(((w>>24)|0x80)&0xBF);
      *ptr++ = (unsigned char)(((w>>18)|0x80)&0xBF);
      *ptr++ = (unsigned char)(((w>>12)|0x80)&0xBF);
      *ptr++ = (unsigned char)(((w>>6)|0x80)&0xBF);
      *ptr++ = (unsigned char)((w|0x80)&0xBF);
    }else
    {
      *ptr++ = 0;
    }
  }
  gs=(char const *)temp;
}

static inline unsigned long
add_char(unsigned long const U, unsigned char const * const r)
{
  unsigned long const C=r[0];
  return ((C|0x3f) == 0xbf)?((U<<6)|(C&0x3f)):0;
}

unsigned long
UnicodeRep::UTF8toWideChar(
  unsigned char const *&s,void const * const eptr)
{
  unsigned long U=0;
  unsigned char const *r=s;
  if(r < eptr)
  {
    unsigned long const C1=r++[0];
    if(C1&0x80)
    {
      if(r < eptr)
      {
        if((U=((C1&0x40)?add_char(U,r++):0)))
        {
          if(C1&0x20)
          {
            if(r < eptr)
            {
              if((U=add_char(U,r++)))
              {
                if(C1&0x10)
                {
                  if(r < eptr)
                  {
                    if((U=add_char(U,r++)))
                    {
                      if(C1&0x8)
                      {
                        if(r < eptr)
                        {
                          if((U=add_char(U,r++)))
                          {
                            if(C1&0x4)
                            {
                              if(r < eptr)
                              {
                                if((U=((!(C1&0x2))?(add_char(U,r++)&0x7fffffff):0)))
                                {
                                  s=r;
                                }else
                                {
                                  U=(unsigned int)(-1)-s++[0];
                                }
                              }
                            }else if((U=((U&0x4000000)?0:(U&0x3ffffff))))
                            {
                              s=r;
                            }
                          }else
                          {
                            U=(unsigned int)(-1)-s++[0];
                          }
                        }
                      }else if((U=((U&0x200000)?0:(U&0x1fffff))))
                      {
                        s=r;
                      }
                    }else
                    {
                      U=(unsigned int)(-1)-s++[0];
                    }
                  }
                }else if((U=((U&0x10000)?0:(U&0xffff))))
                {
                  s=r;
                }
              }else
              {
                U=(unsigned int)(-1)-s++[0];
              }
            }
          }else if((U=((U&0x800)?0:(U&0x7ff))))
          {
            s=r;
          }
        }else
        {
          U=(unsigned int)(-1)-s++[0];
        }
      }
    }else if((U=C1))
    {
      s=r;
    }
  }
  return U;
}

unsigned long
UnicodeRep::UTF16toWideChar(unsigned short const *&s,void const * const eptr)
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
UnicodeRep::UTF16BEtoWideChar(unsigned char const *&s,void const * const eptr)
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
UnicodeRep::UTF16LEtoWideChar(unsigned char const *&s,void const * const eptr)
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
UnicodeRep::UCS4BEtoWideChar(unsigned char const *&s,void const * const eptr)
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
UnicodeRep::UCS4LEtoWideChar(unsigned char const *&s,void const * const eptr)
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
UnicodeRep::UCS4_2143toWideChar(unsigned char const *&s,void const * const eptr)
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
UnicodeRep::UCS4_3412toWideChar(unsigned char const *&s,void const * const eptr)
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


