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
// $Id: DjVuMessage.cpp,v 1.48 2001-05-04 22:55:13 fcrary Exp $
// $Name:  $

#ifdef __GNUC__
#pragma implementation
#endif

#include "DjVuMessage.h"
#include "GOS.h"
#include "XMLTags.h"
#include "ByteStream.h"
#include "GURL.h"
#include "debug.h"
#include <ctype.h>
#include <string.h>
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
static const char uparameter_default[]="\t   Parameter: %1!s!";
static const char *failed_to_parse_XML=ERR_MSG("DjVuMessage.failed_to_parse_XML");
static const char failed_to_parse_XML_default[]=
  "Failed to parse XML message file:&#10;&#09;&apos;%1!s!&apos;.";

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

static const char *lookup_id(const char *language,const int start=0);

static GPList<ByteStream> &
getByteStream(void)
{
  static GPList<ByteStream> gbs;
  return gbs;
}

static GP<DjVuMessage> &
getDjVuMessage(void)
{
  static GP<DjVuMessage> message;
  return message;
}

void
DjVuMessage::AddByteStreamLater(const GP<ByteStream> &bs)
{
  getByteStream().append(bs);
}


#if defined(WIN32)
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
#elif defined(UNIX) && !defined(NO_DEBUG)
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
#ifndef UNDER_CE
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

    const GUTF8String oldlocale(setlocale(LC_CTYPE,0));
    const GUTF8String defaultlocale((oldlocale.search((unsigned long)'_') < 0)
      ?setlocale(LC_CTYPE,""):(const char *)oldlocale);
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
    int underscore=defaultlocale.search((unsigned long)'_');
    if(underscore<0)
      underscore=defaultlocale.length();
    const GUTF8String lookuplocale(lookup_id(defaultlocale.substr(0,underscore)));
    if(lookuplocale != defaultlocale)
    {
      for(pos=paths;pos;++pos)
      {
        path=GURL::UTF8(lookuplocale,paths[pos]);
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

static GUTF8String
getbodies(
  GList<GURL> &paths,
  const GUTF8String &MessageFileName,
  GPList<lt_XMLTags> &body, 
  GMap<GUTF8String, void *> & map )
{
  GUTF8String errors;
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
        G_TRY
        {
          tags.init(bs);
        }
        G_CATCH(ex)
        {
          GUTF8String mesg(failed_to_parse_XML+("\t"+url.get_string()));
          if(errors.length())
          {
            errors+="\n"+mesg;
          }else
          {
            errors=mesg;
          }
          errors+="\n"+GUTF8String(ex.get_cause());
        }
        G_ENDCATCH;
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
          const GUTF8String file=includes.key(pos);
          if(! map.contains(file))
          {
            const GUTF8String err2(getbodies(paths,file,body,map));
            if(err2.length())
            {
              if(errors.length())
              {
                errors+="\n"+err2;
              }else
              {
                errors=err2;
              }
            }
          }
        }
      }
    }
  }
  return errors;
}

static GUTF8String
parse(GMap<GUTF8String,GP<lt_XMLTags> > &retval)
{
  GUTF8String errors;
  GPList<lt_XMLTags> body;
  {
    GList<GURL> paths=GetProfilePaths();
    GMap<GUTF8String, void *> map;
    GUTF8String m(MessageFile);
    errors=getbodies(paths,m,body,map);
  }
  if(! body.isempty())
  {
    lt_XMLTags::getMaps(messagestring,namestring,body,retval);
  }
  return errors;
}


//  There is only object of class DjVuMessage in a program, and here it is:
//DjVuMessage  DjVuMsg;
const DjVuMessage &
DjVuMessage::create(void)
{
  GP<DjVuMessage> &message=getDjVuMessage();
  if(!message)
  {
    message=new DjVuMessage;
  }
  DjVuMessage &m=*message;
  GPList<ByteStream> &bs = getByteStream();
  for(GPosition pos;(pos=bs);bs.del(pos))
  {
    m.AddByteStream(bs[pos]);
  }
  return m;
}


// Constructor
DjVuMessage::DjVuMessage( void )
{
  errors=parse(Map);
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
  GUTF8String result;                       // Result string; begins empty
  if(errors.length())
  {
    const GUTF8String err1(errors);
    (const_cast<GUTF8String &>(errors)).empty();
    result=LookUp(err1)+"\n";
  }

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
      int next_ending = MessageList.search((unsigned long)'\n', start);
      if( next_ending < 0 )
        next_ending = end;
      result += LookUpSingle( MessageList.substr(start, next_ending-start) );
      //  Advance to the next message
      start = next_ending;
    }
  }

  //  All done 
  return result;
}


// Expands a single message and inserts the arguments. Single_Message
// contains no separators (newlines), but includes all the parameters
// separated by tabs.
GUTF8String
DjVuMessage::LookUpSingle( const GUTF8String &Single_Message ) const
{
  //  Isolate the message ID and get the corresponding message text
  int ending_posn = Single_Message.contains("\t\v");
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
    }else if(message == failed_to_parse_XML)
    {
      msg_text = failed_to_parse_XML_default;
    }else
    {
      return LookUpSingle(unrecognized+("\t"+Single_Message));
    }
  }
#ifndef NO_DEBUG
  else
  {
    msg_text = "{!" + msg_text + "!}";    // temporary debug to show the message was translated
  }
