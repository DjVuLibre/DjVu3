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
// $Id: GString.cpp,v 1.47 2001-04-12 15:28:34 bcr Exp $
// $Name:  $

#ifdef __GNUC__
#pragma implementation
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <wchar.h>
#include <locale.h>

#include "GString.h"
#include "debug.h"


// #ifndef WIN32
#define LIBICONV_PLUG true
// #endif


#ifdef HAS_ICONV
#include <iconv.h>//MBCS cvt
#include "libcharset.h"//MBCS cvt
#endif
// #include <locale.h>


// - Author: Leon Bottou, 04/1997

template <class TYPE>
GP<GStringRep>
GStringRep::create(const unsigned int sz, TYPE *)
{
  GStringRep *addr=0;
  if (sz > 0)
  {
    void *vma = ::operator new(sizeof(TYPE) + sz);
    addr = new (vma) TYPE;
    addr->size = sz;
    addr->data[sz] = 0;
  }
  return addr;
}

GP<GStringRep>
GStringRep::strdup(const char *s) const
{
  GP<GStringRep> retval;
  const int length=s?strlen(s):0;
  if(length>0)
  {
    retval=blank(length);
    char const * const end=s+length;
    char *ptr=retval->data;
    for(;*s&&(s!=end);ptr++)
    {
      ptr[0]=s++[0];
    }
    ptr[0]=0;
  }
  return retval;
}

GP<GStringRep>
GStringRep::substr(const char *s,const int start,const int len) const
{
  GP<GStringRep> retval;
  if(s && s[0])
  {
    const unsigned int length=(start<0 || len<0)?(unsigned int)strlen(s):(unsigned int)(-1);
    const char *startptr, *endptr;
    if(start<0)
    {
      startptr=s+length-start;
      if(startptr<s)
        startptr=s;
    }else
    { 
      startptr=s;
      for(const char * const ptr=s+start;(startptr<ptr)&&*startptr;++startptr)
        EMPTY_LOOP;
    }
    if(len<0)
    {
      if(s+length+1 < startptr+len)
      {
        endptr=startptr;
      }else
      {
        endptr=s+length+1-len;
      } 
    }else
    {
      endptr=startptr;
      for(const char * const ptr=startptr+len;(endptr<ptr)&&*endptr;++endptr)
        EMPTY_LOOP;
    }
    if(endptr>startptr)
    {
      retval=blank((size_t)(endptr-startptr));
      char *data=retval->data;
      for(; *startptr&&(startptr<endptr); ++startptr,++data)
      {
        data[0]=startptr[0];
      }
      data[0]=0;
    }
  }
  return retval;
}

GP<GStringRep>
GStringRep::append(const GP<GStringRep> &s2) const
{
  return s2
    ?(s2->concat(data,s2->data))
    :(GP<GStringRep>(const_cast<GStringRep *>(this)));
}

GP<GStringRep>
GStringRep::UTF8::append(const GP<GStringRep> &s2) const
{
  GP<GStringRep> retval;
  if(s2)
  {
    if(s2->isNative())
    {
      G_THROW("GStringRep.appendNativeToUTF8");
    }
    retval=concat(data,s2->data);
  }else
  {
    retval=const_cast<GStringRep::UTF8 *>(this); 
  }
  return retval;
}

GP<GStringRep>
GStringRep::Native::append(const GP<GStringRep> &s2) const
{
  GP<GStringRep> retval;
  if(s2)
  {
    if(s2->isUTF8())
    {
      G_THROW("GStringRep.appendUTF8toNative");
    }
    retval=concat(data,s2->data);
  }else
  {
    retval=const_cast<GStringRep::Native *>(this); 
  }
  return retval;
}

GP<GStringRep>
GStringRep::concat(const GP<GStringRep> &s1,const GP<GStringRep> &s2)
{
  return (s1?(s2?(s1->append(s2)):s1):s2);
}

GP<GStringRep>
GStringRep::concat(const char *s1,const char *s2) const
{
  const int length1=(s1?strlen(s1):0);
  const int length2=(s2?strlen(s2):0);
  const int length=length1+length2;
  GP<GStringRep> retval;
  if(length>0)
  {
    retval=blank(length);
    GStringRep &r=*retval;
    if(length1)
    {
      strcpy(r.data,s1);
      if(length2)
        strcat(r.data,s2);
    }else
    {
      strcpy(r.data,s2);
    }
  }
  return retval;
}

