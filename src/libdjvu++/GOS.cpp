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
// $Id: GOS.cpp,v 1.38 2000-11-30 21:31:07 bcr Exp $
// $Name:  $

#ifdef __GNUC__
#pragma implementation
#endif

#include "GException.h"
#include "GThreads.h"
#include "GOS.h"


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

// -- TRUE FALSE
#undef TRUE
#undef FALSE
#define TRUE 1
#define FALSE 0

// -- MAXPATHLEN
#ifndef MAXPATHLEN
#ifdef _MAX_PATH
#define MAXPATHLEN _MAX_PATH
#else
#define MAXPATHLEN 1024
#endif
#else    // MAXPATHLEN was originally defined.
#if ( MAXPATHLEN < 1024 )
#undef MAXPATHLEN
#define MAXPATHLEN 1024
#endif
#endif

static const char localhost[] = "localhost";
static const char localhostspec[] = "//localhost/";
static const char filespec[] = "file:";
static const char filespecslashes[] = "file://";
static const char slash='/';
static const char backslash='\\';
static const char colon=':';
static const char dot='.';
static const char tilde='~';
static const char percent='%';
static const char nillchar=0;
#ifdef UNIX
  static const char root[] = "/";
#else
#ifdef WIN32
  static const char root[] = "\\";
#else
#ifdef macintosh
  static char const * const root = &nillchar; 
#else
#error "Define something here for your operating system"
#endif
#endif  
#endif


// -----------------------------------------
// Functions for dealing with filenames
// -----------------------------------------

// is_file(filename) --
// -- returns true if filename denotes a regular file.
int 
GOS::is_file(const char *filename)
{
#if defined(UNIX) || defined(macintosh)
  struct stat buf;
  if (stat(filename,&buf)==-1)
    return FALSE;
  if (buf.st_mode & S_IFDIR) 
    return FALSE;
#elif defined(WIN32)
   DWORD           dwAttrib;       ;
   USES_CONVERSION ;
   dwAttrib = GetFileAttributes(A2CT(filename)) ;
   if (dwAttrib == 0xFFFFFFFF)
         return FALSE ;
   if( dwAttrib & FILE_ATTRIBUTE_DIRECTORY )
         return FALSE ;
#else
#error "Define something here for your operating system"
#endif
return TRUE;
  
}

// is_dir(filename) --
// -- returns true if filename denotes a directory.
int 
GOS::is_dir(const char *filename)
{
  if(!filename || !filename[0]) return FALSE;
  // UNIX implementation
#if defined(UNIX) || defined(macintosh)
  struct stat buf;
  if (stat(filename,&buf)==0)
    if (buf.st_mode & S_IFDIR)
      return TRUE;
#elif defined(WIN32)   // (either Windows or WCE)
   USES_CONVERSION ;
   DWORD           dwAttrib;       ;
   dwAttrib = GetFileAttributes(A2CT(filename)) ;
   if (dwAttrib != 0xFFFFFFFF)
    {
      if( dwAttrib & FILE_ATTRIBUTE_DIRECTORY )
      {
        return TRUE ;
      }
    }
#else
#error "Define something here for your operating system"
#endif  
    return FALSE;
}

