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
//C- $Id: DataPool.cpp,v 1.1.2.7 1999-05-04 20:37:13 eaf Exp $

#ifdef __GNUC__
#pragma implementation
#endif

#include "DataPool.h"
#include "IFFByteStream.h"
#include "GString.h"
#include "debug.h"

//****************************************************************************
//******************************* DataPool ***********************************
//****************************************************************************

void
DataPool::add_data(void * buffer, int buffer_size)
{
   DEBUG_MSG("DataPool::add_data(): adding " << buffer_size << " bytes of data...\n");
   DEBUG_MAKE_INDENT(3);

      // First - add data to the underlying stream
   {
      GCriticalSectionLock dlock(&data_lock);
      seek(0, SEEK_END);
      writall(buffer, buffer_size);
   }
   
      // Now - wake up all threads, which may be waiting for this data
   {
      GCriticalSectionLock slock(&readers_lock);
      for(GPosition pos=readers_list;pos;++pos)
      {
	 GP<Reader> reader=readers_list[pos];
	 if (reader->offset<size())
	 {
	    DEBUG_MSG("waking up reader: offset=" << reader->offset <<
		      ", size=" << reader->size << "\n");
	    reader->event.set();
	 };
      };
   }

      // Check if we need to run any trigger callbacks
   {
      GCriticalSectionLock tlock(&triggers_lock);
      for(GPosition pos=triggers_list;pos;)
      {
	 GP<Trigger> trigger=triggers_list[pos];
	 if (trigger->thresh>=0 && trigger->thresh<size())
	 {
	    if (trigger->callback) trigger->callback(trigger->cl_data);
	    GPosition this_pos=pos;
	    ++pos;
	    triggers_list.del(this_pos);
	 } else ++pos;
      };
   }
}

int
DataPool::get_data(void * buffer, int offset, int sz, void * reader_id)
{
   if (stop_flag) THROW("STOP");

   {
	 // Check the cache: may the data is partially there...
      GCriticalSectionLock dlock(&data_lock);
      if (offset<size())
      {
	 if (size()-offset<sz) sz=size()-offset;
	 seek(offset, SEEK_SET);
	 return readall(buffer, sz);
      };
   }
      // No useful data in the underlying byte stream

      // If nothing else is expected => return 0
   if (eof_flag) return 0;
   
      // Some data is still expected => add this reader to the
      // list of readers and call virtual wait_for_data()
   DEBUG_MSG("DataPool::get_data(): There is no data in the pool.\n");
   GP<Reader> reader=new Reader(reader_id, offset, sz);
   TRY {
      {
	 GCriticalSectionLock slock(&readers_lock);
	 readers_list.append(reader);
      }
      wait_for_data(reader);
   } CATCH(exc) {
      {
	 GCriticalSectionLock slock(&readers_lock);
	 GPosition pos;
	 if (readers_list.search(reader, pos)) readers_list.del(pos);
      }
      RETHROW;
   } ENDCATCH;
   
   {
      GCriticalSectionLock slock(&readers_lock);
      GPosition pos;
      if (readers_list.search(reader, pos)) readers_list.del(pos);
   }

      // This call to get_data() should return immediately as there MUST
      // be data in the buffer after wait_for_data(reader) returns
      // or eof_flag should be TRUE
   return get_data(buffer, reader->offset, reader->size, reader->reader_id);
}

void
DataPool::wait_for_data(const GP<Reader> & reader)
      // This function may NOT return until there is some data for the
      // given reader in the internal buffer
{
   DEBUG_MSG("DataPool::wait_for_data(): waiting for data at offset=" << reader->offset <<
	     ", id=" << reader->reader_id << "\n");
   DEBUG_MAKE_INDENT(3);

   if (stop_flag) THROW("STOP");

#if THREADMODEL==NOTHREADS
   THROW("Internal error. This function can't be used in threadless mode.");
#else
   while(1)
   {
      if (stop_flag || reader->stop_flag) THROW("STOP");
      {
	 GCriticalSectionLock dlock(&data_lock);
	 if (reader->offset<size()) return;
      };
      if (eof_flag) return;

      DEBUG_MSG("calling event.wait()...\n");
      reader->event.wait();
   };
#endif
   
   DEBUG_MSG("Got some data to read\n");
}

void
DataPool::set_eof(void)
{
   eof_flag=1;

      // Wake up everybody to let them rescan flags
   {
      GCriticalSectionLock lock(&readers_lock);
      for(GPosition pos=readers_list;pos;++pos)
	 readers_list[pos]->event.set();
   }
      // Activate all trigger callbacks with negative threshold
   {
      GCriticalSectionLock lock(&triggers_lock);
      for(GPosition pos=triggers_list;pos;)
      {
	 GP<Trigger> trigger=triggers_list[pos];
	 if (trigger->thresh<0)
	 {
	    if (trigger->callback) trigger->callback(trigger->cl_data);
	    GPosition this_pos=pos;
	    ++pos;
	    triggers_list.del(this_pos);
	 } else ++pos;
      };
   }
}

