//C-  -*- C++ -*-
//C- DjVu® Reference Library (v. 3.0)
//C- 
//C- Copyright © 1999-2000 LizardTech, Inc. All Rights Reserved.
//C- 
//C- This software (the "Original Code") is subject to, and may be
//C- distributed under, the GNU General Public License, Version 2.
//C- The license should have accompanied the Original Code or you
//C- may obtain a copy of the license from the Free Software
//C- Foundation at http://www.fsf.org .
//C- 
//C- With respect to the Original Code, and subject to any third
//C- party intellectual property claims, LizardTech grants recipient
//C- a worldwide, royalty-free, non-exclusive license under patent
//C- claims infringed by making, using, or selling Original Code
//C- which are now or hereafter owned or controlled by LizardTech,
//C- but solely to the extent that any such patent is reasonably
//C- necessary to enable you to make, have made, practice, sell, or 
//C- otherwise dispose of Original Code (or portions thereof) and
//C- not to any greater extent that may be necessary to utilize
//C- further modifications or combinations.
//C- 
//C- The Original Code is provided "AS IS" WITHOUT WARRANTY OF ANY
//C- KIND, EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO
//C- ANY WARRANTY OF NON-INFRINGEMENT, OR ANY IMPLIED WARRANTY OF 
//C- MERCHANTIBILITY OF FITNESS FOR A PARTICULAR PURPOSE.
// 
// $Id: DjVuDumpHelper.h,v 1.6 2000-11-02 01:08:34 bcr Exp $
// $Name:  $


#ifndef _DJVUDUMPHELPER_H
#define _DJVUDUMPHELPER_H


/** @name DjVuDupmHelper.h
    This file contains code capable of generating information on
    DjVu documents without actually decoding them. The code has
    been extracted from a command line utility \Ref{djvudump.cpp}
    for use in the DjVu plugin.
    @memo
    DjVu Dump Helper.
    @author
    L\'eon Bottou <leonb@research.att.com>,
    Andrei Erofeev <eaf@geocities.com>
    @version
    #$Id: DjVuDumpHelper.h,v 1.6 2000-11-02 01:08:34 bcr Exp $# */
//@{


#ifdef __GNUC__
#pragma interface
#endif

#include "DataPool.h"

/** DjVuDumpHelper.
    This class can dump information on any DjVu file without decoding it.
    Based upon old \Ref{djvudump.cpp} code.
 */

class DjVuDumpHelper
{
public:
      /// Default constructor
   DjVuDumpHelper(void) {}
      /// Destructor
   ~DjVuDumpHelper(void) {}
      /** Interprets the file passed in the \Ref{DataPool}, and returns
	  the results in \Ref{ByteStream}. */
   GP<ByteStream>	dump(const GP<DataPool> & pool);
      /** Interprets the file passed in the \Ref{ByteStream}, and returns
	  the results in \Ref{ByteStream}. */
   GP<ByteStream>	dump(ByteStream & str);
};


//@}

// ----- THE END
#endif
