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
// $Id: DjVuMessage.cpp,v 1.36 2001-04-21 00:16:58 bcr Exp $
// $Name:  $



#include "DjVuMessage.h"
#include "GOS.h"
#include "XMLTags.h"
#include "ByteStream.h"
#include "GURL.h"
#include "debug.h"
#include <ctype.h>
#include <stdlib.h>
// #include <stdio.h>
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
#include <locale.h>

static const char *unrecognized=ERR_MSG("DjVuMessage.Unrecognized");
static const char *uparameter=ERR_MSG("DjVuMessage.Parameter");
static const char unrecognized_default[] =
  "** Unrecognized DjVu Message: [Contact LizardTech for assistance]\n"
  "\t** Message name:  %1!s!";
static const char uparameter_default[]="Parameter: %1!s!";

#ifndef NO_DEBUG
#if defined(UNIX)
  // appended to the home directory.
static const char ModuleDjVuDir[] ="profiles";
  // appended to the home directory.
static const char DebugModuleDjVuDir[] ="../TOPDIR/SRCDIR/profiles";
#elif defined(WIN32)
  // appended to the home directory.
static const char DebugModuleDjVuDir[] ="../../profiles";
#endif
#endif

#ifdef WIN32
  // appended to the home directory.
static const char ModuleDjVuDir[] ="Profiles";
static const char RootDjVuDir[] ="C:/Program Files/LizardTech/Profiles";
static const TCHAR registrypath[]= TEXT("Software\\LizardTech\\DjVu\\Profile Path");
#else
// appended to the home directory.
static const char LocalDjVuDir[] =".DjVu";
static const char RootDjVuDir[] ="/etc/DjVu/";
#endif
static const char DjVuEnv[]="DJVU_CONFIG_DIR";
//  The name of the message file
static const char DjVuMessageFileName[] = "message";
static const char MessageFile[]="messages.xml";
static const char namestring[]="name";
static const char valuestring[]="value";
static const char numberstring[]="number";
static const char bodystring[]="BODY";
static const char headstring[]="HEAD";
static const char includestring[]="INCLUDE";
static const char messagestring[]="MESSAGE";

#ifdef WIN32
static GURL
RegOpenReadConfig ( HKEY hParentKey )
{
  GURL retval;
   // To do:  This needs to be shared with SetProfile.cpp
  LPCTSTR path = registrypath;

  HKEY hKey = 0;
  // MultiByteToWideChar(CP_ACP,MB_PRECOMPOSED,argv[1],strlen(argv[1])+1,wszSrcFile,sizeof(wszSrcFile));
  if (RegOpenKeyEx(hParentKey, path, 0,
              KEY_READ, &hKey) == ERROR_SUCCESS )
  {
    TCHAR path[1024];
    // Success
    LPSTR szPathValue = path;
    LPCTSTR lpszEntry = (LPCTSTR &)TEXT("");
    DWORD dwCount = (sizeof(path)/sizeof(TCHAR))-1;
    DWORD dwType;

    LONG lResult = RegQueryValueEx(hKey, lpszEntry, NULL,
             &dwType, (LPBYTE) szPathValue, &dwCount);

    RegCloseKey(hKey);

    if ((lResult == ERROR_SUCCESS))
    {
      szPathValue[dwCount] = 0;
      USES_CONVERSION;
      retval=GURL::Filename::Native(T2CA(path));
    }
  } 
//  if (hKey)  RegCloseKey(hKey); 
  return retval;
}

static GURL
GetModulePath( void )
{
  TCHAR path[1024];
  DWORD dwCount = (sizeof(path)/sizeof(TCHAR))-1;
  GetModuleFileName(0, path, dwCount);
  USES_CONVERSION;
  return GURL::Filename::Native(T2CA(path)).base();
}
#else
#ifdef UNIX
extern char **environ;
static char **e=environ-1;
static GURL
GetModulePath( void )
{
  GURL retval;
  char **argv=e;
  int argc;
  for(argc=0;*(int *)&(argv[-1]) != argc;argc++,argv--)
    EMPTY_LOOP;
  if(argc>=1)
    retval=GURL::Filename::Native(argv[0]).base();
  return retval;
}
#endif
#endif

