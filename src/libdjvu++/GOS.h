//C-  -*- C++ -*-
//C- DjVu® Reference Library (v. 3.0)
//C- 
//C- Copyright © 1999-2000 LizardTech, Inc. All Rights Reserved.
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
//C- 
// 
// $Id: GOS.h,v 1.16 2001-01-03 19:56:08 bcr Exp $
// $Name:  $

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
    #$Id: GOS.h,v 1.16 2001-01-03 19:56:08 bcr Exp $#
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
  static bool is_file(const char *filename);

  /** Returns true if #filename# exists and is a directory. */
  static bool is_dir(const char *filename);

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

  /** Creates the specified directory. */
  static int mkdir(const char * dirname);

  /** Recursively erases contents of directory #dirname#. The directory
      itself will not be removed. */
  static int cleardir(const char * dirname);
  
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

  /** Encodes all reserved characters, so that the #filename# can be
      used inside a URL. Every reserved character is encoded using escape
      sequence in the form of #%XX#. The legal characters are alphanumeric and
      #$-_.+!*'(),:#.
      Use \Ref{decode_reserved}() to convert the URL back to the filename. */
  static GString encode_reserved(const char * filename);

  /** Decodes reserved characters from the URL.
      See also: \Ref{encode_reserved}(). */
  static GString decode_reserved(const char * url);

};


//@}
// ------------
#endif
