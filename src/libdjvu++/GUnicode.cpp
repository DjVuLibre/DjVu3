//C-  -*- C++ -*-
//C- DjVu� Reference Library (v. 3.0)
//C- 
//C- Copyright � 1999-2001 LizardTech, Inc. All Rights Reserved.
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
// $Id: GUnicode.cpp,v 1.21 2001-06-05 03:19:58 bcr Exp $
// $Name:  $

#ifdef __GNUC__
#pragma implementation
#endif

#include "GString.h"
#ifdef UNIX
#define HAS_ICONV
#endif
#ifdef HAS_ICONV
#include <iconv.h>
#endif

static unsigned char nill=0;

static void const * 
checkmarks(void const * const xbuf,size_t &bufsize,GStringRep::EncodeType &rep)
{
  unsigned char const *buf=(unsigned char const *)xbuf;
  if(bufsize >= 2 || (xbuf && !bufsize && rep != GStringRep::XOTHER))
  {
    const unsigned int s=(((unsigned int)buf[0])<<8)+(unsigned int)buf[1];
    switch(s)
    {
      case 0:
        if((bufsize>=4)||(!bufsize && rep == GStringRep::XUCS4BE)
          ||(!bufsize && rep == GStringRep::XUCS4_2143))
        {
          const unsigned int s=(((unsigned int)buf[2])<<8)+(unsigned int)buf[3];
          if(s == 0xfeff)
          { 
            rep=GStringRep::XUCS4BE;
            buf+=4;
          }else if(s == 0xfffe)
          {
            rep=GStringRep::XUCS4_2143;
            buf+=4;
          }
        }
        break;
      case 0xfffe:
        if(((bufsize>=4)||(!bufsize && rep == GStringRep::XUCS4LE)) && !(unsigned char *)buf[2] && !(unsigned char *)buf[3])
        {
          rep=GStringRep::XUCS4LE;
          buf+=4;
        }else
        {
          rep=GStringRep::XUTF16LE;
          buf+=2;
        }
        break;
      case 0xfeff:
        if(((bufsize>=4)||(!bufsize && rep == GStringRep::XUCS4_3412))&&!(unsigned char *)buf[2] && !(unsigned char *)buf[3])
        {
          rep=GStringRep::XUCS4_3412;
          buf+=4;
        }else
        {
          rep=GStringRep::XUTF16LE;
          buf+=2;
        }
        break;
      case 0xefbb:
        if(((bufsize>=3)||(!bufsize && GStringRep::XUTF8 == rep))&&(buf[2] == 0xbf))
        {
          rep=GStringRep::XUTF8;
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
        buf=(const unsigned char *)&nill;
      }
    }
  }
  return buf;
}

class GStringRep::Unicode : public GStringRep::UTF8
{
public:
  GP<GStringRep> encoding;
  EncodeType encodetype;
  void *remainder;
  GPBufferBase gremainder;
public:
  Unicode(void);
  /// virtual destructor.
  virtual ~Unicode();

  static GP<GStringRep> create(const unsigned int sz);
  static GP<GStringRep> create(void const * const buf,const size_t,const EncodeType,
    const GP<GStringRep> &encoding);
  static GP<GStringRep> create( void const * const buf,
    unsigned int size, const EncodeType encodetype );
  static GP<GStringRep> create( void const * const buf,
    const unsigned int size, GP<GStringRep> encoding );
  static GP<GStringRep> create( void const * const buf,
    const unsigned int size, const GP<Unicode> &remainder );

protected:
  virtual void set_remainder( void const * const buf, const unsigned int size,
    const EncodeType encodetype );
  virtual void set_remainder( void const * const buf, const unsigned int size,
    const GP<GStringRep> &encoding );
  virtual void set_remainder( const GP<Unicode> &remainder );
  virtual GP<Unicode> get_remainder(void) const;
};
// static unsigned long UTF8toUCS4(unsigned char const *&,void const * const);
static unsigned long xUTF16toUCS4(unsigned short const *&s,void const * const);
static unsigned long UTF16BEtoUCS4(unsigned char const *&s,void const * const);
static unsigned long UTF16LEtoUCS4(unsigned char const *&s,void const * const);
static unsigned long UTS4BEtoUCS4(unsigned char const *&s,void const * const);
static unsigned long UCS4BEtoUCS4(unsigned char const *&s,void const * const);
static unsigned long UCS4LEtoUCS4(unsigned char const *&s,void const * const);
static unsigned long UCS4_3412toUCS4(unsigned char const *&s,void const * const);
static unsigned long UCS4_2143toUCS4(unsigned char const *&s,void const * const);