static GList<GURL>
GetProfilePaths(void)
{
  static bool first=true;
  static GList<GURL> realpaths;
  if(first)
  {
    first=false;
    GList<GURL> paths;
    GURL path;
#ifndef WINCE
    const GUTF8String envp(GOS::getenv(DjVuEnv));
    if(envp.length())
      paths.append((path=GURL::Filename::UTF8(envp)));
#endif
#if defined(WIN32) || (defined(UNIX) && !defined(NO_DEBUG))
    GURL mpath(GetModulePath());
    if(!mpath.is_empty() && mpath.is_dir())
    {
#ifndef NO_DEBUG
      path=GURL::UTF8(DebugModuleDjVuDir,mpath);
      if(!path.is_empty() && path.is_dir())
        paths.append(path);
#endif
      path=GURL::UTF8(ModuleDjVuDir,mpath);
      if(!path.is_empty() && path.is_dir())
        paths.append(path);
      path=mpath.base();
      if(path.pathname() != mpath.pathname())
      {
        path=GURL::UTF8(ModuleDjVuDir,path);
        if(!path.is_empty() && path.is_dir())
          paths.append(path);
      }
    }
#endif
#ifdef WIN32
    path=RegOpenReadConfig (HKEY_CURRENT_USER);
    if(!path.is_empty() && path.is_dir())
      paths.append(path);
    path=(RegOpenReadConfig (HKEY_LOCAL_MACHINE));
    if(!path.is_empty() && path.is_dir())
      paths.append(path);
#else
    GUTF8String home=GOS::getenv("HOME");
    struct passwd *pw=0;
    if(home.length())
    {
      pw=getpwuid(getuid());
      if(pw)
        home=GNativeString(pw->pw_dir);
    }
    if(home.length())
    {
      path=GURL::UTF8(LocalDjVuDir,GURL::Filename::UTF8(home));
      if(!path.is_empty() && path.is_dir())
        paths.append(path);
    }
#endif
    path=GURL::Filename::UTF8(RootDjVuDir);
    if(!path.is_empty() && path.is_dir())
      paths.append(path);
    GUTF8String oldlocale(setlocale(LC_CTYPE,0));
    GUTF8String defaultlocale((oldlocale.search('_') < 0)
      ?setlocale(LC_CTYPE,"")
      :(const char *)oldlocale);
    if(oldlocale != defaultlocale)
    {
      setlocale(LC_CTYPE,(const char *)oldlocale);
    }
    GPosition pos;
    for(pos=paths;pos;++pos)
    {
      path=GURL::UTF8(defaultlocale,paths[pos]);
      if(path.is_dir())
      {
        realpaths.append(path);
      }
    }
    const int underscore=defaultlocale.search('_');
    if(underscore > 0)
    {
      defaultlocale=defaultlocale.substr(0,underscore);
      for(pos=paths;pos;++pos)
      {
        path=GURL::UTF8(defaultlocale,paths[pos]);
        if(path.is_dir())
        {
          realpaths.append(path);
        }
      }
    }
    for(pos=paths;pos;++pos)
    {
      realpaths.append(paths[pos]);
    }
  }
  return realpaths;
}

static void
getbodies(
  GList<GURL> &paths,
  const GUTF8String &MessageFileName,
  GPList<lt_XMLTags> &body, 
  GMap<GUTF8String, void *> & map )
{
  bool isdone=false;
  for(GPosition pos=paths;!isdone && pos;++pos)
  {
    const GURL::UTF8 url(MessageFileName,paths[pos]);
    if(url.is_file())
    {
      map[MessageFileName]=0;
      GP<lt_XMLTags> gtags=lt_XMLTags::create();
      lt_XMLTags &tags=*gtags;
      {
        GP<ByteStream> bs=ByteStream::create(url,"rb");
        tags.init(bs);
      }
      GPList<lt_XMLTags> Bodies=tags.getTags(bodystring);
      if(! Bodies.isempty())
      {
        isdone=true;
        for(GPosition pos=Bodies;pos;++pos)
        {
          body.append(Bodies[pos]);
        }
      }
      GPList<lt_XMLTags> Head=tags.getTags(headstring);
      if(! Head.isempty())
      {
        isdone=true;
        GMap<GUTF8String, GP<lt_XMLTags> > includes;
        lt_XMLTags::getMaps(includestring,namestring,Head,includes);
        for(GPosition pos=includes;pos;++pos)
        {
          GUTF8String file=includes.key(pos);
          if(! map.contains(file))
          {
            getbodies(paths,file,body,map);
          }
        }
      }
    }
  }
}

