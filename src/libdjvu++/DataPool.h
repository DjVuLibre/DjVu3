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
//C- $Id: DataPool.h,v 1.4 1999-06-04 16:04:05 leonb Exp $
 
#ifndef _DATAPOOL_H
#define _DATAPOOL_H

#ifdef __GNUC__
#pragma interface
#endif

#include "ByteStream.h"
#include "GThreads.h"
#include "GSmartPointer.h"
#include "GPContainer.h"

/** @name DataPool.h
    Files #"DataPool.h"# and #"DataPool.cpp"# implement classes \Ref{DataPool}
    and \Ref{DataRange} used by DjVu decoder to access data.

    The main goal of class \Ref{DataPool} is to provide concurrent access
    to the same data from many threads with a possibility to add data
    from another thread. It is especially important in the case of the
    Netscape plugin when data is not immediately available, but decoding
    should be started as soon as possible. In this situation it is vital
    to provide transparent access to the data from many threads possibly
    blocking readers that try to access information that has not been
    received yet.

    @memo Thread safe data storage
    @author Andrei Erofeev <eaf@geocities.com>, L\'eon Bottou <leonb@research.att.com>
    @version #$Id: DataPool.h,v 1.4 1999-06-04 16:04:05 leonb Exp $#
*/

//@{

/** Thread safe data file.  
    Each #DataPool# object accepts data from outside (see \Ref{add_data}())
    and stores it inside for immediate access (see \Ref{get_data}()) from
    other threads.

    This class is derived from \Ref{MemoryByteStream}, which allows it to
    handle underlying data storage efficiently, and it's also
    thread-protected.  It means, that more than one thread can attempt to
    request data from it at the same time.

    The interface of \Ref{get_data}() function allows to request data from
    any portion of the #DataPool#. In case if there is still not enough data
    in the #DataPool#, the reader will be {\em blocked} until at least some
    of desired data is available.
    
    This class is ideal for decoding multipage DjVu files in Netscape when
    you want to start decoding as soon as you can not waiting for the arrival
    of the whole file. In this case you may have many decoding threads running
    at the same time and reading/waiting for data from different portions of
    the #DataPool# and the main thread getting data from the Netscape and
    adding it to the #DataPool#.

    The #DataPool# also provides a set of callbacks or {\em triggers} called
    when a given amount of data has been received. It's a useful feature
    when you don't want to block waiting for data in a \Ref{get_data}()
    request, but still want to be informed when a non-block read can be
    possible.  */

class DataPool : public GPEnabled, protected MemoryByteStream
{
private:
   class Reader : public GPEnabled
   {
   public:
      GEvent	event;
      void	* reader_id;
      int	offset;
      int	size;
      int	stop_flag;
      Reader(void) : reader_id(0), offset(0), size(-1), stop_flag(0) {};
      Reader(void * reader_id_in, int offset_in=0, int size_in=-1) :
	    reader_id(reader_id_in), offset(offset_in),
	    size(size_in), stop_flag(0) {};
      virtual ~Reader(void) {};
   };
   class Trigger : public GPEnabled
   {
   public:
      int	thresh;
      void	(* callback)(void *);
      void	* cl_data;

      Trigger(void) : thresh(0), callback(0), cl_data(0) {};
      Trigger(int xthresh, void (* xcallback)(void *), void * xcl_data) :
	    thresh(xthresh), callback(xcallback), cl_data(xcl_data) {};
      virtual ~Trigger(void) {};
   };

   int		eof_flag;
   int		stop_flag;

   GPList<Reader>	readers_list;
   GPList<Trigger>	triggers_list;

   GCriticalSection	data_lock, readers_lock, triggers_lock;
   
   void		wait_for_data(const GP<Reader> & reader);
   void		int_add_data(void * buffer, int size);
public:
      /** Appends the new block of data to the #DataPool#. Will unblock
	  readers waiting for data if this data arrives with this block.
	  It may also trigger some {\em trigger callbacks}, which may have
	  been added by means of \Ref{add_trigger}() function.

	  {\bf Note:} After all the data has been added, it's necessary
	  to call \Ref{set_eof}() to tell the #DataPool# that nothing else
	  is expected.

	  @param buffer data to append
	  @param size length of the {\em buffer}
      */
   void		add_data(void * buffer, int size);
   
      /** Attempts to return a block of data at the given #offset#
	  of the given #size#. If some of the data requested is in the
	  internal array, it is returns immediately. Otherwise the reader
	  (and the thread) is {\bf blocked} until the data actually arrives.
	  Please note, that since the reader is blocked, it should run in a
	  separate thread so that other threads have a chance to call
	  \Ref{add_data}()

	  @param buffer Buffer to be filled with data
	  @param offset Offset in the #DataPool# to read data at
	  @param size Size of the {\em buffer}
	  @param reader_id ID uniquely identifying the reader.
	  	 Basically this is important for stopping a given reader
		 when necessary by means of \Ref{stop_reader}() function.
	  @return The number of bytes actually read
	  @exception EOF End of file encountered
	  @exception STOP The stream has been stopped
      */
   int		get_data(void * buffer, int offset, int size, void * reader_id);
   
      /** Tells the #DataPool# that no more data will be added by means
	  of \Ref{add_data}() function */
   void		set_eof(void);

      /// Returns 1 if no more data is planned to be added
   bool		is_eof(void) const;

      /** Returns the size of stored data. */
   long		get_size(void) const;