const char *GString::nullstr = "";

GString::GString( const GP<GStringRep> &rep) : GP<GStringRep>(rep)
{
  init();
}

GString::GString(const char dat)
{
  const GP<GStringRep> rep = GStringRep::create(1);
  GStringRep &r=*rep;
  r.data[0] = dat;
  r.data[1] = 0;
  (*this) = rep;
}

GString &
GString::operator= ( const GP<GStringRep> &rep)
{
  GP<GStringRep>::operator=(rep);
  gstr=(*this)->data;
  return (*this);
}

#if 0
GString& 
GString::operator= (const char *str)
{
  return ((*this)=GStringRep::create(str));
}
#endif

void
GString::empty( void )
{
  (*this) = GStringRep::create((size_t)0);
}

char *
GString::getbuf(int n)
{
  const char *s = (const char*)(*this);
  if (n < 0)  
    n = strlen(s);
  if (n <= 0) 
    { empty(); return 0; }
  // Copy string
  const GP<GStringRep> rep = (ptr?((*this)->blank(n)):GStringRep::create(n));
  GStringRep &r=*rep;
  char *d = r.data;
  while (d < r.data + n)
    if ((*d++ = *s)) 
      s += 1;
  r.data[n] = 0;
  (*this) = rep;
  return r.data;
}


GP<GStringRep>
GStringRep::upcase( void ) const
{
  GP<GStringRep> retval=const_cast<GStringRep *>(this);
  for(const unsigned char *d=(const unsigned char *)data;*d;d++)
  {
    if(islower(*d))
    {
      const GP<GStringRep> newval=(*(GP<GStringRep> *)(this))->blank(size);
      unsigned char *s=(unsigned char *)(newval->data);
      for(const unsigned char *e=(const unsigned char *)data;e!=d;e++,s++)
      {
        s[0]=e[0];
      }
      s++[0] = toupper(d++[0]);
      for(;*d;s++,d++)
      {
        if(islower(*d))
        {
          s[0] = toupper(d[0]);
        }else
        {
          s[0] = d[0];
        }
      }
      retval=newval;
      break;
    }
  }
  return retval;
}

GP<GStringRep>
GStringRep::downcase( void ) const
{
  GP<GStringRep> retval=const_cast<GStringRep *>(this);
  for(const unsigned char *d=(const unsigned char *)data;*d;d++)
  {
    if(isupper(*d))
    {
      const GP<GStringRep> newval=(*(GP<GStringRep> *)(this))->blank(size);
      unsigned char *s=(unsigned char *)(newval->data);
      for(const unsigned char *e=(const unsigned char *)data;e!=d;e++,s++)
      {
        s[0]=e[0];
      }
      s++[0] = tolower(d++[0]);
      for(;*d;s++,d++)
      {
        if(isupper(*d))
        {
          s[0] = tolower(d[0]);
        }else
        {
          s[0] = d[0];
        }
      }
      retval=(const GP<GStringRep> &)newval;
      break;
    }
  }
  return retval;
}

// Returns a copy of this string with characters used in XML escaped as follows:
//      '<'  -->  "&lt;"
//      '>'  -->  "&gt;"
//      '&'  -->  "&amp;"
//      '\'' -->  "&apos;"
//      '\"' -->  "&quot;"
//  Also escapes characters 0x00 through 0x7e and 0x80 through 0xff.
GUTF8String
GUTF8String::toEscaped( void ) const
{
  GUTF8String ret, special;
  char const * const head=(*this);
  char const *start=head;
  char const *s=start;
  while(*s)
  {
    char const *ss=0;
    switch(*s)
    {
    case '<':
      ss="&lt;";
      break;
    case '>':
      ss="&gt;";
      break;
    case '&':
      ss="&amp;";
      break;
    case '\47':
      ss="&apos;";
      break;
    case '\42':
      ss="&quot;";
      break;
    default:
      if( ( (signed char)(*s) < ' ' ) || ( *s > 126 ) )
      {
        special.format("&#%d;",(unsigned char)*s);
        ss=special;
      }
      break;
    }

    if(ss)
    {
      if(s!=start)
      {
        ret+=substr((size_t)start-(size_t)head,(size_t)s-(size_t)start)+ss;
      }else
      {
        ret+=ss;
      }
      start=(++s);
    }else
    {
      ++s;
    }
  }
  ret+=start;
//  DEBUG_MSG( "Escaped string is '" << ret << "'\n" );
  return (ret == *this)?(*this):ret;
}