static void
parse(GMap<GUTF8String,GP<lt_XMLTags> > &retval)
{
  GPList<lt_XMLTags> body;
  {
    GList<GURL> paths=GetProfilePaths();
    GMap<GUTF8String, void *> map;
    GUTF8String m(MessageFile);
    getbodies(paths,m,body,map);
  }
  if(! body.isempty())
  {
    lt_XMLTags::getMaps(messagestring,namestring,body,retval);
  }
}


#if 0
static void
parse (GMap<GUTF8String,GP<lt_XMLTags> > &retval)
{
  GList<GUTF8String> &paths=GetProfilePaths();
  for(GPosition pos=paths;pos;++pos)
  {
    const GURL url=GOS::filename_to_url(GOS::expand_name(MessageFileName,paths[pos]));
    if(GOS::is_file(GOS::url_to_filename(url)))
    {
      parse(retval,ByteStream::create(url,"rb"));
      if(retval.isempty())
      {
        break;
      }
    }
  }
}
#endif


//  There is only object of class DjVuMessage in a program, and here it is:
//DjVuMessage  DjVuMsg;
const DjVuMessage &
DjVuMessage::create(void)
{
  static const DjVuMessage m;
  return m;
}



// Constructor
DjVuMessage::DjVuMessage( void )
#if 0
: opts(0)
#endif
{
  parse(Map);
}

// Destructor
DjVuMessage::~DjVuMessage( )
{
}


void
DjVuMessage::perror( const GUTF8String & MessageList ) const
{
  GUTF8String mesg=LookUp(MessageList);
  DjVuPrintError("%s",(const char *)mesg);
}


