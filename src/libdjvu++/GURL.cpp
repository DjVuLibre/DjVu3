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
// $Id: GURL.cpp,v 1.63 2001-04-16 23:59:13 bcr Exp $
// $Name:  $

#ifdef __GNUC__
#pragma implementation
#endif

#include "GException.h"
#include "GOS.h"
#include "GURL.h"
#include "debug.h"

#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <math.h>
#include <string.h>

#ifdef WIN32
#include <atlbase.h>
#include <windows.h>

#ifndef UNDER_CE
#include <direct.h>
#endif
#endif   // end win32

// -- MAXPATHLEN
#ifndef MAXPATHLEN
#ifdef _MAX_PATH
#define MAXPATHLEN _MAX_PATH
#else
#define MAXPATHLEN 1024
#endif
#else
#if ( MAXPATHLEN < 1024 )
#undef MAXPATHLEN
#define MAXPATHLEN 1024
#endif
#endif

#ifdef UNIX
#include <sys/types.h>
// Handle the few systems without dirent.h
// 1 -- systems with /usr/include/sys/ndir.h
#if defined(XENIX)
#define USE_DIRECT
#include <sys/ndir.h>
#endif
// 2 -- systems with /usr/include/sys/dir.h
#if defined(OLDBSD)
#define USE_DIRECT
#include <sys/dir.h>
#endif
// The rest should be generic
#ifdef USE_DIRECT
#define dirent direct
#define NAMLEN(dirent) (dirent)->d_namlen
#else
#include <dirent.h>
#define NAMLEN(dirent) strlen((dirent)->d_name)
#endif 
#endif




#ifdef UNIX
# include <errno.h>
# include <sys/types.h>
# include <sys/stat.h>
# include <sys/time.h>
# include <fcntl.h>
# include <pwd.h>
# include <stdio.h>
# include <unistd.h>
#endif

#ifdef macintosh
#include <unix.h>
#include <errno.h>
#include <unistd.h>
#endif

static const char djvuopts[]="DJVUOPTS";
static const char localhost[]="file://localhost/";
static const char fileproto[]="file:";
static const char backslash='\\';
static const char colon=':';
static const char dot='.';
static const char filespecslashes[] = "file://";
static const char filespec[] = "file:";
static const char slash='/';
static const char percent='%';
static const char localhostspec1[] = "//localhost/";
static const char localhostspec2[] = "///";
static const char nillchar=0;
#if defined(UNIX)
  static const char tilde='~';
  static const char root[] = "/";
#elif defined(WIN32)
  static const char root[] = "\\";
#elif defined(macintosh)
  static char const * const root = &nillchar; 
#else
#error "Define something here for your operating system"
#endif

// hexval --
// -- Returns the hexvalue of a character.
//    Returns -1 if illegal;

static int 
hexval(char c)
{
  return ((c>='0' && c<='9')
    ?(c-'0')
    :((c>='A' && c<='F')
      ?(c-'A'+10)
      :((c>='a' && c<='f')
        ?(c-'a'+10):(-1))));
}


static bool
is_argument(const char * start)
      // Returns TRUE if 'start' points to the beginning of an argument
      // (either hash or CGI)
{
   return (*start=='#' || *start=='?' || *start=='&' || *start==';');
}

void
GURL::convert_slashes(void)
{
   if(!validurl) init();
#ifndef UNIX
   GCriticalSectionLock lock(&class_lock);
   for(char *ptr=(url.getbuf()+protocol().length());*ptr;ptr++)
   {
     if(*ptr == backslash)
       *ptr=slash;
   }
#endif
}

static void
collapse(char * ptr, const int chars)
      // Will remove the first 'chars' chars from the string and
      // move the rest toward the beginning. Will take into account
      // string length
{
   const int length=strlen(ptr);
   const char *srcptr=ptr+((chars>length)?length:chars);
   while((*(ptr++) = *(srcptr++)))
     EMPTY_LOOP;
}

void
GURL::beautify_path(void)
{
  if(!validurl) init();
  GCriticalSectionLock lock(&class_lock);
   
  // Eats parts like ./ or ../ or ///
  char * buffer;
  GPBuffer<char> gbuffer(buffer,url.length()+1);
  strcpy(buffer, (const char *)url);
   
  // Find start point
  char * start=buffer+protocol().length()+1;
  while(*start && *start==slash) start++;

  // Find end of the url (don't touch arguments)
  char * ptr;
  GUTF8String args;
  for(ptr=start;*ptr;ptr++)
  {
    if (is_argument(ptr))
    {
      args=ptr;
      *ptr=0;
      break;
    }
  }

  // Eat multiple slashes
  for(;(ptr=strstr(start, "////"));collapse(ptr, 3))
    EMPTY_LOOP;
  for(;(ptr=strstr(start, "//"));collapse(ptr, 1))
    EMPTY_LOOP;
  // Convert /./ stuff into plain /
  for(;(ptr=strstr(start, "/./"));collapse(ptr, 2))
    EMPTY_LOOP;


  // Process /../
  while((ptr=strstr(start, "/../")))
  {
    for(char * ptr1=ptr-1;(ptr1>=start);ptr1--)
    {
      if (*ptr1==slash)
      {
        collapse(ptr1, ptr-ptr1+3);
        break;
      }
    }
  }

  // Remove trailing /.
  ptr=start+strlen(start)-2;
  if((ptr>=start)&& (ptr == GUTF8String("/.")))
  {
    ptr[1]=0;
  }
  // Eat trailing /..
  ptr=start+strlen(start)-3;
  if((ptr >= start) && (ptr == GUTF8String("/..")))
  {
    for(char * ptr1=ptr-1;(ptr1>=start);ptr1--)
    {
      if (*ptr1==slash)
      {
        ptr1[1]=0;
        break;
      }
    }
  }

  // Done. Copy the buffer back into the URL and add arguments.
  url=buffer;
  url+=args;
}

