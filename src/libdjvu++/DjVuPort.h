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
//C- $Id: DjVuPort.h,v 1.13 1999-09-09 21:42:14 leonb Exp $
 
#ifndef _DJVUPORT_H
#define _DJVUPORT_H

#ifdef __GNUC__
#pragma interface
#endif

#include "DjVuGlobal.h"
#include <stddef.h>
#include <stdlib.h>
#include "GException.h"
#include "GSmartPointer.h"
#include "GURL.h"
#include "GThreads.h"
#include "GContainer.h"
#include "DataPool.h"

/** @name DjVuPort.h
    Files #"DjVuPort.h"# and #"DjVuPort.cpp"# implement a communication
    mechanism between different parties involved in decoding DjVu files.
    It should be pretty clear that the creator of \Ref{DjVuDocument} and
    \Ref{DjVuFile} would like to receive some information about the progress
    of decoding, errors occurred, etc. It may also want to provide source data
    for decoders (like it's done in the plugin where the real data is downloaded
    from the net and is fed into DjVu decoders).

    Normally this functionality is implemented by means of callbacks which are
    run when a given condition comes true. Unfortunately it's not quite easy
    to implement this strategy in our case. The reason is that there may be
    more than one "client" working with the same document, and the document
    should send the information to each of the clients. This could be done by
    means of callback {\em lists}, of course, but we want to achieve more
    bulletproof results: we want to be sure that the client, that we're about
    to contact is still alive, and is not being destroyed by another thread.
    Besides, we are going to call these "callbacks" from many places, from
    many different classes.  Maintaining multi-thread safe callback lists is
    very difficult.

    Finally, we want to provide some default implementation of these
    "callbacks" in the library, which should attempt to process the requests
    themselves if they can, and contact the client only if they're unable to
    do it (like in the case of \Ref{DjVuPort::request_data}() with local URL
    where \Ref{DjVuDocument} can get the data from the hard drive itself not
    disturbing the document's creator.

    Two class implemente a general communication mechanism: \Ref{DjVuPort} and
    \Ref{DjVuPortcaster}. Any sender and recepient of requests should be a
    subclass of \Ref{DjVuPort}.  \Ref{DjVuPortcaster} maintains a map of
    routes between \Ref{DjVuPort}s, which should be configured by somebody
    else. Whenever a port wants to send a request, it calls the corresponding
    function of \Ref{DjVuPortcaster}, and the portcaster relays the request to
    all the destinations that it sees in the internal map.

    The \Ref{DjVuPortcaster} is responsible for keeping the map up to date by
    getting rid of destinations that have been destroyed.  Map updates are
    performed from a single place and are serialized by a global monitor.
    
    @memo DjVu decoder communication mechanism.
    @author Andrei Erofeev <eaf@geocities.com>\\
            L\'eon Bottou <leonb@research.att.com>
    @version #$Id: DjVuPort.h,v 1.13 1999-09-09 21:42:14 leonb Exp $# */
//@{

class DjVuPort;
class DjVuPortcaster;

/** Base class for notification targets.
    #DjVuPort# provides base functionality for classes willing to take part in
    sending and receiving messages generated during decoding process.  You
    need to derive your class from #DjVuPort# if you want it to be able to
    send or receive requests. In addition, for receiving requests you should
    override one or more virtual function.

    {\bf Important remark} --- All ports should be allocated on the heap using
    #operator new# and immediatlely secured using a \Ref{GP} smartpointer.
    Ports which are not secured by a smart-pointer are not considered
    ``alive'' and never receive notifications! */

class DjVuPort : public GPEnabled
{
public:
   DjVuPort();
   virtual ~DjVuPort();
   static void *operator new (size_t sz);
   static void operator delete(void *addr) { ::operator delete(addr); } ;

      /**  Use this function to get a copy of the global \Ref{DjVuPortcaster}. */
   static DjVuPortcaster *get_portcaster(void);

      /** Copy constructor. When #DjVuPort#s are copied, the portcaster
          copies all incoming and outgoinf routes of the original. */
   DjVuPort(const DjVuPort & port);

      /** Copy operator. Similarily to the copy constructor, the portcaster
          copies all incoming and outgoingcoming routes of the original. */
   DjVuPort & operator=(const DjVuPort & port);

      /** Should return 1 if the called class inherits class #class_name#.
	  When a destination receives a request, it can retrieve the pointer
	  to the source #DjVuPort#. This virtual function should be able
	  to help to identify the source of the request. For example,
	  \Ref{DjVuFile} is also derived from #DjVuPort#. In order for
	  the receiver to recognize the sender, the \Ref{DjVuFile} should
	  override this function to return #TRUE# when the #class_name#
	  is either #DjVuPort# or #DjVuFile# */
   virtual bool		inherits(const char * class_name) const;

