//C-  -*- C++ -*-
//C- DjVu® Reference Library (v. 3.0)
//C- 
//C- Copyright © 1999-2000 LizardTech, Inc. All Rights Reserved.
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
//C- 
// 
// $Id: GString.cpp,v 1.28 2000-12-01 22:54:16 fcrary Exp $
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
GString::empty()
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
GString::upcase() const
{
  GString ret = (*this);
  unsigned char *d = (unsigned char *)ret.getbuf(-1);
  for (; d && *d; d++)
    if (islower(*d))
      *d = toupper(*d);
  return ret;
}


GString
GString::downcase() const
{
  GString ret = (*this);
  unsigned char *d = (unsigned char *)ret.getbuf(-1);
  for (; d && *d; d++)
    if (isupper(*d))
      *d = tolower(*d);
  return ret;
}


/** Returns a copy of this string with characters used in XML escaped as follows:
        '<'  -->  "&lt;"
        '>'  -->  "&gt;"
        '&'  -->  "&amp;"
        '\'' -->  "&apos;"
        '\"' -->  "&quot;"
    Also escapes characters 0x00 through 0x1f should they appear.*/
GString
GString::toEscaped() const
{
  GString ret;
  for( unsigned i = 0 ; i < length() ; i++ )
  {
    char d = (*this)->data[i];
    switch( d )
    {
    case '<':
      ret += "&lt;";
      break;
    case '>':
      ret += "&gt;";
      break;
    case '&':
      ret += "&amp;";
      break;
    case '\'':
      ret += "&apos;";
      break;
    case '\"':
      ret += "&quot;";
      break;
    default:
      if( d < ' ' )
        ret += "&#" + GString(int( d )) + ";";
      else
        ret += d;
      break;
    }
  }
  DEBUG_MSG( "Escaped string is '" << ret << "'\n" );
  return ret;
}


GString
GString::fromEscaped( GMap<GString,GString> ConvMap )
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

  while( (amp_locn = this->search( '&', start_locn )) > -1 )
  {                             // Found the next apostrophe
    ret += this->substr( start_locn, amp_locn - start_locn );
                                // Locate the closing semicolon
    int semi_locn = this->search( ';', amp_locn );
    if( semi_locn < 0 ) break;  // No closing semicolon, exit and copy
                                //  the rest of the string.
    GString key = this->substr( amp_locn, semi_locn-amp_locn+1 );
    //DEBUG_MSG( "key = '" << key << "'\n" );
    GPosition map_entry = ConvMap.contains( key );
    if( map_entry != NULL)
    {                           // Found in the conversion map, substitute
      ret += ConvMap[key];
    } else {
                                // Not found in the conversion map,
                                //  try to do a numerical conversion.
      int char_value = 0 ;      // Value of converted key
      bool succeeded = false;
      int scan_locn;
      int c;
      if( key.length() > 3 &&   // minimum length for a key
          key[1] == '#' &&      // must always be present
          ( tolower(key[2]) != 'x' || key.length() > 4 )
                                // minimum length for a hexadecimal key
        )
      {
        if( key[2] == 'x' || key[2] == 'X' )
        {                       // Hexadecimal constant
          for( scan_locn = 3 ; isxdigit(c = key[scan_locn]) ; scan_locn++ )
          {
            if( isdigit(c) )
              c = c - '0';
            else
              c = 10 + tolower(c) - 'a';
            char_value = char_value<<4 | c;
          }
        } else {
                                // Decimal constant
          for( scan_locn = 2 ; isdigit(c = key[scan_locn]) ; scan_locn++ )
          {
            char_value = char_value*10 + (c - '0');
          }
        }
                                // success if we have encountered only digits
                                // up to the semicolon
        succeeded = ( key[scan_locn] == ';' );
      }
      if( succeeded )
      {
        ret += "?";
        ret.setat( -1, char_value );
      } else {
        ret += key;
      }
    }
    start_locn = semi_locn + 1;
    DEBUG_MSG( "ret = '" << ret << "'\n" );
  }

                                // Copy the end of the string to the output
  ret += this->substr( start_locn, this->length()-start_locn );

  DEBUG_MSG( "Unescaped string is '" << ret << "'\n" );
  return ret;
}

GMap<GString,GString>
GString::BasicMap( void )
{
  GMap<GString,GString> Basic;
  Basic["&lt;"]   = GString("<");
  Basic["&gt;"]   = GString(">");
  Basic["&amp;"]  = GString("&");
  Basic["&apos;"] = GString("'");
  Basic["&quot;"] = GString("\"");
  return Basic;
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
  char *buffer=new char [buflen];

  // Format string
#ifdef USE_VSNPRINTF
  while(USE_VSNPRINTF(buffer, buflen, fmt, args)<0)
  {
    delete [] buffer;
    buffer=new char [buflen+=32768];
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
  delete [] buffer;
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