      /** Adds a so-called {\em trigger callback} to be called when
	  a given amount of data has been added. Since reading unavailable
	  data may result in a thread block, which may be bad, this
	  appears to be a convenient way to signal availability of data.

	  @param thresh There should be at least {\em thresh}-1 bytes of
	         data for the callback to be executed. If {\em thresh} is
		 negative, the callback is called after all data has been
		 added and \Ref{set_eof}() has been called.
	  @param callback Function to call
	  @param cl_data Argument to pass to the callback when it's called. */
   void		add_trigger(int thresh, void (* callback)(void *), void * cl_data);

      /** Use this function to unregister callbacks, which are no longer
	  needed. {\bf Note!} It's important to do it when the client
	  is about to be destroyed. */
   void		del_trigger(void (* callback)(void *), void * cl_data);
   
      /** Tells the #DataPool# to stop all readers waiting for data.
	  This will unlock the threads (readers) and will throw an exceptions
	  in each of them with text #STOP#. */
   void		stop_all_readers(void);

      /** Tells the #DataPool# to stop the reader with given #reader_id#.
	  Since every reading thread has to pass this ID to the \Ref{get_data}()
	  function (the place where it may be blocked), #DataPool# knows exactly
	  what reader (thread) to stop.

	  @param reader_id ID of the thread to stop
      */
   void		stop_reader(void * reader_id);

      /// The default constructor
   DataPool(void);

   virtual ~DataPool(void);
};

inline
DataPool::DataPool(void) : eof_flag(0), stop_flag(0)
{
}

inline
DataPool::~DataPool(void)
{
}

inline bool
DataPool::is_eof(void) const
{
   return eof_flag;
}

inline long
DataPool::get_size(void) const
{
   return size();
}

/** #DataRange# - convenient way for accessing data in \Ref{DataPool}.

    Normally, the program should not use \Ref{DataPool} directly for
    {\em reading} data from it. It is supposed to create the #DataRange#,
    which is mapped to a given portion of the \Ref{DataPool} and which can
    provide either random or sequential access to the data within the valid
    range.

    The #DataRange# can return data either through its \Ref{get_data}()
    function or using a special stream created and returned by
    \Ref{get_stream}() function in which case the access is obviously
    sequential.
    */

class DataRange : public GPEnabled
{
private:
   class Trigger : public GPEnabled
   {
   public:
      void	(* callback)(void *);
      void	* cl_data;

      Trigger(void) : callback(0), cl_data(0) {};
      Trigger(void (* xcallback)(void *), void * xcl_data) :
	    callback(xcallback), cl_data(xcl_data) {};
      virtual ~Trigger(void) {};
   };
   void		init(void);
public:
      /** The construtor.

	  @param pool The \Ref{DataPool} which contains the data
	  @param start Beginning of the data range in the {\em pool}
	  @param length Length of the data range. If negative, the
	  	#DataRange# is assumed to extend up to the end of
		the {\em pool}. */
   
   DataRange(const GP<DataPool> & pool, long start=0, long length=-1);

      /// Copy constructor
   DataRange(const DataRange & r);

      /// The destructor
   virtual ~DataRange(void);

      /** Returns the \Ref{ByteStream} for sequential access to the underlying
	  data. */
   ByteStream *		get_stream(void);

      /** Returns the \Ref{DataPool} containing the real data */
   GP<DataPool>		get_pool(void) const;

      /** Returns the start offset of the data range in the \Ref{DataPool} */
   long			get_start(void) const;

      /** Returns the length of the data range in the \Ref{DataPool}. The
	  length may now always be known because the #DataRange# may have
	  been created with #length=-1# and EOF condition has not been
	  set yet in the \Ref{DataPool}. In this case the #DataRange#
	  tries its best to guess the length by parsing the data with hope that
	  it's IFF data. If this attempt does not succeed, -1 is returned */
   long			get_length(void) const;

      /** Provides random access to the data range from the pool.

	  @param buffer Where to put data
	  @param offset Offset from the data range's start to get data from
	  @param size How much data to copy
	  @exception EOF End of file encountered
	  @exception STOP The stream has been stopped
      */
   int			get_data(void * buffer, int offset, int size);

      /** When the data that has been requested by a thread is not available
	  in the \Ref{DataPool}, the thread is blocked. By calling this
	  function any blocked thread is unblocked by an exception thrown in
	  it with text #STOP#. All following \Ref{get_data}() calls and
	  attempts to access data by the stream returned by \Ref{get_stream}()
	  function will result in #STOP# exception. */
   void			stop(void);

      /** Like in \Ref{DataPool}, you may add your callbacks to #DataRange#
	  as well. In this case the #thresh# is relative to the
	  #DataRange#, of course.

	  @param thresh There should be at least {\em thresh}-1 bytes
	         of data available for reading from the beginning of the
		 #DataRange# for the callback to be called. If {\em thresh}
		 is negative, the callback will be called when data for the
		 whole #DataRange# is available.
	  @param callback The callback to call
	  @param cl_data Data to be passed to the callback */
   void			add_trigger(int thresh, void (* callback)(void *),
				    void * cl_data);

      /** Removes the given callback from the list. {\bf Note!} It's important
	  to call this function before the client is destroyed */
   void			del_trigger(void (* callback)(void *), void * cl_data);
private:
   GP<DataPool>	pool;
   long		start, length;
   bool		stop_flag;

   GPList<Trigger>	triggers_list;
   GCriticalSection	triggers_lock, trigger_lock;

   static void	static_trigger_cb(void *);
   void		trigger_cb(void);
   void		analyze_iff(void);
};

inline GP<DataPool>
DataRange::get_pool(void) const { return pool; }

inline long
DataRange::get_start(void) const { return start; }

inline long
DataRange::get_length(void) const { return length; }

//@}

#endif