      /** @name Notifications. 
          These virtual functions may be overidden by the subclasses
          of #DjVuPort#.  They are called by the \Ref{DjVuPortcaster}
          when the port is alive and when there is a route between the 
          source of the notification and this port. */
      //@{

      /** This request is issued to request translation of the ID, used
	  in an DjVu INCL chunk to a URL, which may be used to request
	  data associated with included file. \Ref{DjVuDocument} usually
	  intercepts all such requests, and the user doesn't have to
	  worry about the translation */
   virtual GURL		id_to_url(const DjVuPort * source, const char * id);

      /** This request is issued to obtain a cached copy of a \Ref{DjVuFile}.
	  This request is issued by \Ref{DjVuFile} and \Ref{DjVuDocument}
	  when they want to create a new instance of \Ref{DjVuFile}. Normally
	  \Ref{DjVuDocument} handles all these requests itself.
	  If a port can not fulfil the request, it should return #0#. */
   virtual GPBase	get_cached_file(const DjVuPort * source, const GURL & url);

      /** This request is issued to add a given file to cache. It is called by
	  \Ref{DjVuFile} at least in two cases: after the file has just been
	  constructed and after it has been decoded. */
   virtual void		cache_djvu_file(const DjVuPort * source, class DjVuFile * file);
	  
      /** This request is issued when decoder needs additional data
	  for decoding.  Both \Ref{DjVuFile} and \Ref{DjVuDocument} are
	  initialized with a URL, not the document data.  As soon as
	  they need the data, they call this function, whose responsibility
	  is to locate the source of the data basing on the #URL# passed
	  and return it back in the form of the \Ref{DataPool}. If this
	  particular receiver is unable to fullfil the request, it should
	  return #0#. */
   virtual GP<DataPool>	request_data(const DjVuPort * source, const GURL & url);

      /** This notification is sent when an error occurs and the error message
	  should be shown to the user.  The receiver should return #0# if it is 
          unable to process the request. Otherwise the receiver should return 1. */
   virtual bool		notify_error(const DjVuPort * source, const char * msg);

      /** This notification is sent to update the decoding status.  The
          receiver should return #0# if it is unable to process the
          request. Otherwise the receiver should return 1. */
   virtual bool		notify_status(const DjVuPort * source, const char * msg);

      /** This notification is sent by \Ref{DjVuImage} when it should be
	  redrawn. It may be used to implement progressive redisplay.

	  @param source The sender of the request */
   virtual void		notify_redisplay(const class DjVuImage * source);

      /** This notification is sent by \ref{DjVuImage} when its geometry
	  has been changed as a result of decoding. It may be used to
	  implement progressive redisplay. */
   virtual void		notify_relayout(const class DjVuImage * source);

      /** This notification is sent when a new chunk has been decoded. */
   virtual void		notify_chunk_done(const DjVuPort * source, const char * name);

      /** This notification is sent after decoding of a particular file
	  has been finished successfully. */
   virtual void		notify_file_done(const DjVuPort * source);

      /** This notification is sent when decoding of a particular file
	  has been stopped by the user. */
   virtual void		notify_file_stopped(const DjVuPort * source);

      /** This notification is sent when decoding of a particular file
	  has failed. */
   virtual void		notify_file_failed(const DjVuPort * source);

      /** This notification is sent from time to time while decoding is in
	  progress. The purpose is obvious: to provide a way to know how much
	  is done and how long the decoding will continue.  Argument #done# is
	  a number from 0 to 1 reflecting the progress. */
   virtual void		notify_decode_progress(const DjVuPort * source, float done);

      /** This notification is sent after a given file received all the data.
	  The reason why we have it is because the data is passed to the
	  \Ref{DjVuFile}s and \Ref{DjVuDocument}s in the form of
	  \Ref{DataPool}, which may have all the data, some of the data or
	  nothing at all.  The data may be added later (as it happens in the
	  Netscape plugin) and we may want to know when it's done. */
   virtual void		notify_file_data_received(const DjVuPort * source);

      /** This notification is send after a given file {\em and all files
	  included into it} received all the data. */
   virtual void		notify_all_data_received(const DjVuPort * source);
      //@}
};

/** Simple port.  
    An instance of #DjVuSimplePort# is automatically created when you create a
    \Ref{DjVuFile} or a \Ref{DjVuDocument} without specifying a port.  This
    simple port can retrieve data for local urls (i.e. urls referring to local
    files) and display error messages on #stderr#.  All other notifications
    are ignored. */

class DjVuSimplePort : public DjVuPort
{
public:
      /// Returns 1 if #class_name# is #"DjVuPort"# or #"DjVuSimplePort"#.
   virtual bool		inherits(const char * class_name) const;

      /** If #url# is local, it created a \Ref{DataPool}, connects it to the
	  file with the given name and returns.  Otherwise returns #0#. */
   virtual GP<DataPool>	request_data(const DjVuPort * source, const GURL & url);