void
GURL::init(const bool nothrow)
{
   GCriticalSectionLock lock(&class_lock);
   validurl=true;
   
   if (url.length())
   {
      GUTF8String proto=protocol();
      if (proto.length()<2)
      {
        validurl=false;
        if(!nothrow)
          G_THROW( ERR_MSG("GURL.no_protocol") "\t"+url);
        return;
      }

         // Below we have to make this complex test to detect URLs really
         // referring to *local* files. Surprisingly, file://hostname/dir/file
         // is also valid, but shouldn't be treated thru local FS.
      if (proto=="file" && url[5]==slash &&
          (url[6]!=slash || GUTF8String::ncmp(localhost, url, sizeof(localhost))))
      {
            // Separate the arguments
         GUTF8String arg;
         {
           const char * const url_ptr=url;
           const char * ptr;
           for(ptr=url_ptr;*ptr&&!is_argument(ptr);ptr++)
           		EMPTY_LOOP;
           arg=ptr;
           url.setat((int)(ptr-url_ptr), 0);
         }

            // Do double conversion
         GUTF8String tmp=UTF8Filename();
         if (!tmp.length())
         {
           validurl=false;
           if(!nothrow)
             G_THROW( ERR_MSG("GURL.fail_to_file") );
           return;
         }
         url=GURL::Filename::UTF8(tmp).get_string();
         if (!url.length())
         {
           validurl=false;
           if(!nothrow)
             G_THROW( ERR_MSG("GURL.fail_to_URL") );
           return;
         }
            // Return the argument back
         url+=arg;
      }
      convert_slashes();
      beautify_path();
      parse_cgi_args();
   }
}

GURL::GURL(void) : validurl(false) {}

GURL::GURL(const char * url_in) : url(url_in ? url_in : ""), validurl(false)
{
}

GURL::GURL(const GUTF8String & url_in) : url(url_in), validurl(false)
{
}

GURL::GURL(const GURL & url_in) : url(url_in.url), validurl(false)
{
}

GURL &
GURL::operator=(const GURL & url_in)
{
   GCriticalSectionLock lock(&class_lock);
   url=url_in.url;
   init();
   return *this;
}

GUTF8String
GURL::protocol(const GUTF8String& url)
{
   const char * const url_ptr=url;
   const char * ptr=url_ptr;
   for(char c=*ptr;
     c && (isalnum(c) || c == '+' || c == '-' || c == '.');
     c=*(++ptr)) EMPTY_LOOP;
   return(*ptr==colon)?GUTF8String(url_ptr, ptr-url_ptr):GUTF8String();
}

GUTF8String
GURL::hash_argument(void) const
      // Returns the HASH argument (anything after '#' and before '?')
{
   if(!validurl) const_cast<GURL *>(this)->init();
   GCriticalSectionLock lock((GCriticalSection *) &class_lock);
   bool found=false;
   GUTF8String arg;

         // Break if CGI argument is found
   for(const char * start=url;*start&&(*start!='?');start++)
   {
      if (found)
      {
         arg+=*start;
      }else
      {
         found=(*start=='#');
      }
   }
   return decode_reserved(arg);
}

void
GURL::set_hash_argument(const GUTF8String &arg)
{
   if(!validurl) init();
   GCriticalSectionLock lock((GCriticalSection *) &class_lock);

   GUTF8String new_url;
   bool found=false;
   const char * ptr;
   for(ptr=url;*ptr;ptr++)
   {
      if (is_argument(ptr))
      {
         if (*ptr!='#')
         {
           break;
         }
         found=true;
      } else if (!found)
      {
         new_url+=*ptr;
      }
   }

   url=new_url+"#"+GURL::encode_reserved(arg)+ptr;
}

void
GURL::parse_cgi_args(void)
      // Will read CGI arguments from the URL into
      // cgi_name_arr and cgi_value_arr
{
   if(!validurl) init();
   GCriticalSectionLock lock1(&class_lock);
   cgi_name_arr.empty();
   cgi_value_arr.empty();

      // Search for the beginning of CGI arguments
   const char * start=url;
   while(*start)
   {
     if(*(start++)=='?')
     {
       break;
     }
   }

      // Now loop until we see all of them
   while(*start)
   {
      GUTF8String arg;        // Storage for another argument
      while(*start)        // Seek for the end of it
      {
         if (*start=='&')
         {
            start++;
            break;
         } else
         {
           arg+=*start++;
         }
      }
      if (arg.length())
      {
            // Got argument in 'arg'. Split it into 'name' and 'value'
         const char * ptr;
         const char * const arg_ptr=arg;
	 for(ptr=arg_ptr;*ptr&&(*ptr != '=');ptr++)
	   EMPTY_LOOP;

         GUTF8String name, value;
         if (*ptr)
         {
            name=GUTF8String(arg_ptr, (int)((ptr++)-arg_ptr));
            value=GUTF8String(ptr, arg.length()-name.length()-1);
         } else
         {
           name=arg;
         }
            
         int args=cgi_name_arr.size();
         cgi_name_arr.resize(args);
         cgi_value_arr.resize(args);
         cgi_name_arr[args]=decode_reserved(name);
         cgi_value_arr[args]=decode_reserved(value);
      }
   }
}