// dirname(filename) --
// -- returns the name of the parent directory of filename.
//    works like /bin/dirname.
GString 
GOS::dirname(const char *fname)
{
  if(!fname || !fname[0])
    return &nillchar;
  GString retval;
  char * const string_buffer = retval.getbuf(strlen(fname)+16);
  char *q = string_buffer;
  char const *p = 0;
#ifdef WIN32
  // Handle leading drive specifier
  if (fname[0] && fname[1]==colon)
  {
    *q++ = *fname++;
    *q++ = *fname++;
  }
#endif
  char const *s = fname;

  // UNIX implementation
#ifdef UNIX
  while (*s)
  {
    if (s[0] == slash && s[1])
      p = s;
    s++;
  }
  if (!p)
  {
    return (fname[0]== slash)?GString(fname):GString(dot);
  }
#else

  // WIN32 implementation
#ifdef WIN32
  // Search last non terminal slash or backslash
  while (*s)
  {
    if (s[0]==backslash || s[0]== slash)
      if (s[1] && s[1] != slash && s[1]!= backslash)
        p = s;
    s++;
  }
  // Cannot find non terminal slash or backslash
  if (!p)
  {
    if (q>string_buffer)
    {
      if (fname[0]==0 || fname[0]== slash || fname[0]== backslash)
      {
        retval="\\\\";
      }else
      {
        *q = 0;
      }
    } else
    {
      if (fname[0]== slash || fname[0]== backslash)
      {
        retval="\\\\";
      }else
      {
        retval=GString(dot);
      }
    }
    return retval;
  }
  if (p == fname) // Single leading slash
  {
    q[0]=backslash;
    q[1]=0;
    return retval;
  }
  // Backtrack all slashes
  while (p>fname && (p[-1]== slash || p[-1]== backslash))
    p--;
  // Multiple leading slashes
  if (p == fname)
  {
    return "\\\\";
  }
#else
#ifdef macintosh
  while (*s)
  {
    if (s[0]== colon && s[1])
      p = s;
    s++;
  }
  if (!p)
  {
    return (fname[0]== colon)?fname:&nillchar;
  }
#else
#error "Define something here for your operating system"
#endif
#endif  
#endif
  // The normal case
  s = fname;
  do {
    *q++ = *s++;
  } while (s<p);
  *q = 0;
  return retval;
}

static inline const char *
finddirsep(const char * const fname)
{
#ifdef UNIX
  return fname?strrchr(fname,slash):0;
#else
#if defined(WIN32) || defined(macintosh) 
  char const * retval=0;
  if(fname)
  {
    for(const char *q=fname;*q;q++)
    {
#ifdef WIN32
      if(*q == slash || *q == backslash)
#else
      if(*q == slash || *q == colon)
#endif
      {
        retval=q;
      }
    }
  }
  return retval;
#else
#error "Define something here for your operating system"
#endif  
#endif  
}


// basename(filename[, suffix])
// -- returns the last component of filename and removes suffix
//    when present. works like /bin/basename.
GString 
GOS::basename(const char *fname, const char *suffix)
{
  if(!fname || !fname[0])
    return &nillchar;

#ifdef WIN32
  // Special cases
  if (fname[1] == colon)
  {
    if(!fname[2])
    {
      return fname;
    }
    if (!fname[3] && (fname[2]== slash || fname[2]== backslash))
    {
      char string_buffer[4];
      string_buffer[0] = fname[0];
      string_buffer[1] = colon;
      string_buffer[2] = backslash; 
      string_buffer[3] = 0; 
      return string_buffer;
    }
  }
#endif

  const char *q;
  if ((q=finddirsep(fname)))
    fname = q+1;


  // Allocate buffer
  GString retval(fname);

  // Process suffix
  if (suffix)
  {
    if (suffix[0]== dot )
      suffix ++;
    if (suffix[0])
    {
      const GString gsuffix(suffix);
      const int sl = gsuffix.length();
      const char *s = fname + strlen(fname);
      if (s > fname + sl)
      {
        s = s - (sl + 1);
        if(*s == dot && (GString(s+1).downcase() == gsuffix.downcase()))
        {
          retval.setat((int)((size_t)s-(size_t)fname),0);
        }
      }
    }
  }
  return retval;
}



// errmsg --
// -- A small helper function returning a 
//    stdio error message in a static buffer.

static const char *
errmsg()
{
  static char buffer[256];
#ifdef REIMPLEMENT_STRERROR
  const char *errname = "Unknown libc error";
  if (errno>0 && errno<sys_nerr)
    errname = sys_errlist[errno];
#else
#ifndef UNDER_CE
  const char *errname = strerror(errno);
  sprintf(buffer,"%s (errno = %d)", errname, errno);
#else
  const char *errname = "Unknown error from GOS.cpp under Windows CE" ;
  sprintf(buffer,"%s (errno = %d)", errname, -1);
#endif
#endif
  return buffer;
}




