//C-  -*- C++ -*-
//C-
//C-  Copyright © 1999-2001, LizardTech, Inc. All Rights Reserved.
//C-              Unauthorized use prohibited.
//C-
// 
// $Id: path.cpp,v 1.1 2001-08-08 17:38:05 docbill Exp $
// $Name:  $


#ifdef __GNUC__
#pragma implementation
#endif

#include "debug.h"
#include "path.h"
#include "GException.h"
#include "names.h"

#include <stdio.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#if defined(sun) || defined(__osf__) || defined(hpux)
#include <dirent.h>
#else
#include <sys/dir.h>
#endif

static int CheckLibraryPath(const char * path)
{
   DEBUG_MSG("CheckLibraryPath(): Checking path '" << path << "'\n");
   if (!path || !strlen(path)) return 0;
   
   DIR * dir=opendir(path);
   if (dir)
   {
#if defined(sun) || defined(__osf__) || defined(hpux)
      dirent * de;
#else
      direct * de;
#endif
      while((de=readdir(dir)))
      {
         if (!strcmp(de->d_name, LIBRARY_NAME))
	 {
	    char buffer[MAXPATHLEN+1];
	    sprintf(buffer, "%s/%s", path, de->d_name);
	    int fd=open(buffer, O_RDONLY);
	    closedir(dir);
	    if (fd<0) return 0;
	    close(fd);
	    return 1;
	 }
      }
      closedir(dir);
   }
   return 0;
}

GUTF8String GetLibraryPath(void)
   // Returns directory, which is one level above the directory
   // containing the shared library.
   // Typically it's either ~/.netscape, or /usr/local/lib/netscape
   // or anyother specified by NPX_PLUGIN_PATH
   //
   // NB The returned path should end with a slash
{
   static char *library_path=NULL;
   if (library_path) return library_path;
   
   char * ptr;
   if ((ptr=getenv("NPX_PLUGIN_PATH")))
   {
      char * buffer=new char[strlen(ptr)+1];
      if (!buffer) G_THROW("Not enough memory to get the path to the plugin.");
      strcpy(buffer, ptr);
      char *start = buffer;
      while (*start)
        {
          char *end = start;
          while (*end && *end!=':')
            end += 1;
          char *nstart = end;
          if (*nstart)
            *nstart++ = 0;
          if (CheckLibraryPath(start))
            {
              if (start[strlen(start)-1]=='/') start[strlen(start)-1]=0;
              char * slash;
              for(ptr=slash=start;*ptr;ptr++)
                if (*ptr=='/') slash=ptr;
              slash[1]=0;
              library_path=strdup(start); delete buffer;
              if (!library_path) G_THROW("Not enough memory to get path to the plugin.");
              DEBUG_MSG("GetLibraryPath(): Returning '" << library_path << "'\n");
              return library_path;
            }
          start = nstart;
        }
      delete buffer;
   }
   if ((ptr=getenv("HOME")))
   {
      const char ext[]=".netscape/plugins";
      char * buffer=new char[strlen(ptr)+sizeof(ext)+1];
      if (!buffer) G_THROW("Not enough memory to get the path to the plugin.");
      strcpy(buffer,ptr);
      if (buffer[strlen(buffer)-1]!='/') strcat(buffer, "/");
      strcat(buffer,ext);
      // Be sure that the Lib exists below this dir.
      if (CheckLibraryPath(buffer)) 
      {
         //Since the aux file can't be in the plugins dir, move 1 up
	 char * slash;
	 for(ptr=slash=buffer;*ptr;ptr++)
	    if (*ptr=='/') slash=ptr;
	 slash[1]=0;
	 library_path=strdup(buffer); delete buffer;
	 if (!library_path) G_THROW("Not enough memory to get path to the plugin.");
	 DEBUG_MSG("GetLibraryPath(): Returning '" << library_path << "'\n");
	 return library_path;
      }
      delete buffer;
   }
   static char *check_paths[]={NULL CHECK_PATHS,NULL};
   check_paths[0]=getenv("MOZILLA_HOME");
   for(int i=(check_paths[0])?0:1;(ptr=check_paths[i]);i++)
      if (*ptr)
      {
	 const char ext[]="/plugins";
	 char * buffer=new char[strlen(ptr)+sizeof(ext)];
	 if (!buffer) G_THROW("Not enough memory to get the path to the plugin.");
	 strcpy(buffer,ptr);
	 strcat(buffer,ext);
	 if (CheckLibraryPath(buffer))
	 {
	    library_path=new char[strlen(ptr)+2];
	    if (!library_path) G_THROW("Not enough memory to get path to the plugin.");
	    strcpy(library_path, ptr);
	    if (library_path[strlen(library_path)-1]!='/')
	       strcat(library_path, "/");
	    delete buffer;
	    DEBUG_MSG("GetLibraryPath(): Returning '" << library_path << "'\n");
	    return library_path;
	 }
	 delete buffer;
      }
   library_path=strdup("");
   DEBUG_MSG("GetLibraryPath(): Returning ''\n");
   return GUTF8String(library_path);
}