GP<GStringRep>
GStringRep::Unicode::create(const unsigned int sz)
{
  GP<GStringRep> gaddr;
  if (sz > 0)
  {
    GStringRep *addr;
    gaddr=(addr=new GStringRep::Unicode);
    addr->data=(char *)(::operator new(sz+1));
    addr->size = sz;
    addr->data[sz] = 0;
  }
  return gaddr;
}

GStringRep::Unicode::Unicode(void)
: encodetype(XUTF8), gremainder(remainder,0,1) {}

GStringRep::Unicode::~Unicode() {}

GP<GStringRep>
GStringRep::Unicode::create(
  void const * const xbuf,
  const unsigned int bufsize,
  const EncodeType t,
  const GP<GStringRep> &encoding)
{
  return (encoding->size)
    ?create(xbuf,bufsize,encoding)
    :create(xbuf,bufsize,t);
}

GP<GStringRep>
GStringRep::Unicode::create(
  void const * const xbuf,
  const unsigned int bufsize,
  const GP<Unicode> &xremainder )
{
  Unicode *r=xremainder;
  GP<GStringRep> retval;
  if(r)
  {
    const int s=r->gremainder;
    if(xbuf && bufsize)
    {
      if(s)
      {
        void *buf;
        GPBufferBase gbuf(buf,s+bufsize,1);
        memcpy(buf,r->remainder,s);
        memcpy((void *)((size_t)buf+s),xbuf,bufsize);
        retval=((r->encoding)
          ?create(buf,s+bufsize,r->encoding)
          :create(buf,s+bufsize,r->encodetype));
      }else
      {
        retval=((r->encoding)
          ?create(xbuf,bufsize,r->encoding)
          :create(xbuf,bufsize,r->encodetype));
      }
    }else if(s)
	{
      void *buf;
      GPBufferBase gbuf(buf,s,1);
      memcpy(buf,r->remainder,s);
      retval=((r->encoding)
        ?create(buf,s,r->encoding)
        :create(buf,s,r->encodetype));
	}else
    {
      retval=((r->encoding)
        ?create(0,0,r->encoding)
        :create(0,0,r->encodetype));
    }
  }else
  {
    retval=create(xbuf,bufsize,XUTF8);
  }
  return retval;
}

GP<GStringRep>
GStringRep::Unicode::create(
  void const * const xbuf,
  unsigned int bufsize,
  GP<GStringRep> encoding)
{
  GP<GStringRep> retval;
  GStringRep *e=encoding;
  if(e)
  {
    e=(encoding=e->upcase());
  }
  if(!e || !e->size)
  {
    retval=create(xbuf,bufsize,XOTHER);
  }else if(!e->cmp("UTF8") || !e->cmp("UTF-8"))
  {
    retval=create(xbuf,bufsize,XUTF8);
  }else if(!e->cmp("UTF16")|| !e->cmp("UTF-16")
    || !e->cmp("UCS2") || !e->cmp("UCS2"))
  {
    retval=create(xbuf,bufsize,XUTF16);
  }else if(!e->cmp("UCS4") || !e->cmp("UCS-4"))
  {
    retval=create(xbuf,bufsize,XUCS4);
  }else
  {
#ifdef HAS_ICONV
    EncodeType t=XOTHER;
    void const * const buf=checkmarks(xbuf,bufsize,t); 
    if(t != XOTHER)
    {
      retval=create(xbuf,bufsize,t);
    }else if(buf && bufsize)
    {
      unsigned char const *eptr=(unsigned char *)buf;
      unsigned int j=0;
      for(j=0;(j<bufsize)&&*eptr;j++,eptr++)
        EMPTY_LOOP;
      if (j)
      {
        unsigned char const *ptr=(unsigned char *)buf;
        if(e)
        {
          iconv_t cv=iconv_open("UTF-8",(const char *)e);
          if(cv == (iconv_t)(-1))
          { 
            const int i=e->search('-');
            if(i>=0)
            {
              cv=iconv_open("UTF-8",e->data+i+1);
            }
          }
          if(cv == (iconv_t)(-1))
          { 
            retval=create(0,0,XOTHER);
          }else
          {
            size_t ptrleft=(eptr-ptr); 
            char *utf8buf;
            size_t pleft=6*ptrleft+1;
            GPBuffer<char> gutf8buf(utf8buf,pleft);
            char *p=utf8buf;
            unsigned char const *last=ptr;
            for(;iconv(cv,(const char **)&ptr,&ptrleft,&p,&pleft);last=ptr)
              EMPTY_LOOP;
            iconv_close(cv);
            retval=create(utf8buf,(size_t)last-(size_t)buf,t);
            retval->set_remainder(last,(size_t)eptr-(size_t)last,e);
          }
        }
      }else
      {
        retval=create(0,0,XOTHER);
        retval->set_remainder(0,0,e);
      }
    }
#else
    retval=create(xbuf,bufsize,XOTHER);
#endif
  }
  return retval;
}

