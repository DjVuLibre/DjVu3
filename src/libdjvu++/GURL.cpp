//C-  -*- C++ -*-
//C- DjVu� Reference Library (v. 3.0)
//C- 
//C- Copyright � 1999-2000 LizardTech, Inc. All Rights Reserved.
//C- 
//C- This software (the "Original Code") is subject to, and may be
//C- distributed under, the GNU General Public License, Version 2.
//C- The license should have accompanied the Original Code or you
//C- may obtain a copy of the license from the Free Software
//C- Foundation at http://www.fsf.org .
//C- 
//C- With respect to the Original Code, and subject to any third
//C- party intellectual property claims, LizardTech grants recipient
//C- a worldwide, royalty-free, non-exclusive license under patent
//C- claims infringed by making, using, or selling Original Code
//C- which are now or hereafter owned or controlled by LizardTech,
//C- but solely to the extent that any such patent is reasonably
//C- necessary to enable you to make, have made, practice, sell, or 
//C- otherwise dispose of Original Code (or portions thereof) and
//C- not to any greater extent that may be necessary to utilize
//C- further modifications or combinations.
//C- 
//C- The Original Code is provided "AS IS" WITHOUT WARRANTY OF ANY
//C- KIND, EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO
//C- ANY WARRANTY OF NON-INFRINGEMENT, OR ANY IMPLIED WARRANTY OF 
//C- MERCHANTIBILITY OF FITNESS FOR A PARTICULAR PURPOSE.
// 
// $Id: GURL.cpp,v 1.42 2000-11-03 02:08:37 bcr Exp $
// $Name:  $

#ifdef __GNUC__
#pragma implementation
#endif

#include "GException.h"
#include "GOS.h"
#include "GURL.h"

#include <string.h>
#include <ctype.h>

static const char djvuopts[]="DJVUOPTS";
static const char localhost[]="file://localhost/";
static const char fileproto[]="file:";

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
#ifndef UNIX
   GCriticalSectionLock lock(&class_lock);
   for(char *ptr=(url.getbuf()+protocol().length());*ptr;ptr++)
   {
     if(*ptr == '\\')
       *ptr='/';
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
   while((*(ptr++) = *(srcptr++)));
}

void
GURL::beautify_path(void)
{
   GCriticalSectionLock lock(&class_lock);
   
      // Eats parts like ./ or ../ or ///
   char * buffer=new char[url.length()+1];
   strcpy(buffer, url);
   
   G_TRY
   {
	 // Find start point
      char * start=buffer+protocol().length()+1;
      while(*start && *start=='/') start++;

	 // Find end of the url (don't touch arguments)
      char * ptr;
      GString args;
      for(ptr=start;*ptr;ptr++)
	 if (is_argument(ptr))
	 {
	    args=ptr;
	    *ptr=0;
	    break;
	 }

	 // Eat multiple slashes
      for(;(ptr=strstr(start, "////"));collapse(ptr, 3));
      for(;(ptr=strstr(start, "//"));collapse(ptr, 1));
      
	 // Convert /./ stuff into plain /
      for(;(ptr=strstr(start, "/./"));collapse(ptr, 2));

	 // Process /../
      while((ptr=strstr(start, "/../")))
      {
	 for(char * ptr1=ptr-1;(ptr1>=start);ptr1--)
         {
	    if (*ptr1=='/')
	    {
	       collapse(ptr1, ptr-ptr1+3);
	       break;
	    }
         }
      }
	 // Done. Copy the buffer back into the URL and add arguments.
      url=buffer;
      url+=args;
      delete buffer; buffer=0;
   } G_CATCH_ALL {
      delete buffer;
      G_RETHROW;
   } G_ENDCATCH;
}

