//C-  -*- C++ -*-
//C-
//C- This software contains software code or other information proprietary to
//C- AT&T Corp. ("AT&T").  Unless you accept a license to use this software,
//C- you shall not use, execute, compile, modify, redistribute, reverse
//C- compile, disassemble, or otherwise reverse engineer the AT&T software or
//C- any derived work of the AT&T software.  The text of a license can be
//C- found in file "ATTLICENSE" or at the Internet website having the URL
//C- "http://www.djvu.att.com/open".
//C-
//C- This software is provided to you "AS IS".  YOU "AS IS".  YOU ASSUME TOTAL
//C- RESPONSIBILITY AND RISK FOR YOUR USE OF THEM INCLUDING THE RISK OF ANY
//C- DEFECTS OR INACCURACIES THEREIN.  AT&T DOES NOT MAKE, AND EXPRESSLY
//C- DISCLAIMS, ANY EXPRESS OR IMPLIED WARRANTIES OF ANY KIND WHATSOEVER,
//C- INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTIES OF MERCHANTABILITY
//C- OR FITNESS FOR A PARTICULAR PURPOSE, WARRANTIES OF TITLE OR
//C- NON-INFRINGEMENT OF ANY INTELLECTUAL PROPERTY RIGHTS OR TRADEMARK RIGHTS,
//C- ANY WARRANTIES ARISING BY USAGE OF TRADE, COURSE OF DEALING OR COURSE OF
//C- PERFORMANCE, OR ANY WARRANTY THAT THE AT&T SOURCE CODE RELEASE OR AT&T
//C- CAPSULE ARE "ERROR FREE" WILL MEET YOUR REQUIREMENTS.
//C-
//C-     (C) AT&T Corp. All rights reserved.
//C-     AT&T is a registered trademark of AT&T Corp.
//C-
//C- $Id: ATTLicense.h,v 1.7 1999-03-16 20:21:27 leonb Exp $

#ifndef _ATTLICENSE_H_
#define _ATTLICENSE_H_

/** @name ATTLicense.h
    
    Files #"ATTLicense.h"# and #"ATTLicense.cpp"# implement a few simple
    functions for obtaining and displaying all sort of legal texts related
    to the \Ref{AT&T Source Code Agreement}.

    @memo
    Support for AT&T Source Code Licensing
    @version
    #$Id: ATTLicense.h,v 1.7 1999-03-16 20:21:27 leonb Exp $#
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
