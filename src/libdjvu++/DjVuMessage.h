/*
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
// $Id: DjVuMessage.h,v 1.14 2001-04-04 22:12:11 bcr Exp $
// $Name:  $
*/



#ifndef __DJVU_MESSAGE_H__
#define __DJVU_MESSAGE_H__


#ifdef __cplusplus

#include "GString.h"
// class DjVuParseOptions;

class lt_XMLTags;

class DjVuMessage
{
private:
  // Constructor:
  DjVuMessage( void );
  GMap<GString,GP<lt_XMLTags> > Map;
//  DjVuParseOptions *opts;

public:

  static const DjVuMessage &create(void);

  // Destructor: Does any necessary cleanup. Actions depend on how the message
  //    file is implemented.
  ~DjVuMessage();

  //  Exception causes and external messages are passed as message lists which
  //  have the following syntax:
  //
  //  message_list ::= single_message |
  //                   single_message separator message_list
  //
  //  separator ::= newline |
  //                newline | separator
  //
  //  single_message ::= message_ID |
  //                     message_ID parameters
  //
  //  parameters ::= tab string |
  //                 tab string parameters
  //
  //  Message_IDs are looked up an external file and replaced by the message text
  //  strings they are mapped to. The message text may contain the following:
  //
  //    Parameter specifications: These are modelled after printf format
  //      specifications and have one of the following forms:
  //
  //            %#n#s            %#n#d            %#n#x
  //
  //      where n is the parameter number. The parameter number is indicated
  //      explicitly to allow for the possibility that the parameter order may
  //      change when the message text is translated into another language.
  //      The final letter ('s', 'd', or 'x') indicates the form of the parameter (string,
  //      integer or hexadecimal, respectively). But, you say, all the parameters are strings!
  //      The form is indicated in case there is a necessity to change the appearance
  //      (especially of numbers) when translating the message.
  //
  //    Formatting strings: The message text may also contain formatting strings of 
  //      following forms:
  //
  //            "\\n"         [that is, a backslash followed by the letter 'n']
  //            "\\t"         [backslash 't']
  //
  //      After parameters have been inserted in the message text, the formatting 
  //      strings are replaced by their usual equivalents (newline and tab respectively).
  //
  //  If a message_id cannot be found in the external file, a message text is fabricated
  //  giving the message_id and the parameters (if any).
  //
  //  Separators (newlines) are preserved in the translated message list.

//----------------------------------------------------------------------------------

  //  Expands message lists by looking up the message IDs and inserting
  //  arguments into the retrieved messages.
  //  N.B. The resulting string may be encoded in UTF-8 format (ISO 10646-1 Annex R)
  //       and SHOULD NOT BE ASSUMED TO BE ASCII.
  GString LookUp( const GString & MessageList ) const;

  // Same as LookUp, but this is a static method.
  static GString LookUpUTF8( const GString & MessageList )
  { return DjVuMessage::create().LookUp(MessageList); }

  // Same as Lookup, but returns the a multibyte character string in the
  // current locale.
  static GString LookUpNative( const GString & MessageList )
  { return DjVuMessage::create().LookUp(MessageList).getUTF82Native(); }

  // This is a simple alias to the above class, but does an fprintf to stderr.
  void perror( const GString & MessageList ) const;

private:

  //  Looks up the msgID in the file of messages. The strings message_text and
  //  message_number are returned if found. If not found, these strings are empty.
  void LookUpID( const GString & msgID, GString &message_text, GString &message_number ) const;

  //  Expands a single message and inserts the arguments. Single_Message contains no
  //  separators (newlines), but includes all the parameters separated by tabs.
  GString LookUpSingle( const GString & Single_Message ) const;

  //  Insert a string into the message text. Will insert into any field description.
  //  Except for an ArgId of zero (message number), if the ArgId is not found, the
  //  routine adds a line with the parameter so information will not be lost.
  void InsertArg( GString &message, int ArgId, GString arg ) const;
};


//  There is only object of class CDjVuMessage in a program, and here it is (the actual
//  object is in DjVuMessage.cpp).
// extern DjVuMessage  DjVuMsg;
#define DjVuMsg DjVuMessage::create()

#endif /* __cplusplus */


/*
//  A C function to perform a message lookup. Arguments are a buffer to received the
//  translated message, a buffer size (bytes), and a message_list. The translated
//  result is returned in msg_buffer encoded in UTF-8. In case of error, msg_buffer is
//  empty (i.e., msg_buffer[0] == '\0').
*/
#ifdef __cplusplus
extern "C" 
#endif
  void DjVuMessage_LookUp( char *msg_buffer, const unsigned int buffer_size, const char *message ); 


#endif /* __DJVU_MESSAGE_H__ */
