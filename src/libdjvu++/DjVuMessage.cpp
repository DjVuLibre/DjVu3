//C-  -*- C++ -*-
//C- DjVu® Reference Library (v. 3.0)
//C- 
//C- Copyright © 2000-2001 LizardTech, Inc. All Rights Reserved.
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
// $Id: DjVuMessage.cpp,v 1.16 2001-03-09 00:06:33 bcr Exp $
// $Name:  $



#include "DjVuMessage.h"
#include "GOS.h"
#ifndef macintosh
#include "parseoptions.h"
#endif
#include <ctype.h>
#include <stdio.h>
#ifdef WIN32
#include <tchar.h>
#include <atlbase.h>
#include <windows.h>
#include <winreg.h>
#endif
#ifdef UNIX
#include <unistd.h>
#include <pwd.h>
#include <sys/types.h>
#endif

#ifdef WIN32
static const char LocalDjVuDir[] ="Profiles"; // appended to the home directory.
static const char RootDjVuDir[] ="C:/Program Files/LizardTech/Profiles";
#else
static const char LocalDjVuDir[] =".DjVu"; // appended to the home directory.
static const char RootDjVuDir[] ="/etc/DjVu/";
static const char registerypath[]="Software\\LizardTech\\DjVu\\Profile Path";
#endif
static const char DjVuEnv[]="DJVU_CONFIG_DIR";


//  There is only object of class DjVuMessage in a program, and here it is:
//DjVuMessage  DjVuMsg;
const DjVuMessage &
DjVuMessage::get_DjVuMessage(void)
{
  static const DjVuMessage m;
  return m;
}

//  The name of the message file
static const char DjVuMessageFileName[] = "message";


// Constructor
DjVuMessage::DjVuMessage( void ) : opts(0)
{
#ifndef macintosh
   opts=new DjVuParseOptions(DjVuMessageFileName);
#endif
}

// Destructor
DjVuMessage::~DjVuMessage( )
{
}


void
DjVuMessage::perror( const GString & MessageList ) const
{
  GString mesg=LookUp(MessageList);
  fputs((const char *)mesg,stderr);
}


//  Expands message lists by looking up the message IDs and inserting
//  arguments into the retrieved messages.
//  N.B. The resulting string may be encoded in UTF-8 format (ISO 10646-1 Annex R)
//       and SHOULD NOT BE ASSUMED TO BE ASCII.
GString
DjVuMessage::LookUp( const GString & MessageList ) const
{
  GString result;                           // Result string; begins empty

  int start = 0;                            // Beginning of next message
  int end = MessageList.length();           // End of the message string

  //  Isolate single messages and process them
  while( start < end )
  {
    if( MessageList[start] == '\n' )
    {
      result += MessageList[start++];       // move the newline to the result
                                            // and advance to the next message
    }
    else
    {
      //  Find the end of the next message and process it
      int next_ending = MessageList.search('\n', start);
      if( next_ending < 0 ) next_ending = end;
      result += LookUpSingle( MessageList.substr(start, next_ending-start) );
      //  Advance to the next message
      start = next_ending;
    }
  }

  //  All done 
  return result;
}


//  Expands a single message and inserts the arguments. Single_Message contains no
//  separators (newlines), but includes all the parameters.
GString
DjVuMessage::LookUpSingle( const GString &Single_Message ) const
{
  //  Isolate the message ID and get the corresponding message text
  int ending_posn = Single_Message.search('\t');
  if( ending_posn < 0 )
    ending_posn = Single_Message.length();
  GString msg_text = LookUpID( Single_Message.substr(0,ending_posn) );

  //  Check whether we found anything
  if( !msg_text )
  {
    //  Didn't find anything, fabricate a message
#ifndef macintosh
    msg_text = GString("** Unrecognized DjVu Message: [Contact LizardTech for assistance]\n") + 
               "\tMessage name:  " +
               Single_Message.substr(0,ending_posn);
#else
    msg_text = GString("** error messages not implemented for Macintosh: [Contact LizardTech for assistance]\n") + 
               "\tMessage name:  " +
               Single_Message.substr(0,ending_posn);
#endif


  }
#ifdef _DEBUG
  else
  {
    msg_text = "*!* " + msg_text + " *!*";    // temporary debug
  }
#endif
    
  //  Insert the parameters (if any)
  unsigned int param_num = 0;
  while( unsigned(ending_posn) < Single_Message.length() )
  {
    int start_posn = ending_posn+1;
    ending_posn = Single_Message.search('\t',start_posn);
    if( ending_posn < 0 )
      ending_posn = Single_Message.length();
    InsertArg( msg_text,
               ++param_num,
               Single_Message.substr(start_posn, ending_posn-start_posn) );
  }

  return msg_text;
}


