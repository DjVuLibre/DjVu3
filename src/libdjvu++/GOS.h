//C-  -*- C++ -*-
//C-
//C- This software contains software code or other information proprietary to
//C- AT&T Corp. ("AT&T").  Unless you accept a license to use this software,
//C- you shall not use, execute, compile, modify, redistribute, reverse
//C- compile, disassemble, or otherwise reverse engineer the AT&T software or
//C- any derived work of the AT&T software.  The text of a license can be
//C- found in file "ATTLICENSE" or at the Internet website having the URL
//C- "http://www.djvu.att.com/open".
//C
//C- This software is provided to you "AS IS".  YOU ASSUME TOTAL
//C- RESPONSIBILITY AND RISK FOR USE OF THE AT&T SOFTWARE.  AT&T DOES NOT
//C- MAKE, AND EXPRESSLY DISCLAIMS, ANY EXPRESS OR IMPLIED WARRANTIES OF ANY
//C- KIND WHATSOEVER, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTIES OF
//C- MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE, WARRANTIES OF TITLE
//C- OR NON-INFRINGEMENT OF ANY INTELLECTUAL PROPERTY RIGHTS, ANY WARRANTIES
//C- ARISING BY USAGE OF TRADE, COURSE OF DEALING OR COURSE OF PERFORMANCE, OR
//C- ANY WARRANTY THAT THE AT&T SOFTWARE IS ERROR FREE OR WILL MEET YOUR
//C- REQUIREMENTS.
//C-
//C-     (C) AT&T Corp. All rights reserved.
//C-     AT&T is a registered trademark of AT&T Corp.
//C-
//C- $Id: GOS.h,v 1.5 1999-03-15 18:28:51 leonb Exp $

#ifndef _GOS_H_
#define _GOS_H_

/** @name GOS.h
    Files #"GOS.h"# and #"GOS.cpp"# implement operating system 
    dependent functions with a unified interface.  All these functions
    are implemented as static member of class \Ref{GOS}. 
    Functions are provided for testing the presence of a file or a directory
    (\Ref{GOS::is_file}, \Ref{GOS::is_dir}), for manipulating file and directory names
    (\Ref{GOS::dirname}, \Ref{GOS::basename}, \Ref{GOS::expand_name},
    for obtaining and changing the current directory (\Ref{GOS::cwd}),
    for converting between file names and urls (\Ref{GOS::filename_to_url},
    \Ref{GOS::url_to_filename}), and for counting time (\Ref{GOS::ticks},
    \Ref{GOS::sleep}).
    
    @memo
    Operating System dependent functions.
    @author
    L\'eon Bottou <leonb@research.att.com> -- Initial implementation
    @version
    #$Id: GOS.h,v 1.5 1999-03-15 18:28:51 leonb Exp $#
*/
//@{

#ifdef __GNUC__
#pragma interface
#endif
#include "DjVuGlobal.h"
#include "GString.h"

/** Operating System dependent functions. */
class GOS 
{
 public:
  // -----------------------------------------
  // Functions for dealing with filenames
  // -----------------------------------------
  
  /** Returns true if #filename# exists and is a regular file. */
  static int is_file(const char *filename);
  /** Returns true if #filename# exists and is a directory. */
  static int is_dir(const char *filename);
  /** Returns the name of the parent directory of #filename#.
      This function works like the unix command #/bin/dirname#,
      but also supports the naming conventions of other operating systems. */
  static GString dirname(const char *filename);
  /** Returns the last component of file name #filename#.  If the filename
      suffix matches argument #suffix#, the filename suffix is removed.  This
      function works like the unix command #/bin/basename#, but also supports
      the naming conventions of other operating systems. */
  static GString basename(const char *filename, const char *suffix=0);
  /** Sets and returns the current working directory.
      When argument #dirname# is specified, the current directory is changed
      to #dirname#. This function always returns the fully qualified name
      of the current directory. */
  static GString cwd(const char *dirname=0);
  /** Returns fully qualified file names.  This functions constructs the fully
      qualified name of file or directory #filename#. When provided, the
      optional argument #fromdirname# is used as the current directory when
      interpreting relative specifications in #filename#.  Function
      #expand_name# is very useful for logically concatenating file names.  It
      knows which separators should be used for each operating system and it
      knows which syntactical rules apply. */
  static GString expand_name(const char *filename, const char *fromdirname=0);
  /** Deletes file or directory #filename#.
      Directories are not deleted unless the directory is empty.
      Returns a negative number if an error occurs. */
  static int deletefile(const char * filename);
  
  // -----------------------------------------
  // Functions for measuring time
  // -----------------------------------------
  
  /** Returns a number of elapsed milliseconds.  This number counts elapsed
      milliseconds since a operating system dependent date. This function is
      useful for timing code. */
  static unsigned long ticks();
  /** Sleeps during the specified time expressed in milliseconds.
      Other threads can run while the calling thread sleeps. */
  static void sleep(int milliseconds);



  // -------------------------------------------
  // Functions for converting filenames and urls
  // -------------------------------------------
  
  /** Returns a URL for accessing the file #filename#. This function normally
      constructs a standard file URL as described in RFC 1738.  
      Some versions of MSIE do not support this
      standard syntax.  A brain damaged MSIE compatible syntax is generated
      when the optional argument #useragent# contains string #"MSIE"# or
      #"Microsoft"#. */
  static GString filename_to_url(const char *filename, const char *useragent=0);
  /** Returns a filename for a URL. Argument #url# must be a legal file URL.
      This function applies heuristic rules to convert the URL into a valid
      file name. It is guaranteed that this function can properly parse all
      URLs generated by #filename_to_url#. The heuristics also work better when
      the file actually exists.  The empty string is returned when this
      function cannot parse the URL or when the URL is not a file URL. */
  static GString url_to_filename(const char *url);


};


//@}
// ------------
#endif