void
DataPool::stop_all_readers(void)
{
   DEBUG_MSG("DataPool::stop_all_readers(): Stopping all readers\n");
   
   stop_flag=1;

      // Wake up everybody to let them rescan flags
   GCriticalSectionLock slock(&readers_lock);
   for(GPosition pos=readers_list;pos;++pos)
      readers_list[pos]->event.set();
}

void
DataPool::stop_reader(void * reader_id)
{
   DEBUG_MSG("DataPool::stop_reader(): Stopping reader with ID=" << reader_id << "\n");
   DEBUG_MAKE_INDENT(3);
   
      // Find the reader object with given reader_id
   GCriticalSectionLock slock(&readers_lock);
   for(GPosition pos=readers_list;pos;++pos)
   {
      Reader & str=*(readers_list[pos]);
      if (str.reader_id==reader_id)
      {
	 DEBUG_MSG("found one!\n");
	 str.stop_flag=1;	// Set the flag
	 str.event.set();	// And wake the reader up
      };
   };
}

void
DataPool::add_trigger(int thresh, void (* callback)(void *), void * cl_data)
{
   if (callback)
   {
      GCriticalSectionLock lock(&triggers_lock);
      if (thresh<0 && is_eof() || thresh>=0 && size()>thresh) callback(cl_data);
      else if (is_eof() && thresh>=size()) THROW("Threshold is too big.");
      else triggers_list.append(new Trigger(thresh, callback, cl_data));
   }
}

void
DataPool::del_trigger(void (* callback)(void *), void * cl_data)
{
   GCriticalSectionLock lock(&triggers_lock);
   for(GPosition pos=triggers_list;pos;)
   {
      GP<Trigger> t=triggers_list[pos];
      if (t->callback==callback && t->cl_data==cl_data)
      {
	 GPosition this_pos=pos;
	 ++pos;
	 triggers_list.del(this_pos);
      } else ++pos;
   }
}

//****************************************************************************
//****************************** PoolByteStream ******************************
//****************************************************************************

// This is an internal ByteStream receiving data from the associated DataRange.
// It's just a sequential interface, nothing more. All the jon is done in
// DataRange and DataPool

class PoolByteStream : public ByteStream
{
public:
   PoolByteStream(DataRange * data_range);
   virtual ~PoolByteStream() {};

   virtual size_t read(void *buffer, size_t size);
   virtual size_t write(const void *buffer, size_t size);
   virtual void seek(long offset, int whence = SEEK_SET);
   virtual long tell();
   virtual int  is_seekable(void) const { return 1; };
private:
      // Don't make data_range GP<>. The problem is that DataRange creates
      // and soon destroys this ByteStream from the constructor. Since
      // there are no other pointers to the DataRange created yet, it becomes
      // destroyed immediately :(
   DataRange		* data_range;
   long			position;

      // Cancel C++ default stuff
   PoolByteStream & operator=(const PoolByteStream &);
};

inline
PoolByteStream::PoolByteStream(DataRange * xdata_range) :
   data_range(xdata_range), position(0)
{
   if (!data_range) THROW("Internal error: ZERO DataRange passed as input.");
}

size_t
PoolByteStream::read(void *buffer, size_t size)
{
   size=data_range->get_data(buffer, position, size);
   position+=size;
   return size;
}

size_t
PoolByteStream::write(const void *buffer, size_t size)
{
   THROW("write() is not implemented.");
   return 0;	// For compiler not to bark
}

void
PoolByteStream::seek(long offset, int whence)
{
   long length=data_range->get_length();
   long pos;
   switch(whence)
   {
      case SEEK_CUR:
	 pos=position+offset; break;
      case SEEK_END:
	 if (length<0) THROW("Can't seek from the end of the stream.");
	 pos=length-offset;
	 break;
      default:
	 pos=offset;
	 break;
   };
   if (pos<0) pos=0;
   if (length>=0 && pos>length) pos=length;
   position=pos;
}

long
PoolByteStream::tell(void)
{
   return position;
}

//****************************************************************************
//******************************** DataRange *********************************
//****************************************************************************

void
DataRange::init(void)
{
   DEBUG_MSG("DataRange::init(): initializing\n");
   DEBUG_MAKE_INDENT(3);

   if (!pool) THROW("ZERO data pool passed as input.");
   if (length<0 && pool->is_eof()) length=pool->get_size()-start;
   if (length<0)
   {
      GPosition pos=triggers_list;
      pool->add_trigger(-1, static_trigger_cb, this);
      pool->add_trigger(start+32, static_trigger_cb, this);
   }
}

