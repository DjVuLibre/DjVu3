//C-  -*- C++ -*-
//C-
//C- Copyright (c) 1999 AT&T Corp.  All rights reserved.
//C-
//C- This software may only be used by you under license from AT&T
//C- Corp. ("AT&T"). A copy of AT&T's Source Code Agreement is available at
//C- AT&T's Internet website having the URL <http://www.djvu.att.com/open>.
//C- If you received this software without first entering into a license with
//C- AT&T, you have an infringing copy of this software and cannot use it
//C- without violating AT&T's intellectual property rights.
//C-
//C- $Id: DjVuDumpHelper.h,v 1.5 2000-10-04 01:38:01 bcr Exp $

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
    #$Id: DjVuDumpHelper.h,v 1.5 2000-10-04 01:38:01 bcr Exp $# */
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
