//C-  -*- C++ -*-
//C- DjVu� Reference Library (v. 3.5)
//C- 
//C- Copyright � 1999-2001 LizardTech, Inc. All Rights Reserved.
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
// $Id: DjVuDumpHelper.h,v 1.15 2001-10-17 18:56:46 docbill Exp $
// $Name:  $

#ifndef _DJVUDUMPHELPER_H
#define _DJVUDUMPHELPER_H

#include "GSmartPointer.h"

class DataPool;
class ByteStream;


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
    #$Id: DjVuDumpHelper.h,v 1.15 2001-10-17 18:56:46 docbill Exp $# */
//@{


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
   GP<ByteStream>	dump(GP<ByteStream> str);
};


//@}

// ----- THE END
#endif
