//  DjVuMessage.h
//
//  Author:   Fred Crary
//
//  Purpose:  A collection point for all the messages generated by DjVu.



#if !defined(__DJVU_MESSAGE_H__)
#define __DJVU_MESSAGE_H__

#include <stdio.h>
#include "GString.h"


class DjVuMessage
{
public:

  // Constructor:
  DjVuMessage( void );

  // Destructor: Does any necessary cleanup. Actions depend on how the message
  //    file is implemented.
  ~DjVuMessage( );

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
  GString LookUp( GString MessageList );

private:

  //  Looks up the msgID in the file of messages and returns a pointer to the beginning
  //  of the translated message, if found; and an empty string otherwise.
  GString LookUpID( GString msgID );

  //  Expands a single message and inserts the arguments. Single_Message contains no
  //  separators (newlines), but includes all the parameters.
  GString LookUpSingle( GString Single_Message );

  //  Insert a string into the message text. Will insert into any field description.
  //  If the ArgId is not found, adds a line with the parameter so information will
  //  not be lost.
  void InsertArg( GString &message, int ArgId, GString arg );
};


//  There is only object of class CDjVuMessage in a program, and here it is (the actual
//  object is in DjVuMessage.cpp).
extern DjVuMessage  DjVuMsg;

#endif // __DJVU_MESSAGE_H__