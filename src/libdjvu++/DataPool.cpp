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
//C- $Id: DataPool.cpp,v 1.20 1999-09-17 20:19:59 eaf Exp $

#ifdef __GNUC__
#pragma implementation
#endif

#include "DataPool.h"
#include "IFFByteStream.h"
#include "GString.h"
#include "debug.h"

static inline void
call_callback(void (* callback)(void *), void * cl_data)
{
   TRY {
      if (callback) callback(cl_data);
   } CATCH(exc) {} ENDCATCH;
}

//****************************************************************************
//****************************** BlockList ***********************************
//****************************************************************************

// Since data can be added to the DataPool at any offset now, there may
// be white spots, which contain illegal data. This class is to contain
// the list of valid and invalid regions.
// The class is basically a list of integers. Abs(integer)=size of the
// block. If the integer is positive, data for the block is known.
// Otherwise it's unkown.

void
DataPool::BlockList::add_range(int start, int length)
      // Adds range of known data.
{
   if (start<0) THROW("The start offset of the range may not be negative.");
   if (length<0) THROW("The length must be positive.");
   if (length>0)
   {
      GCriticalSectionLock lk(&lock);

	 // Look thru existing zones, change their sign and split if
	 // necessary.
      GPosition pos=list;
      int block_start=0, block_end=0;
      while(pos && block_start<start+length)
      {
	 int size=list[pos];
	 block_end=block_start+abs(size);
	 if (size<0)
	    if (block_start<start)
	    {
	       if (block_end>start && block_end<=start+length)
	       {
		  list[pos]=-(start-block_start);
		  list.insert_after(pos, block_end-start);
		  ++pos;
		  block_start=start;
	       } else if (block_end>start+length)
	       {
		  list[pos]=-(start-block_start);
		  list.insert_after(pos, length);
		  ++pos;
		  list.insert_after(pos, -(block_end-(start+length)));
		  ++pos;
		  block_start=start+length;
	       }
	    } else if (block_start>=start && block_start<start+length)
	    {
	       if (block_end<=start+length) list[pos]=abs(size);
	       else
	       {
		  list[pos]=start+length-block_start;
		  list.insert_after(pos, -(block_end-(start+length)));
		  ++pos;
		  block_start=start+length;
	       }
	    }
	 block_start=block_end;
	 ++pos;
      }
      if (block_end<start)
      {
	 list.append(-(start-block_end));
	 list.append(length);
      } else if (block_end<start+length) list.append(start+length-block_end);

	 // Now merge adjacent areas with the same sign
      pos=list;
      while(pos)
      {
	 GPosition pos1=pos; ++pos1;
	 while(pos1)
	 {
	    if (list[pos]<0 && list[pos1]>0 ||
		list[pos]>0 && list[pos1]<0)
	       break;
	    list[pos]+=list[pos1];
	    GPosition this_pos=pos1;
	    ++pos1;
	    list.del(this_pos);
	 }
	 pos=pos1;
      }
   } // if (length>0)
}

int
DataPool::BlockList::get_bytes(int start, int length) const
      // Returns the number of bytes of data available in the range
      // [start, start+length[. There may be holes between data chunks
{
   if (length<0) THROW("Length must be positive.");

   GCriticalSectionLock lk((GCriticalSection *) &lock);
   int bytes=0;
   int block_start=0, block_end=0;
   for(GPosition pos=list;pos && block_start<start+length;++pos)
   {
      int size=list[pos];
      block_end=block_start+abs(size);
      if (size>0)
	 if (block_start<start)
	 {
	    if (block_end>=start && block_end<start+length)
	       bytes+=block_end-start;
            else if (block_end>=start+length)
	       bytes+=length;
	 } else
	 {
	    if (block_end<=start+length)
	       bytes+=block_end-block_start;
	    else bytes+=start+length-block_start;
	 }
      block_start=block_end;
   }
   return bytes;
}

int
DataPool::BlockList::get_range(int start, int length) const
      // Finds a range covering offset=start and returns the length
      // of intersection of this range with [start, start+length[
      // 0 is returned if nothing can be found
{
   if (start<0) THROW("Start must be non negative.");
   if (length<=0) THROW("Length must be positive.");

   GCriticalSectionLock lk((GCriticalSection *) &lock);
   int block_start=0, block_end=0;
   for(GPosition pos=list;pos && block_start<start+length;++pos)
   {
      int size=list[pos];
      block_end=block_start+abs(size);
      if (block_start<=start && block_end>start)
	 if (size<0) return -1;
         else
	    if (block_end>start+length) return length;
            else return block_end-start;
      block_start=block_end;
   }
   return 0;
}

