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
//C- $Id: DjVuPort.h,v 1.4 1999-08-26 19:25:11 eaf Exp $
 
#ifndef _DJVUPORT_H
#define _DJVUPORT_H

#ifdef __GNUC__
#pragma interface
#endif

#include "GException.h"
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
    to contact is still alive (has not been destroyed yet). Indeed, it's so
    easy to forget to unregister a callback in the client destructor...

    Besides, we're going to call these "callbacks" from many places, from many
    different classes. Maintaining synchronized versions of the callback lists
    can be a pain...

    Finally, we want to provide some default implementation of these "callbacks"
    in the library, which should attempt to process the requests themselves
    if they can, and contact the client only if they're unable to do it (like
    in the case of \Ref{DjVuPort::request_data}() with local URL where
    \Ref{DjVuDocument} can get the data from the hard drive itself not
    disturbing the document's creator.

    In short, we have implemented a communication mechanism supported by
    two classes defined here: \Ref{DjVuPort} and \Ref{DjVuPortcaster}. Any
    sender and recepient of requests should be a subclass of \Ref{DjVuPort}.
    \Ref{DjVuPortcaster} maintains a map of routes between \Ref{DjVuPort}s,
    which should be configured by somebody else. Whenever a port wants to
    send a request, it calls the corresponding function of \Ref{DjVuPortcaster},
    and the portcaster relays the request to all the destinations that it
    sees in the internal map.

    The \Ref{DjVuPortcaster} is responsible for keeping the map up to date
    by getting rid of destinations that have been destroyed.
    
    @memo DjVu decoder communication mechanism.
    @author Andrei Erofeev <eaf@geocities.com>, L\'eon Bottou <leonb@research.att.com>
    @version #$Id: DjVuPort.h,v 1.4 1999-08-26 19:25:11 eaf Exp $#
*/

//@{

/** #DjVuPort# provides base functionality for classes willing to take
    part in sending and receiving messages generated during decoding process.

    You need to derive your class from #DjVuPort# if you want it to be able
    to send or receive requests. In addition, for receiving requests you
    should override one or more virtual function.
    */

class DjVuPort
{
public:
      /** The constructor. You can neglect the #dont_add_to_pcaster#
	  parameter as it's used for internal purposes (the reason is that
	  \Ref{DjVuPortcaster} is implemented as a \Ref{DjVuPort} too and we
	  don't want to add the portcaster to the portcaster). */
   DjVuPort(bool dont_add_to_pcaster=0);

      /** Copy constructor. #DjVuPort#s may be copied. When this happens,
	  \Ref{DjVuPortcaster} copies all incoming and outcoming routes
	  from the original to the copy */
   DjVuPort(const DjVuPort & port);

   virtual ~DjVuPort(void);

      /** Copy operator. Similarily to the copy constructor, the copy
	  operator results in copying of all incoming and outcoming routes
	  (inside \Ref{DjVuPortcaster}) from the original to the copy */
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

      /** This request is issued to request translation of the ID, used
	  in an DjVu INCL chunk to a URL, which may be used to request
	  data associated with included file. \Ref{DjVuDocument} usually
	  intercepts all such requests, and the user doesn't have to
	  worry about the translation */
   virtual GURL		id_to_url(const DjVuPort * source, const char * id);

      /** This request is issued to obtain a cached copy of a \Ref{DjVuFile}.
	  Contrary to other requests, it's passed to the "farthest" destinations
	  first. Then, if they're unable to return the cached copy, it's
	  passed to the most "closest" (to the source) ones. If a port can not
	  fulfil the request, it should return #ZERO#.

	  @param source The sender of the request
	  @param url The URL of the cached file to be returned */
   virtual GPBase	get_cached_file(const DjVuPort * source, const GURL & url);
	  
