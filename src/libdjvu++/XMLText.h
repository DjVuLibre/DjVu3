//C-  -*- C++ -*-
//C- DjVu® Reference Library (v. 3.0)
//C- 
//C- Copyright © 1999-2001 LizardTech, Inc. All Rights Reserved.
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
// $Id: XMLText.h,v 1.1 2001-04-24 17:54:09 jhayes Exp $
// $Name:  $

#ifndef _LT_XMLTEXT__
#define _LT_XMLTEXT__

#ifdef __GNUC__
#pragma interface
#endif

#include "XMLEdit.h"

// this class writes in a new text layer base on the xml input. 
// 
//  note: <HIDDENTEXT> object without any text will remove the text
//  from that page.

class lt_XMLText : public lt_XMLEdit
{
protected:
  lt_XMLText(void) {}
public:
  /// Default creator
  static GP<lt_XMLText> create(void) { return new lt_XMLText(); }
  /// Parse the specified tags.
  void parse(const lt_XMLTags &tags);
  /// write to disk.
protected:
  void ChangeText(const lt_XMLTags &text,const GURL &url,const GUTF8String &id,
    const GUTF8String &width,const GUTF8String &height);

};

#endif /* _LT_XMLTEXT__ */


