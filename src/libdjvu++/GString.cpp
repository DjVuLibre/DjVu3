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
// $Id: GString.cpp,v 1.37.2.1 2001-03-20 00:29:40 bcr Exp $
// $Name:  $

#ifdef __GNUC__
#pragma implementation
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "GString.h"
#include "debug.h"

#ifdef UNIX
#define LIBICONV_PLUG true
#endif
#include "iconv.h"//MBCS cvt
#include "libcharset.h"//MBCS cvt

// - Author: Leon Bottou, 04/1997

GStringRep *
GStringRep::xnew(unsigned int sz)
{
  if (sz > 0)
    {
      void *vma = ::operator new(sizeof(GStringRep) + sz);
      GStringRep *addr = new (vma) GStringRep;
      addr->size = sz;
      addr->data[sz] = 0;
      return addr;
    }
  return 0;
}

const char *GString::nullstr = "";

GString::GString(const char dat)
{
  GStringRep *rep = GStringRep::xnew(1);
  rep->data[0] = dat;
  (*this) = rep;
}

GString::GString(const char *str)
{
  if (!str) 
    return;
  int len = strlen(str);
  if (len <= 0)
    return;
  GStringRep *rep = GStringRep::xnew(strlen(str));
  strcpy(rep->data,str);
  (*this) = rep;
}

GString::GString(const char *str, unsigned int len)
{
  if (!str) return;
  unsigned int nlen = 0;
  while (nlen<len && str[nlen]) 
    nlen++;
  if (nlen <= 0) 
    return;
  GStringRep *rep = GStringRep::xnew(nlen);
  memcpy(rep->data,str,nlen);
  rep->data[nlen] = 0;
  (*this) = rep;
}

GString::GString(const GString& gs, int from, unsigned int len)
{
  GString ngs = gs;
  int gsl = ngs.length();
  if (from < 0)
    from += gsl;
  if (from<0 || from>=gsl)
    from = gsl;
  if ((int)len > gsl - from)
    len = gsl - from;
  if (len <= 0)
    return;
  GStringRep *rep = GStringRep::xnew(len);
  memcpy(rep->data, &ngs->data[from], len);
  rep->data[len] = 0;
  (*this) = rep;
}

GString::GString(const int number) 
{
  this->format("%d",number);
}
  
GString::GString(const double number) 
{
  this->format("%f",number);
}


GString& 
GString::operator= (const char *str)
{
  if (!str) empty();
  else
  {
     GStringRep *rep = GStringRep::xnew(strlen(str));
     if (rep) strcpy(rep->data,str);
     (*this) = rep;
  }
  return *this;
}

void
GString::empty( void )
{
  (*this) = (GStringRep*)0;
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
  GStringRep *rep = GStringRep::xnew(n);
  char *d = rep->data;
  while (d < rep->data + n)
    if ((*d++ = *s)) 
      s += 1;
  rep->data[n] = 0;
  (*this) = rep;
  return rep->data;
}


GString 
GString::upcase( void ) const
{
  GString ret = (*this);
  unsigned char *d = (unsigned char *)ret.getbuf(-1);
  for (; d && *d; d++)
    if (islower(*d))
      *d = toupper(*d);
  return ret;
}


GString
GString::downcase( void ) const
{
  GString ret = (*this);
  unsigned char *d = (unsigned char *)ret.getbuf(-1);
  for (; d && *d; d++)
    if (isupper(*d))
      *d = tolower(*d);
  return ret;
}


// Returns a copy of this string with characters used in XML escaped as follows:
//      '<'  -->  "&lt;"
//      '>'  -->  "&gt;"
//      '&'  -->  "&amp;"
//      '\'' -->  "&apos;"
//      '\"' -->  "&quot;"
//  Also escapes characters 0x00 through 0x7e and 0x80 through 0xff.
GString
GString::toEscaped( void ) const
{
  GString ret, special;
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
  return ret;
}


static inline const GMap<GString,GString> &
BasicMap( void )
{
  static GMap<GString,GString> Basic;
  Basic["lt"]   = GString('<');
  Basic["gt"]   = GString('>');
  Basic["amp"]  = GString('&');
  Basic["apos"] = GString('\47');
  Basic["quot"] = GString('\42');
  return Basic;
}


GString
GString::fromEscaped( const GMap<GString,GString> ConvMap ) const
{
#if 0
  //  Available for debugging ConvMap arguments if needed
  for (GPosition here = ConvMap ; here; ++here)
  {
    DEBUG_MSG( "'" << ConvMap.key(here) << "' --> '" << ConvMap[here] << "'\n" );
  }
#endif

  GString ret;                  // Build output string here
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
      GString key = substr( amp_locn+1, len);
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
          ret+=GString((char const)(value));
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
          static const GMap<GString,GString> &Basic = BasicMap();
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
    DEBUG_MSG( "ret = '" << ret << "'\n" );
  }

                                // Copy the end of the string to the output
  ret += substr( start_locn, length()-start_locn );

  DEBUG_MSG( "Unescaped string is '" << ret << "'\n" );
  return ret;
}

