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
//C- $Id: testgos.cpp,v 1.2 2000-09-18 17:10:34 bcr Exp $

#include "..\GOS.h"
#ifdef WIN32
#include <tchar.h>
#endif
#ifdef UNDER_CE
#include <windows.h>
#endif
#include <stdio.h>

#ifdef UNDER_CE
const char * valid_file = "\\My Documents\\input.djvu";
const char * valid_dir = "\\temp";
const char * invalid_file = "\\My Documents\\xxx.xxx";
const char * invalid_dir = "\\xxx";
#else    // other Win32
const char * valid_file = "C:\\test.txt";
const char * valid_dir = "C:\\test";
const char * invalid_file = "C:\\My Documents\\xxx.xxx";
const char * invalid_dir = "C:\\xxx";
#endif

int
main()
{
   char msg[256];
   char  fullmsg[1024];
   if (GOS::is_file(valid_file))
      sprintf (msg,"Valid file, %s is recognized as a file\n",valid_file);
   else
      sprintf (msg,"FAILURE:  Valid file, %s is not recognized as a file\n",valid_file);
   strcpy(fullmsg, msg);

   if (GOS::is_dir(valid_dir))
      sprintf (msg,"Valid dir, %s is recognized as a directory\n",valid_dir);
   else
      sprintf (msg,"FAILURE:  Valid dir, %s is not recognized as a directory\n",valid_dir);
   strcat(fullmsg, msg);

   if (! GOS::is_file(invalid_file))
      sprintf (msg,"Invalid file, %s is recognized as an invalid file\n",invalid_file);
   else
      sprintf (msg,"FAILURE:  Invalid file, %s is not recognized as an invalid file\n",invalid_file);
   strcat(fullmsg, msg);

   if (! GOS::is_dir(invalid_dir))
      sprintf (msg,"Invalid dir, %s is recognized as an invalid directory\n",invalid_dir);
   else
      sprintf (msg,"FAILURE:  Invalid dir, %s is incorrectly recognized as a directory\n",invalid_dir);
   strcat(fullmsg, msg);

   if ( GOS::mkdir (invalid_dir))
      sprintf (msg,"Created directory, %s.  Verify it really exists.\n",invalid_dir);
   else
      sprintf (msg,"FAILURE:  Could not create directory, %s.\n",invalid_dir);
   strcat(fullmsg, msg);

   if ( GOS::deletefile (valid_dir))
      sprintf (msg,"Deleted directory, %s.  Verify it's gone.\n",valid_dir);
   else
      sprintf (msg,"FAILURE:  Could not delete directory, %s\n",valid_dir);
   strcat(fullmsg, msg);

   if ( GOS::deletefile (valid_file))
      sprintf (msg,"Deleted file, %s.  Verify it's gone.\n",valid_file);
   else
      sprintf (msg,"FAILURE:  Could not delete file, %s\n",valid_file);
   strcat(fullmsg, msg);

#ifdef UNDER_CE
   WCHAR temp[1024];
   wsprintf(temp,L"%S",fullmsg);
   //MessageBox (NULL,temp, L"TestGOS",MB_OK);
   OutputDebugStringW(temp) ;
#else
   printf("%s",fullmsg);
#endif
  return 0;
}

#ifdef UNDER_CE
int WINAPI WinMain (HINSTANCE hInstance,
		             HINSTANCE hPrevInstance,
                     LPTSTR lpCmdLine,
                     int nCmdShow)
{
   main();

   return 0;
}
#endif