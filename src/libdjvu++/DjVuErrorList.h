//C-  -*- C++ -*-
//C- DjVu® Reference Library (v. 3.0)
//C- 
//C- Copyright © 1999-2000 LizardTech, Inc. All Rights Reserved.
//C- 
//C- This software (the "Original Code") is subject to, and may be
//C- distributed under, the GNU General Public License, Version 2.
//C- The license should have accompanied the Original Code or you
//C- may obtain a copy of the license from the Free Software
//C- Foundation at http://www.fsf.org .
//C- 
//C- With respect to the Original Code, and subject to any third
//C- party intellectual property claims, LizardTech grants recipient
//C- a worldwide, royalty-free, non-exclusive license under patent
//C- claims infringed by making, using, or selling Original Code
//C- which are now or hereafter owned or controlled by LizardTech,
//C- but solely to the extent that any such patent is reasonably
//C- necessary to enable you to make, have made, practice, sell, or 
//C- otherwise dispose of Original Code (or portions thereof) and
//C- not to any greater extent that may be necessary to utilize
//C- further modifications or combinations.
//C- 
//C- The Original Code is provided "AS IS" WITHOUT WARRANTY OF ANY
//C- KIND, EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO
//C- ANY WARRANTY OF NON-INFRINGEMENT, OR ANY IMPLIED WARRANTY OF 
//C- MERCHANTIBILITY OF FITNESS FOR A PARTICULAR PURPOSE.
// 
// $Id: DjVuErrorList.h,v 1.8 2000-11-02 01:08:34 bcr Exp $
// $Name:  $

 
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
    @version #$Id: DjVuErrorList.h,v 1.8 2000-11-02 01:08:34 bcr Exp $#
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
  GURL set_stream(GP<ByteStream>);

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

     /// Check if there are any more status messages.
  inline bool HasStatus(void) const;

     /** This gets the data.  We can't use the simple port's request
       data since we want to allow the user to specify the ByteStream. */
  virtual GP<DataPool> request_data (
    const DjVuPort * source, const GURL & url );

private:
  GURL pool_url;
  GP<DataPool> pool;
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
