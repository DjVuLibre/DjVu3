//C-  -*- C++ -*-
//C-
//C-  Copyright © 1999-2001, LizardTech, Inc. All Rights Reserved.
//C-              Unauthorized use prohibited.
//C-
// 
// $Id: execdir.cpp,v 1.1 2001-05-29 22:05:30 bcr Exp $
// $Name:  $


#ifdef __GNUC__
#pragma implementation
#endif

#include <qfileinfo.h>
#include <qdir.h>

#include "execdir.h"

QString
getExecDir(QString argv0)
{
   QString dir;
   //BUGGY-utf8 doest work
   //operate on utf8 
   //const char *progname=argv0.utf8();
   const char *progname=argv0;

   if (progname && progname[0])
   {
      // find out if the progname exists in the path 
      if (progname[0]!='.' && progname[0]!='/')
      {
	 char * path=getenv("PATH");
	 if (path)
	 {
	    char * start=path;
	    char * end;
	    do
	    {
	       for(end=start;*end && *end!=':';end++);
	       if (end>start)
	       {
		  // +1 for '\0' required by QCString
		  QString pdir=QCString(start, end-start+1);
		  QFileInfo fi=QFileInfo(QDir(pdir), progname);
		  if (fi.isFile())
		     return dir=fi.dirPath();
	       }
	       start=end+1;
	    } while(*end);
	 }
      }

      // it may be given as absolute/relative path 
      QFileInfo fi=QFileInfo(progname);
      if (fi.isFile())
	 dir=fi.dirPath();
      
   }
   return dir;
}

// end execdir.cpp
