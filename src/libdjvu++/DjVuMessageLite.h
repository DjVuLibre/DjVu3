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
// $Id: DjVuMessageLite.h,v 1.1 2001-05-09 00:38:26 bcr Exp $
// $Name:  $



#ifndef __DJVU_MESSAGE_LITE_H__
#define __DJVU_MESSAGE_LITE_H__


#include "GString.h"
class lt_XMLTags;
class ByteStream;

/** Exception causes and external messages are passed as message lists which
    have the following syntax:
  
    message_list ::= single_message |
                     single_message separator message_list
    
    separator ::= newline |
                  newline | separator
    
    single_message ::= message_ID |
                       message_ID parameters
    
    parameters ::= tab string |
                   tab string parameters
    
    Message_IDs are looked up an external file and replaced by the message
    text strings they are mapped to. The message text may contain the
    following:
    
    Parameter specifications: These are modelled after printf format
    specifications and have one of the following forms:
  
      %n!s! %n!d! %n!i! %n!u! %n!x! %n!X!
  
    where n is the parameter number. The parameter number is indicated
    explicitly to allow for the possibility that the parameter order may
    change when the message text is translated into another language.
    The final letter ('s', 'd', 'i', 'u', 'x', or 'X') indicates the form
    of the parameter (string, integer, integer, unsigned integer, lowercase
    hexadecimal, or uppercase hexadecimal, respectively).  In addition
    formating options such as precision available in sprintf, may be used.
    So, to print the third argument as 5 digit unsigned number, with leading
    zero's one could use:
      %3!05u!

    All of the arguments are actually strings.  For forms that don't take
    string input ('d', 'i', 'u', 'x', or 'X'), and atoi() conversion is done
    on the string before formatting.  In addition the form indicates to the
    translater whether to expect a word or a number.

    The strings are read in from XML.  To to format the strings, use the
    relavent XML escape sequences, such as follows:

            &#10;        [line feed]
            &#09;        [horizontal tab]
            &apos;       [single quote]
            &#34;        [double quote]
            &lt;         [less than sign]
            &gt;         [greater than sign]
  
    After parameters have been inserted in the message text, the formatting 
    strings are replaced by their usual equivalents (newline and tab
    respectively).

    If a message_id cannot be found in the external file, a message text
    is fabricated giving the message_id and the parameters (if any).

    Separators (newlines) are preserved in the translated message list.

    Expands message lists by looking up the message IDs and inserting
    arguments into the retrieved messages.

    N.B. The resulting string may be encoded in UTF-8 format (ISO 10646-1
    Annex R) and SHOULD NOT BE ASSUMED TO BE ASCII.
  */

class DjVuMessageLite : public GPEnabled
{
protected:
  // Constructor:
  DjVuMessageLite( void );
  GMap<GUTF8String,GP<lt_XMLTags> > Map;
  GUTF8String errors;
  /// Creates a DjVuMessage class.
  static const DjVuMessageLite &real_create(void);

public:
  /// Creates a DjVuMessage class.
  static const DjVuMessageLite& (*create)(void);

  /// Creates this class specifically.
  static const DjVuMessageLite &create_lite(void);

  /** Adds a byte stream to be parsed whenever the next DjVuMessage::create()
      call is made. */
  static void AddByteStreamLater(const GP<ByteStream> &bs);

  /** Destructor: Does any necessary cleanup. Actions depend on how the message
      file is implemented. */
  ~DjVuMessageLite();

  /// Lookup the relavent string and parse the message.
  GUTF8String LookUp( const GUTF8String & MessageList ) const;

  //// Same as LookUp, but this is a static method.
  static GUTF8String LookUpUTF8( const GUTF8String & MessageList )
  { return create().LookUp(MessageList); }

  /** Same as Lookup, but returns the a multibyte character string in the
      current locale. */
  static GNativeString LookUpNative( const GUTF8String & MessageList )
  { return create().LookUp(MessageList).getUTF82Native(); }

  /// This is a simple alias to the above class, but does an fprintf to stderr.
  static void perror( const GUTF8String & MessageList );

protected:

  /*  Looks up the msgID in the file of messages. The strings message_text
      and message_number are returned if found. If not found, these strings
      are empty. */
  void LookUpID( const GUTF8String & msgID,
    GUTF8String &message_text, GUTF8String &message_number ) const;

  /*  Expands a single message and inserts the arguments. Single_Message
      contains no separators (newlines), but includes all the parameters
      separated by tabs. */
  GUTF8String LookUpSingle( const GUTF8String & Single_Message ) const;

  /*  Insert a string into the message text. Will insert into any field
      description.  Except for an ArgId of zero (message number), if the
      #ArgId# is not found, the routine adds a line with the parameter
      so information will not be lost. */
  void InsertArg(
    GUTF8String &message, const int ArgId, const GUTF8String &arg ) const;

  void AddByteStream(const GP<ByteStream> &bs);

protected:
  /*  Static storage of the DjVuMessage class. */
  static GP<DjVuMessageLite> &getDjVuMessageLite(void);
  /*  Static storage of the ByteStream list. */
  static GPList<ByteStream> &getByteStream(void);
};

#endif /* __DJVU_MESSAGE_LITE_H__ */