static inline const GMap<GUTF8String,GUTF8String> &
BasicMap( void )
{
  static GMap<GUTF8String,GUTF8String> Basic;
  Basic["lt"]   = GUTF8String('<');
  Basic["gt"]   = GUTF8String('>');
  Basic["amp"]  = GUTF8String('&');
  Basic["apos"] = GUTF8String('\47');
  Basic["quot"] = GUTF8String('\42');
  return Basic;
}


GUTF8String
GUTF8String::fromEscaped( const GMap<GUTF8String,GUTF8String> ConvMap ) const
{
#if 0
  //  Available for debugging ConvMap arguments if needed
  for (GPosition here = ConvMap ; here; ++here)
  {
    DEBUG_MSG( "'" << ConvMap.key(here) << "' --> '" << ConvMap[here] << "'\n" );
  }
#endif

  GUTF8String ret;                  // Build output string here
  int start_locn = 0;           // Beginning of substring to skip
  int amp_locn;                 // Location of a found ampersand

  while( (amp_locn = search( '&', start_locn )) > -1 )
  {                             // Found the next apostrophe
                                // Locate the closing semicolon
    int semi_locn = search( ';', amp_locn );
    if( semi_locn < 0 ) break;  // No closing semicolon, exit and copy
                                //  the rest of the string.
    ret += substr( start_locn, amp_locn - start_locn );
    int const len = semi_locn - amp_locn - 1;
    if(len)
    {
      GUTF8String key = substr( amp_locn+1, len);
      //DEBUG_MSG( "key = '" << key << "'\n" );
      char const * s=key;
      if( s[0] == '#')
      {
        unsigned long value;
        char *ptr=0;
        if(s[1] == 'x' || s[1] == 'X')
        {
          value=strtoul((char const *)(s+2),&ptr,16);
        }else
        {
          value=strtoul((char const *)(s+1),&ptr,10);
        }
        if(!ptr)
        {
          ret+=GUTF8String((char const)(value));
        }else
        {
          ret += substr( amp_locn, semi_locn - amp_locn + 1 );
        }
      }else
      {  
        GPosition map_entry = ConvMap.contains( key );
        if( map_entry )
        {                           // Found in the conversion map, substitute
          ret += ConvMap[map_entry];
        } else
        {
          static const GMap<GUTF8String,GUTF8String> &Basic = BasicMap();
          GPosition map_entry = Basic.contains( key );
          if ( map_entry )
          {
            ret += Basic[map_entry];
          }else
          {
            ret += substr( amp_locn, len+2 );
          }
        }
      }
    }else
    {
      ret += substr( amp_locn, len+2 );
    }
    start_locn = semi_locn + 1;
//    DEBUG_MSG( "ret = '" << ret << "'\n" );
  }

                                // Copy the end of the string to the output
  ret += substr( start_locn, length()-start_locn );

//  DEBUG_MSG( "Unescaped string is '" << ret << "'\n" );
  return (ret == *this)?(*this):ret;
}

GUTF8String
GUTF8String::fromEscaped(void) const
{
  const GMap<GUTF8String,GUTF8String> nill;
  return fromEscaped(nill);
}


void
GString::setat(int n, char ch)
{
  int len = length();
  if (n < 0) 
    n += len;
  if (n < 0 || n>len) 
    throw_illegal_subscript();
  if (ch == 0)
    {
      getbuf(n);
    }
  else if (n == len)
    {
      char *data = getbuf(len + 1);
      data[n] = ch;
    }
  else 
    {
      char *data=getbuf(length());
      data[n]=ch;
    }
}

#ifdef WIN32
#define USE_VSNPRINTF _vsnprintf
#else
#ifdef linux
#define USE_VSNPRINTF vsnprintf
#endif
#endif

GString &
GString::format(const char fmt[], ... )
{
  va_list args;
  va_start(args, fmt);
  return (*this = GString(fmt,args));
}

GString &
GString::format(const char fmt[], va_list args)
{
  return (*this = GString(fmt,args));
}

