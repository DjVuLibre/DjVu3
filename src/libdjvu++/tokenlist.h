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
//C- $Id: tokenlist.h,v 1.1 1999-11-02 21:44:58 bcr Exp $
 

#ifndef __DJVUTOKENLIST_H__
#define __DJVUTOKENLIST_H__
#ifdef __GNUC__
#pragma interface
#endif

#include "DjVuGlobal.h"

//** @name tokenlist.h \begin{verbatim}
// 
// This is a class very simmular to GMap, only it is limited it is much
// limited scope.  It is an associative array "string" to integer.  But
// the integer is assigned uniquely by this class in sequental order.
// This is of use when you want to store items sequentially in an array
// without making the array to large.  This list is always sorted, so
// this class is also usefull for creating a sorted unique list of words.
//
// At some point the TokenList class may be replaced by a wrapper to the
// GMap class.  We will have to evaluate CPU and memory usage to see if
// the GMap replacement would be adiquate.
//
// \end{verbatim} */
//** @memo tokenlist header file. */
//** @version $Id: tokenlist.h,v 1.1 1999-11-02 21:44:58 bcr Exp $ */
//** @author $Author: bcr $ */

class DjVuTokenList
{
private:
  int ListSize;
  int NextToken;
  class Entries;
  Entries *Entry;
  char **Strings;
public:
  int links;
  DjVuTokenList() : ListSize(0),NextToken(0),Entry(0),Strings(0),links(0) {};
  ~DjVuTokenList();

    // This is just an array lookup.  That is the whole point of tokens.
  inline const char * const GetString(const int token) const
  { return (token<NextToken)?Strings[token]:0; };

  int GetToken(const char name[]) const;
  int SetToken(const char name[]);
};

#ifdef _DJVUTOKENLIST_H_IMPLEMENTATION_
class DjVuTokenList::Entries
{
public:
  int Token;
  char *Name;
  Entries() : Token(0),Name(0) {};
};
#endif /* _DJVUTOKENLIST_H_IMPLEMENTATION_ */
#endif /* __DJVUTOKENLIST_H__ */

