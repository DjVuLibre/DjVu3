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
//C- $Id: GURL.h,v 1.17 2000-01-21 18:24:45 eaf Exp $

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
    @author Andrei Erofeev <eaf@research.att.com>
    @version #$Id: GURL.h,v 1.17 2000-01-21 18:24:45 eaf Exp $#
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
   GCriticalSection	url_lock;
   GString		url;

   GCriticalSection	cgi_lock;
   DArray<GString>	cgi_name_arr, cgi_value_arr;

   void		init(void);
   void		convert_slashes(void);
   void		eat_dots(void);

   static GString	protocol(const char * url);
   void		parse_cgi_args(void);
   void		store_cgi_args(void);
public:
      /// Extracts the {\em protocol} part from the URL and returns it
   GString	protocol(void) const;

      /// Returns string after the first '\#' or '%23'
   GString	hash_argument(void) const;

      /** Returns the total number of CGI arguments in the URL.
	  CGI arguments follow '#?#' sign and are separated by '#&#' signs */
   int		cgi_arguments(void) const;

      /** Returns that part of CGI argument number #num#, which is
	  before the equal sign. */
   GString	cgi_name(int num) const;
   
      /** Returns array of all known CGI names (part of CGI argument before
	  the equal sign) */
   DArray<GString>cgi_names(void) const;

      /** Returns array of names of DjVu-related CGI arguments (arguments
	  following #DJVUOPTS# option. */
   DArray<GString>djvu_cgi_names(void) const;
   
      /** Returns that part of CGI argument number #num#, which is
	  after the equal sign. */
   GString	cgi_value(int num) const;

      /** Returns array of all known CGI names (part of CGI argument before
	  the equal sign) */
   DArray<GString>cgi_values(void) const;

      /** Returns array of values of DjVu-related CGI arguments (arguments
	  following #DJVUOPTS# option. */
   DArray<GString>djvu_cgi_values(void) const;

      /// Erases everything after the first '\#' ('%23') or '?' ('%3f')
   void		clear_all_arguments(void);

      /// Erases everything after the first '\#' ('\#23')
   void		clear_hash_argument(void);

      /// Erases DjVu CGI arguments (following "#DJVUOPTS#")
   void		clear_djvu_cgi_arguments(void);

      /// Erases all CGI arguments (following first '?')
   void		clear_cgi_arguments(void);

      /** Appends the specified CGI argument. Will insert "#DJVUOPTS#" if
	  necessary */
   void		add_djvu_cgi_argument(const char * name, const char * value=0);
   
      /** Returns the URL corresponding to the directory containing the document
	  with this URL. */
   GURL		base(void) const;

      /// Returns the name part of this URL.
   GString	name(void) const;

      /// Returns the extention part of name of document in this URL.
   GString	extension(void) const;

      /// Checks if this is an empty URL
   bool		is_empty(void) const;

      /// Checks if the URL is local (starts from #file:/#) or not
   bool		is_local_file_url(void) const;

      /** @name Concatenation operators
	  Concatenate the GURL with the passed {\em name}. If the {\em name}
	  is absolute (has non empty protocol prefix), we just return
	  #GURL(name)#. Otherwise the name is appended to the GURL after a
	  separating slash preserving possible URL suffixes following #;# or #?#. */
      //@{
      ///
   GURL		operator+(const char * xname) const;
      ///
   GURL		operator+(const GString & xname) const;
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