void
GURL::store_cgi_args(void)
      // Will store CGI arguments from the cgi_name_arr and cgi_value_arr
      // back into the URL
{
   if(!validurl) init();
   GCriticalSectionLock lock1(&class_lock);

   const char * const url_ptr=url;
   const char * ptr;
   for(ptr=url_ptr;*ptr&&(*ptr!='?');ptr++)
   		EMPTY_LOOP;
   
   GUTF8String new_url(url_ptr, ptr-url_ptr);
   
   for(int i=0;i<cgi_name_arr.size();i++)
   {
      GUTF8String name=GURL::encode_reserved(cgi_name_arr[i]);
      GUTF8String value=GURL::encode_reserved(cgi_value_arr[i]);
      new_url+=(i?"&":"?")+name;
      if (value.length())
         new_url+="="+value;
   }

   url=new_url;
}

int
GURL::cgi_arguments(void) const
{
   if(!validurl) const_cast<GURL *>(this)->init();
   GCriticalSectionLock lock((GCriticalSection *) &class_lock);
   return cgi_name_arr.size();
}

int
GURL::djvu_cgi_arguments(void) const
{
   if(!validurl) const_cast<GURL *>(this)->init();
   GCriticalSectionLock lock((GCriticalSection *) &class_lock);

   int args=0;
   for(int i=0;i<cgi_name_arr.size();i++)
   {
      if (cgi_name_arr[i].upcase()==djvuopts)
      {
         args=cgi_name_arr.size()-(i+1);
         break;
      }
   } 
   return args;
}

GUTF8String
GURL::cgi_name(int num) const
{
   if(!validurl) const_cast<GURL *>(this)->init();
   GCriticalSectionLock lock((GCriticalSection *) &class_lock);
   return (num<cgi_name_arr.size())?cgi_name_arr[num]:GUTF8String();
}

GUTF8String
GURL::djvu_cgi_name(int num) const
{
   if(!validurl) const_cast<GURL *>(this)->init();
   GCriticalSectionLock lock((GCriticalSection *) &class_lock);

   GUTF8String arg;
   for(int i=0;i<cgi_name_arr.size();i++)
      if (cgi_name_arr[i].upcase()==djvuopts)
      {
         for(i++;i<cgi_name_arr.size();i++)
            if (! num--)
            {
               arg=cgi_name_arr[i];
               break;
            }
         break;
      }
   return arg;
}

GUTF8String
GURL::cgi_value(int num) const
{
   if(!validurl) const_cast<GURL *>(this)->init();
   GCriticalSectionLock lock((GCriticalSection *) &class_lock);
   return (num<cgi_value_arr.size())?cgi_value_arr[num]:GUTF8String();
}

GUTF8String
GURL::djvu_cgi_value(int num) const
{
   if(!validurl) const_cast<GURL *>(this)->init();
   GCriticalSectionLock lock((GCriticalSection *) &class_lock);

   GUTF8String arg;
   for(int i=0;i<cgi_name_arr.size();i++)
   {
      if (cgi_name_arr[i].upcase()==djvuopts)
      {
         for(i++;i<cgi_name_arr.size();i++)
         {
            if (! num--)
            {
               arg=cgi_value_arr[i];
               break;
            }
         }
         break;
      }
   }
   return arg;
}

DArray<GUTF8String>
GURL::cgi_names(void) const
{
   if(!validurl) const_cast<GURL *>(this)->init();
   GCriticalSectionLock lock((GCriticalSection *) &class_lock);
   return cgi_name_arr;
}

DArray<GUTF8String>
GURL::cgi_values(void) const
{
   if(!validurl) const_cast<GURL *>(this)->init();
   GCriticalSectionLock lock((GCriticalSection *) &class_lock);
   return cgi_value_arr;
}

DArray<GUTF8String>
GURL::djvu_cgi_names(void) const
{
   if(!validurl) const_cast<GURL *>(this)->init();
   GCriticalSectionLock lock((GCriticalSection *) &class_lock);

   int i;
   DArray<GUTF8String> arr;
   for(i=0;(i<cgi_name_arr.size())&&
     (cgi_name_arr[i].upcase()!=djvuopts)
     ;i++)
     	EMPTY_LOOP;

   int size=cgi_name_arr.size()-(i+1);
   if (size>0)
   {
      arr.resize(size-1);
      for(i=0;i<arr.size();i++)
         arr[i]=cgi_name_arr[cgi_name_arr.size()-arr.size()+i];
   }

   return arr;
}

DArray<GUTF8String>
GURL::djvu_cgi_values(void) const
{
   if(!validurl) const_cast<GURL *>(this)->init();
   GCriticalSectionLock lock((GCriticalSection *) &class_lock);

   int i;
   DArray<GUTF8String> arr;
   for(i=0;i<cgi_name_arr.size()&&(cgi_name_arr[i].upcase()!=djvuopts);i++)
   		EMPTY_LOOP;

   int size=cgi_name_arr.size()-(i+1);
   if (size>0)
   {
      arr.resize(size-1);
      for(i=0;i<arr.size();i++)
         arr[i]=cgi_value_arr[cgi_value_arr.size()-arr.size()+i];
   }

   return arr;
}

void
GURL::clear_all_arguments(void)
{
   clear_hash_argument();
   clear_cgi_arguments();
}

void
GURL::clear_hash_argument(void)
      // Clear anything after first '#' and before the following '?'
{
   if(!validurl) init();
   GCriticalSectionLock lock(&class_lock);
   bool found=false;
   GUTF8String new_url;
   for(const char * start=url;*start;start++)
   {
         // Break on first CGI arg.
      if (*start=='?')
      {
         new_url+=start;
         break;
      }

      if (!found)
      { 
        if (*start=='#')
          found=true;
        else
          new_url+=*start;
      }
   }
   url=new_url;
}

void
GURL::clear_cgi_arguments(void)
{
   if(!validurl) init();
   GCriticalSectionLock lock1(&class_lock);

      // Clear the arrays
   cgi_name_arr.empty();
   cgi_value_arr.empty();

      // And clear everything past the '?' sign in the URL
   for(const char * ptr=url;*ptr;ptr++)
      if (*ptr=='?')
      {
         url.setat(ptr-url, 0);
         break;
      }
}