      /// Displays error on #stderr#. Always returns 1.
   virtual bool		notify_error(const DjVuPort * source, const char * msg);
   
      /// Displays status on #stderr#. Always returns 1.
   virtual bool		notify_status(const DjVuPort * source, const char * msg);
};


/** Memory based port.
    This \Ref{DjVuPort} maintains a map associating pseudo urls with data
    segments.  It processes the #request_data# notifications according to this
    map.  After initializing the port, you should add as many pairs #<url,
    pool># as needed need and add a route from a \Ref{DjVuDocument} or
    \Ref{DjVuFile} to this port. */

class DjVuMemoryPort : public DjVuPort
{
public:
      /// Returns 1 if #class_name# is #"DjVuPort"# or #"DjVuMemoryPort"#
   virtual bool		inherits(const char * class_name) const;

      /** If #url# is one of those, that have been added before by means
	  of \Ref{add_data}() function, it will return the associated
	  \Ref{DataPool}. #ZERO# otherwize. */
   virtual GP<DataPool>	request_data(const DjVuPort * source, const GURL & url);

      /** Adds #<url, pool># pair to the internal map. From now on, if
	  somebody asks for data corresponding to the #url#, it will
	  be returning the #pool# */
   void		add_data(const GURL & url, const GP<DataPool> & pool);
private:
   class Pair : public GPEnabled
   {
   public:
      GURL		url;
      GP<DataPool>	pool;
   };
   GCriticalSection	lock;
   GPList<Pair>		list;
};



/** Maintains associations between ports.
    It monitors the status of all ports (have they been destructed yet?),
    accepts requests and notifications from them and forwards them to
    destinations according to internally maintained map of routes.

    The caller can modify the route map any way he likes (see
    \Ref{add_route}(), \Ref{del_route}(), \Ref{copy_routes}(),
    etc. functions). Any port can be either a sender of a message, an
    intermediary receiver or a final destination.  

    When a request is sent, the #DjVuPortcaster# computes the list of
    destinations by consulting with the route map.  Notifications are only
    sent to ``alive'' ports.  A port is alive if it is referenced by a valid
    \Ref{GP} smartpointer.  As a consequence, a port usually becomes alive
    after running the constructor (since the returned pointer is then assigned
    to a smartpointer) and is no longer alive when the port is destroyed
    (because it would not be destroyed if a smartpointer was referencing it).

    Destination ports are sorted according to their distance from the source.
    For example, if port {\bf A} is connected to ports {\bf B} and {\bf C}
    directly, and port {\bf B} is connected to {\bf D}, then {\bf B} and {\bf
    C} are assumed to be one hop away from {\bf A}, while {\bf D} is two hops
    away from {\bf A}.

    In some cases the requests and notifications are sent to every possible
    destination, and the order is not significant (like it is for
    \Ref{notify_file_done}() request). Others should be sent to the closest
    destinations first, and only then to the farthest, in case if they have
    not been processed by the closest. The examples are \Ref{request_data}(),
    \Ref{notify_error}() and \Ref{notify_status}().

    The user is not expected to create the #DjVuPortcaster# itselt. He should
    use \Ref{get_portcaster}() global function instead.  */
class DjVuPortcaster
{
public:
      /**  Use this function to get a copy of the global \Ref{DjVuPortcaster}. */
   static DjVuPortcaster *get_portcaster(void)
    { return DjVuPort::get_portcaster(); } ;

      /** The default constructor. */
   DjVuPortcaster(void);

   virtual ~DjVuPortcaster(void);

      /** Adds route from #src# to #dst#. Whenever a request is
	  sent or received by #src#, it will be forwarded to #dst# as well.
	  @param src The source
	  @param dst The destination */
   void		add_route(const DjVuPort *src, DjVuPort *dst);

      /** The opposite of \Ref{add_route}(). Removes the association
	  between #src# and #dst# */
   void		del_route(const DjVuPort *src, DjVuPort *dst);

      /** Copies all incoming and outgoing routes from #src# to
	  #dst#. This function should be called when a \Ref{DjVuPort} is
	  copied, if you want to preserve the connectivity. */
   void		copy_routes(DjVuPort *dst, const DjVuPort *src);

      /** Returns a smart pointer to the port if #port# is a valid pointer
          to an existing #DjVuPort#.  Returns a null pointer otherwise. */
   GP<DjVuPort> is_port_alive(DjVuPort *port);

      /** Assigns the specified name to the given \Ref{DjVuPort}. Each port
	  may have only one name and each name may correspond to only one
	  port. Thus, if the specified name is already associated with another
	  port, this association will be removed. */
   void		set_name(const DjVuPort * port, const char * name);