#endif
    
  //  Insert the parameters (if any)
  unsigned int param_num = 0;
  while( (unsigned int)ending_posn < Single_Message.length() )
  {
    GUTF8String arg;
    const int start_posn = ending_posn+1;
    if(Single_Message[ending_posn] == '\v')
    {
      ending_posn=Single_Message.length();
      arg=LookUpSingle(Single_Message.substr(start_posn,ending_posn));
    }else
    {
      ending_posn = Single_Message.contains("\v\t",start_posn);
      if( ending_posn < 0 )
        ending_posn = Single_Message.length();
      arg=Single_Message.substr(start_posn, ending_posn-start_posn);
    }
    InsertArg( msg_text, ++param_num, arg);
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
      }else
      {
        const int start_line=tag->raw.search((unsigned long)'\n',0);
      
        const int start_text=tag->raw.nextNonSpace(0);
        const int end_text=tag->raw.firstEndSpace(0);
        if(start_line<0 || start_text<0 || start_text < start_line)
        {
          message_text=tag->raw.substr(0,end_text).fromEscaped();
        }else
        {
          message_text=tag->raw.substr(start_line+1,end_text-start_line-1).fromEscaped();
        }
      }
      GPosition numberpos=tag->args.contains(numberstring);
      if(numberpos)
      {
        message_number=tag->args[numberpos];
      }
    }
  }
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
      const int format_end=message.search((unsigned long)'!',n);
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
            case 'f':
              {
                GUTF8String end;
                bool success;
                narg.format((const char *)format, arg.toDouble(end,success));
                if( !success ) narg = arg;
              }
              break;
            default:
              narg.format((const char *)format,(const char *)arg);
              break;
          }
          message = message.substr( 0, format_start )+narg
            +message.substr( format_end+1, -1 );
        }else
        {
          message = message.substr( 0, format_start )+arg
            +message.substr( format_end+1, -1 );
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

void
DjVuMessage::AddByteStream(const GP<ByteStream> &bs)
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


static const char *
lookup_id(const char *xlanguage, const int start)
{
  struct iso639
  {
    char const * const name;
    const unsigned long id;
    char const * const language;
    iso639(const char xname[],const unsigned long xid,const char xlanguage[])
    : name(xname), id(xid), language(xlanguage) {}
  };
  static iso639 table[]={
  iso639("ab\0",((unsigned long)'a')|((unsigned long)'b'<<8), "Abkhazian" ),
  iso639("abk",((unsigned long)'a')|((unsigned long)'b'<<8)|((unsigned long)'k'<<16), "Abkhazian" ),
  iso639("ace",((unsigned long)'a')|((unsigned long)'c'<<8)|((unsigned long)'e'<<16), "Achinese" ),
  iso639("ach",((unsigned long)'a')|((unsigned long)'c'<<8)|((unsigned long)'h'<<16), "Acoli" ),
  iso639("ada",((unsigned long)'a')|((unsigned long)'d'<<8)|((unsigned long)'a'<<16), "Adangme" ),
  iso639("om\0",((unsigned long)'o')|((unsigned long)'m'<<8), "Afan" ),
  iso639("aa\0",((unsigned long)'a')|((unsigned long)'a'<<8), "Afar" ),
  iso639("aar",((unsigned long)'a')|((unsigned long)'a'<<8)|((unsigned long)'r'<<16), "Afar" ),
  iso639("afh",((unsigned long)'a')|((unsigned long)'f'<<8)|((unsigned long)'h'<<16), "Afrihili" ),
  iso639("af\0",((unsigned long)'a')|((unsigned long)'f'<<8), "Afrikaans" ),
  iso639("afr",((unsigned long)'a')|((unsigned long)'f'<<8)|((unsigned long)'r'<<16), "Afrikaans" ),
  iso639("afa",((unsigned long)'a')|((unsigned long)'f'<<8)|((unsigned long)'a'<<16), "Afro-Asiatic (Other)" ),
  iso639("aka",((unsigned long)'a')|((unsigned long)'k'<<8)|((unsigned long)'a'<<16), "Akan" ),
  iso639("akk",((unsigned long)'a')|((unsigned long)'k'<<8)|((unsigned long)'k'<<16), "Akkadian" ),
  iso639("alb",((unsigned long)'a')|((unsigned long)'l'<<8)|((unsigned long)'b'<<16), "Albanian" ),
  iso639("sq\0",((unsigned long)'s')|((unsigned long)'q'<<8), "Albanian" ),
  iso639("sqi",((unsigned long)'s')|((unsigned long)'q'<<8)|((unsigned long)'i'<<16), "Albanian" ),
  iso639("ale",((unsigned long)'a')|((unsigned long)'l'<<8)|((unsigned long)'e'<<16), "Aleut" ),
  iso639("alg",((unsigned long)'a')|((unsigned long)'l'<<8)|((unsigned long)'g'<<16), "Algonquian languages" ),
  iso639("tut",((unsigned long)'t')|((unsigned long)'u'<<8)|((unsigned long)'t'<<16), "Altaic (Other)" ),
  iso639("en\0",((unsigned long)'e')|((unsigned long)'n'<<8), "American" ),
  iso639("am\0",((unsigned long)'a')|((unsigned long)'m'<<8), "Amharic" ),
  iso639("amh",((unsigned long)'a')|((unsigned long)'m'<<8)|((unsigned long)'h'<<16), "Amharic" ),
  iso639("apa",((unsigned long)'a')|((unsigned long)'p'<<8)|((unsigned long)'a'<<16), "Apache languages" ),
  iso639("ar\0",((unsigned long)'a')|((unsigned long)'r'<<8), "Arabic" ),
  iso639("ara",((unsigned long)'a')|((unsigned long)'r'<<8)|((unsigned long)'a'<<16), "Arabic" ),
  iso639("arc",((unsigned long)'a')|((unsigned long)'r'<<8)|((unsigned long)'c'<<16), "Aramaic" ),
  iso639("arp",((unsigned long)'a')|((unsigned long)'r'<<8)|((unsigned long)'p'<<16), "Arapaho" ),
  iso639("arn",((unsigned long)'a')|((unsigned long)'r'<<8)|((unsigned long)'n'<<16), "Araucanian" ),
  iso639("arw",((unsigned long)'a')|((unsigned long)'r'<<8)|((unsigned long)'w'<<16), "Arawak" ),
  iso639("arm",((unsigned long)'a')|((unsigned long)'r'<<8)|((unsigned long)'m'<<16), "Armenian" ),
  iso639("hy\0",((unsigned long)'h')|((unsigned long)'y'<<8), "Armenian" ),
  iso639("hye",((unsigned long)'h')|((unsigned long)'y'<<8)|((unsigned long)'e'<<16), "Armenian" ),
  iso639("art",((unsigned long)'a')|((unsigned long)'r'<<8)|((unsigned long)'t'<<16), "Artificial (Other)" ),
  iso639("as\0",((unsigned long)'a')|((unsigned long)'s'<<8), "Assamese" ),
  iso639("asm",((unsigned long)'a')|((unsigned long)'s'<<8)|((unsigned long)'m'<<16), "Assamese" ),
  iso639("ath",((unsigned long)'a')|((unsigned long)'t'<<8)|((unsigned long)'h'<<16), "Athapascan languages" ),
  iso639("map",((unsigned long)'m')|((unsigned long)'a'<<8)|((unsigned long)'p'<<16), "Austronesian (Other)" ),
  iso639("ava",((unsigned long)'a')|((unsigned long)'v'<<8)|((unsigned long)'a'<<16), "Avaric" ),
  iso639("ave",((unsigned long)'a')|((unsigned long)'v'<<8)|((unsigned long)'e'<<16), "Avestan" ),
  iso639("awa",((unsigned long)'a')|((unsigned long)'w'<<8)|((unsigned long)'a'<<16), "Awadhi" ),
  iso639("ay\0",((unsigned long)'a')|((unsigned long)'y'<<8), "Aymara" ),
  iso639("aym",((unsigned long)'a')|((unsigned long)'y'<<8)|((unsigned long)'m'<<16), "Aymara" ),
  iso639("az\0",((unsigned long)'a')|((unsigned long)'z'<<8), "Azerbaijani" ),
  iso639("aze",((unsigned long)'a')|((unsigned long)'z'<<8)|((unsigned long)'e'<<16), "Azerbaijani" ),
  iso639("nah",((unsigned long)'n')|((unsigned long)'a'<<8)|((unsigned long)'h'<<16), "Aztec" ),
  iso639("ban",((unsigned long)'b')|((unsigned long)'a'<<8)|((unsigned long)'n'<<16), "Balinese" ),
  iso639("bat",((unsigned long)'b')|((unsigned long)'a'<<8)|((unsigned long)'t'<<16), "Baltic (Other)" ),
  iso639("bal",((unsigned long)'b')|((unsigned long)'a'<<8)|((unsigned long)'l'<<16), "Baluchi" ),
  iso639("bam",((unsigned long)'b')|((unsigned long)'a'<<8)|((unsigned long)'m'<<16), "Bambara" ),
  iso639("bai",((unsigned long)'b')|((unsigned long)'a'<<8)|((unsigned long)'i'<<16), "Bamileke languages" ),
  iso639("bad",((unsigned long)'b')|((unsigned long)'a'<<8)|((unsigned long)'d'<<16), "Banda" ),
  iso639("bn\0",((unsigned long)'b')|((unsigned long)'n'<<8), "Bangla" ),
  iso639("bnt",((unsigned long)'b')|((unsigned long)'n'<<8)|((unsigned long)'t'<<16), "Bantu (Other)" ),
  iso639("bas",((unsigned long)'b')|((unsigned long)'a'<<8)|((unsigned long)'s'<<16), "Basa" ),
  iso639("ba\0",((unsigned long)'b')|((unsigned long)'a'<<8), "Bashkir" ),
  iso639("bak",((unsigned long)'b')|((unsigned long)'a'<<8)|((unsigned long)'k'<<16), "Bashkir" ),
  iso639("baq",((unsigned long)'b')|((unsigned long)'a'<<8)|((unsigned long)'q'<<16), "Basque" ),
  iso639("eu\0",((unsigned long)'e')|((unsigned long)'u'<<8), "Basque" ),
  iso639("eus",((unsigned long)'e')|((unsigned long)'u'<<8)|((unsigned long)'s'<<16), "Basque" ),
  iso639("bej",((unsigned long)'b')|((unsigned long)'e'<<8)|((unsigned long)'j'<<16), "Beja" ),
  iso639("bem",((unsigned long)'b')|((unsigned long)'e'<<8)|((unsigned long)'m'<<16), "Bemba" ),
  iso639("bn\0",((unsigned long)'b')|((unsigned long)'n'<<8), "Bengali" ),
  iso639("ben",((unsigned long)'b')|((unsigned long)'e'<<8)|((unsigned long)'n'<<16), "Bengali" ),
  iso639("ber",((unsigned long)'b')|((unsigned long)'e'<<8)|((unsigned long)'r'<<16), "Berber (Other)" ),
  iso639("bho",((unsigned long)'b')|((unsigned long)'h'<<8)|((unsigned long)'o'<<16), "Bhojpuri" ),
  iso639("dz\0",((unsigned long)'d')|((unsigned long)'z'<<8), "Bhutani" ),
  iso639("bh\0",((unsigned long)'b')|((unsigned long)'h'<<8), "Bihari" ),
  iso639("bih",((unsigned long)'b')|((unsigned long)'i'<<8)|((unsigned long)'h'<<16), "Bihari" ),
  iso639("bik",((unsigned long)'b')|((unsigned long)'i'<<8)|((unsigned long)'k'<<16), "Bikol" ),
  iso639("bin",((unsigned long)'b')|((unsigned long)'i'<<8)|((unsigned long)'n'<<16), "Bini" ),
  iso639("bi\0",((unsigned long)'b')|((unsigned long)'i'<<8), "Bislama" ),
  iso639("bis",((unsigned long)'b')|((unsigned long)'i'<<8)|((unsigned long)'s'<<16), "Bislama" ),
  iso639("bra",((unsigned long)'b')|((unsigned long)'r'<<8)|((unsigned long)'a'<<16), "Braj" ),
  iso639("br\0",((unsigned long)'b')|((unsigned long)'r'<<8), "Breton" ),
  iso639("bre",((unsigned long)'b')|((unsigned long)'r'<<8)|((unsigned long)'e'<<16), "Breton" ),
  iso639("bug",((unsigned long)'b')|((unsigned long)'u'<<8)|((unsigned long)'g'<<16), "Buginese" ),
  iso639("bg\0",((unsigned long)'b')|((unsigned long)'g'<<8), "Bulgarian" ),
  iso639("bul",((unsigned long)'b')|((unsigned long)'u'<<8)|((unsigned long)'l'<<16), "Bulgarian" ),
  iso639("bua",((unsigned long)'b')|((unsigned long)'u'<<8)|((unsigned long)'a'<<16), "Buriat" ),
  iso639("bur",((unsigned long)'b')|((unsigned long)'u'<<8)|((unsigned long)'r'<<16), "Burmese" ),
  iso639("my\0",((unsigned long)'m')|((unsigned long)'y'<<8), "Burmese" ),
  iso639("mya",((unsigned long)'m')|((unsigned long)'y'<<8)|((unsigned long)'a'<<16), "Burmese" ),
  iso639("be\0",((unsigned long)'b')|((unsigned long)'e'<<8), "Byelorussian" ),
  iso639("bel",((unsigned long)'b')|((unsigned long)'e'<<8)|((unsigned long)'l'<<16), "Byelorussian" ),
  iso639("cad",((unsigned long)'c')|((unsigned long)'a'<<8)|((unsigned long)'d'<<16), "Caddo" ),
  iso639("km\0",((unsigned long)'k')|((unsigned long)'m'<<8), "Cambodian" ),
  iso639("car",((unsigned long)'c')|((unsigned long)'a'<<8)|((unsigned long)'r'<<16), "Carib" ),
  iso639("ca\0",((unsigned long)'c')|((unsigned long)'a'<<8), "Catalan" ),
  iso639("cat",((unsigned long)'c')|((unsigned long)'a'<<8)|((unsigned long)'t'<<16), "Catalan" ),
  iso639("cau",((unsigned long)'c')|((unsigned long)'a'<<8)|((unsigned long)'u'<<16), "Caucasian (Other)" ),
  iso639("ceb",((unsigned long)'c')|((unsigned long)'e'<<8)|((unsigned long)'b'<<16), "Cebuano" ),
  iso639("cel",((unsigned long)'c')|((unsigned long)'e'<<8)|((unsigned long)'l'<<16), "Celtic (Other)" ),
  iso639("cai",((unsigned long)'c')|((unsigned long)'a'<<8)|((unsigned long)'i'<<16), "Central American Indian (Other)" ),
  iso639("chg",((unsigned long)'c')|((unsigned long)'h'<<8)|((unsigned long)'g'<<16), "Chagatai" ),
  iso639("cha",((unsigned long)'c')|((unsigned long)'h'<<8)|((unsigned long)'a'<<16), "Chamorro" ),
  iso639("che",((unsigned long)'c')|((unsigned long)'h'<<8)|((unsigned long)'e'<<16), "Chechen" ),
  iso639("chr",((unsigned long)'c')|((unsigned long)'h'<<8)|((unsigned long)'r'<<16), "Cherokee" ),
  iso639("chy",((unsigned long)'c')|((unsigned long)'h'<<8)|((unsigned long)'y'<<16), "Cheyenne" ),
  iso639("chb",((unsigned long)'c')|((unsigned long)'h'<<8)|((unsigned long)'b'<<16), "Chibcha" ),
  iso639("chi",((unsigned long)'c')|((unsigned long)'h'<<8)|((unsigned long)'i'<<16), "Chinese" ),
  iso639("zh\0",((unsigned long)'z')|((unsigned long)'h'<<8), "Chinese" ),
  iso639("zho",((unsigned long)'z')|((unsigned long)'h'<<8)|((unsigned long)'o'<<16), "Chinese" ),
  iso639("chn",((unsigned long)'c')|((unsigned long)'h'<<8)|((unsigned long)'n'<<16), "Chinook jargon" ),
  iso639("cho",((unsigned long)'c')|((unsigned long)'h'<<8)|((unsigned long)'o'<<16), "Choctaw" ),
  iso639("chu",((unsigned long)'c')|((unsigned long)'h'<<8)|((unsigned long)'u'<<16), "Church Slavic" ),
  iso639("chv",((unsigned long)'c')|((unsigned long)'h'<<8)|((unsigned long)'v'<<16), "Chuvash" ),
  iso639("cop",((unsigned long)'c')|((unsigned long)'o'<<8)|((unsigned long)'p'<<16), "Coptic" ),
  iso639("cor",((unsigned long)'c')|((unsigned long)'o'<<8)|((unsigned long)'r'<<16), "Cornish" ),
  iso639("co\0",((unsigned long)'c')|((unsigned long)'o'<<8), "Corsican" ),
  iso639("cos",((unsigned long)'c')|((unsigned long)'o'<<8)|((unsigned long)'s'<<16), "Corsican" ),
  iso639("cre",((unsigned long)'c')|((unsigned long)'r'<<8)|((unsigned long)'e'<<16), "Cree" ),
  iso639("mus",((unsigned long)'m')|((unsigned long)'u'<<8)|((unsigned long)'s'<<16), "Creek" ),
  iso639("cpe",((unsigned long)'c')|((unsigned long)'p'<<8)|((unsigned long)'e'<<16), "Creoles and Pidgins, English-based (Other)" ),
  iso639("cpf",((unsigned long)'c')|((unsigned long)'p'<<8)|((unsigned long)'f'<<16), "Creoles and Pidgins, French-based (Other)" ),
  iso639("crp",((unsigned long)'c')|((unsigned long)'r'<<8)|((unsigned long)'p'<<16), "Creoles and Pidgins (Other)" ),
  iso639("cpp",((unsigned long)'c')|((unsigned long)'p'<<8)|((unsigned long)'p'<<16), "Creoles and Pidgins, Portuguese-based (Other)" ),
  iso639("hr\0",((unsigned long)'h')|((unsigned long)'r'<<8), "Croatian" ),
  iso639("cus",((unsigned long)'c')|((unsigned long)'u'<<8)|((unsigned long)'s'<<16), "Cushitic (Other)" ),
  iso639("ces",((unsigned long)'c')|((unsigned long)'e'<<8)|((unsigned long)'s'<<16), "Czech" ),
  iso639("cs\0",((unsigned long)'c')|((unsigned long)'s'<<8), "Czech" ),
  iso639("cze",((unsigned long)'c')|((unsigned long)'z'<<8)|((unsigned long)'e'<<16), "Czech" ),
  iso639("dak",((unsigned long)'d')|((unsigned long)'a'<<8)|((unsigned long)'k'<<16), "Dakota" ),
  iso639("da\0",((unsigned long)'d')|((unsigned long)'a'<<8), "Danish" ),
  iso639("dan",((unsigned long)'d')|((unsigned long)'a'<<8)|((unsigned long)'n'<<16), "Danish" ),
  iso639("del",((unsigned long)'d')|((unsigned long)'e'<<8)|((unsigned long)'l'<<16), "Delaware" ),
  iso639("din",((unsigned long)'d')|((unsigned long)'i'<<8)|((unsigned long)'n'<<16), "Dinka" ),
  iso639("div",((unsigned long)'d')|((unsigned long)'i'<<8)|((unsigned long)'v'<<16), "Divehi" ),
  iso639("doi",((unsigned long)'d')|((unsigned long)'o'<<8)|((unsigned long)'i'<<16), "Dogri" ),
  iso639("dra",((unsigned long)'d')|((unsigned long)'r'<<8)|((unsigned long)'a'<<16), "Dravidian (Other)" ),
  iso639("dua",((unsigned long)'d')|((unsigned long)'u'<<8)|((unsigned long)'a'<<16), "Duala" ),
  iso639("dut",((unsigned long)'d')|((unsigned long)'u'<<8)|((unsigned long)'t'<<16), "Dutch" ),
  iso639("dum",((unsigned long)'d')|((unsigned long)'u'<<8)|((unsigned long)'m'<<16), "Dutch, Middle (ca. 1050-1350)" ),
  iso639("nl\0",((unsigned long)'n')|((unsigned long)'l'<<8), "Dutch" ),
  iso639("nla",((unsigned long)'n')|((unsigned long)'l'<<8)|((unsigned long)'a'<<16), "Dutch" ),
  iso639("dyu",((unsigned long)'d')|((unsigned long)'y'<<8)|((unsigned long)'u'<<16), "Dyula" ),
  iso639("dzo",((unsigned long)'d')|((unsigned long)'z'<<8)|((unsigned long)'o'<<16), "Dzongkha" ),
  iso639("efi",((unsigned long)'e')|((unsigned long)'f'<<8)|((unsigned long)'i'<<16), "Efik" ),
  iso639("egy",((unsigned long)'e')|((unsigned long)'g'<<8)|((unsigned long)'y'<<16), "Egyptian (Ancient)" ),
  iso639("eka",((unsigned long)'e')|((unsigned long)'k'<<8)|((unsigned long)'a'<<16), "Ekajuk" ),
  iso639("elx",((unsigned long)'e')|((unsigned long)'l'<<8)|((unsigned long)'x'<<16), "Elamite" ),
  iso639("en\0",((unsigned long)'e')|((unsigned long)'n'<<8), "English" ),
  iso639("eng",((unsigned long)'e')|((unsigned long)'n'<<8)|((unsigned long)'g'<<16), "English" ),
  iso639("enm",((unsigned long)'e')|((unsigned long)'n'<<8)|((unsigned long)'m'<<16), "English, Middle (ca. 1100-1500)" ),
  iso639("ang",((unsigned long)'a')|((unsigned long)'n'<<8)|((unsigned long)'g'<<16), "English, Old (ca. 450-1100)" ),
  iso639("esk",((unsigned long)'e')|((unsigned long)'s'<<8)|((unsigned long)'k'<<16), "Eskimo (Other)" ),
  iso639("eo\0",((unsigned long)'e')|((unsigned long)'o'<<8), "Esperanto" ),
  iso639("epo",((unsigned long)'e')|((unsigned long)'p'<<8)|((unsigned long)'o'<<16), "Esperanto" ),
  iso639("est",((unsigned long)'e')|((unsigned long)'s'<<8)|((unsigned long)'t'<<16), "Estonian" ),
  iso639("et\0",((unsigned long)'e')|((unsigned long)'t'<<8), "Estonian" ),
  iso639("ewe",((unsigned long)'e')|((unsigned long)'w'<<8)|((unsigned long)'e'<<16), "Ewe" ),
  iso639("ewo",((unsigned long)'e')|((unsigned long)'w'<<8)|((unsigned long)'o'<<16), "Ewondo" ),
  iso639("fo\0",((unsigned long)'f')|((unsigned long)'o'<<8), "Faeroese" ),
  iso639("fan",((unsigned long)'f')|((unsigned long)'a'<<8)|((unsigned long)'n'<<16), "Fang" ),
  iso639("fat",((unsigned long)'f')|((unsigned long)'a'<<8)|((unsigned long)'t'<<16), "Fanti" ),
  iso639("fao",((unsigned long)'f')|((unsigned long)'a'<<8)|((unsigned long)'o'<<16), "Faroese" ),
  iso639("fij",((unsigned long)'f')|((unsigned long)'i'<<8)|((unsigned long)'j'<<16), "Fijian" ),
  iso639("fj\0",((unsigned long)'f')|((unsigned long)'j'<<8), "Fiji" ),
  iso639("fi\0",((unsigned long)'f')|((unsigned long)'i'<<8), "Finnish" ),
  iso639("fin",((unsigned long)'f')|((unsigned long)'i'<<8)|((unsigned long)'n'<<16), "Finnish" ),
  iso639("fiu",((unsigned long)'f')|((unsigned long)'i'<<8)|((unsigned long)'u'<<16), "Finno-Ugrian (Other)" ),
  iso639("fon",((unsigned long)'f')|((unsigned long)'o'<<8)|((unsigned long)'n'<<16), "Fon" ),
  iso639("fr\0",((unsigned long)'f')|((unsigned long)'r'<<8), "French" ),
  iso639("fra",((unsigned long)'f')|((unsigned long)'r'<<8)|((unsigned long)'a'<<16), "French" ),
  iso639("fre",((unsigned long)'f')|((unsigned long)'r'<<8)|((unsigned long)'e'<<16), "French" ),
  iso639("frm",((unsigned long)'f')|((unsigned long)'r'<<8)|((unsigned long)'m'<<16), "French, Middle (ca. 1400-1600)" ),
  iso639("fro",((unsigned long)'f')|((unsigned long)'r'<<8)|((unsigned long)'o'<<16), "French, Old (842- ca. 1400)" ),
  iso639("fry",((unsigned long)'f')|((unsigned long)'r'<<8)|((unsigned long)'y'<<16), "Frisian" ),
  iso639("fy\0",((unsigned long)'f')|((unsigned long)'y'<<8), "Frisian" ),
  iso639("ful",((unsigned long)'f')|((unsigned long)'u'<<8)|((unsigned long)'l'<<16), "Fulah" ),
  iso639("gd\0",((unsigned long)'g')|((unsigned long)'d'<<8), "Gaelic" ),
  iso639("gae",((unsigned long)'g')|((unsigned long)'a'<<8)|((unsigned long)'e'<<16), "Gaelic (Scots)"),
  iso639("gdh",((unsigned long)'g')|((unsigned long)'d'<<8)|((unsigned long)'h'<<16), "Gaelic (Scots)"),
  iso639("gaa",((unsigned long)'g')|((unsigned long)'a'<<8)|((unsigned long)'a'<<16), "Ga" ),
  iso639("gl\0",((unsigned long)'g')|((unsigned long)'l'<<8), "Galician" ),
  iso639("glg",((unsigned long)'g')|((unsigned long)'l'<<8)|((unsigned long)'g'<<16), "Gallegan" ),
  iso639("lug",((unsigned long)'l')|((unsigned long)'u'<<8)|((unsigned long)'g'<<16), "Ganda" ),
  iso639("gay",((unsigned long)'g')|((unsigned long)'a'<<8)|((unsigned long)'y'<<16), "Gayo" ),
  iso639("gez",((unsigned long)'g')|((unsigned long)'e'<<8)|((unsigned long)'z'<<16), "Geez" ),
  iso639("geo",((unsigned long)'g')|((unsigned long)'e'<<8)|((unsigned long)'o'<<16), "Georgian" ),
  iso639("ka\0",((unsigned long)'k')|((unsigned long)'a'<<8), "Georgian" ),
  iso639("kat",((unsigned long)'k')|((unsigned long)'a'<<8)|((unsigned long)'t'<<16), "Georgian" ),
  iso639("de\0",((unsigned long)'d')|((unsigned long)'e'<<8), "German" ),
  iso639("deu",((unsigned long)'d')|((unsigned long)'e'<<8)|((unsigned long)'u'<<16), "German" ),
  iso639("ger",((unsigned long)'g')|((unsigned long)'e'<<8)|((unsigned long)'r'<<16), "German" ),
  iso639("gem",((unsigned long)'g')|((unsigned long)'e'<<8)|((unsigned long)'m'<<16), "Germanic (Other)" ),
  iso639("gmh",((unsigned long)'g')|((unsigned long)'m'<<8)|((unsigned long)'h'<<16), "German, Middle High (ca. 1050-1500)" ),
  iso639("goh",((unsigned long)'g')|((unsigned long)'o'<<8)|((unsigned long)'h'<<16), "German, Old High (ca. 750-1050)" ),
  iso639("gil",((unsigned long)'g')|((unsigned long)'i'<<8)|((unsigned long)'l'<<16), "Gilbertese" ),
  iso639("gon",((unsigned long)'g')|((unsigned long)'o'<<8)|((unsigned long)'n'<<16), "Gondi" ),
  iso639("got",((unsigned long)'g')|((unsigned long)'o'<<8)|((unsigned long)'t'<<16), "Gothic" ),
  iso639("grb",((unsigned long)'g')|((unsigned long)'r'<<8)|((unsigned long)'b'<<16), "Grebo" ),
  iso639("grc",((unsigned long)'g')|((unsigned long)'r'<<8)|((unsigned long)'c'<<16), "Greek, Ancient (to 1453)" ),
  iso639("el\0",((unsigned long)'e')|((unsigned long)'l'<<8), "Greek" ),
  iso639("ell",((unsigned long)'e')|((unsigned long)'l'<<8)|((unsigned long)'l'<<16), "Greek, Modern (1453-)" ),
  iso639("gre",((unsigned long)'g')|((unsigned long)'r'<<8)|((unsigned long)'e'<<16), "Greek, Modern (1453-)" ),
  iso639("kal",((unsigned long)'k')|((unsigned long)'a'<<8)|((unsigned long)'l'<<16), "Greenlandic" ),
  iso639("kl\0",((unsigned long)'k')|((unsigned long)'l'<<8), "Greenlandic" ),
  iso639("gn\0",((unsigned long)'g')|((unsigned long)'n'<<8), "Guarani" ),
  iso639("grn",((unsigned long)'g')|((unsigned long)'r'<<8)|((unsigned long)'n'<<16), "Guarani" ),
  iso639("gu\0",((unsigned long)'g')|((unsigned long)'u'<<8), "Gujarati" ),
  iso639("guj",((unsigned long)'g')|((unsigned long)'u'<<8)|((unsigned long)'j'<<16), "Gujarati" ),
  iso639("hai",((unsigned long)'h')|((unsigned long)'a'<<8)|((unsigned long)'i'<<16), "Haida" ),
  iso639("ha\0",((unsigned long)'h')|((unsigned long)'a'<<8), "Hausa" ),
  iso639("hau",((unsigned long)'h')|((unsigned long)'a'<<8)|((unsigned long)'u'<<16), "Hausa" ),
  iso639("haw",((unsigned long)'h')|((unsigned long)'a'<<8)|((unsigned long)'w'<<16), "Hawaiian" ),
  iso639("heb",((unsigned long)'h')|((unsigned long)'e'<<8)|((unsigned long)'b'<<16), "Hebrew" ),
  iso639("iw\0",((unsigned long)'i')|((unsigned long)'w'<<8), "Hebrew" ),
  iso639("her",((unsigned long)'h')|((unsigned long)'e'<<8)|((unsigned long)'r'<<16), "Herero" ),
  iso639("hil",((unsigned long)'h')|((unsigned long)'i'<<8)|((unsigned long)'l'<<16), "Hiligaynon" ),
  iso639("him",((unsigned long)'h')|((unsigned long)'i'<<8)|((unsigned long)'m'<<16), "Himachali" ),
  iso639("hi\0",((unsigned long)'h')|((unsigned long)'i'<<8), "Hindi" ),
  iso639("hin",((unsigned long)'h')|((unsigned long)'i'<<8)|((unsigned long)'n'<<16), "Hindi" ),
  iso639("hmo",((unsigned long)'h')|((unsigned long)'m'<<8)|((unsigned long)'o'<<16), "Hiri Motu" ),
  iso639("hu\0",((unsigned long)'h')|((unsigned long)'u'<<8), "Hungarian" ),
  iso639("hun",((unsigned long)'h')|((unsigned long)'u'<<8)|((unsigned long)'n'<<16), "Hungarian" ),
  iso639("hup",((unsigned long)'h')|((unsigned long)'u'<<8)|((unsigned long)'p'<<16), "Hupa" ),
  iso639("iba",((unsigned long)'i')|((unsigned long)'b'<<8)|((unsigned long)'a'<<16), "Iban" ),
  iso639("ice",((unsigned long)'i')|((unsigned long)'c'<<8)|((unsigned long)'e'<<16), "Icelandic" ),
  iso639("is\0",((unsigned long)'i')|((unsigned long)'s'<<8), "Icelandic" ),
  iso639("isl",((unsigned long)'i')|((unsigned long)'s'<<8)|((unsigned long)'l'<<16), "Icelandic" ),
  iso639("ibo",((unsigned long)'i')|((unsigned long)'b'<<8)|((unsigned long)'o'<<16), "Igbo" ),
  iso639("ijo",((unsigned long)'i')|((unsigned long)'j'<<8)|((unsigned long)'o'<<16), "Ijo" ),
  iso639("ilo",((unsigned long)'i')|((unsigned long)'l'<<8)|((unsigned long)'o'<<16), "Iloko" ),
  iso639("inc",((unsigned long)'i')|((unsigned long)'n'<<8)|((unsigned long)'c'<<16), "Indic (Other)" ),
  iso639("ine",((unsigned long)'i')|((unsigned long)'n'<<8)|((unsigned long)'e'<<16), "Indo-European (Other)" ),
  iso639("in\0",((unsigned long)'i')|((unsigned long)'n'<<8), "Indonesian" ),
  iso639("ind",((unsigned long)'i')|((unsigned long)'n'<<8)|((unsigned long)'d'<<16), "Indonesian" ),
  iso639("ia\0",((unsigned long)'i')|((unsigned long)'a'<<8), "Interlingua" ),
  iso639("ina",((unsigned long)'i')|((unsigned long)'n'<<8)|((unsigned long)'a'<<16), "Interlingua (International Auxiliary language Association)" ),
  iso639("ie\0",((unsigned long)'i')|((unsigned long)'e'<<8), "Interlingue" ),
  iso639("ine",((unsigned long)'i')|((unsigned long)'n'<<8)|((unsigned long)'e'<<16), "Interlingue" ),
  iso639("iku",((unsigned long)'i')|((unsigned long)'k'<<8)|((unsigned long)'u'<<16), "Inuktitut" ),
  iso639("ik\0",((unsigned long)'i')|((unsigned long)'k'<<8), "Inupiak" ),
  iso639("ipk",((unsigned long)'i')|((unsigned long)'p'<<8)|((unsigned long)'k'<<16), "Inupiak" ),
  iso639("ira",((unsigned long)'i')|((unsigned long)'r'<<8)|((unsigned long)'a'<<16), "Iranian (Other)" ),
  iso639("ga\0",((unsigned long)'g')|((unsigned long)'a'<<8), "Irish" ),
  iso639("gai",((unsigned long)'g')|((unsigned long)'a'<<8)|((unsigned long)'i'<<16), "Irish" ),
  iso639("iri",((unsigned long)'i')|((unsigned long)'r'<<8)|((unsigned long)'i'<<16), "Irish" ),
  iso639("mga",((unsigned long)'m')|((unsigned long)'g'<<8)|((unsigned long)'a'<<16), "Irish, Middle (900 - 1200)" ),
  iso639("sga",((unsigned long)'s')|((unsigned long)'g'<<8)|((unsigned long)'a'<<16), "Irish, Old (to 900)" ),
  iso639("iro",((unsigned long)'i')|((unsigned long)'r'<<8)|((unsigned long)'o'<<16), "Iroquoian languages" ),
  iso639("it\0",((unsigned long)'i')|((unsigned long)'t'<<8), "Italian" ),
  iso639("ita",((unsigned long)'i')|((unsigned long)'t'<<8)|((unsigned long)'a'<<16), "Italian" ),
  iso639("ja\0",((unsigned long)'j')|((unsigned long)'a'<<8), "Japanese" ),
  iso639("jpn",((unsigned long)'j')|((unsigned long)'p'<<8)|((unsigned long)'n'<<16), "Japanese" ),
  iso639("jav",((unsigned long)'j')|((unsigned long)'a'<<8)|((unsigned long)'v'<<16), "Javanese" ),
  iso639("jav",((unsigned long)'j')|((unsigned long)'a'<<8)|((unsigned long)'v'<<16), "Javanese" ),
  iso639("jaw",((unsigned long)'j')|((unsigned long)'a'<<8)|((unsigned long)'w'<<16), "Javanese" ),
  iso639("jaw",((unsigned long)'j')|((unsigned long)'a'<<8)|((unsigned long)'w'<<16), "Javanese" ),
  iso639("jw\0",((unsigned long)'j')|((unsigned long)'w'<<8), "Javanese" ),
  iso639("jrb",((unsigned long)'j')|((unsigned long)'r'<<8)|((unsigned long)'b'<<16), "Judeo-Arabic" ),
  iso639("jpr",((unsigned long)'j')|((unsigned long)'p'<<8)|((unsigned long)'r'<<16), "Judeo-Persian" ),
  iso639("kab",((unsigned long)'k')|((unsigned long)'a'<<8)|((unsigned long)'b'<<16), "Kabyle" ),
  iso639("kac",((unsigned long)'k')|((unsigned long)'a'<<8)|((unsigned long)'c'<<16), "Kachin" ),
  iso639("kam",((unsigned long)'k')|((unsigned long)'a'<<8)|((unsigned long)'m'<<16), "Kamba" ),
  iso639("kan",((unsigned long)'k')|((unsigned long)'a'<<8)|((unsigned long)'n'<<16), "Kannada" ),
  iso639("kn\0",((unsigned long)'k')|((unsigned long)'n'<<8), "Kannada" ),
  iso639("kau",((unsigned long)'k')|((unsigned long)'a'<<8)|((unsigned long)'u'<<16), "Kanuri" ),
  iso639("kaa",((unsigned long)'k')|((unsigned long)'a'<<8)|((unsigned long)'a'<<16), "Kara-Kalpak" ),
  iso639("kar",((unsigned long)'k')|((unsigned long)'a'<<8)|((unsigned long)'r'<<16), "Karen" ),
  iso639("kas",((unsigned long)'k')|((unsigned long)'a'<<8)|((unsigned long)'s'<<16), "Kashmiri" ),
  iso639("ks\0",((unsigned long)'k')|((unsigned long)'s'<<8), "Kashmiri" ),
  iso639("kaw",((unsigned long)'k')|((unsigned long)'a'<<8)|((unsigned long)'w'<<16), "Kawi" ),
  iso639("kaz",((unsigned long)'k')|((unsigned long)'a'<<8)|((unsigned long)'z'<<16), "Kazakh" ),
  iso639("kk\0",((unsigned long)'k')|((unsigned long)'k'<<8), "Kazakh" ),
  iso639("kha",((unsigned long)'k')|((unsigned long)'h'<<8)|((unsigned long)'a'<<16), "Khasi" ),
  iso639("khm",((unsigned long)'k')|((unsigned long)'h'<<8)|((unsigned long)'m'<<16), "Khmer" ),
  iso639("khi",((unsigned long)'k')|((unsigned long)'h'<<8)|((unsigned long)'i'<<16), "Khoisan (Other)" ),
  iso639("kho",((unsigned long)'k')|((unsigned long)'h'<<8)|((unsigned long)'o'<<16), "Khotanese" ),
  iso639("kik",((unsigned long)'k')|((unsigned long)'i'<<8)|((unsigned long)'k'<<16), "Kikuyu" ),
  iso639("kin",((unsigned long)'k')|((unsigned long)'i'<<8)|((unsigned long)'n'<<16), "Kinyarwanda" ),
  iso639("rw\0",((unsigned long)'r')|((unsigned long)'w'<<8), "Kinyarwanda" ),
  iso639("kir",((unsigned long)'k')|((unsigned long)'i'<<8)|((unsigned long)'r'<<16), "Kirghiz" ),
  iso639("ky\0",((unsigned long)'k')|((unsigned long)'y'<<8), "Kirghiz" ),
  iso639("rn\0",((unsigned long)'r')|((unsigned long)'n'<<8), "Kirundi" ),
  iso639("kom",((unsigned long)'k')|((unsigned long)'o'<<8)|((unsigned long)'m'<<16), "Komi" ),
  iso639("kon",((unsigned long)'k')|((unsigned long)'o'<<8)|((unsigned long)'n'<<16), "Kongo" ),
  iso639("kok",((unsigned long)'k')|((unsigned long)'o'<<8)|((unsigned long)'k'<<16), "Konkani" ),
  iso639("ko\0",((unsigned long)'k')|((unsigned long)'o'<<8), "Korean" ),
  iso639("kor",((unsigned long)'k')|((unsigned long)'o'<<8)|((unsigned long)'r'<<16), "Korean" ),
  iso639("kpe",((unsigned long)'k')|((unsigned long)'p'<<8)|((unsigned long)'e'<<16), "Kpelle" ),
  iso639("kro",((unsigned long)'k')|((unsigned long)'r'<<8)|((unsigned long)'o'<<16), "Kru" ),
  iso639("kua",((unsigned long)'k')|((unsigned long)'u'<<8)|((unsigned long)'a'<<16), "Kuanyama" ),
  iso639("kum",((unsigned long)'k')|((unsigned long)'u'<<8)|((unsigned long)'m'<<16), "Kumyk" ),
  iso639("ku\0",((unsigned long)'k')|((unsigned long)'u'<<8), "Kurdish" ),
  iso639("kur",((unsigned long)'k')|((unsigned long)'u'<<8)|((unsigned long)'r'<<16), "Kurdish" ),
  iso639("kru",((unsigned long)'k')|((unsigned long)'r'<<8)|((unsigned long)'u'<<16), "Kurukh" ),
  iso639("kus",((unsigned long)'k')|((unsigned long)'u'<<8)|((unsigned long)'s'<<16), "Kusaie" ),
  iso639("kut",((unsigned long)'k')|((unsigned long)'u'<<8)|((unsigned long)'t'<<16), "Kutenai" ),
  iso639("lad",((unsigned long)'l')|((unsigned long)'a'<<8)|((unsigned long)'d'<<16), "Ladino" ),
  iso639("lah",((unsigned long)'l')|((unsigned long)'a'<<8)|((unsigned long)'h'<<16), "Lahnda" ),
  iso639("lam",((unsigned long)'l')|((unsigned long)'a'<<8)|((unsigned long)'m'<<16), "Lamba" ),
  iso639("oci",((unsigned long)'o')|((unsigned long)'c'<<8)|((unsigned long)'i'<<16), "Langue d'Oc (post 1500)" ),
  iso639("lao",((unsigned long)'l')|((unsigned long)'a'<<8)|((unsigned long)'o'<<16), "Lao" ),
  iso639("lo\0",((unsigned long)'l')|((unsigned long)'o'<<8), "Laothian" ),
  iso639("la\0",((unsigned long)'l')|((unsigned long)'a'<<8), "Latin" ),
  iso639("lat",((unsigned long)'l')|((unsigned long)'a'<<8)|((unsigned long)'t'<<16), "Latin" ),
  iso639("lav",((unsigned long)'l')|((unsigned long)'a'<<8)|((unsigned long)'v'<<16), "Latvian" ),
  iso639("lv\0",((unsigned long)'l')|((unsigned long)'v'<<8), "Latvian" ),
  iso639("lv\0",((unsigned long)'l')|((unsigned long)'v'<<8), "Lettish" ),
  iso639("ltz",((unsigned long)'l')|((unsigned long)'t'<<8)|((unsigned long)'z'<<16), "Letzeburgesch" ),
  iso639("lez",((unsigned long)'l')|((unsigned long)'e'<<8)|((unsigned long)'z'<<16), "Lezghian" ),
  iso639("lin",((unsigned long)'l')|((unsigned long)'i'<<8)|((unsigned long)'n'<<16), "Lingala" ),
  iso639("ln\0",((unsigned long)'l')|((unsigned long)'n'<<8), "Lingala" ),
  iso639("lit",((unsigned long)'l')|((unsigned long)'i'<<8)|((unsigned long)'t'<<16), "Lithuanian" ),
  iso639("lt\0",((unsigned long)'l')|((unsigned long)'t'<<8), "Lithuanian" ),
  iso639("loz",((unsigned long)'l')|((unsigned long)'o'<<8)|((unsigned long)'z'<<16), "Lozi" ),
  iso639("lub",((unsigned long)'l')|((unsigned long)'u'<<8)|((unsigned long)'b'<<16), "Luba-Katanga" ),
  iso639("lui",((unsigned long)'l')|((unsigned long)'u'<<8)|((unsigned long)'i'<<16), "Luiseno" ),
  iso639("lun",((unsigned long)'l')|((unsigned long)'u'<<8)|((unsigned long)'n'<<16), "Lunda" ),
  iso639("luo",((unsigned long)'l')|((unsigned long)'u'<<8)|((unsigned long)'o'<<16), "Luo (Kenya and Tanzania)" ),
  iso639("mac",((unsigned long)'m')|((unsigned long)'a'<<8)|((unsigned long)'c'<<16), "Macedonian" ),
  iso639("mak",((unsigned long)'m')|((unsigned long)'a'<<8)|((unsigned long)'k'<<16), "Macedonian" ),
  iso639("mk\0",((unsigned long)'m')|((unsigned long)'k'<<8), "Macedonian" ),
  iso639("mad",((unsigned long)'m')|((unsigned long)'a'<<8)|((unsigned long)'d'<<16), "Madurese" ),
  iso639("mag",((unsigned long)'m')|((unsigned long)'a'<<8)|((unsigned long)'g'<<16), "Magahi" ),
  iso639("mai",((unsigned long)'m')|((unsigned long)'a'<<8)|((unsigned long)'i'<<16), "Maithili" ),
  iso639("mak",((unsigned long)'m')|((unsigned long)'a'<<8)|((unsigned long)'k'<<16), "Makasar" ),
  iso639("mg\0",((unsigned long)'m')|((unsigned long)'g'<<8), "Malagasy" ),
  iso639("mlg",((unsigned long)'m')|((unsigned long)'l'<<8)|((unsigned long)'g'<<16), "Malagasy" ),
  iso639("mal",((unsigned long)'m')|((unsigned long)'a'<<8)|((unsigned long)'l'<<16), "Malayalam" ),
  iso639("ml\0",((unsigned long)'m')|((unsigned long)'l'<<8), "Malayalam" ),
  iso639("may",((unsigned long)'m')|((unsigned long)'a'<<8)|((unsigned long)'y'<<16), "Malay" ),
  iso639("ms\0",((unsigned long)'m')|((unsigned long)'s'<<8), "Malay" ),
  iso639("msa",((unsigned long)'m')|((unsigned long)'s'<<8)|((unsigned long)'a'<<16), "Malay" ),
  iso639("mlt",((unsigned long)'m')|((unsigned long)'l'<<8)|((unsigned long)'t'<<16), "Maltese" ),
  iso639("mt\0",((unsigned long)'m')|((unsigned long)'t'<<8), "Maltese" ),
  iso639("man",((unsigned long)'m')|((unsigned long)'a'<<8)|((unsigned long)'n'<<16), "Mandingo" ),
  iso639("mni",((unsigned long)'m')|((unsigned long)'n'<<8)|((unsigned long)'i'<<16), "Manipuri" ),
  iso639("mno",((unsigned long)'m')|((unsigned long)'n'<<8)|((unsigned long)'o'<<16), "Manobo languages" ),
  iso639("max",((unsigned long)'m')|((unsigned long)'a'<<8)|((unsigned long)'x'<<16), "Manx" ),
  iso639("mao",((unsigned long)'m')|((unsigned long)'a'<<8)|((unsigned long)'o'<<16), "Maori" ),
  iso639("mi\0",((unsigned long)'m')|((unsigned long)'i'<<8), "Maori" ),
  iso639("mri",((unsigned long)'m')|((unsigned long)'r'<<8)|((unsigned long)'i'<<16), "Maori" ),
  iso639("mar",((unsigned long)'m')|((unsigned long)'a'<<8)|((unsigned long)'r'<<16), "Marathi" ),
  iso639("mr\0",((unsigned long)'m')|((unsigned long)'r'<<8), "Marathi" ),
  iso639("chm",((unsigned long)'c')|((unsigned long)'h'<<8)|((unsigned long)'m'<<16), "Mari" ),
  iso639("mah",((unsigned long)'m')|((unsigned long)'a'<<8)|((unsigned long)'h'<<16), "Marshall" ),
  iso639("mwr",((unsigned long)'m')|((unsigned long)'w'<<8)|((unsigned long)'r'<<16), "Marwari" ),
  iso639("mas",((unsigned long)'m')|((unsigned long)'a'<<8)|((unsigned long)'s'<<16), "Masai" ),
  iso639("myn",((unsigned long)'m')|((unsigned long)'y'<<8)|((unsigned long)'n'<<16), "Mayan languages" ),
  iso639("men",((unsigned long)'m')|((unsigned long)'e'<<8)|((unsigned long)'n'<<16), "Mende" ),
  iso639("mic",((unsigned long)'m')|((unsigned long)'i'<<8)|((unsigned long)'c'<<16), "Micmac" ),
  iso639("min",((unsigned long)'m')|((unsigned long)'i'<<8)|((unsigned long)'n'<<16), "Minangkabau" ),
  iso639("mis",((unsigned long)'m')|((unsigned long)'i'<<8)|((unsigned long)'s'<<16), "Miscellaneous (Other)" ),
  iso639("moh",((unsigned long)'m')|((unsigned long)'o'<<8)|((unsigned long)'h'<<16), "Mohawk" ),
  iso639("mo\0",((unsigned long)'m')|((unsigned long)'o'<<8), "Moldavian" ),
  iso639("mol",((unsigned long)'m')|((unsigned long)'o'<<8)|((unsigned long)'l'<<16), "Moldavian" ),
  iso639("mn\0",((unsigned long)'m')|((unsigned long)'n'<<8), "Mongolian" ),
  iso639("mon",((unsigned long)'m')|((unsigned long)'o'<<8)|((unsigned long)'n'<<16), "Mongolian" ),
  iso639("lol",((unsigned long)'l')|((unsigned long)'o'<<8)|((unsigned long)'l'<<16), "Mongo" ),
  iso639("mkh",((unsigned long)'m')|((unsigned long)'k'<<8)|((unsigned long)'h'<<16), "Mon-Kmer (Other)" ),
  iso639("mos",((unsigned long)'m')|((unsigned long)'o'<<8)|((unsigned long)'s'<<16), "Mossi" ),
  iso639("mul",((unsigned long)'m')|((unsigned long)'u'<<8)|((unsigned long)'l'<<16), "Multiple languages" ),
  iso639("mun",((unsigned long)'m')|((unsigned long)'u'<<8)|((unsigned long)'n'<<16), "Munda languages" ),
  iso639("na\0",((unsigned long)'n')|((unsigned long)'a'<<8), "Nauru" ),
  iso639("nau",((unsigned long)'n')|((unsigned long)'a'<<8)|((unsigned long)'u'<<16), "Nauru" ),
  iso639("nav",((unsigned long)'n')|((unsigned long)'a'<<8)|((unsigned long)'v'<<16), "Navajo" ),
  iso639("nde",((unsigned long)'n')|((unsigned long)'d'<<8)|((unsigned long)'e'<<16), "Ndebele, North" ),
  iso639("nbl",((unsigned long)'n')|((unsigned long)'b'<<8)|((unsigned long)'l'<<16), "Ndebele, South" ),
  iso639("ndo",((unsigned long)'n')|((unsigned long)'d'<<8)|((unsigned long)'o'<<16), "Ndongo" ),
  iso639("ne\0",((unsigned long)'n')|((unsigned long)'e'<<8), "Nepali" ),
  iso639("nep",((unsigned long)'n')|((unsigned long)'e'<<8)|((unsigned long)'p'<<16), "Nepali" ),
  iso639("new",((unsigned long)'n')|((unsigned long)'e'<<8)|((unsigned long)'w'<<16), "Newari" ),
  iso639("nic",((unsigned long)'n')|((unsigned long)'i'<<8)|((unsigned long)'c'<<16), "Niger-Kordofanian (Other)" ),
  iso639("ssa",((unsigned long)'s')|((unsigned long)'s'<<8)|((unsigned long)'a'<<16), "Nilo-Saharan (Other)" ),
  iso639("niu",((unsigned long)'n')|((unsigned long)'i'<<8)|((unsigned long)'u'<<16), "Niuean" ),
  iso639("non",((unsigned long)'n')|((unsigned long)'o'<<8)|((unsigned long)'n'<<16), "Norse, Old" ),
  iso639("nai",((unsigned long)'n')|((unsigned long)'a'<<8)|((unsigned long)'i'<<16), "North American Indian (Other)" ),
  iso639("no\0",((unsigned long)'n')|((unsigned long)'o'<<8), "Norwegian" ),
  iso639("nor",((unsigned long)'n')|((unsigned long)'o'<<8)|((unsigned long)'r'<<16), "Norwegian" ),
  iso639("nno",((unsigned long)'n')|((unsigned long)'n'<<8)|((unsigned long)'o'<<16), "Norwegian (Nynorsk)" ),
  iso639("nub",((unsigned long)'n')|((unsigned long)'u'<<8)|((unsigned long)'b'<<16), "Nubian languages" ),
  iso639("nym",((unsigned long)'n')|((unsigned long)'y'<<8)|((unsigned long)'m'<<16), "Nyamwezi" ),
  iso639("nya",((unsigned long)'n')|((unsigned long)'y'<<8)|((unsigned long)'a'<<16), "Nyanja" ),
  iso639("nyn",((unsigned long)'n')|((unsigned long)'y'<<8)|((unsigned long)'n'<<16), "Nyankole" ),
  iso639("nyo",((unsigned long)'n')|((unsigned long)'y'<<8)|((unsigned long)'o'<<16), "Nyoro" ),
  iso639("nzi",((unsigned long)'n')|((unsigned long)'z'<<8)|((unsigned long)'i'<<16), "Nzima" ),
  iso639("oc\0",((unsigned long)'o')|((unsigned long)'c'<<8), "Occitan" ),
  iso639("oji",((unsigned long)'o')|((unsigned long)'j'<<8)|((unsigned long)'i'<<16), "Ojibwa" ),
  iso639("or\0",((unsigned long)'o')|((unsigned long)'r'<<8), "Oriya" ),
  iso639("ori",((unsigned long)'o')|((unsigned long)'r'<<8)|((unsigned long)'i'<<16), "Oriya" ),
  iso639("om\0",((unsigned long)'o')|((unsigned long)'m'<<8), "Oromo" ),
  iso639("orm",((unsigned long)'o')|((unsigned long)'r'<<8)|((unsigned long)'m'<<16), "Oromo" ),
  iso639("osa",((unsigned long)'o')|((unsigned long)'s'<<8)|((unsigned long)'a'<<16), "Osage" ),
  iso639("oss",((unsigned long)'o')|((unsigned long)'s'<<8)|((unsigned long)'s'<<16), "Ossetic" ),
  iso639("oto",((unsigned long)'o')|((unsigned long)'t'<<8)|((unsigned long)'o'<<16), "Otomian languages" ),
  iso639("pal",((unsigned long)'p')|((unsigned long)'a'<<8)|((unsigned long)'l'<<16), "Pahlavi" ),
  iso639("pau",((unsigned long)'p')|((unsigned long)'a'<<8)|((unsigned long)'u'<<16), "Palauan" ),
  iso639("pli",((unsigned long)'p')|((unsigned long)'l'<<8)|((unsigned long)'i'<<16), "Pali" ),
  iso639("pam",((unsigned long)'p')|((unsigned long)'a'<<8)|((unsigned long)'m'<<16), "Pampanga" ),
  iso639("pag",((unsigned long)'p')|((unsigned long)'a'<<8)|((unsigned long)'g'<<16), "Pangasinan" ),
  iso639("pan",((unsigned long)'p')|((unsigned long)'a'<<8)|((unsigned long)'n'<<16), "Panjabi" ),
  iso639("pap",((unsigned long)'p')|((unsigned long)'a'<<8)|((unsigned long)'p'<<16), "Papiamento" ),
  iso639("paa",((unsigned long)'p')|((unsigned long)'a'<<8)|((unsigned long)'a'<<16), "Papuan-Australian (Other)" ),
  iso639("fa\0",((unsigned long)'f')|((unsigned long)'a'<<8), "Persian" ),
  iso639("fas",((unsigned long)'f')|((unsigned long)'a'<<8)|((unsigned long)'s'<<16), "Persian" ),
  iso639("peo",((unsigned long)'p')|((unsigned long)'e'<<8)|((unsigned long)'o'<<16), "Persian, Old (ca 600 - 400 B.C.)" ),
  iso639("per",((unsigned long)'p')|((unsigned long)'e'<<8)|((unsigned long)'r'<<16), "Persian" ),
  iso639("phn",((unsigned long)'p')|((unsigned long)'h'<<8)|((unsigned long)'n'<<16), "Phoenician" ),
  iso639("pl\0",((unsigned long)'p')|((unsigned long)'l'<<8), "Polish" ),
  iso639("pol",((unsigned long)'p')|((unsigned long)'o'<<8)|((unsigned long)'l'<<16), "Polish" ),
  iso639("pon",((unsigned long)'p')|((unsigned long)'o'<<8)|((unsigned long)'n'<<16), "Ponape" ),
  iso639("por",((unsigned long)'p')|((unsigned long)'o'<<8)|((unsigned long)'r'<<16), "Portuguese" ),
  iso639("pt\0",((unsigned long)'p')|((unsigned long)'t'<<8), "Portuguese" ),
  iso639("pra",((unsigned long)'p')|((unsigned long)'r'<<8)|((unsigned long)'a'<<16), "Prakrit languages" ),
  iso639("pro",((unsigned long)'p')|((unsigned long)'r'<<8)|((unsigned long)'o'<<16), "Provencal, Old (to 1500)" ),
  iso639("pa\0",((unsigned long)'p')|((unsigned long)'a'<<8), "Punjabi" ),
  iso639("ps\0",((unsigned long)'p')|((unsigned long)'s'<<8), "Pashto" ),
  iso639("ps\0",((unsigned long)'p')|((unsigned long)'s'<<8), "Pushto" ),
  iso639("pus",((unsigned long)'p')|((unsigned long)'u'<<8)|((unsigned long)'s'<<16), "Pushto" ),
  iso639("qu\0",((unsigned long)'q')|((unsigned long)'u'<<8), "Quechua" ),
  iso639("que",((unsigned long)'q')|((unsigned long)'u'<<8)|((unsigned long)'e'<<16), "Quechua" ),
  iso639("raj",((unsigned long)'r')|((unsigned long)'a'<<8)|((unsigned long)'j'<<16), "Rajasthani" ),
  iso639("rar",((unsigned long)'r')|((unsigned long)'a'<<8)|((unsigned long)'r'<<16), "Rarotongan" ),
  iso639("rm\0",((unsigned long)'r')|((unsigned long)'m'<<8), "Rhaeto-Romance" ),
  iso639("roh",((unsigned long)'r')|((unsigned long)'o'<<8)|((unsigned long)'h'<<16), "Rhaeto-Romance" ),
  iso639("roa",((unsigned long)'r')|((unsigned long)'o'<<8)|((unsigned long)'a'<<16), "Romance (Other)" ),
  iso639("ro\0",((unsigned long)'r')|((unsigned long)'o'<<8), "Romanian" ),
  iso639("ron",((unsigned long)'r')|((unsigned long)'o'<<8)|((unsigned long)'n'<<16), "Romanian" ),
  iso639("rum",((unsigned long)'r')|((unsigned long)'u'<<8)|((unsigned long)'m'<<16), "Romanian" ),
  iso639("rom",((unsigned long)'r')|((unsigned long)'o'<<8)|((unsigned long)'m'<<16), "Romany" ),
  iso639("run",((unsigned long)'r')|((unsigned long)'u'<<8)|((unsigned long)'n'<<16), "Rundi" ),
  iso639("ru\0",((unsigned long)'r')|((unsigned long)'u'<<8), "Russian" ),
  iso639("rus",((unsigned long)'r')|((unsigned long)'u'<<8)|((unsigned long)'s'<<16), "Russian" ),
  iso639("sal",((unsigned long)'s')|((unsigned long)'a'<<8)|((unsigned long)'l'<<16), "Salishan languages" ),
  iso639("sam",((unsigned long)'s')|((unsigned long)'a'<<8)|((unsigned long)'m'<<16), "Samaritan Aramaic" ),
  iso639("smi",((unsigned long)'s')|((unsigned long)'m'<<8)|((unsigned long)'i'<<16), "Sami languages" ),
  iso639("sm\0",((unsigned long)'s')|((unsigned long)'m'<<8), "Samoan" ),
  iso639("smo",((unsigned long)'s')|((unsigned long)'m'<<8)|((unsigned long)'o'<<16), "Samoan" ),
  iso639("sad",((unsigned long)'s')|((unsigned long)'a'<<8)|((unsigned long)'d'<<16), "Sandawe" ),
  iso639("sag",((unsigned long)'s')|((unsigned long)'a'<<8)|((unsigned long)'g'<<16), "Sango" ),
  iso639("sg\0",((unsigned long)'s')|((unsigned long)'g'<<8), "Sangro" ),
  iso639("sa\0",((unsigned long)'s')|((unsigned long)'a'<<8), "Sanskrit" ),
  iso639("san",((unsigned long)'s')|((unsigned long)'a'<<8)|((unsigned long)'n'<<16), "Sanskrit" ),
  iso639("srd",((unsigned long)'s')|((unsigned long)'r'<<8)|((unsigned long)'d'<<16), "Sardinian" ),
  iso639("gd\0",((unsigned long)'g')|((unsigned long)'d'<<8), "Scots Gaelic" ),
  iso639("sco",((unsigned long)'s')|((unsigned long)'c'<<8)|((unsigned long)'o'<<16), "Scots" ),
  iso639("sel",((unsigned long)'s')|((unsigned long)'e'<<8)|((unsigned long)'l'<<16), "Selkup" ),
  iso639("sem",((unsigned long)'s')|((unsigned long)'e'<<8)|((unsigned long)'m'<<16), "Semitic (Other)" ),
  iso639("sr\0",((unsigned long)'s')|((unsigned long)'r'<<8), "Serbian" ),
  iso639("scr",((unsigned long)'s')|((unsigned long)'c'<<8)|((unsigned long)'r'<<16), "Serbo-Croatian" ),
  iso639("sh\0",((unsigned long)'s')|((unsigned long)'h'<<8), "Serbo-Croatian" ),
  iso639("srr",((unsigned long)'s')|((unsigned long)'r'<<8)|((unsigned long)'r'<<16), "Serer" ),
  iso639("st\0",((unsigned long)'s')|((unsigned long)'t'<<8), "Sesotho" ),
  iso639("tn\0",((unsigned long)'t')|((unsigned long)'n'<<8), "Setswana" ),
  iso639("shn",((unsigned long)'s')|((unsigned long)'h'<<8)|((unsigned long)'n'<<16), "Shan" ),
  iso639("sn\0",((unsigned long)'s')|((unsigned long)'n'<<8), "Shona" ),
  iso639("sna",((unsigned long)'s')|((unsigned long)'n'<<8)|((unsigned long)'a'<<16), "Shona" ),
  iso639("sid",((unsigned long)'s')|((unsigned long)'i'<<8)|((unsigned long)'d'<<16), "Sidamo" ),
  iso639("bla",((unsigned long)'b')|((unsigned long)'l'<<8)|((unsigned long)'a'<<16), "Siksika" ),
  iso639("sd\0",((unsigned long)'s')|((unsigned long)'d'<<8), "Sindhi" ),
  iso639("snd",((unsigned long)'s')|((unsigned long)'n'<<8)|((unsigned long)'d'<<16), "Sindhi" ),
  iso639("si\0",((unsigned long)'s')|((unsigned long)'i'<<8), "Singhalese" ),
  iso639("sin",((unsigned long)'s')|((unsigned long)'i'<<8)|((unsigned long)'n'<<16), "Singhalese" ),
  iso639("sit",((unsigned long)'s')|((unsigned long)'i'<<8)|((unsigned long)'t'<<16), "Sino-Tibetan (Other)" ),
  iso639("sio",((unsigned long)'s')|((unsigned long)'i'<<8)|((unsigned long)'o'<<16), "Siouan languages" ),
  iso639("ssw",((unsigned long)'s')|((unsigned long)'s'<<8)|((unsigned long)'w'<<16), "Siswant" ),
  iso639("ss\0",((unsigned long)'s')|((unsigned long)'s'<<8), "Siswati" ),
  iso639("sla",((unsigned long)'s')|((unsigned long)'l'<<8)|((unsigned long)'a'<<16), "Slavic (Other)" ),
  iso639("sk\0",((unsigned long)'s')|((unsigned long)'k'<<8), "Slovak" ),
  iso639("slk",((unsigned long)'s')|((unsigned long)'l'<<8)|((unsigned long)'k'<<16), "Slovak" ),
  iso639("slo",((unsigned long)'s')|((unsigned long)'l'<<8)|((unsigned long)'o'<<16), "Slovak" ),
  iso639("sl\0",((unsigned long)'s')|((unsigned long)'l'<<8), "Slovenian" ),
  iso639("slv",((unsigned long)'s')|((unsigned long)'l'<<8)|((unsigned long)'v'<<16), "Slovenian" ),
  iso639("sog",((unsigned long)'s')|((unsigned long)'o'<<8)|((unsigned long)'g'<<16), "Sogdian" ),
  iso639("so\0",((unsigned long)'s')|((unsigned long)'o'<<8), "Somali" ),
  iso639("som",((unsigned long)'s')|((unsigned long)'o'<<8)|((unsigned long)'m'<<16), "Somali" ),
  iso639("son",((unsigned long)'s')|((unsigned long)'o'<<8)|((unsigned long)'n'<<16), "Songhai" ),
  iso639("wen",((unsigned long)'w')|((unsigned long)'e'<<8)|((unsigned long)'n'<<16), "Sorbian languages" ),
  iso639("nso",((unsigned long)'n')|((unsigned long)'s'<<8)|((unsigned long)'o'<<16), "Sotho, Northern" ),
  iso639("sot",((unsigned long)'s')|((unsigned long)'o'<<8)|((unsigned long)'t'<<16), "Sotho, Southern" ),
  iso639("sai",((unsigned long)'s')|((unsigned long)'a'<<8)|((unsigned long)'i'<<16), "South American Indian (Other)" ),
  iso639("es\0",((unsigned long)'e')|((unsigned long)'s'<<8), "Spanish" ),
  iso639("esl",((unsigned long)'e')|((unsigned long)'s'<<8)|((unsigned long)'l'<<16), "Spanish" ),
  iso639("spa",((unsigned long)'s')|((unsigned long)'p'<<8)|((unsigned long)'a'<<16), "Spanish" ),
  iso639("su\0",((unsigned long)'s')|((unsigned long)'u'<<8), "Sudanese" ),
  iso639("sun",((unsigned long)'s')|((unsigned long)'u'<<8)|((unsigned long)'n'<<16), "Sudanese" ),
  iso639("suk",((unsigned long)'s')|((unsigned long)'u'<<8)|((unsigned long)'k'<<16), "Sukuma" ),
  iso639("sux",((unsigned long)'s')|((unsigned long)'u'<<8)|((unsigned long)'x'<<16), "Sumerian" ),
  iso639("sus",((unsigned long)'s')|((unsigned long)'u'<<8)|((unsigned long)'s'<<16), "Susu" ),
  iso639("sw\0",((unsigned long)'s')|((unsigned long)'w'<<8), "Swahili" ),
  iso639("swa",((unsigned long)'s')|((unsigned long)'w'<<8)|((unsigned long)'a'<<16), "Swahili" ),
  iso639("ssw",((unsigned long)'s')|((unsigned long)'s'<<8)|((unsigned long)'w'<<16), "Swazi" ),
  iso639("sv\0",((unsigned long)'s')|((unsigned long)'v'<<8), "Swedish" ),
  iso639("sve",((unsigned long)'s')|((unsigned long)'v'<<8)|((unsigned long)'e'<<16), "Swedish" ),
  iso639("swe",((unsigned long)'s')|((unsigned long)'w'<<8)|((unsigned long)'e'<<16), "Swedish" ),
  iso639("syr",((unsigned long)'s')|((unsigned long)'y'<<8)|((unsigned long)'r'<<16), "Syriac" ),
  iso639("tgl",((unsigned long)'t')|((unsigned long)'g'<<8)|((unsigned long)'l'<<16), "Tagalog" ),
  iso639("tl\0",((unsigned long)'t')|((unsigned long)'l'<<8), "Tagalog" ),
  iso639("tah",((unsigned long)'t')|((unsigned long)'a'<<8)|((unsigned long)'h'<<16), "Tahitian" ),
  iso639("tg\0",((unsigned long)'t')|((unsigned long)'g'<<8), "Tajik" ),
  iso639("tgk",((unsigned long)'t')|((unsigned long)'g'<<8)|((unsigned long)'k'<<16), "Tajik" ),
  iso639("tmh",((unsigned long)'t')|((unsigned long)'m'<<8)|((unsigned long)'h'<<16), "Tamashek" ),
  iso639("ta\0",((unsigned long)'t')|((unsigned long)'a'<<8), "Tamil" ),
  iso639("tam",((unsigned long)'t')|((unsigned long)'a'<<8)|((unsigned long)'m'<<16), "Tamil" ),
  iso639("tat",((unsigned long)'t')|((unsigned long)'a'<<8)|((unsigned long)'t'<<16), "Tatar" ),
  iso639("tt\0",((unsigned long)'t')|((unsigned long)'t'<<8), "Tatar" ),
  iso639("te\0",((unsigned long)'t')|((unsigned long)'e'<<8), "Tegulu" ),
  iso639("tel",((unsigned long)'t')|((unsigned long)'e'<<8)|((unsigned long)'l'<<16), "Telugu" ),
  iso639("ter",((unsigned long)'t')|((unsigned long)'e'<<8)|((unsigned long)'r'<<16), "Tereno" ),
  iso639("th\0",((unsigned long)'t')|((unsigned long)'h'<<8), "Thai" ),
  iso639("tha",((unsigned long)'t')|((unsigned long)'h'<<8)|((unsigned long)'a'<<16), "Thai" ),
  iso639("bo\0",((unsigned long)'b')|((unsigned long)'o'<<8), "Tibetan" ),
  iso639("bod",((unsigned long)'b')|((unsigned long)'o'<<8)|((unsigned long)'d'<<16), "Tibetan" ),
  iso639("tib",((unsigned long)'t')|((unsigned long)'i'<<8)|((unsigned long)'b'<<16), "Tibetan" ),
  iso639("tig",((unsigned long)'t')|((unsigned long)'i'<<8)|((unsigned long)'g'<<16), "Tigre" ),
  iso639("ti\0",((unsigned long)'t')|((unsigned long)'i'<<8), "Tigrinya" ),
  iso639("tir",((unsigned long)'t')|((unsigned long)'i'<<8)|((unsigned long)'r'<<16), "Tigrinya" ),
  iso639("tem",((unsigned long)'t')|((unsigned long)'e'<<8)|((unsigned long)'m'<<16), "Timne" ),
  iso639("tiv",((unsigned long)'t')|((unsigned long)'i'<<8)|((unsigned long)'v'<<16), "Tivi" ),
  iso639("tli",((unsigned long)'t')|((unsigned long)'l'<<8)|((unsigned long)'i'<<16), "Tlingit" ),
  iso639("tog",((unsigned long)'t')|((unsigned long)'o'<<8)|((unsigned long)'g'<<16), "Tonga (Nyasa)" ),
  iso639("to\0",((unsigned long)'t')|((unsigned long)'o'<<8), "Tonga" ),
  iso639("ton",((unsigned long)'t')|((unsigned long)'o'<<8)|((unsigned long)'n'<<16), "Tonga (Tonga Islands)" ),
  iso639("tru",((unsigned long)'t')|((unsigned long)'r'<<8)|((unsigned long)'u'<<16), "Truk" ),
  iso639("tsi",((unsigned long)'t')|((unsigned long)'s'<<8)|((unsigned long)'i'<<16), "Tsimshian" ),
  iso639("ts\0",((unsigned long)'t')|((unsigned long)'s'<<8), "Tsonga" ),
  iso639("tso",((unsigned long)'t')|((unsigned long)'s'<<8)|((unsigned long)'o'<<16), "Tsonga" ),
  iso639("tsn",((unsigned long)'t')|((unsigned long)'s'<<8)|((unsigned long)'n'<<16), "Tswana" ),
  iso639("tum",((unsigned long)'t')|((unsigned long)'u'<<8)|((unsigned long)'m'<<16), "Tumbuka" ),
  iso639("ota",((unsigned long)'o')|((unsigned long)'t'<<8)|((unsigned long)'a'<<16), "Turkish, Ottoman (1500 - 1928)" ),
  iso639("tr\0",((unsigned long)'t')|((unsigned long)'r'<<8), "Turkish" ),
  iso639("tur",((unsigned long)'t')|((unsigned long)'u'<<8)|((unsigned long)'r'<<16), "Turkish" ),
  iso639("tk\0",((unsigned long)'t')|((unsigned long)'k'<<8), "Turkmen" ),
  iso639("tuk",((unsigned long)'t')|((unsigned long)'u'<<8)|((unsigned long)'k'<<16), "Turkmen" ),
  iso639("tyv",((unsigned long)'t')|((unsigned long)'y'<<8)|((unsigned long)'v'<<16), "Tuvinian" ),
  iso639("tw\0",((unsigned long)'t')|((unsigned long)'w'<<8), "Twi" ),
  iso639("twi",((unsigned long)'t')|((unsigned long)'w'<<8)|((unsigned long)'i'<<16), "Twi" ),
  iso639("uga",((unsigned long)'u')|((unsigned long)'g'<<8)|((unsigned long)'a'<<16), "Ugaritic" ),
  iso639("uig",((unsigned long)'u')|((unsigned long)'i'<<8)|((unsigned long)'g'<<16), "Uighur" ),
  iso639("uk\0",((unsigned long)'u')|((unsigned long)'k'<<8), "Ukrainian" ),
  iso639("ukr",((unsigned long)'u')|((unsigned long)'k'<<8)|((unsigned long)'r'<<16), "Ukrainian" ),
  iso639("umb",((unsigned long)'u')|((unsigned long)'m'<<8)|((unsigned long)'b'<<16), "Umbundu" ),
  iso639("und",((unsigned long)'u')|((unsigned long)'n'<<8)|((unsigned long)'d'<<16), "Undetermined" ),
  iso639("ur\0",((unsigned long)'u')|((unsigned long)'r'<<8), "Urdu" ),
  iso639("urd",((unsigned long)'u')|((unsigned long)'r'<<8)|((unsigned long)'d'<<16), "Urdu" ),
  iso639("uz\0",((unsigned long)'u')|((unsigned long)'z'<<8), "Uzbek" ),
  iso639("uzb",((unsigned long)'u')|((unsigned long)'z'<<8)|((unsigned long)'b'<<16), "Uzbek" ),
  iso639("vai",((unsigned long)'v')|((unsigned long)'a'<<8)|((unsigned long)'i'<<16), "Vai" ),
  iso639("ven",((unsigned long)'v')|((unsigned long)'e'<<8)|((unsigned long)'n'<<16), "Venda" ),
  iso639("vi\0",((unsigned long)'v')|((unsigned long)'i'<<8), "Vietnamese" ),
  iso639("vie",((unsigned long)'v')|((unsigned long)'i'<<8)|((unsigned long)'e'<<16), "Vietnamese" ),
  iso639("vo\0",((unsigned long)'v')|((unsigned long)'o'<<8), "Volapuk" ),
  iso639("vol",((unsigned long)'v')|((unsigned long)'o'<<8)|((unsigned long)'l'<<16), "Volapük" ),
  iso639("vot",((unsigned long)'v')|((unsigned long)'o'<<8)|((unsigned long)'t'<<16), "Votic" ),
  iso639("wak",((unsigned long)'w')|((unsigned long)'a'<<8)|((unsigned long)'k'<<16), "Wakashan languages" ),
  iso639("wal",((unsigned long)'w')|((unsigned long)'a'<<8)|((unsigned long)'l'<<16), "Walamo" ),
  iso639("war",((unsigned long)'w')|((unsigned long)'a'<<8)|((unsigned long)'r'<<16), "Waray" ),
  iso639("was",((unsigned long)'w')|((unsigned long)'a'<<8)|((unsigned long)'s'<<16), "Washo" ),
  iso639("cy\0",((unsigned long)'c')|((unsigned long)'y'<<8), "Welsh" ),
  iso639("cym",((unsigned long)'c')|((unsigned long)'y'<<8)|((unsigned long)'m'<<16), "Welsh" ),
  iso639("wel",((unsigned long)'w')|((unsigned long)'e'<<8)|((unsigned long)'l'<<16), "Welsh" ),
  iso639("wo\0",((unsigned long)'w')|((unsigned long)'o'<<8), "Wolof" ),
  iso639("wol",((unsigned long)'w')|((unsigned long)'o'<<8)|((unsigned long)'l'<<16), "Wolof" ),
  iso639("xh\0",((unsigned long)'x')|((unsigned long)'h'<<8), "Xhosa" ),
  iso639("xho",((unsigned long)'x')|((unsigned long)'h'<<8)|((unsigned long)'o'<<16), "Xhosa" ),
  iso639("sah",((unsigned long)'s')|((unsigned long)'a'<<8)|((unsigned long)'h'<<16), "Yakut" ),
  iso639("yao",((unsigned long)'y')|((unsigned long)'a'<<8)|((unsigned long)'o'<<16), "Yao" ),
  iso639("yap",((unsigned long)'y')|((unsigned long)'a'<<8)|((unsigned long)'p'<<16), "Yap" ),
  iso639("ji\0",((unsigned long)'j')|((unsigned long)'i'<<8), "Yiddish" ),
  iso639("yid",((unsigned long)'y')|((unsigned long)'i'<<8)|((unsigned long)'d'<<16), "Yiddish" ),
  iso639("yo\0",((unsigned long)'y')|((unsigned long)'o'<<8), "Yoruba" ),
  iso639("yor",((unsigned long)'y')|((unsigned long)'o'<<8)|((unsigned long)'r'<<16), "Yoruba" ),
  iso639("zap",((unsigned long)'z')|((unsigned long)'a'<<8)|((unsigned long)'p'<<16), "Zapotec" ),
  iso639("zen",((unsigned long)'z')|((unsigned long)'e'<<8)|((unsigned long)'n'<<16), "Zenaga" ),
  iso639("zha",((unsigned long)'z')|((unsigned long)'h'<<8)|((unsigned long)'a'<<16), "Zhuang" ),
  iso639("zu\0",((unsigned long)'z')|((unsigned long)'u'<<8), "Zulu" ),
  iso639("zul",((unsigned long)'z')|((unsigned long)'u'<<8)|((unsigned long)'l'<<16), "Zulu" ),
  iso639("zun",((unsigned long)'z')|((unsigned long)'u'<<8)|((unsigned long)'n'<<16), "Zuni" ) };
  static const int size=(int)(sizeof(table)/sizeof(iso639));
  const char *retval=xlanguage;
  int len=strlen(xlanguage);
  if((len > 1)&&(len < 4))
  {
    const unsigned long id=((unsigned long)tolower(xlanguage[0]))
     |((unsigned long)tolower(xlanguage[1])<<8)
     |((unsigned long)tolower(xlanguage[3])<<16);
    for(int i=start;i<size;++i)
    {
      if(id == table[i].id)
      {
        const char *language=table[i].language;
        const int k=i;
        for(int j=k-1;
          j>=0&&!strcmp(xlanguage,table[j].language);
          --j)
        {
          const char *jlanguage=table[j].language;
          if(strcmp(jlanguage,xlanguage))
            break;
          if(!table[j].name[2])
          {
            i=j;
            len=2;
          }else if(len == 3)
          {
            i=j;
          }
        }
        if(len>2)
        {
          for(int j=k+1;j>0&&!strcmp(language,table[j].language);++j)
          {
            if(!table[j].name[2])
            {
              i=j;
              len=2;
              break;
            }
          }
        }
        retval=table[i].name;
        break;
      }
    }
  }else
  {
    int minGuess=start;
    int maxGuess=size+1;
    while(minGuess<maxGuess)
    {
      const int guess=minGuess+((maxGuess-minGuess)/2);
      const int i=strcmp(xlanguage,table[guess].language);
      if(i<0)
      {
        maxGuess=guess;
      }else if(i)
      {
        minGuess=guess+1;
      }else
      {
        retval=lookup_id(table[guess].name,guess);
        break;
      }
    }
  }
  return retval;
}