GP<GStringRep>
GStringRep::Unicode::create(
  void const * const xbuf,
  unsigned int bufsize,
  EncodeType t)
{
  GP<GStringRep> gretval;
  GStringRep *retval=0;
  void const * const buf=checkmarks(xbuf,bufsize,t); 
  if(buf && bufsize)
  {
    unsigned char const *eptr=(unsigned char *)buf;
    unsigned int maxutf8size=0;
    void const* const xeptr=(void const *)((size_t)eptr+bufsize);
    switch(t)
    {
      case XUCS4:
      case XUCS4BE:
      case XUCS4LE:
      case XUCS4_2143:
      case XUCS4_3412:
      {
        for(unsigned long w;
          (eptr<xeptr)&&(w=*(unsigned long const *)eptr);
          eptr+=sizeof(unsigned long))
        {
          maxutf8size+=(w>0x7f)?6:1;
        }
        break;
      }
      case XUTF16:
      case XUTF16BE:
      case XUTF16LE:
      {
        for(unsigned short w;
          (eptr<xeptr)&&(w=*(unsigned short const *)eptr);
          eptr+=sizeof(unsigned short))
        {
          maxutf8size+=(w>0x7f)?3:1;
        }
        break;
      }
      case XUTF8:
        for(;(eptr<xeptr)&&*eptr;maxutf8size++,eptr++)
          EMPTY_LOOP;
        break;
      case XEBCDIC:
        for(;(eptr<xeptr)&&*eptr;eptr++)
        {
          maxutf8size+=(*eptr>0x7f)?2:1;
        }
        break;
      default:
        break;
    }
    unsigned char *utf8buf=0;
    GPBuffer<unsigned char> gutf8buf(utf8buf,maxutf8size+1);
    utf8buf[0]=0;
    if (maxutf8size)
    {
      unsigned char *optr=utf8buf;
      int len=0;
      unsigned char const *iptr=(unsigned char *)buf;
      unsigned long w;
      switch(t)
      {
        case XUCS4:
          for(;
            (iptr<eptr)&&(w=*(unsigned long const *)iptr);
            len++,iptr+=sizeof(unsigned long const))
          {
            optr=UCS4toUTF8(w,optr);
          }
          break;
        case XUCS4BE:
          for(;(w=UCS4BEtoUCS4(iptr,eptr));len++)
          {
            optr=UCS4toUTF8(w,optr);
          }
          break;
        case XUCS4LE:
          for(;(w=UCS4LEtoUCS4(iptr,eptr));len++)
          {
            optr=UCS4toUTF8(w,optr);
          }
          break;
        case XUCS4_2143:
          for(;(w=UCS4_2143toUCS4(iptr,eptr));len++)
          {
            optr=UCS4toUTF8(w,optr);
          }
          break;
        case XUCS4_3412:
          for(;(w=UCS4_3412toUCS4(iptr,eptr));len++)
          {
            optr=UCS4toUTF8(w,optr);
          }
          break;
        case XUTF16:
          for(;
            (w=xUTF16toUCS4((unsigned short const*&)iptr,eptr));
            len++)
          {
            optr=UCS4toUTF8(w,optr);
          }
          break;
        case XUTF16BE:
          for(;(w=UTF16BEtoUCS4(iptr,eptr));len++)
          {
            optr=UCS4toUTF8(w,optr);
          }
          break;
        case XUTF16LE:
          for(;(w=UTF16LEtoUCS4(iptr,eptr));len++)
          {
            optr=UCS4toUTF8(w,optr);
          }
          break;
        case XUTF8:
          for(;(w=UTF8toUCS4(iptr,eptr));len++)
          {
            optr=UCS4toUTF8(w,optr);
          }
          break;
        case XEBCDIC:
          for(;(iptr<eptr)&&(w=*iptr++);len++)
          {
            optr=UCS4toUTF8(w,optr);
          }
          break;
        default:
          break;
      }
      const unsigned int size=(size_t)optr-(size_t)utf8buf;
      if(size)
      {
		  retval=(gretval=GStringRep::Unicode::create(size));
        memcpy(retval->data,utf8buf,size);
      }else
      {
		  retval=(gretval=GStringRep::Unicode::create(1));
        retval->size=size;
      }
      retval->data[size]=0;
      gutf8buf.resize(0);
      const size_t s=(size_t)eptr-(size_t)iptr;
      retval->set_remainder(iptr,s,t);
    }
  }
  if(!retval)
  {
    retval=(gretval=GStringRep::Unicode::create(1));
    retval->data[0]=0;
    retval->size=0;
    retval->set_remainder(0,0,t);
  }
  return gretval;
}

