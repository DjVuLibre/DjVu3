//C-  -*- C++ -*-
//C-
//C- Copyright (c) 1999 AT&T Corp.  All rights reserved.
//C-
//C- This software may only be used by you under license from AT&T
//C- Corp. ("AT&T"). A copy of AT&T's Source Code Agreement is available at
//C- AT&T's Internet website having the URL <http://www.djvu.att.com/open>.
//C- If you received this software without first entering into a license with
//C- AT&T, you have an infringing copy of this software and cannot use it
//C- without violating AT&T's intellectual property rights.
//C-
//C- $Id: GURL.cpp,v 1.10 1999-09-30 21:48:36 praveen Exp $

#ifdef __GNUC__
#pragma implementation
#endif

#include "GException.h"
#include "GOS.h"
#include "GURL.h"

#include <string.h>
#include <ctype.h>

void
GURL::convert_slashes(void)
{
   GString proto=protocol();
   for(int i=proto.length();i<(int) url.length();i++)
      if (url[i]=='\\') url.setat(i, '/');
}

void
GURL::eat_dots(void)
{
      // Eats parts like ./ or ../
   char * buffer=new char[url.length()+1];
   strcpy(buffer, url);
   TRY {
      GString proto=protocol();
      char * ptr, * last_slash=(char *) ((const char *) url+proto.length()+1);
      for(ptr=last_slash;*ptr && *ptr!='/';ptr++);
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
	 }
      }
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
      GString proto=protocol();
      if (!proto.length()) THROW("URL '"+url+"' does not contain a protocol prefix.");

	 // Below we have to make this complex test to detect URLs really
	 // referring to *local* files. Surprisingly, file://hostname/dir/file
	 // is also valid, but shouldn't be treated thru local FS.
      if (proto=="file" && url[5]=='/' &&
	  (url[6]!='/' || !strncmp(url, "file://localhost/", strlen("file://localhost/"))))
      {
	 url=GOS::url_to_filename(url);
	 if (!url.length()) THROW("Failed to convert URL to filename.");
	 url=GOS::filename_to_url(url);
	 if (!url.length()) THROW("Failed to convert filename back to URL.");
      }

      convert_slashes();
      eat_dots();
   }
}

GURL::GURL(const char * url_in) : url(url_in ? url_in : "")
{
   init();
}

GURL::GURL(const GString & url_in) : url(url_in)
{
   init();
}

GString
GURL::protocol(const char * url)
{
   const char * ptr;
   for(ptr=url;*ptr;ptr++)
      if (!isalpha(*ptr) && !isdigit(*ptr) &&
	  *ptr!='+' && *ptr!='-' && *ptr!='.') break;
   if (*ptr==':') return GString(url, ptr-url);
   else return "";
}

GString
GURL::hash_argument(void) const
{
   for(const char * start=url;*start;start++)
      if (start[0]=='#' || start[0]=='%' &&
	  start[1]=='2' && start[2]=='3')
      {
	 if (start[0]=='#') return start+1;
	 else return start+3;
      }
   return GString();
}

void
GURL::clear_hash_argument(void)
{
   for(const char * start=url;*start;start++)
      if (start[0]=='#' || start[0]=='%' &&
	  start[1]=='2' && start[2]=='3')
	 url.setat(start-url, 0);
}

bool
GURL::is_local_file_url(void) const
{
   return
      protocol()=="file" && url[5]=='/' &&
      (url[6]!='/' || !strncmp(url, "file://localhost/", strlen("file://localhost/")));
}

GURL
GURL::base(void) const
{
   GString proto=protocol();
   const char * ptr, * slash=(const char *) url+proto.length()+1;
   for(ptr=slash;*ptr && *ptr!=';' && *ptr!='?';ptr++)
      if (*ptr=='/') slash=ptr;
   
   return GURL(GString(url, slash-url)+GString(ptr, url.length()-(ptr-url)));
}

GString
GURL::name(void) const
{
   GString proto=protocol();
   const char * ptr, * slash=(const char *) url+proto.length()+1;
   for(ptr=slash;*ptr && *ptr!=';' && *ptr!='?';ptr++)
      if (*ptr=='/') slash=ptr;
   
   return GString(slash+1, ptr-slash-1);
}

GURL
GURL::operator+(const char * name) const
{
   GURL res;
   if (!protocol(name).length())
   {
      GString proto=protocol();

      const char * ptr;
      for(ptr=(const char *) url+proto.length()+1;*ptr;ptr++)
	 if (*ptr==';' || *ptr=='?') break;

      GString str(url, ptr-url);
      if (str[(int)str.length()-1]!='/') str+='/';
      str+=name;
      str+=ptr;

      res=str;
   } else res=name;
   return res;
}