void
GURL::clear_djvu_cgi_arguments(void)
{
   if(!validurl) init();
      // First - modify the arrays
   GCriticalSectionLock lock(&class_lock);
   for(int i=0;i<cgi_name_arr.size();i++)
   {
      if (cgi_name_arr[i].upcase()==djvuopts)
      {
         cgi_name_arr.resize(i-1);
         cgi_value_arr.resize(i-1);
         break;
      }
   }

      // And store them back into the URL
   store_cgi_args();
}

void
GURL::add_djvu_cgi_argument(const GUTF8String &name, const char * value)
{
   if(!validurl) init();
   GCriticalSectionLock lock1(&class_lock);

      // Check if we already have the "DJVUOPTS" argument
   bool have_djvuopts=false;
   for(int i=0;i<cgi_name_arr.size();i++)
   {
      if (cgi_name_arr[i].upcase()==djvuopts)
      {
         have_djvuopts=true;
         break;
      }
   }

      // If there is no DJVUOPTS, insert it
   if (!have_djvuopts)
   {
      int pos=cgi_name_arr.size();
      cgi_name_arr.resize(pos);
      cgi_value_arr.resize(pos);
      cgi_name_arr[pos]=djvuopts;
   }

      // Add new argument to the array
   int pos=cgi_name_arr.size();
   cgi_name_arr.resize(pos);
   cgi_value_arr.resize(pos);
   cgi_name_arr[pos]=name;
   cgi_value_arr[pos]=value;

      // And update the URL
   store_cgi_args();
}

bool
GURL::is_local_file_url(void) const
{
   if(!validurl) const_cast<GURL *>(this)->init();
   GCriticalSectionLock lock((GCriticalSection *) &class_lock);
   return (protocol()=="file" && url[5]==slash);
}

GUTF8String
GURL::pathname(void) const
{
  return (is_local_file_url())
    ?GURL::encode_reserved(UTF8Filename()) 
    :url.substr(url.search(slash),(unsigned int)(-1));
}

GURL
GURL::base(void) const
{
   if(!validurl) const_cast<GURL *>(this)->init();
   GCriticalSectionLock lock((GCriticalSection *) &class_lock);

   const char * const url_ptr=url;
   const char * ptr, * xslash;
   for(ptr=xslash=url_ptr+protocol().length()+1;*ptr && !is_argument(ptr);ptr++)
   {
      if (*ptr==slash)
        xslash=ptr;
   }
   return GURL::UTF8(
#ifdef WIN32
   (*(xslash-1) == colon)?
     GUTF8String(url,(int)(xslash-url))+"/"+GUTF8String(ptr,url.length()-(int)(ptr-url_ptr)) :
#endif
     GUTF8String(url.substr(0,(int)(xslash-url_ptr))+GUTF8String(ptr,url.length()-(int)(ptr-url_ptr))) );
}

GUTF8String
GURL::name(void) const
{
   if(!validurl) const_cast<GURL *>(this)->init();
   GCriticalSectionLock lock((GCriticalSection *) &class_lock);
   const char * ptr, * xslash;
   for(ptr=xslash=(const char *)url+protocol().length()+1;
     *ptr && !is_argument(ptr);ptr++)
   {
      if (*ptr==slash)
        xslash=ptr;
   }
   
   return GUTF8String(xslash+1, ptr-xslash-1);
}

GUTF8String
GURL::fname(void) const
{
   if(!validurl) const_cast<GURL *>(this)->init();
   return decode_reserved(name());
}

GUTF8String
GURL::extension(void) const
{
   if(!validurl) const_cast<GURL *>(this)->init();
   GUTF8String xfilename=name();
   GUTF8String retval;

   for(int i=xfilename.length()-1;i>=0;i--)
   {
      if (xfilename[i]=='.')
      {
         retval=(const char*)xfilename+i+1;
         break;
      }
   } 
   return retval;
}

GURL
GURL::operator+(const GUTF8String &gname) const
{
   if(!validurl) const_cast<GURL *>(this)->init();
   GCriticalSectionLock lock((GCriticalSection *) &class_lock);
   GURL res;
   if (!protocol(gname).length())
   {
      const char * const url_ptr=(const char *)url;
      const char * ptr;
      for(ptr=url_ptr+protocol().length()+1;*ptr&&!is_argument(ptr);ptr++)
      	EMPTY_LOOP;

      res=GUTF8String(GUTF8String(url_ptr,(int)(ptr-url_ptr))
        +((*(ptr-1) != slash)?GUTF8String(slash):GUTF8String())+gname+ptr);
   } else
   {
     res=gname;
   }
   res.parse_cgi_args();
   return res;
}

GUTF8String
GURL::decode_reserved(const GUTF8String &gurl)
{
  const char *url=gurl;
  GUTF8String res;

  for(const char * ptr=url;*ptr;ptr++)
  {
    if (*ptr!=percent)
    {
      res+=*ptr;
    }else
    {
      int c1,c2;
      if ( ((c1=hexval(ptr[1]))>=0)
        && ((c2=hexval(ptr[2]))>=0) )
      {
        res+=(c1<<4)|c2;
        ptr+=2;
      } else
      {
        res+=*ptr;
      }
    }
  }
  return res;
}