      /** Returns \Ref{DjVuPort} associated with the given #name#. If nothing
	  is known about name #name#, or the port associated with it has
	  already been destroyed #ZERO# pointer will be returned. */
   GP<DjVuPort>	name_to_port(const char * name);

      /** Returns name associated with the given \Ref{DjVuPort}. If no name
	  has been assigned to this port before, an empty string will
	  be returned. */
   GString	port_to_name(const DjVuPort * port);

      /** Computes destination list for #source# and calls the corresponding
	  function in each of the ports from the destination list starting from
	  the closest until one of them returns non-empty \Ref{GURL}. */
   virtual GURL		id_to_url(const DjVuPort * source, const char * id);
      /** Computes destination list for #source# and calls the corresponding
	  function in each of the ports from the destination list starting from
	  the closest until one of then returns non #ZERO# pointer
	  to \Ref{DjVuFile}. */
   virtual GPBase	get_cached_file(const DjVuPort * source, const GURL & url);
      /** Computes destination list for #source# and calls the corresponding
	  function in each of the ports from the destination list starting from
	  the closest until one. */
   virtual void		cache_djvu_file(const DjVuPort * source, class DjVuFile * file);
      /** Computes destination list for #source# and calls the corresponding
	  function in each of the ports from the destination list starting from
	  the closest until one of them returns non-zero \Ref{DataPool}. */
   virtual GP<DataPool>	request_data(const DjVuPort * source, const GURL & url);

      /** Computes destination list for #source# and calls the corresponding.
	  function in each of the ports from the destination starting from
	  the closest until one of them returns 1. */
   virtual bool		notify_error(const DjVuPort * source, const char * msg);

      /** Computes destination list for #source# and calls the corresponding
	  function in each of the ports from the destination list starting from
	  the closest until one of them returns 1. */
   virtual bool		notify_status(const DjVuPort * source, const char * msg);

      /** Computes destination list for #source# and calls the corresponding
	  function in each of the ports from the destination list starting from
	  the closest. */
   virtual void		notify_redisplay(const class DjVuImage * source);

      /** Computes destination list for #source# and calls the corresponding
	  function in each of the ports from the destination list starting from
	  the closest. */
   virtual void		notify_relayout(const class DjVuImage * source);

      /** Computes destination list for #source# and calls the corresponding
	  function in each of the ports from the destination list starting from
	  the closest. */
   virtual void		notify_chunk_done(const DjVuPort * source, const char * name);

      /** Computes destination list for #source# and calls the corresponding
	  function in each of the ports from the destination list starting from
	  the closest. */
   virtual void		notify_file_done(const DjVuPort * source);

      /** Computes destination list for #source# and calls the corresponding
	  function in each of the ports from the destination list starting from
	  the closest. */
   virtual void		notify_file_stopped(const DjVuPort * source);

      /** Computes destination list for #source# and calls the corresponding
	  function in each of the ports from the destination list starting from
	  the closest. */
   virtual void		notify_file_failed(const DjVuPort * source);

      /** Computes destination list for #source# and calls the corresponding
	  function in each of the ports from the destination list starting from
	  the closest. */
   virtual void		notify_decode_progress(const DjVuPort * source, float done);

      /** Computes destination list for #source# and calls the corresponding
	  function in each of the ports from the destination list starting from
	  the closest. */
   virtual void		notify_file_data_received(const DjVuPort * source);

      /** Computes destination list for #source# and calls the corresponding
	  function in each of the ports from the destination list starting from
	  the closest. */
   virtual void		notify_all_data_received(const DjVuPort * source);

private:
      // We use these 'void *' to minimize template instanciations.
   friend class DjVuPort;
   GCriticalSection		map_lock;
   GMap<const void *, void *>	route_map;	// GMap<DjVuPort *, GList<DjVuPort *> *>
   GMap<const void *, void *>	cont_map;	// GMap<DjVuPort *, DjVuPort *>
   GMap<GString, const void *>	n2p_map;	// GMap<GString, DjVuPort *>
   GMap<const void *, void *>	p2n_map;	// GMap<DjVuPort *, const char *>
   void del_port(const DjVuPort * port);
   void add_to_closure(GMap<const void*, void*> & set,
                       const DjVuPort *dst, int distance);
   void compute_closure(const DjVuPort *src, GPList<DjVuPort> &list,
                        bool sorted=false);
};


inline bool
DjVuPort::inherits(const char * class_name) const
{
   return !strcmp("DjVuPort", class_name);
}

inline bool
DjVuSimplePort::inherits(const char * class_name) const
{
   return
      !strcmp("DjVuSimplePort", class_name) ||
      DjVuPort::inherits(class_name);
}

inline bool
DjVuMemoryPort::inherits(const char * class_name) const
{
   return
      !strcmp("DjVuMemoryPort", class_name) ||
      DjVuPort::inherits(class_name);
}

//@}

#endif