//  Looks up the msgID in the file of messages and returns a pointer to the beginning
//  of the translated message, if found; and an empty string otherwise.
GString
DjVuMessage::LookUpID( const GString &msgID ) const
{
	GString result;

#ifndef macintosh
  
  result=opts->GetValue(msgID);
  opts->perror();
#endif

/*
  //  Find the message file
  const char *ss;
  struct djvu_parse opt = djvu_parse_init( "-" );
  ss = djvu_parse_configfile( opt, DjVuMessageFileName, -1 );
  FILE *MessageFile = fopen( ss, "r" );
  GString result;
  if( MessageFile != NULL )
  {
    enum {BUFSIZE=500};
    char buffer[BUFSIZE];             // holds the message file lines
    char *status;                     // holds success or failure of the fgets call
    while( (status = fgets(buffer, BUFSIZE, MessageFile)) != NULL )
    {
      if( strncmp( msgID, buffer, msgID.length() ) == 0 &&
          buffer[msgID.length()] == '\t' )
        break;                              // stop when the correct string is found
    }

    if( status != NULL )
    {
      int text_start = msgID.length()+1;
      while( buffer[text_start] == '\t' ) 
        text_start++;                                 // allow for multiple tabs
      result = &buffer[text_start];                   // extract the message text
      result = result.substr(0, result.length()-1);   // remove trailing newline
    }
    else
      result.empty();                 // end of file (or error), so we return an
                                      // empty string to indicate failure

    fclose( MessageFile );
  }
  else
    result.empty();  // Message text file not open
*/
  return result;
}


//  Insert a string into the message text. Will insert into any field description.
//  If the ArgId is not found, adds a line with the parameter so information will
//  not be lost.
void
DjVuMessage::InsertArg( GString &message, int ArgId, GString arg ) const
{
  GString target = GString("%#") + GString(ArgId) + "#";           // argument target string
  int format_start = message.search( target );            // location of target string
  if( format_start >= 0 )
  {
    int format_end = format_start;
    while( !isalpha( message[format_end++] ) ) {}  // locate end of format
    //GString format = "%" + message.substr( format_start + target.length(),
    //                                       format_end - format_start - target.length() );
    message = message.substr( 0, format_start ) +
              arg +
              message.substr( format_end, message.length() - format_end );
  }
  else
  {
    //  Not found, fake it
    message += GString("\n\tParameter ") + GString(ArgId) + ":  " + arg;
  }
}

#if 0
#ifdef WIN32
static GString
RegOpenReadConfig ( HKEY hParentKey )
{
  GString retval;
   // To do:  This needs to be shared with SetProfile.cpp
  LPCTSTR path = TEXT(registerypath) ;

  HKEY hKey = 0;
  // MultiByteToWideChar(CP_ACP,MB_PRECOMPOSED,argv[1],strlen(argv[1])+1,wszSrcFile,sizeof(wszSrcFile));
  if (RegOpenKeyEx(hParentKey, path, 0,
              KEY_READ, &hKey) == ERROR_SUCCESS )
  {
    TCHAR path[1024];
    // Success
    LPCTSTR szPathValue = path;
    LPCTSTR lpszEntry = TEXT("");
    DWORD dwCount = (sizeof(path)/sizeof(TCHAR))-1;
    DWORD dwType;

//    LONG lResult = RegQueryValueEx(hKey, lpszEntry, NULL,
//             &dwType, (LPBYTE) szPathValue, &dwCount);
    LONG lResult = RegQueryValueEx(hKey, lpszEntry, NULL,
             &dwType, szPathValue, &dwCount);

    RegCloseKey(hKey);

    if ((lResult == ERROR_SUCCESS))
    {
      szPathValue[dwCount] = 0;
      USES_CONVERSION;
      strcpy(retval.getbuf(_tcslen(path)),T2CA(path));
    }
  } 
//  if (hKey)  RegCloseKey(hKey); 
  return retval;
}

static GString
GetModualPath( void )
{
  TCHAR path[1024];
  DWORD dwCount = (sizeof(path)/sizeof(TCHAR))-1;
  GetModuleFileName(0, path, dwCount);
  GString retval;
  USES_CONVERSION;
  strcpy(retval.getbuf(_tcslen(path)),T2CA(path));
  return GOS::expand_name(LocalDjVuDir,GOS::dirname(retval));
}
#endif

GList<GString> &
GetProfilePaths(void)
{
  static bool first=true;
  static GList<GString> paths;
  if(first)
  {
    first=false;
    const char *envp=getenv(DjVuEnv);
    if(envp)
      paths.append(envp);
#ifdef WIN32
    GString path(GetModulePath());
    if(path.length() && GOS::is_dir(path))
      paths.append(path);
    path=RegOpenReadConfig (HKEY_CURRENT_USER);
    if(path.length() && GOS::is_dir(path))
      paths.append(path);
    path=(RegOpenReadConfig (HKEY_LOCAL_MACHINE));
    if(path.length() && GOS::is_dir(path))
      paths.append(path);
#else
    GString path;
    const char* home=getenv("HOME");
    struct passwd *pw=0;
    if(!home)
    {
      pw=getpwuid(getuid());
      if(pw)
        home=pw->pw_dir;
    }
    if(home)
    {
      path=GOS::expand_name(LocalDjVuDir,home);
      if(path.length() && GOS::is_dir(path))
        paths.append(path);
    }
    if(pw)
    {
      free(pw);
    }
#endif
    if(GOS::is_dir(RootDjVuDir))
      paths.append(RootDjVuDir);
  }
  return paths;
}
#endif