// cwd([dirname])
// -- changes directory to dirname (when specified).
//    returns the full path name of the current directory. 
GString 
GOS::cwd(const char *dirname)
{
#if defined(UNIX) || defined(macintosh) 
  if (dirname && chdir(dirname)==-1)
    G_THROW(errmsg());
  GString temp;
  char *string_buffer = temp.getbuf(MAXPATHLEN+1);
  char *result = getcwd(string_buffer,MAXPATHLEN);
  if (!result)
    G_THROW(errmsg());
  return result;
#else
#if defined (WIN32) || defined (UNDER_CE)
#ifndef UNDER_CE
  char drv[2];
  if (dirname && _chdir(dirname)==-1)
    G_THROW(errmsg());
  drv[0]= dot ; drv[1]=0;
  GString temp;
  char *string_buffer = temp.getbuf(MAXPATHLEN+1);
  char *result = getcwd(string_buffer,MAXPATHLEN);
  GetFullPathName(drv, MAXPATHLEN, string_buffer, &result);
  return string_buffer;
#else
  return GString(dot) ;
#endif
#else
#error "Define something here for your operating system"
#endif 
#endif
}


// expand_name(filename[, fromdirname])
// -- returns the full path name of filename interpreted
//    relative to fromdirname.  Use current working dir when
//    fromdirname is null.
GString 
GOS::expand_name(const char *fname, const char *from)
{
  GString temp;
  char *string_buffer = temp.getbuf(MAXPATHLEN+10);
  // UNIX implementation
#ifdef UNIX
  char *s;
  // Perform tilde expansion
  if (fname && fname[0]==tilde)
    {
      int n = 1;
      while (fname[n] && fname[n]!= slash) 
        n += 1;
      GString user(fname+1, n-1);
      if (n==1 && (s=getenv("LOGNAME")))
        user = s;
      struct passwd *pw = getpwnam(user);
      if (n==1 && pw==0)
        pw = getpwuid(getuid());
      if (pw) {
        from = pw->pw_dir;
        fname = fname + n;
      }
      while (fname[0] && fname[0]== slash)
        fname += 1;
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
    strcpy(string_buffer, cwd());
  }
  // Process path components
  s = string_buffer + strlen(string_buffer);
  for (;;) {
    while (fname && fname[0]== slash)
      fname++;
    if (!fname || !fname[0]) {
      while (s>string_buffer+1 && s[-1]== slash)
	s--;
      *s = 0;
      return string_buffer;
    }
    if (fname[0]== dot ) {
      if (fname[1]== slash || fname[1]==0) {
	fname +=1;
	continue;
      }
      if (fname[1]== dot )
	if (fname[2]== slash || fname[2]==0) {
	  fname +=2;
	  while (s>string_buffer+1 && s[-1]== slash)
	    s--;
	  while (s>string_buffer+1 && s[-1]!= slash)
	    s--;
	  continue;
	}
    }
    if (s==string_buffer || s[-1]!= slash)
      *s++ = slash;
    while (*fname!=0 && *fname!= slash) {
      *s++ = *fname++;
      if (s-string_buffer > MAXPATHLEN)
        G_THROW("GOS.big_name");
    }
    *s = 0;
  }
#else
  // WIN32 implementation
#if defined (WIN32) && !defined (UNDER_CE)
  char *s;
  char  drv[4];
  // Handle base
  if (from)
    strcpy(string_buffer, expand_name(from));
  else
    strcpy(string_buffer, cwd());
  s = string_buffer;
  if (fname==0)
    return s;
  // Handle absolute part of fname
  if (fname[0]== slash || fname[0]== backslash)
  {
    if (fname[1]== slash || fname[1]== backslash)
    {	// Case "//abcd"
      s[0]=s[1]= backslash; s[2]=0;
    } else
    {	// Case "/abcd" 
      if (s[0]==0 || s[1]!=colon)
	s[0] = _getdrive() + 'A' - 1;
      s[1]=colon; s[2]= 0;
    }
  } else if (fname[0] && fname[1]==colon)
  {
    if (fname[2]!= slash && fname[2]!= backslash)
    { // Case "x:abcd"
      if ( toupper((unsigned char)s[0])!=toupper((unsigned char)fname[0]) || s[1]!=colon) {
	drv[0]=fname[0]; drv[1]=colon; drv[2]= dot ; drv[3]=0;
	GetFullPathName(drv, MAXPATHLEN, string_buffer, &s);
        s = string_buffer;
      }
      fname += 2;
    } else if (fname[3]!= slash && fname[3]!= backslash)
    {	// Case "x:/abcd"
      s[0]=toupper((unsigned char)fname[0]);
      s[1]=colon;
      s[2]=backslash;
      s[3]=0;
      fname += 3;
    }else
    {	// Case "x://abcd"
      s[0]=s[1]=backslash;
      s[2]=0;
      fname += 4;
    }
  }
  // Process path components
  while(*fname)
  {
    while (*fname== slash || *fname==backslash)
      fname ++;
    if (*fname == 0)
      break;
    if (fname[0]== dot )
    {
      if (fname[1]== slash || fname[1]==backslash || fname[1]==0)
      {
        fname += 1;
        continue;
      }else if (fname[1]== dot )
      {
        if (fname[2]== slash || fname[2]==backslash || fname[2]==0)
        {
          fname += 2;
          strcpy(string_buffer, dirname(string_buffer));
          s = string_buffer;
          continue;
        }
        if (fname[1]== dot )
          if (fname[2]== slash || fname[2]==backslash || fname[2]==0) {
            fname += 2;
            strcpy(string_buffer, dirname(string_buffer));
            s = string_buffer;
	  continue;
          }
      }
      while (*s) 
        s++;
      if (s[-1]!= slash && s[-1]!= backslash)
        *s++ = backslash;
      while (*fname && *fname!= slash && *fname!=backslash) {
        *s++ = *fname++;
        if (s-string_buffer > MAXPATHLEN)
          G_THROW("GOS.big_name");
      }
      *s = 0;
    }
    while (*s) 
      s++;
    if ((s == string_buffer)||(*(s-1)!= slash && *(s-1)!=backslash))
      *s++ = backslash;
    while (*fname && *fname!= slash && *fname!=backslash)
    {
      *s++ = *fname++;
      if (s-string_buffer > MAXPATHLEN)
        G_THROW("GOS.big_name");
    }
    *s = 0;
  }
  return string_buffer;
#else
  // MACINTOSH implementation
#ifdef macintosh
  char *s;

  if (from)
    strcpy(string_buffer, from);
  else
    strcpy(string_buffer, cwd());
    
  if (!strncmp(string_buffer,fname,strlen(string_buffer)) || is_file(fname))
    strcpy(string_buffer, "");//please don't expand, the logic of filename is chaos.
    
  // Process path components
  s = string_buffer + strlen(string_buffer);
  for (;;) {
    while (fname && fname[0]==colon)
      fname++;
    if (!fname || !fname[0]) {
      while (s>string_buffer+1 && s[-1]==colon)
	s--;
      *s = 0;
      if (string_buffer[0]==colon)
      	return &string_buffer[1];
      else
      	return string_buffer;
    }
    if (fname[0]== dot ) {
      if (fname[1]==colon || fname[1]==0) {
	fname +=1;
	continue;
      }
      if (fname[1]== dot )
	if (fname[2]==colon || fname[2]==0) {
	  fname +=2;
	  while (s>string_buffer+1 && s[-1]==colon)
	    s--;
	  while (s>string_buffer+1 && s[-1]!=colon)
	    s--;
	  continue;
	}
    }
    if (s==string_buffer || s[-1]!=colon)
      *s++ = colon;
    while (*fname!=0 && *fname!=colon) {
      *s++ = *fname++;
      if (s-string_buffer > MAXPATHLEN)
        G_THROW("GOS.big_name");
    }
    *s = 0;
  }
#else
#if   defined (UNDER_CE) 
  strcpy(string_buffer, fname) ;    // CE has no concept of a "current directory"
  return string_buffer ;            //  so all file references should be absolute.
#else
#error "Define something here for your operating system"
#endif  
#endif  
#endif
#endif  
}