//****************************************************************************
//******************************* DataPool ***********************************
//****************************************************************************

void
DataPool::init(void)
{
   DEBUG_MSG("DataPool::init(): Initializing\n");
   DEBUG_MAKE_INDENT(3);
   
   start=0; length=-1; add_at=0;
   eof_flag=false;
   stop_flag=false;
   stop_blocked_flag=false;

   data=new MemoryByteStream();
}

DataPool::DataPool(void)
{
   init();

      // If we maintain the data ourselves, we want to interpret its
      // IFF structure to predict its length
   add_trigger(0, 32, static_trigger_cb, this);
}

DataPool::DataPool(const GP<DataPool> & pool, int start, int length)
{
   init();
   connect(pool, start, length);
}

DataPool::DataPool(const char * fname, int start, int length)
{
   init();
   connect(fname, start, length);
}

DataPool::~DataPool(void)
{
   {
	 // Wait until the static_trigger_cb() exits
      GCriticalSectionLock lock(&trigger_lock);
      if (pool) pool->del_trigger(static_trigger_cb, this);
      del_trigger(static_trigger_cb, this);
   }

   if (pool)
   {
      GCriticalSectionLock lock(&triggers_lock);
      for(GPosition pos=triggers_list;pos;++pos)
      {
	 GP<Trigger> trigger=triggers_list[pos];
	 pool->del_trigger(trigger->callback, trigger->cl_data);
      }
   }
}

void
DataPool::connect(const GP<DataPool> & pool_in, int start_in, int length_in)
{
   DEBUG_MSG("DataPool::connect(): connecting to another DataPool\n");
   DEBUG_MAKE_INDENT(3);
   
   if (pool) THROW("Already connected to another DataPool.");
   if (stream) THROW("Already connected to a file.");
   if (start_in<0) THROW("Start must be non negative");

   pool=pool_in;
   start=start_in;
   length=length_in;

      // The following will work for length<0 too
   if (pool->has_data(start, length)) set_eof();
   else pool->add_trigger(start, length, static_trigger_cb, this);

   data=0;

   wake_up_all_readers();
   
      // Pass registered trigger callbacks to the DataPool
   GCriticalSectionLock lock(&triggers_lock);
   for(GPosition pos=triggers_list;pos;++pos)
   {
      GP<Trigger> t=triggers_list[pos];
      int tlength=t->length;
      if (tlength<0 && length>0) tlength=length-t->start;
      pool->add_trigger(start+t->start, tlength, t->callback, t->cl_data);
   }
}

void
DataPool::connect(const char * fname, int start_in, int length_in)
{
   DEBUG_MSG("DataPool::connect(): connecting to a file\n");
   DEBUG_MAKE_INDENT(3);
   
   if (pool) THROW("Already connected to a DataPool.");
   if (stream) THROW("Already connected to another file.");
   if (start_in<0) THROW("Start must be non negative");
     
   GP<StdioByteStream> str=new StdioByteStream(fname, "rb");
   str->seek(0, SEEK_END);
   int file_size=str->tell();

   stream=str;
   start=start_in;
   length=length_in;
   if (start>=file_size) length = 0;
   else if (length<0 || start+length>=file_size) length=file_size-start;

   eof_flag=true;

   data=0;

   wake_up_all_readers();
   
      // Call every trigger callback
   GCriticalSectionLock lock(&triggers_lock);
   for(GPosition pos=triggers_list;pos;++pos)
   {
      GP<Trigger> t=triggers_list[pos];
      call_callback(t->callback, t->cl_data);
   }
   triggers_list.empty();
}

int
DataPool::get_length(void) const
{
      // Not connected and length has been guessed
      // Or connected to a file
      // Or connected to a pool, but length was preset
   if (length>=0) return length;
   else if (pool)
   {
      int plength=pool->get_length();
      if (plength>=0) return plength-start;
   }
   return -1;	// Still unknown
}

int
DataPool::get_size(int dstart, int dlength) const
{
   if (dlength<0 && length>0)
   {
      dlength=length-dstart;
      if (dlength<0) return 0;
   }
   
   if (pool) return pool->get_size(start+dstart, dlength);
   else if (stream)
   {
      if (start+dstart+dlength>length) return length-(start+dstart);
      else return dlength;
   } else
   {
      if (dlength<0)
      {
	 GCriticalSectionLock lock((GCriticalSection *) &data_lock);
	 dlength=data->size()-dstart;
      }
      if (dlength<0) return 0;
      else return block_list.get_bytes(dstart, dlength);
   }
}

