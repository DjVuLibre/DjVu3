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
//C- $Id: DjVuErrorList.h,v 1.4 2000-02-24 22:23:54 haffner Exp $
 
#ifndef _DJVUERRORLIST_H
#define _DJVUERRORLIST_H

#ifdef __GNUC__
#pragma interface
#endif

#include "DjVuPort.h"
#include "GContainer.h"


/** @name DjVuErrorList.h
    This file implements a very simple class for redirecting port caster
    messages that would normally end up on stderr to a double linked list.

    @memo DjVuErrorList class.
    @author Bill C Riemers <bcr@att.com>
    @version #$Id: DjVuErrorList.h,v 1.4 2000-02-24 22:23:54 haffner Exp $#
*/

//@{

/** #DjVuErrorList# provides a convenient way to redirect error messages
    from classes derived from DjVuPort to a list that can be accessed at
    any time. */

class DjVuErrorList : public DjVuSimplePort
{
public:
     /// The normal port caster constructor. 
  DjVuErrorList();
     /// This constructor allows the user to specify the ByteStream.
  void set_stream(GP<ByteStream>);
     /// Append all error messages to the list
  virtual bool notify_error(const DjVuPort * source, const char * msg);
     /// Append all status messages to the list
  virtual bool notify_status(const DjVuPort * source, const char * msg);
     /// Add a new class to have its messages redirected here.
  inline void connect( const DjVuPort &port);
     /// Get the listing of errors, and clear the list.
  inline GList<GString> GetErrorList(void);
     /// Just clear the list.
  inline void ClearError(void);
     /// Get one error message and clear that message from the list.
  const char *GetError(void);
     /// Check if there are anymore error messages.
  inline bool HasError(void) const;
     /// Get the listing of status messages, and clear the list.
  inline GList<GString> GetStatusList(void);
     /// Just clear the list.
  inline void ClearStatus(void);
     /// Get one status message and clear that message from the list.
  const char *GetStatus(void);
     /// Check if there are anymore status messages.
  inline bool HasStatus(void) const;
     /** This gets the data.  We can't use the simple port's request
       data since we want to allow the user to specify the ByteStream. */
  virtual GP<DataPool> request_data (
    const DjVuPort * source, const GURL & url );

private:
  GP<ByteStream> ibs;
  GList<GString> Errors;
  GString PrevError;
  GList<GString> Status;
  GString PrevStatus;
};

inline void
DjVuErrorList::connect( const DjVuPort &port )
{ get_portcaster()->add_route(&port, this); }

inline GList<GString>
DjVuErrorList::GetErrorList(void)
{
  GList<GString> retval=(const GList<GString>)Errors;
  Errors.empty();
  return retval;
}

inline void
DjVuErrorList::ClearError(void)
{ Errors.empty(); }

inline GList<GString>
DjVuErrorList::GetStatusList(void)
{
  GList<GString> retval=(const GList<GString>)Status;
  Status.empty();
  return retval;
}

inline void
DjVuErrorList::ClearStatus(void)
{ Status.empty(); }

inline bool
DjVuErrorList::HasError(void) const
{ return !Errors.isempty(); }

inline bool
DjVuErrorList::HasStatus(void) const
{ return !Status.isempty(); }

#endif