void
GURL::init(void)
{
   GCriticalSectionLock lock(&class_lock);
   
   if (url.length())
   {
      GString proto=protocol();
      if (!proto.length())
        G_THROW("GURL.no_protocol\t"+url);

	 // Below we have to make this complex test to detect URLs really
	 // referring to *local* files. Surprisingly, file://hostname/dir/file
	 // is also valid, but shouldn't be treated thru local FS.
      if (proto=="file" && url[5]=='/' &&
	  (url[6]!='/' || !strncmp((const char *)url,localhost,sizeof(localhost))))
      {
	    // Separate the arguments
         GString arg;
         {
           const char * const url_ptr=url;
	   const char * ptr;
	   for(ptr=url_ptr;*ptr;ptr++)
	      if (is_argument(ptr))
	         break;
	   arg=ptr;
	   url.setat((int)(ptr-url_ptr), 0);
         }

	    // Do double conversion
	 GString tmp=GOS::url_to_filename(url);
	 if (!tmp.length()) G_THROW("GURL.fail_to_file");
	 url=GOS::filename_to_url(tmp);
	 if (!url.length()) G_THROW("GURL.fail_to_URL");

	    // Return the argument back
	 url+=arg;
      }

      convert_slashes();
      beautify_path();
      parse_cgi_args();
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

GURL::GURL(const GURL & url_in) : url(url_in.url)
{
   init();
}

GURL &
GURL::operator=(const GURL & url_in)
{
   GCriticalSectionLock lock(&class_lock);
   url=url_in.url;
   init();
   return *this;
}

GString
GURL::protocol(const char * url)
{
   const char * const url_ptr=url;
   const char * ptr;
   for(ptr=url_ptr;*ptr && !(!isalpha(*ptr) && !isdigit(*ptr) &&
	  *ptr!='+' && *ptr!='-' && *ptr!='.');ptr++);
   return(*ptr==':')?GString(url_ptr, ptr-url_ptr):GString();
}

GString
GURL::hash_argument(void) const
      // Returns the HASH argument (anything after '#' and before '?')
{
   GCriticalSectionLock lock((GCriticalSection *) &class_lock);
   bool found=false;
   GString arg;
   for(const char * start=url;*start;start++)
   {
	 // Break if CGI argument is found
      if (*start=='?')
         break;

      if (found)
      {
         arg+=*start;
      }else
      {
         found=(*start=='#');
      }
   }
   return GOS::decode_reserved(arg);
}

void
GURL::set_hash_argument(const char * arg)
{
   GCriticalSectionLock lock((GCriticalSection *) &class_lock);

   GString new_url;
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

   url=new_url+"#"+GOS::encode_reserved(arg)+ptr;
}

void
GURL::parse_cgi_args(void)
      // Will read CGI arguments from the URL into
      // cgi_name_arr and cgi_value_arr
{
   GCriticalSectionLock lock1(&class_lock);
   cgi_name_arr.empty();
   cgi_value_arr.empty();

      // Search for the beginning of CGI arguments
   const char * start=url;
   while(*start &&(*(start++)!='?'));

      // Now loop until we see all of them
   while(*start)
   {
      GString arg;	// Storage for another argument
      while(*start)	// Seek for the end of it
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
	 for(ptr=arg_ptr;*ptr&&(*ptr != '=');ptr++);

	 GString name, value;
	 if (*ptr)
	 {
	    name=GString(arg_ptr, (int)((ptr++)-arg_ptr));
	    value=GString(ptr, arg.length()-name.length()-1);
	 } else
         {
           name=arg;
         }
	    
	 int args=cgi_name_arr.size();
	 cgi_name_arr.resize(args);
	 cgi_value_arr.resize(args);
	 cgi_name_arr[args]=GOS::decode_reserved(name);
	 cgi_value_arr[args]=GOS::decode_reserved(value);
      }
   }
}

void
GURL::store_cgi_args(void)
      // Will store CGI arguments from the cgi_name_arr and cgi_value_arr
      // back into the URL
{
   GCriticalSectionLock lock1(&class_lock);

   const char * const url_ptr=url;
   const char * ptr;
   for(ptr=url_ptr;*ptr;ptr++)
      if (*ptr=='?') break;
   
   GString new_url=GString(url_ptr, ptr-url_ptr);
   
   for(int i=0;i<cgi_name_arr.size();i++)
   {
      GString name=GOS::encode_reserved(cgi_name_arr[i]);
      GString value=GOS::encode_reserved(cgi_value_arr[i]);
      if (i)
        new_url+="&"+name;
      else
        new_url+="?"+name;
      if (value.length())
	 new_url+="="+value;
   }

   url=new_url;
}

int
GURL::cgi_arguments(void) const
{
   GCriticalSectionLock lock((GCriticalSection *) &class_lock);
   return cgi_name_arr.size();
}

int
GURL::djvu_cgi_arguments(void) const
{
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

GString
GURL::cgi_name(int num) const
{
   GCriticalSectionLock lock((GCriticalSection *) &class_lock);
   if (num<cgi_name_arr.size()) return cgi_name_arr[num];
   else return GString();
}

GString
GURL::djvu_cgi_name(int num) const
{
   GCriticalSectionLock lock((GCriticalSection *) &class_lock);

   GString arg;
   for(int i=0;i<cgi_name_arr.size();i++)
      if (cgi_name_arr[i].upcase()==djvuopts)
      {
	 for(i++;i<cgi_name_arr.size();i++)
	    if (num--==0)
	    {
	       arg=cgi_name_arr[i];
	       break;
	    }
	 break;
      }
   return arg;
}

GString
GURL::cgi_value(int num) const
{
   GCriticalSectionLock lock((GCriticalSection *) &class_lock);
   if (num<cgi_value_arr.size())
   {
     return cgi_value_arr[num];
   }else
   {
     return GString();
   }
}

GString
GURL::djvu_cgi_value(int num) const
{
   GCriticalSectionLock lock((GCriticalSection *) &class_lock);

   GString arg;
   for(int i=0;i<cgi_name_arr.size();i++)
      if (cgi_name_arr[i].upcase()==djvuopts)
      {
	 for(i++;i<cgi_name_arr.size();i++)
	    if (num--==0)
	    {
	       arg=cgi_value_arr[i];
	       break;
	    }
	 break;
      }
   return arg;
}

DArray<GString>
GURL::cgi_names(void) const
{
   GCriticalSectionLock lock((GCriticalSection *) &class_lock);
   return cgi_name_arr;
}

DArray<GString>
GURL::cgi_values(void) const
{
   GCriticalSectionLock lock((GCriticalSection *) &class_lock);
   return cgi_value_arr;
}

DArray<GString>
GURL::djvu_cgi_names(void) const
{
   GCriticalSectionLock lock((GCriticalSection *) &class_lock);

   int i;
   DArray<GString> arr;
   for(i=0;i<cgi_name_arr.size();i++)
      if (cgi_name_arr[i].upcase()==djvuopts)
	 break;

   int size=cgi_name_arr.size()-(i+1);
   if (size>0)
   {
      arr.resize(size-1);
      for(i=0;i<arr.size();i++)
	 arr[i]=cgi_name_arr[cgi_name_arr.size()-arr.size()+i];
   }

   return arr;
}

DArray<GString>
GURL::djvu_cgi_values(void) const
{
   GCriticalSectionLock lock((GCriticalSection *) &class_lock);

   int i;
   DArray<GString> arr;
   for(i=0;i<cgi_name_arr.size();i++)
      if (cgi_name_arr[i].upcase()==djvuopts)
	 break;

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
   GCriticalSectionLock lock(&class_lock);
   bool found=false;
   GString new_url;
   for(const char * start=url;*start;start++)
   {
	 // Break on first CGI arg.
      if (*start=='?')
      {
	 new_url+=start;
	 break;
      }

      if (!found)
	 if (*start=='#')
	    found=true;

      if (!found) new_url+=*start;
   }
   url=new_url;
}

void
GURL::clear_cgi_arguments(void)
{
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
GURL::add_djvu_cgi_argument(const char * name, const char * value)
{
   GCriticalSectionLock lock1(&class_lock);

      // Check if we already have the "DJVUOPTS" argument
   bool have_djvuopts=false;
   for(int i=0;i<cgi_name_arr.size();i++)
      if (cgi_name_arr[i].upcase()==djvuopts)
      {
	 have_djvuopts=true;
	 break;
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
   GCriticalSectionLock lock((GCriticalSection *) &class_lock);
   return (protocol()=="file" && url[5]=='/');
}

GURL
GURL::base(void) const
{
   GCriticalSectionLock lock((GCriticalSection *) &class_lock);

   const char * const url_ptr=url;
   const char * ptr, * slash;
   for(ptr=slash=url_ptr+protocol().length()+1;*ptr && !is_argument(ptr);ptr++)
   {
      if (*ptr=='/')
        slash=ptr;
   }
   return
#ifdef WIN32
   (*(slash-1) == ':')?
     GString(url,(int)(slash-url))+"/"+GString(ptr,url.length()-(int)(ptr-url_ptr)) :
#endif
     GString(url,(int)(slash-url))+GString(ptr,url.length()-(int)(ptr-url_ptr));
}

GString
GURL::name(void) const
{
   GCriticalSectionLock lock((GCriticalSection *) &class_lock);
   const char * ptr, * slash;
   for(ptr=slash=(const char *)url+protocol().length()+1;
     *ptr && !is_argument(ptr);ptr++)
   {
      if (*ptr=='/')
        slash=ptr;
   }
   
   return GString(slash+1, ptr-slash-1);
}

GString
GURL::fname(void) const
{
   return GOS::decode_reserved(name());
}

GString
GURL::extension(void) const
{
   GString filename=name();
   GString retval;

   for(int i=filename.length()-1;i>=0;i--)
      if (filename[i]=='.')
      {
	 retval=GString((const char*) filename+i+1);
	 break;
      }
   
   return retval;
}

GURL
GURL::operator+(const char * xname) const
{
   GCriticalSectionLock lock((GCriticalSection *) &class_lock);
   GURL res;
   if (!protocol(xname).length())
   {
      const char * const url_ptr=(const char *)url;
      const char * ptr;
      for(ptr=url_ptr+protocol().length()+1;*ptr&&!is_argument(ptr);ptr++);

      if (*(ptr-1) != '/')
      {
        res=GString(url_ptr,(int)(ptr-url_ptr))+"/"+xname+ptr;
      }else
      {
        res=GString(url_ptr,(int)(ptr-url_ptr))+xname+ptr;
      }
   } else
   {
     res=xname;
   }
   res.parse_cgi_args();
   return res;
}