void
DataPool::add_data(const void * buffer, int size)
      // This function adds data sequentially at 'add_at' position
{
   DEBUG_MSG("DataPool::add_data(): adding " << size << " bytes of data...\n");
   DEBUG_MAKE_INDENT(3);

   add_data(buffer, add_at, size);
   add_at+=size;
}

void
DataPool::add_data(const void * buffer, int offset, int size)
{
   DEBUG_MSG("DataPool::add_data(): adding " << size << " bytes at pos=" <<
	     offset << "...\n");
   DEBUG_MAKE_INDENT(3);
   
   if (stream || pool)
      THROW("Function DataPool::add_data() may not be called for connected DataPools.");
   
      // Add data to the data storage
   {
      GCriticalSectionLock lock(&data_lock);
      if (offset>data->size())
      {
	 char ch=0;
	 data->seek(0, SEEK_END);
	 for(int i=data->size();i<offset;i++)
	    data->write(&ch, 1);
      } else
      {
	 data->seek(offset, SEEK_SET);
	 data->writall(buffer, size);
      }
   }

      // Modify map of blocks
   block_list.add_range(offset, size);
   
      // Wake up all threads, which may be waiting for this data
   {
      GCriticalSectionLock lock(&readers_lock);
      for(GPosition pos=readers_list;pos;++pos)
      {
	 GP<Reader> reader=readers_list[pos];
	 if (block_list.get_bytes(reader->offset, 1))
	 {
	    DEBUG_MSG("waking up reader: offset=" << reader->offset <<
		      ", size=" << reader->size << "\n");
	    reader->event.set();
	 }
      }
   }

      // And call triggers
   check_triggers();
}

bool
DataPool::has_data(int dstart, int dlength)
{
   if (dlength<0 && length>0) dlength=length-dstart;

   if (pool) return pool->has_data(start+dstart, dlength);
   else if (stream) return start+dstart+dlength<=length;
   else if (dlength<0) return is_eof();
   else return block_list.get_bytes(dstart, dlength)==dlength;
}

int
DataPool::get_data(void * buffer, int offset, int sz)
{
   return get_data(buffer, offset, sz, 0);
}

int
DataPool::get_data(void * buffer, int offset, int sz, int level)
{
   if (stop_flag) THROW("STOP");
   if (stop_blocked_flag && !is_eof() &&
       !has_data(offset, sz)) THROW("STOP");
   
   if (sz < 0) THROW("Size must be non negative");
   if (sz == 0) return 0;

   if (pool)
   {
      if (length>0 && offset+sz>length) sz=length-offset;
      if (sz<0) sz=0;
      while(1)
      {
	    // Ask the underlying (master) DataPool for data. Note, that
	    // master DataPool may throw the "DATA_POOL_REENTER" exception
	    // demanding all readers to restart. This happens when
	    // a DataPool in the chain of DataPools stops. All readers
	    // should return to the most upper level and then reenter the
	    // DataPools hierarchy. Some of them will be stopped by "STOP"
	    // exception.
	 TRY {
	    if (stop_flag || stop_blocked_flag && !is_eof() &&
		!has_data(offset, sz)) THROW("STOP");
	    return pool->get_data(buffer, start+offset, sz, level+1);
	 } CATCH(exc) {
	    if (strcmp(exc.get_cause(), "DATA_POOL_REENTER") || level)
	       RETHROW;
	 } ENDCATCH;
      }
   } 
   else if (stream)
   {
      if (length>0 && offset+sz>length) sz=length-offset;
      if (sz<0) sz=0;
      GCriticalSectionLock lock(&stream_lock);
      stream->seek(start+offset, SEEK_SET);
      return stream->readall(buffer, sz);
   } 
   else
   {
	 // We're not connected to anybody => handle the data
      int size=block_list.get_range(offset, sz);
      if (size>0)
      {
	    // Hooray! Some data is there
	 GCriticalSectionLock lock(&data_lock);
	 data->seek(offset, SEEK_SET);
	 return data->readall(buffer, size);
      }
      
	 // If nothing else is expected => return 0
      if (eof_flag) return 0;
   
	 // Some data is still expected => add this reader to the
	 // list of readers and call virtual wait_for_data()
      DEBUG_MSG("DataPool::get_data(): There is no data in the pool.\n");
      DEBUG_MSG("offset=" << offset << ", size=" << sz <<
		", data_size=" << data->size() << "\n");
      GP<Reader> reader=new Reader(offset, sz);
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
      return get_data(buffer, reader->offset, reader->size, level);
   }
   return 0;
}