GUTF8String
GURL::encode_reserved(const GUTF8String &gs)
{
  const char *s=(const char *)gs;
  // Potentially unsafe characters (cf. RFC1738 and RFC1808)
  static const char hex[] = "0123456789ABCDEF";
  
  unsigned char *retval;
  GPBuffer<unsigned char> gd(retval,strlen(s)*3);
  unsigned char *d=retval;
  for (; *s; s++,d++)
  {
    // Convert directory separator to slashes
#ifdef WIN32
    if (*s == backslash || *s== slash)
#else
#ifdef macintosh
    if (*s == colon )
#else
#ifdef UNIX
    if (*s == slash )
#else
#error "Define something here for your operating system"
#endif  
#endif
#endif
    {
      *d = slash; 
      continue;
    }
    unsigned char const ss=(unsigned char const)(*s);
    // WARNING: Whenever you modify this conversion code,
    // make sure, that the following functions are in sync:
    //   encode_reserved()
    //   decode_reserved()
    //   url_to_filename()
    //   filename_to_url()
    // unreserved characters
    if ( (ss>='a' && ss<='z') ||
       (ss>='A' && ss<='Z') ||
       (ss>='0' && ss<='9') ||
       (strchr("$-_.+!*'(),:~&;", ss)) )   // Added : because of windows!
    {
      *d = ss;
      continue;
    }
    // escape sequence
    d[0] = percent;
    d[1] = hex[ (ss >> 4) & 0xf ];
    d[2] = hex[ (ss) & 0xf ];
    d+=2;
  }
  *d = 0;
  return retval;
}

// -------------------------------------------
// Functions for converting filenames and urls
// -------------------------------------------

static GUTF8String
url_from_UTF8filename(const GUTF8String &gfilename)
{
  if(GURL::UTF8(gfilename).is_valid())
  {
    DEBUG_MSG("Illegal URL: " << gfilename << "\n");
  } 
  const char *filename=gfilename;
  if(filename && (unsigned char)filename[0] == (unsigned char)0xEF
     && (unsigned char)filename[1] == (unsigned char)0xBB && (unsigned char)filename[2] == (unsigned char)0xBF)
  {
    filename+=3;
  }

  // Special case for blank pages
  if(!filename || !filename[0])
  {
    return "about:blank";
  } 

  // Normalize file name to url slash-and-escape syntax
  GUTF8String oname=GURL::expand_name(filename);
  GUTF8String nname=GURL::encode_reserved(oname);

  // Preprend "file://" to file name. If file is on the local
  // machine, include "localhost".
  GUTF8String url=filespecslashes;
  const char *cnname=nname;
  if (cnname[0] == slash)
  {
    if (cnname[1] == slash)
    {
      url += cnname+2;
    }else
    {
      url = localhost + nname;
    }
  }else
  {
    url += (localhostspec1+2) + nname;
  }
#if 0
  // Special case for stupid MSIE 
  GUTF8String agent(useragent ? useragent : "default");
  if (agent.search("MSIE")>=0 || agent.search("Microsoft")>=0)
  {
    // We now remove all the escaping we just did.  The reason for adding
    // it to begin with is so at least the slashes are converted.
    url=GOS::url_to_filename(url);
    url=filespecslashes + GURL::expand_name(url);
  }
#endif
  return url;
}

// -- Returns a url for accessing a given file.
//    If useragent is not provided, standard url will be created,
//    but will not be understood by some versions if IE.
GUTF8String 
GURL::get_string(const GUTF8String &useragent) const
{
  GUTF8String retval(url);
  if(is_local_file_url()&&useragent.length())
  {
    if(useragent.search("MSIE") >= 0 || useragent.search("Microsoft")>=0)
    {
      retval=filespecslashes + expand_name(UTF8Filename());
    }
  }
  return retval;
}

GURL::UTF8::UTF8(const GUTF8String &xurl,const GURL &codebase)
{
  if(GURL::UTF8(xurl).is_valid())
  {
    url=xurl;
  }else
  {
    const char *c=xurl;
    if(c[0] == slash)
    {
      GURL base(codebase);
      for(GURL newbase=base.base();newbase!=base;newbase=base.base())
      {
        base=newbase;
      }
      url=base.get_string()+GURL::encode_reserved(xurl);
    }else
    {
      url=codebase.get_string()+GUTF8String(slash)+GURL::encode_reserved(xurl);
    }
  }
}

GURL::Native::Native(const GNativeString &xurl,const GURL &codebase)
{
  GURL::UTF8 retval(xurl.getNative2UTF8(),codebase);
  url=retval.get_string();
}

GURL::Filename::Native::Native(const GNativeString &gfilename)
{
  url=url_from_UTF8filename(gfilename.getNative2UTF8());
}


GURL::Filename::UTF8::UTF8(const GUTF8String &gfilename)
{
  url=url_from_UTF8filename(gfilename);
}