GString
GString::fromEscaped(void) const
{
  static const GMap<GString,GString> nill;
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
      if ((*this)->count > 1)
        getbuf(length());
      (*this)->data[n] = ch;
    }
}

#ifdef WIN32
#define USE_VSNPRINTF _vsnprintf
#else
#ifdef linux
#define USE_VSNPRINTF vsnprintf
#endif
#endif

void
GString::format(const char *fmt, ... )
{
  va_list args;
  va_start(args, fmt);
  format(fmt, args);
}

void
GString::format(const char *fmt, va_list args)
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
  (*this) = (const char *)buffer;
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


GString& 
GString::operator+= (char ch)
{
  const char *str1 = (const char*)(*this);
  GStringRep *rep = GStringRep::xnew(strlen(str1) + 1);
  strcpy(rep->data, str1);
  int len = strlen(rep->data);
  rep->data[len] = ch;
  rep->data[len+1] = 0;
  (*this) = rep;
  return *this;
}


GString& 
GString::operator+= (const char *str2)
{
  const char *str1 = (const char*)(*this);
  GStringRep *rep = GStringRep::xnew(strlen(str1) + strlen(str2));
  if (rep && str1) strcpy(rep->data, str1);
  if (rep && str2) strcat(rep->data, str2);
  (*this) = rep;
  return *this;
}

GString
GString::concat(const char *str1, const char *str2)
{
  GStringRep *rep = GStringRep::xnew(strlen(str1) + strlen(str2));
  if (rep && str1) strcpy(rep->data, str1);
  if (rep && str2) strcat(rep->data, str2);
  return GString(rep);
}

bool
GString::is_int(void) const
{
   const char * buf=*this;
   char * ptr;
   strtol(buf, &ptr, 10);
   while(*ptr && isspace(*ptr)) ptr++;
   return *ptr==0;
}

bool
GString::is_float(void) const
{
   const char * buf=*this;
   char * ptr;
   strtod(buf, &ptr);
   while(*ptr && isspace(*ptr)) ptr++;
   return *ptr==0;
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

/*MBCS*/
GString
GString::getUTF82Native(const char* tocode) const { //MBCS cvt
    const char * s=*this;
	char* result = NULL;
	size_t length = 0;
		if (iconv_string(locale_charset(), "UTF-8",
			s, s+strlen(s)+1, &result, &length) < 0) {
			if (iconv_string("ASCII", "UTF-8",
				s, s+strlen(s)+1, &result, &length) < 0) {
				if (iconv_string("ISO-8859-1", "UTF-8",
					s, s+strlen(s)+1, &result, &length) < 0) {
					if (iconv_string("CP932", "UTF-8",
						s, s+strlen(s)+1, &result, &length) < 0) {
						if (iconv_string("EUC-JP", "UTF-8",
							s, s+strlen(s)+1, &result, &length) < 0) {
							if (tocode) strcpy((char*)tocode,locale_charset()); 
							return *this; //invalid codeset
						} else {if (tocode) strcpy((char*)tocode,"EUC-JP"); return result;} //text converted to native
					} else {if (tocode) strcpy((char*)tocode,"CP932"); return result;} //text converted to native
				} else {if (tocode) strcpy((char*)tocode,"ISO-8859-1"); return result;} //text converted to native
			} else {if (tocode) strcpy((char*)tocode,"ASCII"); return result;} //text converted to native
		} else {if (tocode) strcpy((char*)tocode,locale_charset()); return result;} //text converted to native

}

GString
GString::getNative2UTF8(const char* fromcode) const { //MBCS cvt
    const char * s=*this;
	char* result = NULL;
	size_t length = 0;
	if (!(strcmp(fromcode,""))) {
		if (iconv_string("UTF-8", locale_charset(),
			s, s+strlen(s)+1, &result, &length) < 0) 
			if (iconv_string("UTF-8", "autodetect_jp",
				s, s+strlen(s)+1, &result, &length) < 0) 
				if (iconv_string("UTF-8", "autodetect_utf8",
					s, s+strlen(s)+1, &result, &length) < 0) 
					if (iconv_string("UTF-8", "autodetect_kr",
						s, s+strlen(s)+1, &result, &length) < 0) 
						return *this; //can't convert
					else 
						return result; //text converted
				else 
					return result; //text converted
			else 
				return result; //text converted
		else 
			return result; //text converted
	}
	else {
		if (!(strcmp(fromcode,"autodetect"))) {
			if (iconv_string("UTF-8", "autodetect_jp",
				s, s+strlen(s)+1, &result, &length) < 0) 
				if (iconv_string("UTF-8", "autodetect_utf8",
					s, s+strlen(s)+1, &result, &length) < 0) 
					if (iconv_string("UTF-8", "autodetect_kr",
						s, s+strlen(s)+1, &result, &length) < 0) 
						return *this; //can't convert
					else 
						return result; //text converted
				else 
					return result; //text converted
			else 
				return result; //text converted
		}
		else
			if (iconv_string("UTF-8", fromcode,
				s, s+strlen(s)+1, &result, &length) < 0) 
				return *this; //can't convert
			else 
				return result; //text converted
	}
}
/*MBCS*/
