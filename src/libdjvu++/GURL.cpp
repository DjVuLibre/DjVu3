//C-  -*- C++ -*-
//C-
//C-  Copyright (c) 1988 AT&T	
//C-  All Rights Reserved 
//C-
//C-  THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
//C-  The copyright notice above does not evidence any
//C-  actual or intended publication of such source code.
//C-
//C-  $Id: GURL.cpp,v 1.1.1.1 1999-01-22 00:40:19 leonb Exp $

// "$Id: GURL.cpp,v 1.1.1.1 1999-01-22 00:40:19 leonb Exp $"

#ifdef __GNUC__
#pragma implementation
#endif

#include "GException.h"
#include "GOS.h"
#include "GURL.h"

#include <string.h>

void
GURL::convertSlashes(void)
{
  for(int i=0;i<(int)url.length();i++)
    if (url[i]=='\\') url.setat(i, '/');
}

void
GURL::eatDots(void)
{
      // Eats parts like ./ or ../
   char * buffer=new char[url.length()+1];
   strcpy(buffer, url);
   TRY {
     char *ptr;
     char *last_slash = 0;
     for(ptr=buffer;*ptr && *ptr!='/';ptr++);
     while(*ptr)
       {
	 while(*ptr && *ptr=='/') { last_slash=ptr; ptr++; };
	 if (*ptr)
	 {
	       // ptr[-1]=='/' here
	    if (ptr[0]=='.' && ptr[1]=='/') strcpy(ptr, ptr+2);
	    else if (ptr[0]=='.' && ptr[1]=='.' && ptr[2]=='/')
	    {
	       strcpy(last_slash+1, ptr+3);
	       ptr=last_slash+1;
	    } else while(*ptr && *ptr!='/') ptr++;
	 };
      };
      url=buffer;
      delete buffer; buffer=0;
   } CATCH(exc) {
      delete buffer;
      RETHROW;
   } ENDCATCH;
}

void
GURL::init(void)
{
   if (url.length())
   {
      if (!isAbsolute(url))
	 THROW("URL must have either http: or file: prefix.");
	 
      if (!strncmp(url.downcase(), "http:", 5))
      {
	    // Remote URL: assume that everything is OK with it except for
	    // maybe backward slashes and extra dots
	 convertSlashes();
	 eatDots();
      } else if (!strncmp(url.downcase(), "file:", 5))
      {
	    // Local URL: just convert it twice
	 url=GOS::url_to_filename(url);
	 if (!url.length()) THROW("Failed to convert URL to filename.");
	 url=GOS::filename_to_url(url);
	 if (!url.length()) THROW("Failed to convert filename back to URL.");

	 convertSlashes();	// Just in case
	 eatDots();
      } else THROW("Malformed URL '"+url+"'.");
   };
}

GURL::GURL(const char * url_in) : url(url_in ? url_in : "")
{
   init();
}

GURL::GURL(const GString & url_in) : url(url_in)
{
   init();
}

bool
GURL::isAbsolute(const char * url)
{
   return !strncmp(GString(url).downcase(), "http:", 5) ||
	  !strncmp(GString(url).downcase(), "file:", 5);
}

int
GURL::isLocal(void) const
{
   return !strncmp(url.downcase(), "file:", 5);
}

GURL
GURL::baseURL(void) const
{
   const char * ptr, * slash=url;
   for(ptr=url;*ptr;ptr++)
      if (*ptr=='/') slash=ptr;
   return GString(url, slash-url+1);
}

GString
GURL::fileURL(void) const
{
   const char * ptr, * slash=0;
   for(ptr=url;*ptr;ptr++)
      if (*ptr=='/') slash=ptr;
   if (slash) return slash+1;
   else return url;
}

GURL
GURL::operator+(const char * name) const
{
   GURL res;
   if (strncmp(name, "http:", 5) &&
       strncmp(name, "file:", 5))
   {
      res=*this;
      if (res.url.length() && res.url[res.url.length()-1]!='/') res.url+="/";
      res.url+=name;
      
      res.convertSlashes();
      res.eatDots();
   } else res=name;
   return res;
}