// filename --
// -- Applies heuristic rules to convert a url into a valid file name.  
//    Returns a simple basename in case of failure.
GUTF8String 
GURL::UTF8Filename(void) const
{
  GUTF8String retval;
  if(! is_empty())
  {
    const char *url_ptr=url;
  
    // WARNING: Whenever you modify this conversion code,
    // make sure, that the following functions are in sync:
    //   encode_reserved()
    //   decode_reserved()
    //   url_to_filename()
    //   filename_to_url()

    GUTF8String urlcopy=decode_reserved(url);
    url_ptr = urlcopy;

#if 0
    // Check if we have a simple file name already
    {
      GUTF8String tmp=expand_name(url_ptr,root);
      if (GOS::is_file(tmp)) 
        return tmp;
    }
#endif

    // All file urls are expected to start with filespec which is "file:"
    if (!GUTF8String::ncmp(filespec, url_ptr, sizeof(filespec)-1))  //if not
      return GOS::basename(url_ptr);
    url_ptr += sizeof(filespec)-1;
  
#ifdef macintosh
    //remove all leading slashes
    for(;*url_ptr==slash;url_ptr++)
      EMPTY_LOOP;
    // Remove possible localhost spec
    if ( GUTF8String::ncmp(localhost, url_ptr, sizeof(localhost)-1) )
      url_ptr += sizeof(localhost)-1;
    //remove all leading slashes
    while(*url_ptr==slash)
      url_ptr++;
#else
    // Remove possible localhost spec
    if ( GUTF8String::ncmp(localhostspec1, url_ptr, sizeof(localhostspec1)-1) )        // RFC 1738 local host form
      url_ptr += sizeof(localhostspec1)-1;
    else if ( GUTF8String::ncmp(localhostspec2, url_ptr, sizeof(localhostspec2)-1 ) )   // RFC 1738 local host form
      url_ptr += sizeof(localhostspec2)-1;
    else if ( (strlen(url_ptr) > 4) // "file://<letter>:/<path>"
        && (url_ptr[0] == slash)      // "file://<letter>|/<path>"
        && (url_ptr[1] == slash)
        && isalpha(url_ptr[2])
        && ( url_ptr[3] == colon || url_ptr[3] == '|' )
        && (url_ptr[4] == slash) )
      url_ptr += 2;
    else if ( (strlen(url_ptr)) > 2 // "file:/<path>"
        && (url_ptr[0] == slash)
        && (url_ptr[1] != slash) )
      url_ptr++;
#endif

    // Check if we are finished
#ifdef macintosh
    {
      char *l_url;
      GPBuffer<char> gl_url(l_url,strlen(url_ptr)+1);
      const char *s;
      char *r;
      for ( s=url_ptr,r=l_url; *s; s++,r++)
      {
        *r=(*s == slash)?colon:*s;
      }
      *r=0;
      retval = expand_name(l_url,root);
    }
#else  
    retval = expand_name(url_ptr,root);
#endif
    
#ifdef WIN32
    if (url_ptr[0] && url_ptr[1]=='|' && url_ptr[2]== slash)
    {
      if ((url_ptr[0]>='a' && url_ptr[0]<='z') 
          || (url_ptr[0]>='A' && url_ptr[0]<='Z'))
      {
//        if (!is_file()) 
        {
      // Search for a drive letter (encoded a la netscape)
          GUTF8String drive;
          drive.format("%c%c%c", url_ptr[0],colon,backslash);
          retval = expand_name(url_ptr+3, drive);
        }
      }
    }
#endif
  }
  // Return what we have
  return retval;
}

GNativeString 
GURL::NativeFilename(void) const
{
  return UTF8Filename().getUTF82Native();
}

#if defined(UNIX) || defined(macintosh)
static int
urlstat(const GURL &url,struct stat &buf)
{
  return stat(url.NativeFilename(),&buf);
}
#endif

// is_file(url) --
// -- returns true if filename denotes a regular file.
bool
GURL::is_file(void) const
{
  bool retval=false;
  if(is_local_file_url())
  {
#if defined(UNIX) || defined(macintosh)
    struct stat buf;
    if (!urlstat(*this,buf))
    {
      retval=!(buf.st_mode & S_IFDIR);
    }
#elif defined(WIN32)
    DWORD           dwAttrib;       ;
    USES_CONVERSION ;
    dwAttrib = GetFileAttributes(A2CT(NativeFilename())) ;//MBCS cvt
    retval=!((dwAttrib == 0xFFFFFFFF)
     ||( dwAttrib & FILE_ATTRIBUTE_DIRECTORY ));
#else
#error "Define something here for your operating system"
#endif
  }
  return retval;
}

bool
GURL::is_local_path(void) const
{
  bool retval=false;
  if(is_local_file_url())
  {
#if defined(UNIX) || defined(macintosh)
    struct stat buf;
    retval=!urlstat(*this,buf);
#else
    DWORD           dwAttrib;       ;
    USES_CONVERSION ;
    dwAttrib = GetFileAttributes(A2CT(NativeFilename())) ;//MBCS cvt
    retval=!(dwAttrib == 0xFFFFFFFF);
#endif
  }
  return retval;
}

// is_dir(url) --
// -- returns true if url denotes a directory.
bool 
GURL::is_dir(void) const
{
  bool retval=false;
  if(is_local_file_url())
  {
    // UNIX implementation
#if defined(UNIX) || defined(macintosh)
    struct stat buf;
    if (!urlstat(*this,buf))
    {
      retval=(buf.st_mode & S_IFDIR);
    }
#elif defined(WIN32)   // (either Windows or WCE)
    USES_CONVERSION ;
    DWORD           dwAttrib;       ;
    dwAttrib = GetFileAttributes(A2CT(NativeFilename())) ;//MBCS cvt
    retval=((dwAttrib != 0xFFFFFFFF)&&( dwAttrib & FILE_ATTRIBUTE_DIRECTORY ));
#else
#error "Define something here for your operating system"
#endif
  }
  return retval;
}

int
GURL::mkdir() const
{
  int retval;
  if(!is_local_file_url())
  {
    retval=(-1);
  }else
  {
    retval=0;
    const GURL baseURL=base();
    if(baseURL.get_string() != url && !baseURL.is_dir())
    {
      retval=baseURL.mkdir();
    }
    if(!retval)
    {
      retval=is_dir();
#ifdef WIN32
      USES_CONVERSION;
      retval =(is_dir()?0:CreateDirectory(A2CT(NativeFilename()), NULL));//MBCS cvt
#else
      retval=(is_dir()?0:(::mkdir(NativeFilename(), 0755)));//MBCS cvt
#endif
    }
  }
  return retval;
}

// deletefile
// -- deletes a file or directory
  
int
GURL::deletefile(void) const
{
  int retval=(-1);
  if(is_local_file_url())
  {
#ifdef WIN32
    USES_CONVERSION;
    retval=is_dir()
      ?RemoveDirectory(A2CT(NativeFilename()))
      :DeleteFile(A2CT(NativeFilename())); //MBCS cvt
#else
    retval=is_dir()
      ?rmdir(NativeFilename())
      :unlink(NativeFilename());//MBCS cvt
#endif
  }
  return retval;
}