      /** This request is issued when decoder needs additional data
	  for decoding. Both \Ref{DjVuFile} and \Ref{DjVuDocument} are
	  initialized with a #URL#, not the document data. As soon as
	  they need the data, they call this function, which responsibility
	  is to locate the source of the data basing on the #URL# passed
	  and return it back in the form of the \Ref{DataRange}. If this
	  particular receiver is unable to fullfil the request, it should
	  return #ZERO#.

	  @param source The sender of the request
	  @param url The URL of the data.
	  @return The \Ref{DataRange} attached to the \Ref{DataPool} with data. */
   virtual GP<DataRange>request_data(const DjVuPort * source, const GURL & url);

      /** This notification is sent when an error occurs and the error message
	  should be shown to the user. If the receiver is unable to process
	  the request, it should return #ZERO# so that the \Ref{DjVuPortcaster}
	  can call somebody else.

	  @param source The sender of the request
	  @param msg The error message
	  @return 1 if the request has been processed. */
   virtual bool		notify_error(const DjVuPort * source, const char * msg);

      /** This notification is sent to update the decoding status. If the
	  receiver is unable to process the notification (display the status),
	  it should return #ZERO# so that the \Ref{DjVuPortcaster} can call
	  somebody else.

	  @param source The sender of the request
	  @param msg The status message
	  @return 1 if the request has been processed. */
   virtual bool		notify_status(const DjVuPort * source, const char * msg);

      /** This notification is sent when the image corresponding to the
	  decoded file should be redrawn. It may be used to implement
	  progressive redisplay.

	  @param source The sender of the request */
   virtual void		notify_redisplay(const DjVuPort * source);

      /** This notification is sent when the image geometry has been changed
	  as a result of decoding. It may be used to implement progressive
	  redisplay.

	  @param source The sender of the request */
   virtual void		notify_relayout(const DjVuPort * source);

      /** This notification is sent when a new chunk has been decoded.

	  @param source The sender of the request
	  @param name The name of the chunk that has been decoded */
   virtual void		notify_chunk_done(const DjVuPort * source, const char * name);

      /** This notification is sent after decoding of a particular file
	  has been finished successfully.

	  @param source The sender of the request (the file finished) */
   virtual void		notify_file_done(const DjVuPort * source);

      /** This notification is sent when decoding of a particular file
	  has been stopped.

	  @param source The sender of the request (the file stopped) */
   virtual void		notify_file_stopped(const DjVuPort * source);

      /** This notification is sent when decoding of a particular file
	  has failed.

	  @param source The sender of the request (the file failed) */
   virtual void		notify_file_failed(const DjVuPort * source);

      /** This notification is sent from time to time while decoding is
	  in progress. The purpose is obvious: to provide a way to know how
	  much is done and how long the decoding will continue.

	  @param source The sender of the request (the file being decoded)
	  @param done Number from 0 to 1 reflecting the progress */
   virtual void		notify_decode_progress(const DjVuPort * source, float done);

      /** This notification is sent after a given file received all the data.
	  The reason why we have it is because the data is passed to the
	  \Ref{DjVuFile}s and \Ref{DjVuDocument}s in the form of \Ref{DataPool},
	  which may have all the data, some of the data or nothing at all.
	  The data may be added later (as it happens in the Netscape plugin)
	  and we may want to know when it's done.

	  @param source The sender of the request (the file with data) */
   virtual void		notify_file_data_received(const DjVuPort * source);

      /** This notification is send after a given file {\em and all files
	  included into it} received all the data.

	  @param source The sender of the request (the file with all data) */
   virtual void		notify_all_data_received(const DjVuPort * source);
};

