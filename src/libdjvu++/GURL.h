//C-  -*- C++ -*-
//C-
//C-  Copyright (c) 1988 AT&T	
//C-  All Rights Reserved 
//C-
//C-  THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
//C-  The copyright notice above does not evidence any
//C-  actual or intended publication of such source code.
//C-
//C-  $Id: GURL.h,v 1.1 1999-01-22 00:40:19 leonb Exp $

#ifndef _GURL_H_
#define _GURL_H_

#ifdef __GNUC__
#pragma interface
#endif

#include "GString.h"

/** @name GURL.h
    File #"GURL.h"# contains the implementation of the \Ref{GURL}
    class used to store URLs in a system independent format.
    @memo System independent URL representation.
    @author Andrei Erofeev
    @version #$Id: GURL.h,v 1.1 1999-01-22 00:40:19 leonb Exp $#
*/

//@{

/** Accepts a URL starting from either #http:/# or #file:/# and
    transforms it to a system independent format.

    This class is nothing without \Ref{GOS} which does smart normalization
    of local URLs. Basically it works as follows:

    \begin{itemize}
       \item If the URL starts from #http://# then #GURL# makes sure, that
       every back slash is replaced by a forward one
       \item If the URL starts from #file://# then it's passed to \Ref{GOS}
       for normilizing (double convertion to file name and back to URL) to
       a form similar to #file://localhost/dir/file#
    \end{itemize}

    The class guarantees, that only forward slashes are used in the URL
    name, and the URL is absolute.
*/

class GURL
{
private:
   GString	url;

   void		init(void);
   void		convertSlashes(void);
   void		eatDots(void);
public:
      /// Returns everything from the beginning and up to the last slash)
   GURL		baseURL(void) const;

      /// Returns part of the URL after the last slash
   GString	fileURL(void) const;

      /// Checks if the object contains NULL URL
   int		isEmpty(void) const { return !url.length(); };

      /// Checks if the URL is local (starts from #file:/#) or not
   int		isLocal(void) const;

      /** Checks that {\em url} is absolute, that is starts from
	  #http:/# or #file:/# */
   static bool	isAbsolute(const char * url_string);

      /** @name Concatenation operators
	  Concatenate the GURL with the passed {\em name}. If the {\em name}
	  is absolute, we just return #GURL(name)#. Otherwise it's appended to
	  the GURL after a separating slash */
      //@{
      ///
   GURL		operator+(const char * name) const;
      ///
   GURL		operator+(const GString & name) const;
      //@}

      /// @return TRUE if {\em gurl1} and {\em gurl2} are the same
   friend int	operator==(const GURL & gurl1, const GURL & gurl2);

      /// @return Internal URL representation
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
   GURL(const GURL & gurl) : url(gurl.url) {};

      /// The descructor
   virtual ~GURL(void) {};

      /** Hashing function.
	  @return hash suitable for usage in \Ref{GMap} */
   friend unsigned int	hash(const GURL & gurl);
};

inline GURL
GURL::operator+(const GString & name) const
{
   return (*this)+(const char *) name;
}


inline int
operator==(const GURL & gurl1, const GURL & gurl2)
{
   return gurl1.url==gurl2.url;
}

inline unsigned int
hash(const GURL & gurl)
{
   return hash(gurl.url);
}

//@}

#endif
