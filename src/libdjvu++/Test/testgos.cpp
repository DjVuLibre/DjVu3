//C-  -*- C++ -*-
//C-
//C-  Copyright ｩ 2000-2001, LizardTech, Inc. All Rights Reserved.
//C-              Unauthorized use prohibited.
//C-
//C-

#ifdef __GNUG__
#pragma implementation
#endif
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

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
#ifdef JA_JP	//MBCS
const char * valid_file = "F:\\Projects\\LizardTech\\Test\\日本語\\漢字.txt";
const char * valid_dir = "F:\\Projects\\LizardTech\\Test\\日本語";
const char * invalid_file = "F:\\Projects\\LizardTech\\Test\\日本語\\新漢字.txt";
const char * invalid_dir = "F:\\Projects\\LizardTech\\Test\\新日本語";
const char * clear_dir = "F:\\Projects\\LizardTech\\Test\\日本語dir2";
#else
#ifdef En_US
const char * valid_file = "F:\\Projects\\LizardTech\\Test\\English\\test.txt";
const char * valid_dir = "F:\\Projects\\LizardTech\\Test\\English";
const char * invalid_file = "F:\\Projects\\LizardTech\\Test\\English\\newtest.txt";
const char * invalid_dir = "F:\\Projects\\LizardTech\\Test\\newEnglish";
const char * clear_dir = "F:\\Projects\\LizardTech\\Test\\newEnglish2";
#else		//MBCS
const char * valid_file = "C:\\test.txt";
const char * valid_dir = "C:\\test";
const char * invalid_file = "C:\\My Documents\\xxx.xxx";
const char * invalid_dir = "C:\\xxx";
#endif
#endif
#endif

int
main()
{
   GP<ByteStream> out = ByteStream::create(GURL::Filename::UTF8("out.txt"), "w", true);
   GString msg;
   GString  fullmsg;
   if (GURL::Filename::UTF8(valid_file).is_file())
      msg.format("Valid file, %s is recognized as a file\n",valid_file);
   else
      msg.format("FAILURE:  Valid file, %s is not recognized as a file\n",valid_file);
   fullmsg=msg;

   if (GURL::Filename::UTF8(valid_dir).is_dir())
      msg.format("Valid dir, %s is recognized as a directory\n",valid_dir);
   else
      msg.format("FAILURE:  Valid dir, %s is not recognized as a directory\n",valid_dir);
   fullmsg+=msg;

   if (! GURL::Filename::UTF8(invalid_file).is_file())
      msg.format("Invalid file, %s is recognized as an invalid file\n",invalid_file);
   else
      msg.format("FAILURE:  Invalid file, %s is not recognized as an invalid file\n",invalid_file);
   fullmsg+=msg;

   if (! GURL::Filename::UTF8(invalid_dir).is_dir())
      msg.format("Invalid dir, %s is recognized as an invalid directory\n",invalid_dir);
   else
      msg.format("FAILURE:  Invalid dir, %s is incorrectly recognized as a directory\n",invalid_dir);
   fullmsg+=msg;

   if ( GOS::Filename::UTF8(invalid_dir).mkdir())
      msg.format("Created directory, %s.  Verify it really exists.\n",invalid_dir);
   else
      msg.format("FAILURE:  Could not create directory, %s.\n",invalid_dir);
   fullmsg+=msg;

//MBCS start additional tests
// basename(filename[, suffix])
// -- returns the last component of filename and removes suffix
//    when present. works like /bin/basename.
//GString GOS::basename(const char *fname, const char *suffix)
   GString gsname;
   gsname = GOS::basename (valid_dir);
   msg.format("basename file, %s, gsname = %s.  Verify gsname.\n",valid_dir,(const char*)gsname);
   fullmsg+=msg;
// expand_name(filename[, fromdirname])
// -- returns the full path name of filename interpreted
//    relative to fromdirname.  Use current working dir when
//    fromdirname is null.
//GString GOS::expand_name(const char *fname, const char *from)
   gsname = GURL::expand_name(valid_file, valid_dir);
   msg.format("expand_name file, %s directory, %s, gsname = %s. Verify gsname.\n",valid_file,valid_dir, (const char*)gsname);
   fullmsg+=msg;

   gsname = GURL::expand_name (valid_file);
   msg.format("expand_name file, %s, gsname = %s.  Verify gsname.\n",valid_file,(const char*)gsname);
   fullmsg+=msg;

// filename_to_url --
// -- Returns a url for accessing a given file.
//    If useragent is not provided, standard url will be created,
//    but will not be understood by some versions if IE.
//GString GOS::filename_to_url(const char *filename, const char *useragent)
   GString url;
   url = GURL::Filename::UTF8(valid_dir);
   msg.format("filename_to_url directory, %s, url = %s.  Verify url.\n",valid_dir,(const char *)url);
   fullmsg+=msg;
// url_to_filename --
// -- Applies heuristic rules to convert a url into a valid file name.  
//    Returns a simple basename in case of failure.
//GString GOS::url_to_filename(const char *url)
   gsname = GURL::UTF8(url).UTF8Filename();
   msg.format("url_to_filename directory, %s, gsname = %s.  Verify gsname.\n",(const char *)url,(const char*)gsname);
   fullmsg+=msg;
//GString GOS::encode_reserved(const char * filename)
//   url = GOS::encode_reserved (valid_dir);
//   msg.format("encode_reserved directory, %s, gsname = %s.  Verify url.\n",valid_dir,(const char *)url);
//   strcat(fullmsg, msg);
//GString GOS::decode_reserved(const char * url)
//   gsname = GOS::decode_reserved (url);
//   msg.format("decode_reserved directory, %s, gsname = %s.  Verify gsname.\n",(const char *)url,(const char*)gsname);
//   strcat(fullmsg, msg);

   if ( GURL::Filename::UTF8(valid_file).deletefile())
      msg.format("Deleted file, %s.  Verify it's gone.\n",valid_file);
   else
      msg.format("FAILURE:  Could not delete file, %s\n",valid_file);
   fullmsg+=msg;

   if ( GURL::Filename::UTF8(valid_dir).deletefile ())
      msg.format("Deleted directory, %s.  Verify it's gone.\n",valid_dir);
   else
      msg.format("FAILURE:  Could not delete directory, %s\n",valid_dir);
   fullmsg+=msg;

//int GOS::cleardir(const char * dirname)
   if ( ! GURL::Filename::UTF8(clear_dir).cleardir ())
      msg.format("cleardir directory, %s.  Verify it's gone.\n",clear_dir);
   else
      msg.format("FAILURE:  Could not cleardir directory, %s\n",clear_dir);
   fullmsg+=msg;

//MBCS end

#ifndef UNDER_CE
  DjVuPrintMessage("%s",fullmsg);
#endif
  out->writestring(fullmsg);
  return 0;
}

#ifdef UNDER_CE
int WINAPI WinMain (HINSTANCE hInstance,
		             HINSTANCE hPrevInstance,
                     LPTSTR lpCmdLine,
                     int nCmdShow)
{
   return main();
}
#endif
