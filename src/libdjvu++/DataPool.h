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
//C- $Id: DataPool.h,v 1.1.2.2 1999-04-26 18:31:35 eaf Exp $
 
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
    File #"DataPool.h"# contains the implementation of the \Ref{DataPool}
    and \Ref{PoolSubByteStream} classes, which are used by \Ref{GBSTranslator}
    and \Ref{DjVuDecManager} to share file contents between many threads
    and to feed the data into \Ref{DejaVuDecoder}s.

    @memo Data storage with compatible byte streams.
    @author Andrei Erofeev
    @version #$Id: DataPool.h,v 1.1.2.2 1999-04-26 18:31:35 eaf Exp $#
*/

//@{

/** #DataPool# is an object, which accepts data from outside (see
    \Ref{add_data}()) and stores it inside for later access through
    \Ref{get_data}() function.

    It is derived from \Ref{MemoryByteStream}, which allows it to handle
    underlying data storage efficiently, and it's also thread-protected.
    It means, that more than one thread can attempt to request data from
    it at the same time.

    The interface of \Ref{get_data}() function allows to request data from
    any portion of the #DataPool#. In case if there is still not enough data
    in the #DataPool#, the reader will be {\em blocked} until at least some
    of desired data is available.
    
    This class is ideal for decoding DJVM files in Netscape when you want
    to start decoding as fast as you can not waiting for the arrival of the
    whole file. In this case you may have many decoding threads running
    at the same time and reading/waiting for data from different portions of
    the #DataPool# and the main thread getting data from the Netscape and
    adding it to the #DataPool#.
*/

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
   
      /** Attempts to return a block of data at the given {\em offset}
	  of the given {\em size}. If some of the data requested is in the
	  internal array, it is returns immediately. Otherwise the reader
	  (and the thread) is {\bf blocked} until the data actually arrives.
	  Please note, that since the reader is blocked, it should run in a
	  separate thread so that other threads have a chance to call
	  \Ref{add_data}()

	  @param buffer Buffer to be filled with data
	  @param offset Offset in the \Ref{DataPool} to read data at
	  @param size Size of the {\em buffer}
	  @param reader_id ID uniquely identifying the reader.
	  	 Basically this is important for stopping a given reader
		 when necessary by means of \Ref{stop_reader}() function.
	  @return The number of bytes actually read
	  @exception EOF End of file encountered
	  @exception STOP The stream has been stopped
      */
   int		get_data(void * buffer, int offset, int size, void * reader_id);
   
      /// Tells the #DataPool# that no more data will arrive
   void		set_eof(void);

      /// Returns 1 if no more data is planned to be added
   bool		is_eof(void) const;

      /** Returns the size of stored data. */
   long		get_size(void) const;

      /** {\em Trigger callbacks} are functions called when a given amount
	  of data has been added. Since writing and reading can be done
	  from separate threads, this appears to be a convenient way to
	  signal availability of data.

	  @param thresh There should be at least {\em thresh} bytes of
	         data for the callback to be executed. If {\em thresh} is
		 negative, the callback is called after all data has been
		 added and \Ref{set_eof}() has been called.
	  @param callback Function to call
	  @param cl_data Argument to pass to the callback when it's called. */
   void		add_trigger(int thresh, void (* callback)(void *), void * cl_data);
   
      /** Tells the #DataPool# to stop all readers waiting for data.
	  This will unlock the threads (readers) and will throw an exceptions
	  in each of them with text #STOP#. */
   void		stop_all_readers(void);

      /** Tells the #DataPool# to stop the reader with given {\em reader_id}.
	  Since every reading thread has to pass this ID to the \Ref{get_data}()
	  function (the place where it may be blocked), #DataPool# knows exactly
	  what reader (thread) to stop.

	  @param reader_id ID of the thread to stop
      */
   void		stop_reader(void * reader_id);

      /// The constructor
   DataPool(void);

      /// The destructor
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

/** #DataRange# - range of data in \Ref{DataPool}.
    
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
   DataRange(const GP<DataPool> & pool, long start=0, long length=-1);
   DataRange(const DataRange & r);

   ByteStream *		get_stream(void);
   GP<DataPool>		get_pool(void) const;
   long			get_start(void) const;
   long			get_length(void) const;
   int			get_data(void * buffer, int offset, int size);

   void			stop(void);

      // Make thresh<0 to be called when all data for THIS DataRange
      // has been received. DataPool may still miss some megs.
   void			add_trigger(int thresh, void (* callback)(void *),
				    void * cl_data);
private:
   GP<DataPool>	pool;
   long		start, length;
   bool		stop_flag;

   GPList<Trigger>	triggers_list;
   GCriticalSection	triggers_lock;

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
