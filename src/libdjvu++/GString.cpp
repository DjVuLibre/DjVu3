//C-  -*- C++ -*-
//C-
//C- Copyright (c) 1999 AT&T Corp.
//C- Copyright (c) 2000 LizardTech Inc.
//C- All rights reserved.
//C-
//C- This software may only be used by you under license from LizardTech
//C- Inc. A copy of LizardTech's Source Code Agreement is available at
//C- LizardTech's Internet website having the URL <http://www.djvu.com/open>.
//C- If you received this software without first entering into a license with
//C- LizardTech, you have an infringing copy of this software and cannot use it
//C- without violating LizardTech's intellectual property rights.
//C-
//C- $Id: GString.cpp,v 1.18 2000-05-01 16:15:22 bcr Exp $


#ifdef __GNUC__
#pragma implementation
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <new.h>

#include "GString.h"

// File "$Id: GString.cpp,v 1.18 2000-05-01 16:15:22 bcr Exp $"
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
    THROW("Memory Overwrite.  Program in unstable state.");
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
  THROW("Illegal subscript in GString::operator[]");
}