void
DataPool::wait_for_data(const GP<Reader> & reader)
      // This function may NOT return until there is some data for the
      // given reader in the internal buffer
{
   DEBUG_MSG("DataPool::wait_for_data(): waiting for data at offset=" << reader->offset <<
	     ", length=" << reader->size << "\n");
   DEBUG_MAKE_INDENT(3);

#if THREADMODEL==NOTHREADS
   THROW("Internal error. This function can't be used in threadless mode.");
#else
   while(1)
   {
      if (stop_flag) THROW("STOP");
      if (reader->reenter_flag) THROW("DATA_POOL_REENTER");
      if (eof_flag || block_list.get_bytes(reader->offset, 1)) return;
      if (pool || stream) return;

      if (stop_blocked_flag) THROW("STOP");

      DEBUG_MSG("calling event.wait()...\n");
      reader->event.wait();
   }
#endif
   
   DEBUG_MSG("Got some data to read\n");
}

void
DataPool::wake_up_all_readers(void)
{
   DEBUG_MSG("DataPool::wake_up_all_readers(): waking up all readers\n");
   DEBUG_MAKE_INDENT(3);

   GCriticalSectionLock lock(&readers_lock);
   for(GPosition pos=readers_list;pos;++pos)
      readers_list[pos]->event.set();
}

void
DataPool::set_eof(void)
      // Has no effect on connected DataPools
{
   if (!stream && !pool)
   {
      eof_flag=true;
      
	 // Can we set the length now?
      if (length<0)
      {
	 GCriticalSectionLock lock(&data_lock);
	 length=data->size();
      }

	 // Wake up all readers to let them rescan the flags
      wake_up_all_readers();
   
	 // Activate all trigger callbacks with negative threshold
      check_triggers();
   }
}

void
DataPool::stop(bool only_blocked)
{
   DEBUG_MSG("DataPool::stop(): Stopping this and dependent DataPools, only_blocked="
	     << only_blocked << "\n");
   DEBUG_MAKE_INDENT(3);

   if (only_blocked) stop_blocked_flag=true;
   else stop_flag=true;
   

   wake_up_all_readers();

      // Now let all readers, which already go thru to the master DataPool,
      // come back and reenter. While reentering some of them will go
      // thru this DataPool again and will be stopped ("STOP" exception)
      // Others (which entered the master DataPool thru other slave DataPools)
      // will simply continue waiting for their data.
   if (pool) pool->restart_readers();
}

void
DataPool::restart_readers(void)
{
   DEBUG_MSG("DataPool::restart_readers(): telling all readers to reenter\n");
   DEBUG_MAKE_INDENT(3);
   
   GCriticalSectionLock slock(&readers_lock);
   for(GPosition pos=readers_list;pos;++pos)
   {
      GP<Reader> reader=readers_list[pos];
      reader->reenter_flag=true;
      reader->event.set();
   }
      
   if (pool) pool->restart_readers();
}

void
DataPool::check_triggers(void)
      // This function is for not connected DataPools only
{
   DEBUG_MSG("DataPool::check_triggers(): calling activated trigger callbacks.\n");
   DEBUG_MAKE_INDENT(3);

   if (!pool && !stream)
      while(1)
      {
	 GP<Trigger> trigger;
	 {
	    GCriticalSectionLock lock(&triggers_lock);
	    for(GPosition pos=triggers_list;pos;++pos)
	    {
	       GP<Trigger> t=triggers_list[pos];
	       if (is_eof() || t->length>=0 &&
		   block_list.get_bytes(t->start, t->length)==t->length)
	       {
		  trigger=t;
		  triggers_list.del(pos);
		  break;
	       }
	    }
	 }
	 if (!trigger) break;
	 else call_callback(trigger->callback, trigger->cl_data);
      }
}

void
DataPool::add_trigger(int thresh, void (* callback)(void *), void * cl_data)
{
  if (thresh>=0)
    add_trigger(0, thresh+1, callback, cl_data);
  else
    add_trigger(0, -1, callback, cl_data);
}

