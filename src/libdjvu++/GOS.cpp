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
//C- $Id: GOS.cpp,v 1.28 2000-09-18 17:10:16 bcr Exp $

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


// -----------------------------------------
// Functions for dealing with filenames
// -----------------------------------------


// This code is derived from TL3 file fileio.c that I wrote a while ago.
// I, Leon Bottou, hereby give AT&T a royalty-free non-exclusive
// license to do whatever they want with this code.


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
  /* UNIX implementation */
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
//  if (!fname) fname="";
  if(!fname || !fname[0]) return GString("");
  /* UNIX implementation */  
#ifdef UNIX
  GString temp;
  char *string_buffer = temp.getbuf(strlen(fname)+16);
  const char *s = fname;
  const char *p = 0;
  char *q = string_buffer;
  while (*s) {
    if (s[0]=='/' && s[1])
      p = s;
    s++;
  }
  if (!p) {
    if (fname[0]=='/')
      return fname;
    else
      return ".";
  }
  s = fname;
  do {
    *q++ = *s++;
  } while (s<p);
  *q = 0;
  return string_buffer;
#else

  /* WIN32 implementation */
#ifdef WIN32
  GString temp;
  char *string_buffer = temp.getbuf(strlen(fname)+16);
  char *q = string_buffer;
  /* Handle leading drive specifier */
  if (fname[0] && fname[1]==':') {
    *q++ = *fname++;
    *q++ = *fname++;
  }
  /* Search last non terminal / or \ */
  const char *p = 0;
  const char *s = fname;
  while (*s) {
    if (s[0]=='\\' || s[0]=='/')
      if (s[1] && s[1]!='/' && s[1]!='\\')
        p = s;
    s++;
  }
  /* Cannot find non terminal / or \ */
  if (p == 0) {
    if (q>string_buffer) {
      if (fname[0]==0 || fname[0]=='/' || fname[0]=='\\')
	return "\\\\";
      *q = 0;
      return string_buffer;
    } else {
      if (fname[0]=='/' || fname[0]=='\\')
	return "\\\\";
      else
	return ".";
    }
  }
  /* Single leading slash */
  if (p == fname) {
    strcpy(q,"\\");
    return string_buffer;
  }
  /* Backtrack all slashes */
  while (p>fname && (p[-1]=='/' || p[-1]=='\\'))
    p--;
  /* Multiple leading slashes */
  if (p == fname)
    return "\\\\";
  /* Regular case */
  s = fname;
  do {
    *q++ = *s++;
  } while (s<p);
  *q = 0;
  return string_buffer;
#else
#ifdef macintosh
  GString temp;
  char *string_buffer = temp.getbuf(strlen(fname)+16);
  const char *s = fname;
  const char *p = 0;
  char *q = string_buffer;
  while (*s) {
    if (s[0]==':' && s[1])
      p = s;
    s++;
  }
  if (!p) {
    if (fname[0]==':')
      return fname;
    else
      return "";
  }
  s = fname;
  do {
    *q++ = *s++;
  } while (s<p);
  *q = 0;
  return string_buffer;
#else
#error "Define something here for your operating system"
#endif
#endif  
#endif
}