static unsigned long
xUTF16toUCS4(unsigned short const *&s,void const * const eptr)
{
  unsigned long U=0;
  unsigned short const * const r=s+1;
  if(r <= eptr)
  {
    unsigned long const W1=s[0];
    if((W1<0xD800)||(W1>0xDFFF))
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
        if(((W2>=0xDC00)||(W2<=0xDFFF))&&((U=(0x1000+((W1&0x3ff)<<10))|(W2&0x3ff))))
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

static unsigned long
UTF16BEtoUCS4(unsigned char const *&s,void const * const eptr)
{
  unsigned long U=0;
  unsigned char const * const r=s+2;
  if(r <= eptr)
  {
    unsigned long const C1MSB=s[0];
    if((C1MSB<0xD8)||(C1MSB>0xDF))
    {
      if((U=((C1MSB<<8)|((unsigned long)s[1]))))
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
          U=(unsigned int)(-1)-((C1MSB<<8)|((unsigned long)s[1]));
          s=r;
        }
      }
    }
  }
  return U;
}

static unsigned long
UTF16LEtoUCS4(unsigned char const *&s,void const * const eptr)
{
  unsigned long U=0;
  unsigned long const C1MSB=s[1];
  unsigned char const * const r=s+2;
  if(r <= eptr)
  {
    if((C1MSB<0xD8)||(C1MSB>0xDF))
    {
      if((U=((C1MSB<<8)|((unsigned long)s[0]))))
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
          U=(unsigned int)(-1)-((C1MSB<<8)|((unsigned long)s[1]));
          s=r;
        }
      }
    }
  }
  return U;
}

static unsigned long
UCS4BEtoUCS4(unsigned char const *&s,void const * const eptr)
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

static unsigned long
UCS4LEtoUCS4(unsigned char const *&s,void const * const eptr)
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

static unsigned long
UCS4_2143toUCS4(unsigned char const *&s,void const * const eptr)
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

static unsigned long
UCS4_3412toUCS4(unsigned char const *&s,void const * const eptr)
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

void
GStringRep::Unicode::set_remainder( void const * const buf,
   const unsigned int size, const EncodeType xencodetype )
{
  gremainder.resize(size,1);
  if(size)
    memcpy(remainder,buf,size);
  encodetype=xencodetype;
  encoding=0;
}

void
GStringRep::Unicode::set_remainder( void const * const buf,
   const unsigned int size, const GP<GStringRep> &xencoding )
{
  gremainder.resize(size,1);
  if(size)
    memcpy(remainder,buf,size);
  encoding=xencoding;
  encodetype=XOTHER;
}

void
GStringRep::Unicode::set_remainder( const GP<GStringRep::Unicode> &xremainder )
{
  if(xremainder)
  {
    const int size=xremainder->gremainder;
    gremainder.resize(size,1);
    if(size)
      memcpy(remainder,xremainder->remainder,size);
    encodetype=xremainder->encodetype;
  }else
  {
    gremainder.resize(0,1);
    encodetype=XUTF8;
  }
}

GP<GStringRep::Unicode>
GStringRep::Unicode::get_remainder( void ) const
{
  return const_cast<GStringRep::Unicode *>(this);
}

GUTF8String 
GUTF8String::create(void const * const buf,const unsigned int size,
    const EncodeType encodetype, const GUTF8String &encoding)
{
  return encoding.length()
    ?create(buf,size,encodetype)
    :create(buf,size,encoding);
}

GUTF8String 
GUTF8String::create( void const * const buf,
  unsigned int size, const EncodeType encodetype )
{
  GUTF8String retval;
  retval.init(GStringRep::Unicode::create(buf,size,encodetype));
  return retval;
}

GUTF8String 
GUTF8String::create( void const * const buf,
  const unsigned int size, const GP<GStringRep::Unicode> &remainder)
{
  GUTF8String retval;
  retval.init(GStringRep::Unicode::create(buf,size,remainder));
  return retval;
}

GUTF8String 
GUTF8String::create( void const * const buf,
  const unsigned int size, const GUTF8String &encoding )
{
  GUTF8String retval;
  retval.init(GStringRep::Unicode::create(buf,size,encoding ));
  return retval;
}