//  Expands message lists by looking up the message IDs and inserting
//  arguments into the retrieved messages.
//  N.B. The resulting string may be encoded in UTF-8 format (ISO 10646-1 Annex R)
//       and SHOULD NOT BE ASSUMED TO BE ASCII.
GUTF8String
DjVuMessage::LookUp( const GUTF8String & MessageList ) const
{
  GUTF8String result;                           // Result string; begins empty

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
//  separators (newlines), but includes all the parameters separated by tabs.
GUTF8String
DjVuMessage::LookUpSingle( const GUTF8String &Single_Message ) const
{
  //  Isolate the message ID and get the corresponding message text
  int ending_posn = Single_Message.search('\t');
  if( ending_posn < 0 )
    ending_posn = Single_Message.length();
  GUTF8String msg_text;
  GUTF8String msg_number;
  const GUTF8String message=Single_Message.substr(0,ending_posn);
  LookUpID( message, msg_text, msg_number );

  //  Check whether we found anything
  if( !msg_text.length())
  {
    if(message == unrecognized)
    {
      //  Didn't find anything, fabricate a message
      msg_text = unrecognized_default;
    }else if(message == uparameter)
    {
      msg_text = uparameter_default;
    }else
    {
      return LookUpSingle(GUTF8String(unrecognized)+"\t"+message);
    }
  }
#ifndef NO_DEBUG
  else
  {
    msg_text = "*!* " + msg_text + " *!*";    // temporary debug to show the message was translated
  }
#endif
    
  //  Insert the parameters (if any)
  unsigned int param_num = 0;
  while( (unsigned int)ending_posn < Single_Message.length() )
  {
    const int start_posn = ending_posn+1;
    ending_posn = Single_Message.search('\t',start_posn);
    if( ending_posn < 0 )
      ending_posn = Single_Message.length();
    InsertArg( msg_text, ++param_num,
               Single_Message.substr(start_posn, ending_posn-start_posn) );
  }
  //  Insert the message number
  InsertArg( msg_text, 0, msg_number );

  return msg_text;
}


// Looks up the msgID in the file of messages and returns a pointer to
// the beginning of the translated message, if found; and an empty string
// otherwise.
void
DjVuMessage::LookUpID( const GUTF8String &msgID,
                       GUTF8String &message_text,
                       GUTF8String &message_number ) const
{
  if(!Map.isempty())
  {
    GPosition pos=Map.contains(msgID);
    if(pos)
    {
      GP<lt_XMLTags> tag=Map[pos];
      GPosition valuepos=tag->args.contains(valuestring);
      if(valuepos)
      {
        message_text=tag->args[valuepos];
      }
      GPosition numberpos=tag->args.contains(numberstring);
      if(numberpos)
      {
        message_number=tag->args[numberpos];
      }
    }
  }
#if 0
#ifndef macintosh
  else if(opts)
  {  
    result=opts->GetValue(msgID);
    opts->perror();
  }
#endif
#endif

#if 0
  //  Find the message file
  const char *ss;
  struct djvu_parse opt = djvu_parse_init( "-" );
  ss = djvu_parse_configfile( opt, DjVuMessageFileName, -1 );
  FILE *MessageFile = fopen( ss, "r" );
  GUTF8String result;
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
#endif
}


// Insert a string into the message text. Will insert into any field
// description.  Except for an ArgId of zero (message number), if the ArgId
// is not found, the routine adds a line with the parameter so information
// will not be lost.
void
DjVuMessage::InsertArg( GUTF8String &message,
  const int ArgId, const GUTF8String &arg ) const
{
    // argument target string
  const GUTF8String target= "%"+GUTF8String(ArgId)+"!";
    // location of target string
  int format_start = message.search( target );
  if( format_start >= 0 )
  {
    do
    {
      const int n=format_start+target.length()+1;
      const int format_end=message.search('!',n);
      if(format_end > format_start)
      { 
        const int len=1+format_end-n;
        if(len && isascii(message[n-1]))
        {
          GUTF8String narg;
          GUTF8String format="%"+message.substr(n-1,len);
          switch(format[len])
          {
            case 'd':
            case 'i':
              narg.format((const char *)format,arg.toInt());
              break;
            case 'u':
            case 'o':
            case 'x':
            case 'X':
              narg.format((const char *)format,(unsigned int)arg.toInt());
              break;
            default:
              narg.format((const char *)format,(const char *)arg);
              break;
          }
          message = message.substr( 0, format_start )+narg
            +message.substr( format_end+1, (unsigned int)(-1));
        }else
        {
          message = message.substr( 0, format_start )+arg
            +message.substr( format_end+1, (unsigned int)(-1));
        }
      }
      format_start=message.search(target,format_start+arg.length());
    } while(format_start >= 0);
  }
  else
  {
    //  Not found, fake it
    if( ArgId != 0 )
    {
      message += "\n"+LookUpSingle(uparameter+("\t"+arg));
    }
  }
}


//  A C function to perform a message lookup. Arguments are a buffer to received the
//  translated message, a buffer size (bytes), and a message_list. The translated
//  result is returned in msg_buffer encoded in UTF-8. In case of error, msg_buffer is
//  empty (i.e., msg_buffer[0] == '\0').
void DjVuMessage_LookUp( char *msg_buffer, const unsigned int buffer_size, const char *message )
{
  GUTF8String converted = DjVuMessage::LookUpUTF8( message );
  if( converted.length() >= buffer_size )
    msg_buffer[0] = '\0';
  else
    strcpy( msg_buffer, converted );
}

const DjVuMessage &
DjVuMessage::create(const GP<ByteStream> &bs)
{
  DjVuMessage &m=const_cast<DjVuMessage &>(create());
  m.AddByteStream(bs);
  return m;
}

void
DjVuMessage::AddByteStream(GP<ByteStream> bs)
{
  GP<lt_XMLTags> gtags=lt_XMLTags::create();
  lt_XMLTags &tags=*gtags;
  tags.init(bs);
  GPList<lt_XMLTags> Bodies=tags.getTags(bodystring);
  if(! Bodies.isempty())
  {
    lt_XMLTags::getMaps(messagestring,namestring,Bodies,Map);
  }
}