// deletefile
// -- deletes a file or directory
  
int
GOS::deletefile(const char * filename)
{
  int retval=-1;
  if(filename && filename[0])
  {
#ifdef WIN32
   USES_CONVERSION;
   if (is_dir(filename))
   {
      retval= RemoveDirectory(A2CT(filename));
   }
   else
   {
      retval = DeleteFile(A2CT(filename)) ;
   }
#else
    if (is_dir(filename))
      retval=rmdir(filename);
    else
      retval=unlink(filename);
#endif
  }
  return retval;
}


int
GOS::mkdir(const char * dirname)
{
   if (!dirname || !dirname[0]) return -1;

      // See if we need to create the parent directory
   GString parent=GOS::dirname(dirname);
   if (!GOS::is_dir(parent))
   {
      int rc=mkdir(parent);
      if (rc<0) return rc;
   }
#ifdef WIN32
   USES_CONVERSION;
   return CreateDirectory(A2CT(dirname), NULL);
#else
   return ::mkdir(dirname, 0755);
#endif
}


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


int
GOS::cleardir(const char * dirname)
{
   if (!dirname || !dirname[0]) return -1;
   
#ifdef UNIX
   DIR * dir=opendir(dirname);
   if (dir)
     {
       dirent * de;
       while((de=readdir(dir)))
         {
           int len = NAMLEN(de);
           if (de->d_name[0]== dot  && len==1) continue;
           if (de->d_name[0]== dot  && de->d_name[1]== dot  && len==2) continue;
           GString name = GOS::expand_name( GString(de->d_name, len), dirname);
           int status = 0;
           if (GOS::is_dir(name))
             status = cleardir(name);
           if (status < 0) return status;
           status = GOS::deletefile(name);
           if (status < 0) return status;
         }
       closedir(dir);
       return 0;
     }
#else
#if defined (WIN32) && !defined (UNDER_CE)
   GString buffer=dirname;
   buffer += "\\*.*";

   WIN32_FIND_DATA finddata;
   HANDLE handle = FindFirstFile(buffer, &finddata);
   if( handle == INVALID_HANDLE_VALUE)
       return -1;

   do
   {
       buffer = dirname;
       buffer += GString("\\") + finddata.cFileName;

       if( finddata.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )                      {
           /// if it is directory, clear it
           if( GString(dot)!=finddata.cFileName && GString("..")!=finddata.cFileName)
           {
               if( cleardir(buffer) >= 0 )
                   RemoveDirectory(buffer);
           }
       }
       else
       {
           /// if file, remove it
           DeleteFile(buffer);
       }
       /// get next entry
   } while( FindNextFile(handle, &finddata));

   FindClose(handle);
   return 0;
#else
   // WCE and MAC is missing
   G_THROW("GOS.cleardir");
#endif
#endif
   return -1;
}

