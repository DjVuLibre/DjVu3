//C-  -*- C++ -*-
//C-
//C-  Copyright (c) 1998 AT&T	
//C-  All Rights Reserved 
//C-
//C-  THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
//C-  The copyright notice above does not evidence any
//C-  actual or intended publication of such source code.
//C-
//C-  $Id: ATTLicense.h,v 1.1 1999-02-27 00:53:17 leonb Exp $

#ifndef _ATTLICENSE_H_
#define _ATTLICENSE_H_

#include "GString.h"

/** @name ATTLicense.h
    
    Files #"ATTLicense.h"# and #"ATTLicense.cpp"# implement a few simple
    functions for obtaining and displaying the text of the AT&T Source Code
    License.  This is just to make sure that this text is not replicated a
    million times in the code.

    @memo
    Support for AT&T Source Code Licensing
    @version
    #$Id: ATTLicense.h,v 1.1 1999-02-27 00:53:17 leonb Exp $#
    @authors
    Leon Bottou <leonb@research.att.com>\\
    Jeffrey S. Dickey <jsdickey@att.com>\\
    James Tramontana <jtramontana@att.com>\\
    Marya Yee <myee@yeellp.com> */

/** Functions for displaying license information.
 */
class ATTLicense
{
public:
  /** Returns a constant pointer to the license text. */
  static GString get_license_text();
  /** Returns a constant pointer to the copyright notice. */
  static GString get_copyright_text();
  /** Returns a constant pointer to the usage fragment. */
  static GString get_usage_text(char *program);
  /** Parse license argument.  If the command line contains #"-license"#, 
      this function shows the AT&T Source Code license and exits. */
  static void process_cmdline(int argc, char **argv);
};


// --------- END
#endif