/** This is a {\em simple port} capable of retrieving data from the hard drive
    and showing error messages on #stderr#.

    When all the #URL#s used in decoding refer to local files, there is no
    reason to ask client (\Ref{DjVuDocument} creator) for data: the library
    can get it itself. All you need to do is to create \Ref{DjVuFile} and
    \Ref{DjVuDocument} passing #ZERO# port in the arguments, and the
    library will create this #DjVuSimplePort# and will use it to work
    with files stored locally.

    The #DjVuSimplePort# responds to only one request (\Ref{request_data}())
    and accepts two notifications (\Ref{notify_error}() and
    \Ref{notify_status}()). The rest is ignored.

*/
class DjVuSimplePort : public DjVuPort
{
public:
      /// Returns 1 if #class_name# is #"DjVuPort"# or #"DjVuSimplePort"#.
   virtual bool		inherits(const char * class_name) const;

      /** If #url# is local, it opens the corresponding file, creates and
	  fills \Ref{DataPool} and returns \Ref{DataRange} attached to it.
	  Otherwise returns #ZERO#. */
   virtual GP<DataRange>request_data(const DjVuPort * source, const GURL & url);

      /// Displays error on #stderr#. Always returns 1.
   virtual bool		notify_error(const DjVuPort * source, const char * msg);
   
      /// Displays status on #stderr#. Always returns 1.
   virtual bool		notify_status(const DjVuPort * source, const char * msg);
};

/** This \Ref{DjVuPort} maps an abstract URL to data in the memory.
    It's useful when you have the data necessary for decoding in the memory,
    not on the hard drive or anywhere else. If this is the case, then you
    should initialize #DjVuMemoryPort#, add as many pairs #<url, pool># to
    it as you need and connect the \Ref{DjVuDocument} or \Ref{DjVuFile} to
    it by means of \Ref{DjVuPortcaster}.

    The #DjVuMemoryPort# will be listening for \Ref{request_data}() requests
    and in case if it recognizes the {\em URL} passed with the request, it
    will return the associated data to the caller.
*/
class DjVuMemoryPort : public DjVuPort
{
public:
      /// Returns 1 if #class_name# is #"DjVuPort"# or #"DjVuMemoryPort"#
   virtual bool		inherits(const char * class_name) const;

      /** If #url# is one of those, that have been added before by means
	  of \Ref{add_data}() function, it will return the associated
	  \Ref{DataPool}. #ZERO# otherwize. */
   virtual GP<DataRange>request_data(const DjVuPort * source, const GURL & url);

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

/** This class maintains associations between different \Ref{DjVuPort}s.

    It monitors ports status (have they been destructed yet?), accepts
    requests and notifications from them and forwards them to destinations
    according to internally maintained map of routes.

    The caller can modify the route map any way he likes (see \Ref{add_route}(),
    \Ref{del_route}(), \Ref{copy_routes}(), etc. functions). Any port can be
    either a sender of a message, an intermediary receiver or a final
    destination.

    When a request is sent, the #DjVuPortcaster# computes the list of
    destinations by consulting with the route map. It sorts the destinations
    basing on their distance from the source. And then sends the request
    following this order. For example, if port {\bf A} is connected to ports
    {\bf B} and {\bf C} directly, and port {\bf B} is connected to {\bf D},
    then {\bf B} and {\bf C} are assumed to be one unit away from {\bf A},
    while {\bf D} is two units away from {\bf A}.

    In some cases the requests and notifications are sent to every possible
    destination, and the order is not significant (like it is for
    \Ref{notify_file_done}() request). Others should be sent to the closest
    destinations first, and only then to the farthest, in case if they have
    not been processed by the closest. The examples are \Ref{request_data}(),
    \Ref{notify_error}() and \Ref{notify_status}().

    The user is not expected to create the #DjVuPortcaster# itselt. He should
    use \Ref{get_portcaster}() global function instead.
    */
class DjVuPortcaster : public DjVuPort
{
public:
      /// The default constructor
   DjVuPortcaster(void);

   virtual ~DjVuPortcaster(void);

      /** Adds route from #src# to #dst#. Whenever a request is
	  sent or received by #src#, it will be forwarded to #dst#
	  as well.

	  @param src The source
	  @param dst The destination */
   void		add_route(const DjVuPort * src, DjVuPort * dst);