// -----------------------------------------
// Functions for measuring time
// -----------------------------------------

// ticks() --
// -- returns the number of milliseconds elapsed since 
//    a system dependent date.
unsigned long 
GOS::ticks()
{
#ifdef UNIX
  struct timeval tv;
  if (gettimeofday(&tv, NULL) < 0)
    G_THROW(errmsg());
  return (unsigned long)( ((tv.tv_sec & 0xfffff)*1000) 
                          + (tv.tv_usec/1000) );
#else
#ifdef WIN32
  DWORD clk = GetTickCount();
  return (unsigned long)clk;
#else
#ifdef macintosh
  return (unsigned long)((double)TickCount()*16.66);
#else
#error "Define something here for your operating system"
#endif
#endif  
#endif
}

// sleep(int milliseconds) --
// -- sleeps during the specified time (in milliseconds)
void 
GOS::sleep(int milliseconds)
{
#ifdef UNIX
  struct timeval tv;
  tv.tv_sec = milliseconds / 1000;
  tv.tv_usec = (milliseconds - (tv.tv_sec * 1000)) * 1000;
#if defined(THREADMODEL) && (THREADMODEL==COTHREADS)
  GThread::select(0, NULL, NULL, NULL, &tv);
#else
  select(0, NULL, NULL, NULL, &tv);
#endif
#endif
#ifdef WIN32
  Sleep(milliseconds);
#endif
#ifdef macintosh
    unsigned long tick = ticks(), now;
    while (1) {
        now = ticks();
        if ((tick+milliseconds) < now)
            break;
        GThread::yield();
    }
#endif
}

  


// -------------------------------------------
// Functions for converting filenames and urls
// -------------------------------------------

