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
//C- $Id: ATTLicense.h,v 1.8 1999-03-17 19:24:56 leonb Exp $

#ifndef _ATTLICENSE_H_
#define _ATTLICENSE_H_

/** @name ATTLicense.h
    
    Files #"ATTLicense.h"# and #"ATTLicense.cpp"# implement a few simple
    functions for obtaining and displaying all sort of legal texts related
    to the \Ref{AT&T Source Code Agreement}.

    @memo
    Support for AT&T Source Code Licensing
    @version
    #$Id: ATTLicense.h,v 1.8 1999-03-17 19:24:56 leonb Exp $#
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