GString::GString(const char fmt[], va_list args)
{
  int buflen=32768;
  char *buffer;
  GPBuffer<char> gbuffer(buffer,buflen);

  // Format string
#ifdef USE_VSNPRINTF
  while(USE_VSNPRINTF(buffer, buflen, fmt, args)<0)
  {
    gbuffer.resize(0);
    gbuffer.resize(buflen+32768);
  }
  va_end(args);
#else
  buffer[buflen-1] = 0;
  vsprintf(buffer, fmt, args);
  va_end(args);
  if (buffer[buflen-1])
  {
    // This isn't as fatal since it is on the stack, but we
    // definitely should stop the current operation.
    G_THROW("GString.overwrite");
  }
#endif
  // Go altering the string
  (*this) = GStringRep::create((const char *)buffer);
}

int 
GString::search(char c, int from) const
{
  const char *src = (const char*)(*this);
  int len = strlen(src);
  if (from<0)
    from += len;
  if (from<0 || from>=len)
    return -1;
  const char *str = (const char*)(*this);
  char *s = strchr(&str[from], c);
  return (s ? s - str : -1);
}

int 
GString::search(const char *str, int from) const
{
  const char *src = (const char*)(*this);
  int len = strlen(src);
  if (from<0)
    from += len;
  if (from<0 || from>=len)
    return -1;
  char *s = strstr(&src[from], str);
  return (s ? s - src : -1);
}

int 
GString::rsearch(char c, int from) const
{
  const char *src = (const char*)(*this);
  int len = strlen(src);
  if (from<0)
    from += len;
  if (from<0 || from>=len)
    return -1;
  int ans = -1;
  const char *s = src;
  while ((s=strchr(s,c)) && (s<=src+from))
    {
      ans = s - src;
      s += 1;
    }
  return ans;
}

int 
GString::rsearch(const char *str, int from) const
{
  const char *src = (const char*)(*this);
  int len = strlen(src);
  if (from<0)
    from += len;
  if (from<0 || from>=len)
    return -1;
  int ans = -1;
  const char *s = src;
  while ((s=strstr(s,str)) && (s<=src+from))
    {
      ans = s - src;
      s += 1;
    }
  return ans;
}

int
GString::contains(const char accept[],const int from) const
{
  int retval=(-1);
  if(accept && strlen(accept) && (from < (int)length()))
  {
    const char *src = (const char*)(*this)+((from<0)?0:from);
    const char *ptr;
    if((ptr=strpbrk(src,accept)))
    {
      retval=(int)(ptr-src);
    }
  }
  return retval;
}

GString& 
GString::operator+= (char ch)
{
  const char *str1 = (const char*)(*this);
  if(str1)
  {
    const GP<GStringRep> rep = (*this)->blank((str1?strlen(str1):0) + 1);
    GStringRep &r=*rep;
    strcpy(r.data, str1);
    int len = strlen(r.data);
    r.data[len] = ch;
    r.data[len+1] = 0;
    (*this) = rep;
  }else
  {
    return (*this)=GString(ch);
  }
  return (*this);
}


GString& 
GString::operator+= (const char *str2)
{
  if(str2&&str2[0])
  {
    (*this)=(*this)->concat(*this,GStringRep::create(str2)); 
  }
  return (*this);
}

GString& 
GString::operator+= (const GString &str2)
{
  return ((*this)=(*this)->concat(*this,str2));
}

bool
GString::is_int(void) const
{
  const char * buf=*this;
  char * ptr;
  strtol(buf, &ptr, 10);
  while(*ptr && isspace(*ptr))
    ptr++;
  return (*ptr==0);
}

bool
GString::is_float(void) const
{
  const char * buf=*this;
  char * ptr;
  strtod(buf, &ptr);
  while(*ptr && isspace(*ptr))
    ptr++;
  return (*ptr==0);
}

unsigned int 
hash(const GString &str)
{
  unsigned int x = 0;
  const char *s = (const char*)str;
  while (*s) 
    x = x ^ (x<<6) ^ (unsigned char)(*s++);
  return x;
}

void 
GString::throw_illegal_subscript()
{
  G_THROW("GString.bad_subscript");
}

#ifdef HAS_ICONV
class DjVuIconv {
private:
  char *buf;
  GPBuffer<char> gbuf;
  GMap<GString,iconv_t> iconv_map;
  GList<GString> codes;
  DjVuIconv() : gbuf(buf) {}
public:
  static size_t iconv_string(const GString &fromcode,const GString &tocode,
    const char *fromstr, const size_t fromstr_len, char *&retval,
    size_t &retval_len,GPBuffer<char> &);
  iconv_t getmap(const GString &fromcode, const GString &tocode);
  ~DjVuIconv();
};