DataRange::DataRange(const GP<DataPool> & xpool, long xstart, long xlength) :
      pool(xpool), start(xstart), length(xlength), stop_flag(0)
{
   init();
}

DataRange::DataRange(const DataRange & r) : pool(r.pool),
   start(r.start), length(r.length), stop_flag(0)
{
   init();
}

DataRange::~DataRange(void)
{
   DEBUG_MSG("DataRange::~DataRange(): destroying, this=" << this << "\n");
   DEBUG_MAKE_INDENT(3);

   GCriticalSectionLock lock(&trigger_lock);
   pool->del_trigger(static_trigger_cb, this);

   DEBUG_MSG("done destroying DataRange\n");
}

int
DataRange::get_data(void * buffer, int offset, int size)
{
   if (stop_flag) THROW("STOP");

   if (offset<0) THROW("Internal error: attempt to read outside of DataRange.");
   
   if (length>=0)
   {
      if (offset>length) THROW("Internal error: attempt to read outside of DataRange.");
      if (offset+size>length) size=length-offset;
   };

   return pool->get_data(buffer, start+offset, size, this);
}

void
DataRange::stop(void)
{
   stop_flag=1;
   pool->stop_reader(this);
}

ByteStream *
DataRange::get_stream(void)
{
   return new PoolByteStream(this);
}

void
DataRange::static_trigger_cb(void * cl_data)
{
   DataRange * th=(DataRange *) cl_data;
   th->trigger_cb();
}

void
DataRange::trigger_cb(void)
{
      // Don't want to be destroyed while I'm here. Can't use GP<> life saver
      // because it may be called from the constructor
   GCriticalSectionLock lock(&trigger_lock);
   
   DEBUG_MSG("DataRange::trigger_cb(): DataPool has enough data now\n");
   DEBUG_MAKE_INDENT(3);

   if (length<0 && pool->is_eof())
      length=pool->get_size()-start;
   if (length<0) analyze_iff();

   if (length>=0)
   {
	 // Since we know the length now, we can pass the list of triggers
	 // to the DataPool.
      GCriticalSectionLock lock(&triggers_lock);
      for(GPosition pos=triggers_list;pos;++pos)
      {
	 GP<Trigger> trigger=triggers_list[pos];
	 pool->add_trigger(start+length-1, trigger->callback, trigger->cl_data);
      }
      triggers_list.empty();
   }
}

void
DataRange::add_trigger(int thresh, void (* callback)(void *), void * cl_data)
{
   if (length>=0 && thresh>=length)
      THROW("Trigger threshold is beyond DataRange.");
   if (thresh>=0) pool->add_trigger(start+thresh, callback, cl_data);
   else if (length>=0) pool->add_trigger(start+length-1, callback, cl_data);
   else
   {
      GCriticalSectionLock lock(&triggers_lock);
      triggers_list.append(new Trigger(callback, cl_data));
   }
}

void
DataRange::del_trigger(void (* callback)(void *), void * cl_data)
{
   GCriticalSectionLock lock(&triggers_lock);
   for(GPosition pos=triggers_list;pos;)
   {
      GP<Trigger> t=triggers_list[pos];
      if (t->callback==callback && t->cl_data==cl_data)
      {
	 GPosition this_pos=pos;
	 ++pos;
	 triggers_list.del(this_pos);
      } else ++pos;
   }
   pool->del_trigger(callback, cl_data);
}

void
DataRange::analyze_iff(void)
      // In order to display decode progress properly, we need to know
      // the size of the data. Sometimes DataRange is created with length>0
      // and this length is exactly the size. But length may be negative, which
      // means, that DataRange goes up to the end of DataPool. Of course,
      // if pool->is_eof() is TRUE, we can get the DataRange's size. But if
      // it's not, the best thing we can do is to try to analyze the
      // data (it should be IFF most of the time) and set length accordingly.
{
   DEBUG_MSG("DataRange::analyze_iff(): Trying to decode IFF structure.\n");
   DEBUG_MSG("in order to determine the DataRange's size\n");
   DEBUG_MAKE_INDENT(3);
   
   ByteStream * str=0;
   TRY {
      str=get_stream();
      IFFByteStream iff(*str);
      GString chkid;
      int size;
      if ((size=iff.get_chunk(chkid)) &&
	  size>=0 && size<10*1024*1024)
      {
	 length=size+iff.tell()-4;
	 DEBUG_MSG("Got size=" << size << ", length=" << length << "\n");
      }
   } CATCH(exc) {
      delete str; str=0;
   } ENDCATCH;
   delete str; str=0;
}