// basename(filename[, suffix])
// -- returns the last component of filename and removes suffix
//    when present. works like /bin/basename.
GString 
GOS::basename(const char *fname, const char *suffix)
{
  if(!fname || !fname[0]) return GString("");
  /* UNIX implementation */
#ifdef UNIX
  char *s = strrchr(fname,'/');
  if (s)
    fname = s+1;
  /* Process suffix */
  if (suffix==0 || suffix[0]==0)
    return fname;
  if (suffix[0]=='.')
    suffix++;
  if (suffix[0]==0)
    return fname;
  GString temp;
  char *string_buffer = temp.getbuf(strlen(fname)+16);
  strcpy(string_buffer,fname);
  int sl = strlen(suffix);
  s = string_buffer + strlen(string_buffer);
  if (s > string_buffer + sl) {
    s =  s - (sl + 1);
    if (s[0]=='.' && strcmp(s+1,suffix)==0)
      *s = 0;
  }
  return string_buffer;
#else
  
  /* WIN32 implementation */
#ifdef WIN32
  /* Position p after last slash */
  const char *p = fname;
  for (const char *q = fname; *q; q++)
    if (q[0]=='\\' || q[0]=='/')
      p = q + 1;
  /* Allocate buffer */
  GString temp;
  char *string_buffer = temp.getbuf(strlen(p)+16);
  /* Special cases */
  if (fname[0] && fname[1]==':') {
    strncpy(string_buffer,fname,4);
    if (fname[2]==0)
      return string_buffer;
    string_buffer[2] = '\\'; 
    string_buffer[3] = 0; 
    if (fname[3]==0 && (fname[2]=='/' || fname[2]=='\\'))
      return string_buffer;
  }
  /* Copy into buffer */
  char *s = string_buffer;
  while (*p && *p!='/' && *p!='\\')
    *s++ = *p++;
  *s = 0;
  /* Process suffix */
  if (suffix==0 || suffix[0]==0)
    return string_buffer;
  if (suffix[0]=='.')
    suffix += 1;
  if (suffix[0]==0)
    return string_buffer;    
  int sl = strlen(suffix);
  if (s > string_buffer + sl) {
    s = s - (sl + 1);
#ifndef UNDER_CE
    if (s[0]=='.' && stricmp(s+1,suffix)==0)
#else    // To do:  make this case-insensitive under CE.
    if (s[0]=='.' && strcmp(s+1,suffix)==0)
#endif
      *s = 0;
  }
  return string_buffer;
#else
#ifdef macintosh
    char l_fname[1024], *l_fname2;
    l_fname2 = l_fname;
    strcpy(l_fname,fname);
    char *s = l_fname;
    for (; *s; s++)
      if (*s == '/' )
         *s=':';
  s = strrchr(l_fname,':');
  if (s)
    l_fname2 = s+1;
  /* Process suffix */
  if (suffix==0 || suffix[0]==0)
    return l_fname2;
  if (suffix[0]=='.')
    suffix ++;
  if (suffix[0]==0)
    return l_fname2;
  GString temp;
  char *string_buffer = temp.getbuf(strlen(l_fname2)+16);
  strcpy(string_buffer,l_fname2);
  int sl = strlen(suffix);
  s = string_buffer + strlen(string_buffer);
  if (s > string_buffer + sl) {
    s =  s - (sl + 1);
    if (s[0]=='.' && strcmp(s+1,suffix)==0)
      *s = 0;
  }
  return string_buffer;
#else  
#error "Define something here for your operating system"
#endif
#endif
#endif
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
  drv[0]='.'; drv[1]=0;
  GString temp;
  char *string_buffer = temp.getbuf(MAXPATHLEN+1);
  char *result = getcwd(string_buffer,MAXPATHLEN);
  GetFullPathName(drv, MAXPATHLEN, string_buffer, &result);
  return string_buffer;
#else
  return "." ;
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
  /* UNIX implementation */
#ifdef UNIX
  char *s;
  /* Perform tilde expansion */
  if (fname && fname[0]=='~')
    {
      int n = 1;
      while (fname[n] && fname[n]!='/') 
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
      while (fname[0] && fname[0]=='/')
        fname += 1;
    }
  /* Process absolute vs. relative path */
  if (fname && fname[0]=='/')
    strcpy(string_buffer,"/");
  else if (from)
    strcpy(string_buffer, expand_name(from));
  else
    strcpy(string_buffer, cwd());
  /* Process path components */
  s = string_buffer + strlen(string_buffer);
  for (;;) {
    while (fname && fname[0]=='/')
      fname++;
    if (!fname || !fname[0]) {
      while (s>string_buffer+1 && s[-1]=='/')
	s--;
      *s = 0;
      return string_buffer;
    }
    if (fname[0]=='.') {
      if (fname[1]=='/' || fname[1]==0) {
	fname +=1;
	continue;
      }
      if (fname[1]=='.')
	if (fname[2]=='/' || fname[2]==0) {
	  fname +=2;
	  while (s>string_buffer+1 && s[-1]=='/')
	    s--;
	  while (s>string_buffer+1 && s[-1]!='/')
	    s--;
	  continue;
	}
    }
    if (s==string_buffer || s[-1]!='/')
      *s++ = '/';
    while (*fname!=0 && *fname!='/') {
      *s++ = *fname++;
      if (s-string_buffer > MAXPATHLEN)
	G_THROW("filename length exceeds system limits");
    }
    *s = 0;
  }
#else
  /* WIN32 implementation */
#if defined (WIN32) && !defined (UNDER_CE)
  char *s;
  char  drv[4];
  /* Handle base */
  if (from)
    strcpy(string_buffer, expand_name(from));
  else
    strcpy(string_buffer, cwd());
  s = string_buffer;
  if (fname==0)
    return s;
  /* Handle absolute part of fname */
  if (fname[0]=='/' || fname[0]=='\\')
  {
    if (fname[1]=='/' || fname[1]=='\\')
    {	// Case "//abcd"
      s[0]=s[1]='\\'; s[2]=0;
    } else
    {	// Case "/abcd" 
      if (s[0]==0 || s[1]!=':')
	s[0] = _getdrive() + 'A' - 1;
      s[1]=':'; s[2]= 0;
    }
  } else if (fname[0] && fname[1]==':')
  {
    if (fname[2]!='/' && fname[2]!='\\')
    { // Case "x:abcd"
      if ( toupper((unsigned char)s[0])!=toupper((unsigned char)fname[0]) || s[1]!=':') {
	drv[0]=fname[0]; drv[1]=':'; drv[2]='.'; drv[3]=0;
	GetFullPathName(drv, MAXPATHLEN, string_buffer, &s);
        s = string_buffer;
      }
      fname += 2;
    } else if (fname[3]!='/' && fname[3]!='\\')
    {	// Case "x:/abcd"
      s[0]=toupper((unsigned char)fname[0]);
      s[1]=':';
      s[2]='\\';
      s[3]=0;
      fname += 3;
    }else
    {	// Case "x://abcd"
      s[0]=s[1]='\\';
      s[2]=0;
      fname += 4;
    }
  }
  /* Process path components */
  while(*fname)
  {
    while (*fname=='/' || *fname=='\\')
      fname ++;
    if (*fname == 0)
      break;
    if (fname[0]=='.')
    {
      if (fname[1]=='/' || fname[1]=='\\' || fname[1]==0)
      {
        fname += 1;
        continue;
      }else if (fname[1]=='.')
      {
        if (fname[2]=='/' || fname[2]=='\\' || fname[2]==0)
        {
          fname += 2;
          strcpy(string_buffer, dirname(string_buffer));
          s = string_buffer;
          continue;
        }
        if (fname[1]=='.')
          if (fname[2]=='/' || fname[2]=='\\' || fname[2]==0) {
            fname += 2;
            strcpy(string_buffer, dirname(string_buffer));
            s = string_buffer;
	  continue;
          }
      }
      while (*s) 
        s++;
      if (s[-1]!='/' && s[-1]!='\\')
        *s++ = '\\';
      while (*fname && *fname!='/' && *fname!='\\') {
        *s++ = *fname++;
        if (s-string_buffer > MAXPATHLEN)
          G_THROW("filename length exceeds system limits");
      }
      *s = 0;
    }
    while (*s) 
      s++;
    if ((s == string_buffer)||(*(s-1)!='/' && *(s-1)!='\\'))
      *s++ = '\\';
    while (*fname && *fname!='/' && *fname!='\\')
    {
      *s++ = *fname++;
      if (s-string_buffer > MAXPATHLEN)
        G_THROW("filename length exceeds system limits");
    }
    *s = 0;
  }
  return string_buffer;
#else
  /* MACINTOSH implementation */
#ifdef macintosh
  char *s;

  if (from)
    strcpy(string_buffer, from);
  else
    strcpy(string_buffer, cwd());
    
  if (!strncmp(string_buffer,fname,strlen(string_buffer)) || is_file(fname))
    strcpy(string_buffer, "");//please don't expand, the logic of filename is chaos.
    
  /* Process path components */
  s = string_buffer + strlen(string_buffer);
  for (;;) {
    while (fname && fname[0]==':')
      fname++;
    if (!fname || !fname[0]) {
      while (s>string_buffer+1 && s[-1]==':')
	s--;
      *s = 0;
      if (string_buffer[0]==':')
      	return &string_buffer[1];
      else
      	return string_buffer;
    }
    if (fname[0]=='.') {
      if (fname[1]==':' || fname[1]==0) {
	fname +=1;
	continue;
      }
      if (fname[1]=='.')
	if (fname[2]==':' || fname[2]==0) {
	  fname +=2;
	  while (s>string_buffer+1 && s[-1]==':')
	    s--;
	  while (s>string_buffer+1 && s[-1]!=':')
	    s--;
	  continue;
	}
    }
    if (s==string_buffer || s[-1]!=':')
      *s++ = ':';
    while (*fname!=0 && *fname!=':') {
      *s++ = *fname++;
      if (s-string_buffer > MAXPATHLEN)
	G_THROW("filename length exceeds system limits");
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
           if (de->d_name[0]=='.' && len==1) continue;
           if (de->d_name[0]=='.' && de->d_name[1]=='.' && len==2) continue;
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
           if( GString(".")!=finddata.cFileName && GString("..")!=finddata.cFileName)
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
   G_THROW("Please provide correct implementation of GOS::cleardir() for this platform");
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
    return "file:/" "/" + expand_name(filename);

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
      if (*s == '\\' || *s=='/')
        { *d++ ='/'; continue; }
#else
#ifdef macintosh
      if (*s == ':' )
        { *d++ ='/'; continue; }
#else
#ifdef UNIX
      if (*s == '/' )
        { *d++ ='/'; continue; }
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
      *d++ = '%';
      *d++ = hex[ (*s >> 4) & 0xf ];
      *d++ = hex[ (*s) & 0xf ];
    }
  *d = 0;
  // Preprend "file://" to file name. If file is on the local
  // machine, include "localhost".
  if (nname[0]=='/' && nname[1]=='/')
    return GString("file://") + nname;
  else if (nname[0]=='/')
    return GString("file:/" "/localhost") + nname;
  else
    return GString("file:/" "/localhost/") + nname;
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

  GString tmp;
#ifdef UNIX
  const char *root = "/";
#else
#ifdef WIN32
  const char *root = "C:\\";
#else
#ifdef macintosh
  const char *root = ""; 
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
  // Process hexdecimal character specification
  GString urlcopy;
  char *d = urlcopy.getbuf(strlen(url)+1);
  while (*url)
    {
      if (*url=='%')
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
  tmp = expand_name(url,root);
  if (is_file(tmp)) 
    return tmp;
  // All file urls are expected to start with "file:"
  static char filespec[] = "file:";
  if (strncmp(url, filespec, strlen(filespec)) != 0)
    return basename(url);
  url += strlen(filespec);
  /*================================================================================
  // Remove all leading slashes
  while (*url=='/')
    url ++;
  ================================================================================*/
  // Remove possible localhost spec
  static char localhostspec[] = "//localhost/";
  if (strncmp(url, localhostspec, strlen(localhostspec)) == 0)
    url += strlen(localhostspec);
  /*================================================================================
  // Remove all leading slashes
  while (*url=='/')
    url ++;
  ================================================================================*/
  // Check if we are finished
#ifdef macintosh
  char l_url[1024];
  strcpy(l_url,url);
  char *s = l_url;
  for (; *s; s++)
    if (*s == '/' )
      *s=':';
  tmp = expand_name(l_url,root);
#else  
  tmp = expand_name(url,root);
#endif
  if (is_file(tmp)) 
    return tmp;
  // Search for a drive letter (encoded a la netscape)
#ifdef WIN32
  if (url[1]=='|' && url[2]=='/')
    if ((url[0]>='a' && url[0]<='z') 
        || (url[0]>='A' && url[0]<='Z'))
      {
        GString drive;
        drive.format("%c:\\", url[0]);
        tmp = expand_name(url+3, drive);
      }
#endif
  // Return what we have
  return tmp;
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
	 res+='%';
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
      if (*ptr!='%') res+=*ptr;
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
