//C-  -*- C++ -*-
//C- DjVu� Reference Library (v. 3.0)
//C- 
//C- Copyright � 2000-2001 LizardTech, Inc. All Rights Reserved.
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
// $Id: DjVuMessageLite.cpp,v 1.1 2001-05-09 00:38:26 bcr Exp $
// $Name:  $

#ifdef __GNUC__
#pragma implementation
#endif

#include "DjVuMessageLite.h"
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

const DjVuMessageLite& (*DjVuMessageLite::create)(void)=DjVuMessageLite::create_lite; 

static const char *failed_to_parse_XML=ERR_MSG("DjVuMessage.failed_to_parse_XML");

static const char *unrecognized=ERR_MSG("DjVuMessage.Unrecognized");
static const char *uparameter=ERR_MSG("DjVuMessage.Parameter");
static const char unrecognized_default[] =
  "** Unrecognized DjVu Message: [Contact LizardTech for assistance]\n"
  "\t** Message name:  %1!s!";
static const char uparameter_default[]="\t   Parameter: %1!s!";
static const char failed_to_parse_XML_default[]=
  "Failed to parse XML message file:&#10;&#09;&apos;%1!s!&apos;.";

static const char namestring[]="name";
static const char valuestring[]="value";
static const char numberstring[]="number";
static const char bodystring[]="BODY";
static const char headstring[]="HEAD";
static const char includestring[]="INCLUDE";
static const char messagestring[]="MESSAGE";

GPList<ByteStream> &
DjVuMessageLite::getByteStream(void)
{
  static GPList<ByteStream> gbs;
  return gbs;
}

GP<DjVuMessageLite> &
DjVuMessageLite::getDjVuMessageLite(void)
{
  static GP<DjVuMessageLite> message;
  return message;
}

void
DjVuMessageLite::AddByteStreamLater(const GP<ByteStream> &bs)
{
  getByteStream().append(bs);
}

//  There is only object of class DjVuMessage in a program, and here it is:
//DjVuMessage  DjVuMsg;
const DjVuMessageLite &
DjVuMessageLite::create_lite(void)
{
  GP<DjVuMessageLite> &static_message=getDjVuMessageLite();
  if(!static_message)
  {
    static_message=new DjVuMessageLite;
  }
  DjVuMessageLite &m=*static_message;
  GPList<ByteStream> &bs = getByteStream();
  for(GPosition pos;(pos=bs);bs.del(pos))
  {
    m.AddByteStream(bs[pos]);
  }
  return m;
}

// Constructor
DjVuMessageLite::DjVuMessageLite( void ) {}

// Destructor
DjVuMessageLite::~DjVuMessageLite( ) {}


void
DjVuMessageLite::perror( const GUTF8String & MessageList )
{
  DjVuPrintError("%s\n",(const char *)DjVuMessageLite::LookUpUTF8(MessageList));
}


//  Expands message lists by looking up the message IDs and inserting
//  arguments into the retrieved messages.
//  N.B. The resulting string may be encoded in UTF-8 format (ISO 10646-1 Annex R)
//       and SHOULD NOT BE ASSUMED TO BE ASCII.
GUTF8String
DjVuMessageLite::LookUp( const GUTF8String & MessageList ) const
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
DjVuMessageLite::LookUpSingle( const GUTF8String &Single_Message ) const
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
DjVuMessageLite::LookUpID( const GUTF8String &msgID,
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
DjVuMessageLite::InsertArg( GUTF8String &message,
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
void DjVuMessageLite_LookUp( char *msg_buffer, const unsigned int buffer_size, const char *message )
{
  GUTF8String converted = DjVuMessageLite::LookUpUTF8( message );
  if( converted.length() >= buffer_size )
    msg_buffer[0] = '\0';
  else
    strcpy( msg_buffer, converted );
}

void
DjVuMessageLite::AddByteStream(const GP<ByteStream> &bs)
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