void
DataPool::add_trigger(int tstart, int tlength,
		      void (* callback)(void *), void * cl_data)
{
   DEBUG_MSG("DataPool::add_trigger(): start=" << tstart <<
	     ", length=" << tlength << ", func=" << (void *) callback << "\n");
   DEBUG_MAKE_INDENT(3);
   
   if (callback)
   {
      if (is_eof()) call_callback(callback, cl_data);
      
      if (pool)
      {
	    // We're connected to a DataPool
	    // Just pass the triggers down remembering it in the list
	 if (tlength<0 && length>0) tlength=length-tstart;
	 GP<Trigger> trigger=new Trigger(tstart, tlength, callback, cl_data);
	 pool->add_trigger(start+tstart, tlength, callback, cl_data);
	 GCriticalSectionLock lock(&triggers_lock);
	 triggers_list.append(trigger);
      } else if (!stream)
      {
	    // We're not connected to anything and maintain our own data
	 if (tlength>=0 && block_list.get_bytes(tstart, tlength)==tlength)
	    call_callback(callback, cl_data);
	 else
	 {
	    GCriticalSectionLock lock(&triggers_lock);
	    triggers_list.append(new Trigger(tstart, tlength, callback, cl_data));
	 }
      }
   }
}

void
DataPool::del_trigger(void (* callback)(void *), void * cl_data)
{
   DEBUG_MSG("DataPool::del_trigger(): func=" << (void *) callback << "\n");
   DEBUG_MAKE_INDENT(3);
   
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

   if (pool) pool->del_trigger(callback, cl_data);
}

void
DataPool::static_trigger_cb(void * cl_data)
{
   DataPool * th=(DataPool *) cl_data;
   th->trigger_cb();
}

void
DataPool::trigger_cb(void)
      // This function may be triggered by the DataPool, which we're
      // connected to, or by ourselves, if we're connected to nothing
{
      // Don't want to be destroyed while I'm here. Can't use GP<> life saver
      // because it may be called from the constructor
   GCriticalSectionLock lock(&trigger_lock);
   
   DEBUG_MSG("DataPool::trigger_cb() called\n");
   DEBUG_MAKE_INDENT(3);

   if (pool)
   {
	 // Connected to a pool
	 // We may be here when either EOF is set on the master DataPool
	 // Or when it may have learnt its length (from IFF or whatever)
      if (pool->is_eof() || pool->has_data(start, length)) set_eof();
   } else if (!stream)
   {
	 // Not connected to anything => Try to guess the length
      if (length<0) analyze_iff();
      
	 // Failed to analyze? Check, maybe it's EOF already
      if (length<0 && is_eof())
      {
	 GCriticalSectionLock lock(&data_lock);
	 length=data->size();
      }
   }
}

void
DataPool::analyze_iff(void)
      // In order to display decode progress properly, we need to know
      // the size of the data. It's trivial to figure it out if is_eof()
      // is true. Otherwise we need to make a prediction. Luckily all
      // DjVuFiles have IFF structure, which makes it possible to do it.
      // If due to some reason we fail, the length will remain -1.
{
   DEBUG_MSG("DataPool::analyze_iff(): Trying to decode IFF structure.\n");
   DEBUG_MSG("in order to predict the DataPool's size\n");
   DEBUG_MAKE_INDENT(3);

   GP<ByteStream> str=get_stream();
   
   IFFByteStream iff(*str);
   GString chkid;
   int size;
   if ((size=iff.get_chunk(chkid)) &&
       size>=0 && size<10*1024*1024)
   {
      length=size+iff.tell()-4;
      DEBUG_MSG("Got size=" << size << ", length=" << length << "\n");
   }
}


//****************************************************************************
//****************************** PoolByteStream ******************************
//****************************************************************************

// This is an internal ByteStream receiving data from the associated DataPool.
// It's just a sequential interface, nothing more. All the job for data
// retrieval, waiting and thread synchronization is done by DataPool

class PoolByteStream : public ByteStream
{
public:
   PoolByteStream(DataPool * data_pool);
   virtual ~PoolByteStream() {};

   virtual size_t read(void *buffer, size_t size);
   virtual size_t write(const void *buffer, size_t size);
   virtual void seek(long offset, int whence = SEEK_SET);
   virtual long tell();
   virtual int  is_seekable(void) const { return 1; };
private:
      // Don't make data_pool GP<>. The problem is that DataPool creates
      // and soon destroys this ByteStream from the constructor. Since
      // there are no other pointers to the DataPool created yet, it becomes
      // destroyed immediately :(
   DataPool		* data_pool;
   long			position;

      // Cancel C++ default stuff
   PoolByteStream & operator=(const PoolByteStream &);
};

inline
PoolByteStream::PoolByteStream(DataPool * xdata_pool) :
   data_pool(xdata_pool), position(0)
{
   if (!data_pool) THROW("Internal error: ZERO DataPool passed as input.");
}

size_t
PoolByteStream::read(void *buffer, size_t size)
{
   size=data_pool->get_data(buffer, position, size);
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
   long length=data_pool->get_length();
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

inline GP<ByteStream>
DataPool::get_stream(void)
{
   return new PoolByteStream(this);
}