DjVuIconv::~DjVuIconv()
{
  GPosition pos;
  while((pos=iconv_map))
  {
    iconv_t cd=iconv_map[pos];
    iconv_map.del(pos);
    if(cd != (iconv_t)(-1))
      iconv_close(cd);
  }
}

iconv_t
DjVuIconv::getmap(const GString &tocode, const GString &fromcode)
{
  GString code(tocode+fromcode);
  GPosition pos;
  iconv_t retval;
  if(!(pos=iconv_map.contains(code)))
  {
    codes.append(tocode);
    const char *tc=codes[codes.lastpos()];
    codes.append(fromcode);
    const char *fc=codes[codes.lastpos()];
    iconv_map[code]=retval=iconv_open(tc,fc);
  }else
  {
    retval=iconv_map[pos];
  }
  return retval;
}

size_t
DjVuIconv::iconv_string(const GString &tocode,const GString &fromcode,
  const char *fromstr, const size_t fromstr_len, char *&outbuf, size_t &outbuf_len,GPBuffer<char> &gbufout)
{
  static DjVuIconv conv;
  iconv_t cd=conv.getmap(tocode,fromcode);
  const char *inbuf=fromstr;
  size_t insize=fromstr_len;
  size_t outsize=fromstr_len*6;
  gbufout.resize(fromstr_len*6+1);
  char *retbuf=outbuf;
  outbuf[0]=0;
  const size_t retval=(cd!=(iconv_t)-1)?iconv(cd,&inbuf,&insize,&retbuf,&outsize):(size_t)(-1);
  if(retval != (size_t)(-1))
  {
    gbufout.resize(fromstr_len*6-outsize);
  }else
  {
    gbufout.resize(0);
  }
  return retval;
}
#if 0
static GString locale_charset(void)
{
  GString retval;
  GString glocale_charset=setlocale(LC_CTYPE,0);
  const int at=glocale_charset.search('@');
  if(at > -1)
  {
    glocale_charset.setat(at,0);
    const int dot=glocale_charset.search('.');
    if(dot > -1)
    {
      retval=glocale_charset.substr(dot+1,glocale_charset.length());
    }
  }
  return retval;
}
#endif
#endif /* HAS_ICONV */

GP<GStringRep> 
GStringRep::toNative(const bool noconvert) const
{
  GP<GStringRep> retval;
  if(!isUTF8()&&noconvert)
  {
    retval=GStringRep::Native::create(data);
  }else
  {
    if(data[0])
    {
      const size_t length=strlen(data);
      const unsigned char * const eptr=(const unsigned char *)(data+length);
      char *buf;
      GPBuffer<char> gbuf(buf,12*length+12); 
      char *r=buf;
      mbstate_t ps;
      for(const unsigned char *s=(const unsigned char *)data;(s<eptr)&& *s;)
      {
        const wchar_t w=UTF8toUCS4(s,eptr);
        char bytes[12];
        int i=wcrtomb(bytes,w,&ps);
        if(i<0)
        {
          r=buf;
          break;
        }else if(!i)
        {
          break;
        }else
        {
          for(int j=0;j<i;++j)
          {
            (r++)[0]=bytes[j];
          }
        }
      }
      r[0]=0;
      retval=Native::create(buf);
    }else
    {
      retval=Native::create((size_t)0);
    }
  }
  return retval;
}

GP<GStringRep>
GStringRep::Native::toNative(const bool nothrow) const
{
  if(!nothrow)
    G_THROW("GStringRep.NativeToNative");
  return const_cast<GStringRep::Native *>(this);
}

GP<GStringRep>
GStringRep::toUTF8(const bool noconvert) const
{
  GP<GStringRep> retval;
  if(!isNative()&&noconvert)
  {
    retval=GStringRep::UTF8::create(data);
  }else
  {
    size_t n=size;
    const char *source=data;
    int i;
    mbstate_t ps;
    unsigned char *buf;
    GPBuffer<unsigned char> gbuf(buf,n*6+1);
    unsigned char *ptr=buf;
    for(wchar_t w;
      (n>0)&&(i=mbrtowc(&w,source,n,&ps));
      n-=i,source+=i)
    {
      if(i<0)
      {
        gbuf.resize(1);
        ptr=buf;
        break;
      }
      ptr=UCS4toUTF8(w,ptr);
    }
    ptr[0]=0;
    retval=GStringRep::UTF8::create((const char *)buf);
  }
  return retval;
}