GList<GURL>
GURL::listdir(void) const
{
  GList<GURL> retval;
  if(is_dir())
  {
#if defined(UNIX)
    DIR * dir=opendir(NativeFilename());//MBCS cvt
    for(dirent *de=readdir(dir);de;de=readdir(dir))
    {
      const int len = NAMLEN(de);
      if (de->d_name[0]== dot  && len==1)
        continue;
      if (de->d_name[0]== dot  && de->d_name[1]== dot  && len==2)
        continue;
      retval.append(GURL::Native(de->d_name,*this));
    }
    closedir(dir);
#elif defined (WIN32) && !defined (UNDER_CE)
    GURL::UTF8 wildcard("*.*",*this);
    WIN32_FIND_DATA finddata;
    HANDLE handle = FindFirstFile(wildcard.NativeFilename(), &finddata);//MBCS cvt
    const GUTF8String gpathname=pathname();
    const GUTF8String gbase=base().pathname();
    if( handle != INVALID_HANDLE_VALUE)
    {
      do
      {
        GURL::UTF8 Entry(finddata.cFileName,*this);
        const GUTF8String gentry=Entry.pathname();
        if((gentry != gpathname) && (gentry != gbase))
          retval.append(Entry);
      } while( FindNextFile(handle, &finddata) );

      FindClose(handle);
    }
#else
    // WCE and MAC is missing
    G_THROW( ERR_MSG("GURL.listdir") );
#endif
  }
  return retval;
}

int
GURL::cleardir(const int timeout) const
{
  int retval=(-1);
  if(is_dir())
  {
    GList<GURL> dirlist=listdir();
    retval=0;
    for(GPosition pos=dirlist;pos&&!retval;++pos)
    {
      const GURL &Entry=dirlist[pos];
      if(Entry.is_dir())
      {
        if((retval=Entry.cleardir(timeout)) < 0)
        {
          break;
        }
      }
      if(((retval=Entry.deletefile())<0) && (timeout>0))
      {
        GOS::sleep(timeout);
        retval=Entry.deletefile();
      }
    }
  }
  return retval;
}

int
GURL::renameto(const GURL &newurl) const
{
  return (is_local_file_url() && newurl.is_local_file_url())
    ?rename(NativeFilename(),newurl.NativeFilename())
    :(-1);
}