// filename_to_url --
// -- Returns a url for accessing a given file.
//    If useragent is not provided, standard url will be created,
//    but will not be understood by some versions if IE.
GString 
GOS::filename_to_url(const char *filename, const char *useragent)
{
  // Special case for blank pages
  if(!filename || !filename[0])
    return GString("about:blank");

  // Special case for stupid MSIE 
  GString agent(useragent ? useragent : "default");
  if (agent.search("MSIE")>=0 || agent.search("Microsoft")>=0)
    return filespecslashes + expand_name(filename);

  // Potentially unsafe characters (cf. RFC1738 and RFC1808)
  const char *hex = "0123456789ABCDEF";
  
  // Normalize file name to url slash-and-escape syntax
  GString nname;
  GString oname = expand_name(filename);
  const unsigned char *s = (const unsigned char*) (const char*) oname;
  unsigned char *d = (unsigned char*) nname.getbuf( oname.length() * 3 );
  for (; *s; s++)
    {
      // Convert directory separator to slashes
#ifdef WIN32
      if (*s == backslash || *s== slash)
        { *d++ = slash; continue; }
#else
#ifdef macintosh
      if (*s == colon )
        { *d++ = slash; continue; }
#else
#ifdef UNIX
      if (*s == slash )
        { *d++ = slash; continue; }
#else
#error "Define something here for your operating system"
#endif  
#endif
#endif
	// WARNING: Whenever you modify this conversion code,
	// make sure, that the following functions are in sync:
	//   encode_reserved()
	//   decode_reserved()
	//   url_to_filename()
	//   filename_to_url()
	// unreserved characters
      if ( (*s>='a' && *s<='z') ||
           (*s>='A' && *s<='Z') ||
           (*s>='0' && *s<='9') ||
           (strchr("$-_.+!*'(),:", *s)) )   // Added : because of windows!
        { *d++ = *s; continue; }
      // escape sequence
      *d++ = percent;
      *d++ = hex[ (*s >> 4) & 0xf ];
      *d++ = hex[ (*s) & 0xf ];
    }
  *d = 0;
  // Preprend "file://" to file name. If file is on the local
  // machine, include "localhost".
  GString retval(filespecslashes);
  const char *cnname=nname;
  if (cnname[0] == slash)
  {
    if (cnname[1] == slash)
    {
      retval+=cnname+2;
    }else
    {
      retval+=localhost+nname;
    }
  }else
  {
    retval+=(localhostspec+2)+nname;
  }
  return retval;
}


// hexval --
// -- Returns the hexvalue of a character.
//    Returns -1 if illegal;

static int 
hexval(char c)
{
  if (c>='0' && c<='9')
    return c-'0';
  if (c>='A' && c<='F')
    return c-'A'+10;
  if (c>='a' && c<='f')
    return c-'a'+10;
  return -1;
}



// url_to_filename --
// -- Applies heuristic rules to convert a url into a valid file name.  
//    Returns a simple basename in case of failure.
GString 
GOS::url_to_filename(const char *url)
{
  if(!url||!strcmp(url,"about:blank"))
    return GString("");

	// WARNING: Whenever you modify this conversion code,
	// make sure, that the following functions are in sync:
	//   encode_reserved()
	//   decode_reserved()
	//   url_to_filename()
	//   filename_to_url()
  // Process hexdecimal character specification
  GString urlcopy;
  char *d = urlcopy.getbuf(strlen(url)+1);
  while (*url)
    {
      if (*url== percent)
        {
          int c1 = hexval(url[1]);
          int c2 = hexval(url[2]);
          if (c1>=0 && c2>=0)
            {
              *d++ = (c1<<4)|c2;
              url += 3;
              continue;
            }
        }
      *d++ = *url++;
    }
  *d = 0;
  url = (const char*)urlcopy;
  // Check if we have a simple file name already
  {
    GString tmp=expand_name(url,root);
    if (is_file(tmp)) 
      return tmp;
  }
  // All file urls are expected to start with filespec which is "file:"
  if (strncmp(url, filespec, strlen(filespec)))  //if not
    return basename(url);

  //url does start with "file:", so move pointer to the position next to ":"
  url += strlen(filespec);

  //remove all leading slashes
  while(*url=='/')
	  url++;
  // Remove possible localhost spec
  if (!strncmp(url, localhost, strlen(localhost)))
	  url += strlen(localhost);
  //remove all leading slashes
  while(*url=='/')
	  url++;
  // Check if we are finished

#ifdef macintosh
  char l_url[1024];
  strcpy(l_url,url);
  char *s = l_url;
  for (; *s; s++)
    if (*s == slash )
      *s=colon;
  GString retval = expand_name(l_url,root);
#else  
  GString retval = expand_name(url,root);
#endif

#ifdef WIN32
  if (!is_file(retval)) 
  {
  // Search for a drive letter (encoded a la netscape)
    if (url[1]=='|' && url[2]== slash)
    {
	  if ((url[0]>='a' && url[0]<='z') 
          || (url[0]>='A' && url[0]<='Z'))
	  {
          GString drive;
          drive.format("%c:\\", url[0]);
          retval = expand_name(url+3, drive);
	  }
	}
  }
#endif
  // Return what we have
  return retval;
}