GP<GStringRep>
GStringRep::UTF8::toUTF8(const bool nothrow) const
{
  if(!nothrow)
    G_THROW("GStringRep.UTF8ToUTF8");
  return const_cast<GStringRep::UTF8 *>(this);
}

GNativeString
GString::UTF8ToNative(const bool currentlocale=false) const
{
  const char *source=(*this);
  GP<GStringRep> retval;
  GString lc_ctype;
  if(source && source[0]) 
  {
    GString lc_ctype(setlocale(LC_CTYPE,0));
    bool repeat;
    for(repeat=!currentlocale;;repeat=false)
    {
      retval=(*this)->toNative();
      if(!repeat
        || retval
        || (lc_ctype == setlocale(LC_CTYPE,""))
      ) break;
    }
    if(!repeat)
    {
      setlocale(LC_CTYPE,(const char *)lc_ctype);
    }
  }
  return GString(retval);
}

/*MBCS*/
GNativeString
GString::getUTF82Native(char* tocode) const
{ //MBCS cvt
  GNativeString retval;
  const size_t slen=length()+1;
  if(slen>1)
  {
    retval=UTF8ToNative();
    if(!retval.length())
    {
#ifdef HAS_ICONV 
      char *result;
      GPBuffer<char> gresult(result);
      size_t length = 0;
      const char *locales[]={0,"ASCII","ISO-8859-1","CP932","EUC-JP",0};
      GString glocale_charset=locale_charset();
      locales[0]=glocale_charset;
      const char **pos=locales;
      for(;pos[0];++pos)
      {
        if (pos[0][0]&&!DjVuIconv::iconv_string(pos[0], "UTF-8",
          s, slen, result, length, gresult))
        {
          if (tocode)
            strcpy((char*)tocode,pos[0]);
          retval=result;
          break;
        }
      }
      if(!pos[0])
      {
        if (tocode)
          strcpy(tocode,locales[0]);
        retval=*this; //invalid codeset
      }
#else /* HAS_ICONV */
      retval=*this;
#endif /* HAS_ICONV */
    }
  }
  return retval;
}

GUTF8String
GString::NativeToUTF8(void) const
{
  GP<GStringRep> retval;
  if(length())
  {
    const char *source=(*this);
    GString lc_ctype=setlocale(LC_CTYPE,0);
    bool repeat;
    for(repeat=true;;repeat=false)
    {
      if( (retval=GStringRep::NativeToUTF8(source)) )
      {
        if(source != GString(GStringRep::UTF8ToNative(retval->data)))
        {
          retval=GStringRep::UTF8::create((size_t)0);
        }
        if(!repeat || retval || (lc_ctype == setlocale(LC_CTYPE,"")))
          break;
      }
    }
    if(!repeat)
    {
      setlocale(LC_CTYPE,(const char *)lc_ctype);
    }
  }
  return GString(retval);
}

GUTF8String
GString::getNative2UTF8(const char *fromcode) const
{ //MBCS cvt
  const size_t slen=length()+1;
  GUTF8String retval;
  if(slen > 1)
  {
    retval=NativeToUTF8();
    if(!retval.length())
    {
#ifdef HAS_ICONV
      char *result;
      GPBuffer<char> gresult(result);
      const char *locales[]={0,"ASCII","ISO-8859-1","CP932","EUC-JP",0};
      size_t length = 0;
      GString glocale_charset;
      if (!fromcode || !fromcode[0])
      {
        fromcode="UTF-8";
        locales[0]=glocale_charset=locale_charset();
      }
      const char **pos=locales;
      for(;pos[0];++pos)
      {
        if (!DjVuIconv::iconv_string(fromcode, pos[0],
          s, slen, result, length, gresult))
        {
          retval=result;
          break;
        }
      }
      if(!pos[0])
      {
        retval=*this;
      }
#else /* HAS_ICONV */
      retval=*this;
#endif /* HAS_ICONV */
    }
  }
  return retval;
} /*MBCS*/

static inline unsigned long
add_char(unsigned long const U, unsigned char const * const r)
{
  unsigned long const C=r[0];
  return ((C|0x3f) == 0xbf)?((U<<6)|(C&0x3f)):0;
}

unsigned long
GStringRep::UTF8toUCS4(
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
        U=C1;
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

unsigned char *
GStringRep::UCS4toUTF8(const unsigned long w,unsigned char *ptr)
{
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
  return ptr;
}


