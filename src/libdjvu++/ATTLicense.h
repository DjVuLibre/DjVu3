//C-  -*- C++ -*-
//C-
//C-  Copyright (c) 1998 AT&T	
//C-  All Rights Reserved 
//C-
//C-  THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
//C-  The copyright notice above does not evidence any
//C-  actual or intended publication of such source code.
//C-
//C-  $Id: ATTLicense.h,v 1.5 1999-03-15 18:22:13 leonb Exp $

#ifndef _ATTLICENSE_H_
#define _ATTLICENSE_H_

/** @name ATTLicense.h
    
    Files #"ATTLicense.h"# and #"ATTLicense.cpp"# implement a few simple
    functions for obtaining and displaying all sort of legal texts related
    to the \Ref{AT&T Source Code Agreement}.

    @memo
    Support for AT&T Source Code Licensing
    @version
    #$Id: ATTLicense.h,v 1.5 1999-03-15 18:22:13 leonb Exp $#
    @author
    L\'eon Bottou <leonb@research.att.com>\\
    Jeffrey S. Dickey <jsdickey@att.com>\\
    James Tramontana <jtramontana@att.com>\\
    Marya Yee <myee@yeellp.com> */
//@{

/** Functions for displaying license information. */
class ATTLicense
{
public:
  /** Returns a constant pointer to the copyright notice text. */
  static const char* get_notice_text();
  /** Returns a constant pointer to the copyright notice. */
  static const char* get_copyright_text();
  /** Returns a constant pointer to the usage fragment. */
  static const char* get_usage_text();
  /** Parse license argument.  If the command line contains #"-license"#, 
      this function shows the AT&T Source Code license and exits. */
  static void process_cmdline(int argc, char **argv);
};

//@}

// --------- END
#endif
