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
// $Id: GURL.h,v 1.31 2000-11-09 20:15:07 jmw Exp $
// $Name:  $

#ifndef _GURL_H_
#define _GURL_H_

#ifdef __GNUC__
#pragma interface
#endif

#include "GString.h"
#include "Arrays.h"
#include "GThreads.h"

/** @name GURL.h
    Files #"GURL.h"# and #"GURL.cpp"# contain the implementation of the
    \Ref{GURL} class used to store URLs in a system independent format.
    @memo System independent URL representation.
    @author Andrei Erofeev <eaf@geocities.com>
    @version #$Id: GURL.h,v 1.31 2000-11-09 20:15:07 jmw Exp $#
*/

//@{

/** System independent URL representation.

    This class is used in the library to store URLs in a system independent
    format. The idea to use a general class to hold URL arose after we
    realized, that DjVu had to be able to access files both from the WEB
    and from the local disk. While it is strange to talk about system
    independence of HTTP URLs, file names formats obviously differ from
    platform to platform. They may contain forward slashes, backward slashes,
    colons as separators, etc. There maybe more than one URL corresponding
    to the same file name. Compare #file:/dir/file.djvu# and
    #file://localhost/dir/file.djvu#.

    To simplify a developer's life we have created this class, which contains
    inside a canonical representation of URLs.

    File URLs are converted to internal format with the help of \Ref{GOS} class.

    All other URLs are modified to contain only forward slashes.
*/

class GURL
{
private:
      // The 'class_lock' should be locked whenever you're accessing
      // url, or cgi_name_arr, or cgi_value_arr.
   GCriticalSection	class_lock;
   GString		url;
   DArray<GString>	cgi_name_arr, cgi_value_arr;

   void		init(void);
   void		convert_slashes(void);
   void		beautify_path(void);

   static GString	protocol(const char * url);
   void		parse_cgi_args(void);
   void		store_cgi_args(void);
public:
      /// Extracts the {\em protocol} part from the URL and returns it
   GString	protocol(void) const;

      /** Returns string after the first '\#' with decoded
	  escape sequences. */
   GString	hash_argument(void) const;

      /** Inserts the #arg# after a separating hash into the URL.
	  The function encodes any illegal character in #arg# using
	  \Ref{GOS::encode_reserved}(). */
   void		set_hash_argument(const char * arg);

      /** Returns the total number of CGI arguments in the URL.
	  CGI arguments follow '#?#' sign and are separated by '#&#' signs */
   int		cgi_arguments(void) const;

      /** Returns the total number of DjVu-related CGI arguments (arguments
	  following #DJVUOPTS# in the URL). */
   int		djvu_cgi_arguments(void) const;

      /** Returns that part of CGI argument number #num#, which is
	  before the equal sign. */
   GString	cgi_name(int num) const;

      /** Returns that part of DjVu-related CGI argument number #num#,
	  which is before the equal sign. */
   GString	djvu_cgi_name(int num) const;

      /** Returns that part of CGI argument number #num#, which is
	  after the equal sign. */
   GString	cgi_value(int num) const;
   
      /** Returns that part of DjVu-related CGI argument number #num#,
	  which is after the equal sign. */
   GString	djvu_cgi_value(int num) const;
   
      /** Returns array of all known CGI names (part of CGI argument before
	  the equal sign) */
   DArray<GString>cgi_names(void) const;

      /** Returns array of names of DjVu-related CGI arguments (arguments
	  following #DJVUOPTS# option. */
   DArray<GString>djvu_cgi_names(void) const;
   
      /** Returns array of all known CGI names (part of CGI argument before
	  the equal sign) */
   DArray<GString>cgi_values(void) const;

      /** Returns array of values of DjVu-related CGI arguments (arguments
	  following #DJVUOPTS# option. */
   DArray<GString>djvu_cgi_values(void) const;

      /// Erases everything after the first '\#' or '?'
   void		clear_all_arguments(void);

      /// Erases everything after the first '\#'
   void		clear_hash_argument(void);

      /// Erases DjVu CGI arguments (following "#DJVUOPTS#")
   void		clear_djvu_cgi_arguments(void);

      /// Erases all CGI arguments (following the first '?')
   void		clear_cgi_arguments(void);

      /** Appends the specified CGI argument. Will insert "#DJVUOPTS#" if
	  necessary */
   void		add_djvu_cgi_argument(const char * name, const char * value=0);
   
      /** Returns the URL corresponding to the directory containing
	  the document with this URL. The function basically takes the
	  URL and clears everything after the last slash. */
   GURL		base(void) const;

      /** Returns the name part of this URL.
	  For example, if the URL is #http://www.lizardtech.com/file%201.djvu# then
          this function will return #file%201.djvu#. \Ref{fname}() will
          return #file 1.djvu# at the same time. */
   GString	name(void) const;

      /** Returns the name part of this URL with escape sequences expanded.
	  For example, if the URL is #http://www.lizardtech.com/file%201.djvu# then
          this function will return #file 1.djvu#. \Ref{name}() will
          return #file%201.djvu# at the same time. */
   GString	fname(void) const;

      /// Returns the extention part of name of document in this URL.
   GString	extension(void) const;

      /// Checks if this is an empty URL
   bool		is_empty(void) const;

      /// Checks if the URL is local (starts from #file:/#) or not
   bool		is_local_file_url(void) const;

      /** @name Concatenation operators
	  Concatenate the GURL with the passed {\em name}. If the {\em name}
	  is absolute (has non empty protocol prefix), we just return
	  #GURL(name)#. Otherwise the #name# is appended to the GURL after a
	  separating slash.
      */
      //@{
      ///
   GURL		operator+(const char * name) const;
      ///
   GURL		operator+(const GString & name) const;
      //@}

      /// Returns TRUE if #gurl1# and #gurl2# are the same
   friend int	operator==(const GURL & gurl1, const GURL & gurl2);

      /// Returns TRUE if #gurl1# and #gurl2# are different
   friend int	operator!=(const GURL & gurl1, const GURL & gurl2);

      /// Assignment operator
   GURL &	operator=(const GURL & url);

      /// Returns Internal URL representation
   operator	const char*(void) const { return url; };

      /** @name Constructors
	  Accept the string URL, check that it starts from #file:/#
	  or #http:/# and convert to internal system independent
	  representation.
      */
      //@{
      ///
   GURL(const GString & url_string);
      ///
   GURL(const char * url_string=0);
      //@}

      /// Copy constructor
   GURL(const GURL & gurl);

      /// The descructor
   virtual ~GURL(void) {}

      /** Hashing function.
	  @return hash suitable for usage in \Ref{GMap} */
   friend unsigned int	hash(const GURL & gurl);
};

inline GURL
GURL::operator+(const GString & xname) const
{
   return (*this)+(const char *) xname;
}

inline int
operator==(const GURL & gurl1, const GURL & gurl2)
{
   return gurl1.url==gurl2.url;
}

inline int
operator!=(const GURL & gurl1, const GURL & gurl2)
{
   return gurl1.url!=gurl2.url;
}

inline unsigned int
hash(const GURL & gurl)
{
   return hash(gurl.url);
}

inline GString
GURL::protocol(void) const
{
   return protocol(url);
}

inline bool
GURL::is_empty(void) const
{
   return !url.length();
}

//@}

#endif