GString
GOS::encode_reserved(const char * filename)
      // WARNING: Whenever you modify this conversion code,
      // make sure, that the following functions are in sync:
      //   encode_reserved()
      //   decode_reserved()
      //   url_to_filename()
      //   filename_to_url()
{
   const char *hex = "0123456789ABCDEF";
   
   GString res;

   for(const char * ptr=filename;*ptr;ptr++)
   {
      if ((*ptr>='a' && *ptr<='z') ||
	  (*ptr>='A' && *ptr<='Z') ||
	  (*ptr>='0' && *ptr<='9') ||
	  (strchr("$-_.+!*'(),:", *ptr)))	// Added : because of windows!
	 res+=*ptr;
      else
      {
	    // escape sequence
	 res+=percent;
	 res+=hex[(*ptr >> 4) & 0xf];
	 res+=hex[(*ptr) & 0xf];
      }
   }
   
   return res;
}

GString
GOS::decode_reserved(const char * url)
      // WARNING: Whenever you modify this conversion code,
      // make sure, that the following functions are in sync:
      //   encode_reserved()
      //   decode_reserved()
      //   url_to_filename()
      //   filename_to_url()
{
   GString res;

   for(const char * ptr=url;*ptr;ptr++)
   {
      if (*ptr!=percent) res+=*ptr;
      else
      {
	 int c1=hexval(ptr[1]);
	 int c2=hexval(ptr[2]);
	 if (c1>=0 && c2>=0)
	 {
	    res+=(c1<<4)|c2;
	    ptr+=2;
	 } else res+=*ptr;
      }
   }

   return res;
}


// -----------------------------------------
// Testing
// -----------------------------------------

#if defined(sun) && ! defined(svr4)
// strerror() is not defined under SunOS.
char *
strerror(int errno)
{
  extern int sys_nerr;
  extern char *sys_errlist[];
  if (errno>0 && errno<sys_nerr) 
    return sys_errlist[errno];
  return "unknown stdio error";
}
#endif



// -----------------------------------------
// Testing
// -----------------------------------------

#ifdef TEST

int main(int argc, char **argv)
{
  GString op;
  if (argc>1) 
    op = argv[1];
  if (op == "is_file" && argc==3) {
      printf("%d\n", GOS::is_file(argv[2]));
      return 0;
  } else if (op =="is_dir" && argc==3) {
    printf("%d\n", GOS::is_dir(argv[2]));
    return 0;
  } else if (op == "dirname" && argc==3) {
    printf("%s\n", (const char*)GOS::dirname(argv[2]));
    return 0;
  } else if (op == "basename" && argc==3) {
    printf("%s\n", (const char*)GOS::basename(argv[2]));
    return 0;
  } else if (op == "basename" && argc==4) {
    printf("%s\n", (const char*)GOS::basename(argv[2], argv[3]));
    return 0;
  } else if (op == "cwd" && argc==2) {
    printf("%s\n", (const char*)GOS::cwd());
    return 0;
  } else if (op == "cwd" && argc==3) {
    printf("%s\n", (const char*)GOS::cwd(argv[2]));
    return 0;
  } else if (op == "cleardir" && argc==3) {
    printf("%d\n", GOS::cleardir(argv[2]));
    return 0;
  } else if (op == "expand_name" && argc==3) {
    printf("%s\n", (const char*)GOS::expand_name(argv[2]));
    return 0;
  } else if (op == "expand_name" && argc==4) {
    printf("%s\n", (const char*)GOS::expand_name(argv[2], argv[3]));
    return 0;
  } else if (op == "ticks" && argc==2) {
    printf("%lu\n", GOS::ticks());
    return 0;
  } else if (op == "sleep" && argc==3) {
    GOS::sleep(atoi(argv[2]));
    return 0;
  } else if (op == "filename_to_url" && argc==3) {
    printf("%s\n", (const char*)GOS::filename_to_url(argv[2]));
    return 0;
  } else if (op == "url_to_filename" && argc==3) {
    printf("%s\n", (const char*)GOS::url_to_filename(argv[2]));
    return 0;
  }
  fprintf(stderr,"syntax error\n");
  return 10;
}



#endif