// expand_name(filename[, fromdirname])
// -- returns the full path name of filename interpreted
//    relative to fromdirname.  Use current working dir when
//    fromdirname is null.
GUTF8String 
GURL::expand_name(const GUTF8String &xfname, const char *from)
{
  const char *fname=xfname;
  GUTF8String retval;
  char * const string_buffer = retval.getbuf(MAXPATHLEN+10);
  // UNIX implementation
#ifdef UNIX
  // Perform tilde expansion
  GUTF8String senv;
  if (fname && fname[0]==tilde)
  {
    int n;
    for(n=1;fname[n] && fname[n]!= slash;n++) 
      EMPTY_LOOP;
    struct passwd *pw=0;
    if (n!=1)
    {
      GUTF8String user(fname+1, n-1);
      pw=getpwnam(user);
    }else if ((senv=GOS::getenv("HOME")).length())
    {
      from=(const char *)senv;
      fname = fname + n;
    }else if ((senv=GOS::getenv("LOGNAME")).length())
    {
      pw = getpwnam((const char *)senv.getUTF82Native());
    }else
    {
      pw=getpwuid(getuid());
    }
    if (pw)
    {
      senv=GNativeString(pw->pw_dir).getNative2UTF8();
      from = (const char *)senv;
      fname = fname + n;
    }
    for(;fname[0] == slash; fname++)
      EMPTY_LOOP;
  }
  // Process absolute vs. relative path
  if (fname && fname[0]== slash)
  {
    string_buffer[0]=slash;
    string_buffer[1]=0;
  }else if (from)
  {
    strcpy(string_buffer, expand_name(from));
  }else
  {
    strcpy(string_buffer, GOS::cwd());
  }
  char *s = string_buffer + strlen(string_buffer);
  if(fname)
  {
    for(;fname[0]== slash;fname++)
      EMPTY_LOOP;
    // Process path components
    while(fname[0])
    {
      if (fname[0] == dot )
      {
        if (!fname[1] || fname[1]== slash)
        {
          fname++;
          continue;
        }else if (fname[1]== dot && (fname[2]== slash || !fname[2]))
        {
          fname +=2;
          for(;s>string_buffer+1 && *(s-1)== slash; s--)
            EMPTY_LOOP;
          for(;s>string_buffer+1 && *(s-1)!= slash; s--)
            EMPTY_LOOP;
          continue;
        }
      }
      if ((s==string_buffer)||(*(s-1)!= slash))
      {
        *s = slash;
        s++;
      }
      while (*fname &&(*fname!= slash))
      {
        *s = *fname++;
        if ((++s)-string_buffer > MAXPATHLEN)
          G_THROW( ERR_MSG("GURL.big_name") );
      }
      *s = 0;
      for(;fname[0]== slash;fname++)
        EMPTY_LOOP;
    }
  }
  if (!fname || !fname[0])
  {
    for(;s>string_buffer+1 && *(s-1) == slash; s--)
      EMPTY_LOOP;
    *s = 0;
  }
#elif defined (WIN32) && !defined (UNDER_CE) // WIN32 implementation
  // Handle base
  strcpy(string_buffer, (char const *)(from?expand_name(from):GOS::cwd()));
//  GNativeString native;
  if (fname)
  {
    char *s = string_buffer;
    char  drv[4];
    // Handle absolute part of fname
    if (fname[0]== slash || fname[0]== backslash)
    {
      if (fname[1]== slash || fname[1]== backslash)
      { // Case "//abcd"
        s[0]=s[1]= backslash; s[2]=0;
      } else
      { // Case "/abcd" 
          /*
        if( _getdrive() )
        {
            if (s[0]==0 || s[1]!=colon)
            {
              s[0] = _getdrive() + 'A' - 1;
            }
            s[1]=colon;   
        }
        
        s[2]= 0;
        */
          s[0]=s[1]= backslash; s[2]=0;
      }
    } else if (fname[0] && fname[1]==colon)
    {
      if (fname[2]!= slash && fname[2]!= backslash)
      { // Case "x:abcd"
        if ( toupper((unsigned char)s[0]) != toupper((unsigned char)fname[0])
             || s[1]!=colon)
        {
          drv[0]=fname[0];
          drv[1]=colon;
          drv[2]= dot ;
          drv[3]=0;
          GetFullPathName(drv, MAXPATHLEN, string_buffer, &s);
		  strcpy(string_buffer,(const char *)GUTF8String(string_buffer).getNative2UTF8());
          s = string_buffer;
        }
        fname += 2;
      } else if (fname[3]!= slash && fname[3]!= backslash)
      { // Case "x:/abcd"
        s[0]=toupper((unsigned char)fname[0]);
        s[1]=colon;
        s[2]=backslash;
        s[3]=0;
        fname += 3;
      }else
      { // Case "x://abcd"
        s[0]=s[1]=backslash;
        s[2]=0;
        fname += 4;
      }
    }
    // Process path components
    for(;*fname== slash || *fname==backslash;fname++)
      EMPTY_LOOP;
    while(*fname)
    {
      if (fname[0]== dot )
      {
        if (fname[1]== slash || fname[1]==backslash || !fname[1])
        {
          fname++;
          continue;
        }else if ((fname[1] == dot)
          && (fname[2]== slash || fname[2]==backslash || !fname[2]))
        {
          fname += 2;
		  char *back=_tcsrchr(string_buffer,backslash);
		  char *forward=_tcsrchr(string_buffer,slash);
		  if(back>forward)
		  {
			*back=0;
		  }else if(forward)
		  {
            *forward=0;
		  }
          s = string_buffer;
          continue;
        }
		char* s2=s;//MBCS DBCS
        for(;*s;s++) 
          EMPTY_LOOP;
		char* back = _tcsrchr(s2,backslash);//MBCS DBCS
        if ((s>string_buffer)&&(*(s-1)!= slash)&&(back == NULL || (back!=NULL && s-1 != back) ))//MBCS DBCS
        //if ((s>string_buffer)&&(*(s-1)!= slash)&&(*(s-1)!= backslash))
        {
          *s = backslash;
          s++;
        }
        while (*fname && *fname!= slash && *fname!=backslash)
        {
          *s = *fname++;
          if ((++s)-string_buffer > MAXPATHLEN)
            G_THROW( ERR_MSG("GURL.big_name") );
        }
        *s = 0;
      }
	  char* s2=s;//MBCS DBCS
      for(;*s;s++) 
        EMPTY_LOOP;
	  char* back = _tcsrchr(s2,backslash);//MBCS DBCS
      if ((s>string_buffer)&&(*(s-1)!= slash)&&(back == NULL || (back!=NULL && s-1 != back) ))//MBCS DBCS
      //if ((s == string_buffer)||((*(s-1)!= slash) && (*(s-1)!=backslash)))
      {
        *s = backslash;
        s++;
      }
      while (*fname && (*fname!= slash) && (*fname!=backslash))
      {
        *s = *fname++;
        if ((++s)-string_buffer > MAXPATHLEN)
          G_THROW( ERR_MSG("GURL.big_name") );
      }
      *s = 0;
      for(;(*fname== slash)||(*fname==backslash);fname++)
        EMPTY_LOOP;
    }
  }
#elif defined(macintosh) // MACINTOSH implementation
  strcpy(string_buffer, (const char *)(from?from:GOS::cwd()));

  if (GUTF8String::ncmp(fname, string_buffer,strlen(string_buffer)) || is_file(fname))
  {
    strcpy(string_buffer, "");//please don't expand, the logic of filename is chaos.
  }
    
  // Process path components
  char *s = string_buffer + strlen(string_buffer);
  if(fname)
  {
    for(;fname[0]==colon;fname++)
      EMPTY_LOOP;
    while(fname[0])
    {
      if (fname[0]== dot )
      {
        if (fname[1]==colon || !fname[1])
        {
          fname++;
          continue;
        }
        if ((fname[1]== dot )
             &&(fname[2]==colon || fname[2]==0))
        {
          fname +=2;
          for(;(s>string_buffer+1)&&(*(s-1)==colon);s--)
            EMPTY_LOOP;
          for(;(s>string_buffer+1)&&(*(s-1)!=colon);s--)
            EMPTY_LOOP;
          continue;
        }
      }
      if ((s==string_buffer)||(*(s-1)!=colon))
      {
        *s = colon;
        s++;
      }
      while (*fname!=0 && *fname!=colon)
      {
        *s = *fname++;
        if ((++s)-string_buffer > MAXPATHLEN)
          G_THROW( ERR_MSG("GURL.big_name") );
      }
      *s = 0;
      for(;fname[0]==colon;fname++)
        EMPTY_LOOP;
    }
  }
  for(;(s>string_buffer+1) && (*(s-1)==colon);s--)
    EMPTY_LOOP;
  *s = 0;
  return ((string_buffer[0]==colon)?(string_buffer+1):string_buffer);
#elif   defined(UNDER_CE) 
  retval=fname;
#else
#error "Define something here for your operating system"
#endif  
  return retval;
}