      /** The opposite of \Ref{add_route}(). Removes the association
	  between #src# and #dst# */
   void		del_route(const DjVuPort * src, DjVuPort * dst);

      /** Copies all incoming and outgoing routes from #src# to
	  #dst#. This function should be called when a \Ref{DjVuPort} is
	  copied, if you want to preserve the connectivity. */
   void		copy_routes(DjVuPort * dst, const DjVuPort * src);

      /** Every \Ref{DjVuPort} calls this function in its constructor to
	  signal its availability for signaling. You will hardly want
	  to call this function yourself. */
   void		add_port(const DjVuPort * port);

      /** Every \Ref{DjVuPort} calls this function in its destructor to
	  request its removal from the route maps. You may want to call it
	  yourself when you need to disconnect a given port from any
	  senders or recipients. */
   void		del_port(const DjVuPort * port);

      /// Returns 1 if the port is listed in the internal map. #ZERO# otherwise.
   bool		is_port_alive(const DjVuPort * port);

      /// Returns 1 if #class_name# is #"DjVuPort"# or #"DjVuPortcaster".#
   virtual bool		inherits(const char * class_name) const;

      /** Computes destination list for #source# and calls the corresponding
	  function in each of the ports from the destination list starting from
	  the closest until one of them returns non-empty \Ref{GURL}. */
   virtual GURL		id_to_url(const DjVuPort * source, const char * id);
      /** Computes destination list for #source# and calls the corresponding
	  function in each of the ports from the destination list {\bf
	  starting from the farthest} until one of then returns non #ZERO#
	  pointer to \Ref{DjVuFile}. */
   virtual GPBase	get_cached_file(const DjVuPort * source, const GURL & url);
      /** Computes destination list for #source# and calls the corresponding
	  function in each of the ports from the destination list starting from
	  the closest until one of them returns non-zero \Ref{DataRange}. */
   virtual GP<DataRange>request_data(const DjVuPort * source, const GURL & url);

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
   virtual void		notify_redisplay(const DjVuPort * source);

      /** Computes destination list for #source# and calls the corresponding
	  function in each of the ports from the destination list starting from
	  the closest. */
   virtual void		notify_relayout(const DjVuPort * source);

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
      // We use these 'void *' to minimize number of template, which need
      // to be expanded. Ugly, but helps save space.
   GCriticalSection		map_lock;
   GMap<const void *, void *>	route_map;	// GMap<DjVuPort *, GList<DjVuPort *> *>
   GMap<const void *, void *>	cont_map;	// GMap<DjVuPort *, void *>

   void		add_to_closure(GMap<const void *, void *> & set,
			       const DjVuPort * dst, int distance);
   void		compute_closure(GMap<const void *, void *> & set,
				const DjVuPort * src);
   void		sort_closure(const GMap<const void *, void *> & set,
			     GList<const void *> & list);
};

/// Use this function to get a copy of the global \Ref{DjVuPortcaster}.
extern DjVuPortcaster * get_portcaster(void);

inline
DjVuPort::DjVuPort(bool dont_add_to_pcaster)
{
   if (!dont_add_to_pcaster) get_portcaster()->add_port(this);
}

inline
DjVuPort::DjVuPort(const DjVuPort & port)
{
   get_portcaster()->copy_routes(this, &port);
}

inline
DjVuPort::~DjVuPort(void)
{
   get_portcaster()->del_port(this);
}

inline DjVuPort &
DjVuPort::operator=(const DjVuPort & port)
{
   if (this!=&port)
      get_portcaster()->copy_routes(this, &port);
   return *this;
}

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

inline bool
DjVuPortcaster::inherits(const char * class_name) const
{
   return
      !strcmp("DjVuPortcaster", class_name) ||
      DjVuPort::inherits(class_name);
}

//@}

#endif
