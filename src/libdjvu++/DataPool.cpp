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
//C- $Id: DataPool.cpp,v 1.45 2000-03-21 01:09:25 parag Exp $

#ifdef __GNUC__
#pragma implementation
#endif

#if defined(macintosh) //MCW can't compile
#else
#include <sys/types.h>
#endif
#include "DataPool.h"
#include "IFFByteStream.h"
#include "GString.h"
#include "GOS.h"
#include "debug.h"

static void
call_callback(void (* callback)(void *), void * cl_data)
{
   TRY {
      if (callback) callback(cl_data);
   } CATCH(exc) {} ENDCATCH;
}

//****************************************************************************
//****************************** OpenFiles ***********************************
//****************************************************************************

#define MAX_OPEN_FILES	15

/** The purpose of this class is to limit the number of files open by
    connected DataPools. Now, when a DataPool is connected to a file, it
    doesn't necessarily has it open. Every time it needs access to data
    it's supposed to ask this file for the StdioByteStream. It should
    also inform the class when it's going to die (so that the file can
    be closed). OpenFiles makes sure, that the number of open files
    doesn't exceed MAX_OPEN_FILES. When it does, it looks for the oldest
    file, closes it and asks all DataPools working with it to ZERO
    their GP<> pointers. */
class OpenFiles
{
private:
   class File : public GPEnabled
   {
   public:
      GString			name;
      GP<StdioByteStream>	stream;		// Stream connected to 'name'
      GCriticalSection		stream_lock;
      GList<void *>		pools_list;	// List of pools using this stream
      GCriticalSection		pools_lock;
      unsigned long		open_time;	// Time when stream was open

      int	add_pool(DataPool * pool);
      int	del_pool(DataPool * pool);
      
      File(const char * name, DataPool * pool);
      virtual ~File(void);
   };
   static OpenFiles	* global_ptr;

   GPList<File>		files_list;
   GCriticalSection	files_lock;
public:
   static OpenFiles	* get(void);

      // Opend the specified file if necessary (or finds an already open one)
      // and returns it. The caller (pool) is stored in the list associated
      // with the stream. Whenever OpenFiles decides, that this stream
      // had better be closed, it will order every pool from the list to
      // ZERO their references to it
   void		request_stream(const char * name, DataPool * pool,
			       GP<StdioByteStream> & stream,
			       GCriticalSection ** stream_lock);
      // Removes the pool from the list associated with the stream.
      // If there is nobody else using this stream, the stream will
      // be closed too.
   void		stream_released(StdioByteStream * stream, DataPool * pool);
};

OpenFiles * OpenFiles::global_ptr;

OpenFiles::File::File(const char * xname, DataPool * pool) : name(xname)
{
   DEBUG_MSG("OpenFiles::File::File(): Opening file '" << name << "'\n");
   
   open_time=GOS::ticks();
   stream=new StdioByteStream(name, "rb");
   add_pool(pool);
}

OpenFiles::File::~File(void)
{
   DEBUG_MSG("OpenFiles::File::~File(): Closing file '" << name << "'\n");

      // Make all DataPools using this stream release it (so that
      // the stream can actually be closed)
   GCriticalSectionLock lock(&pools_lock);
   for(GPosition pos=pools_list;pos;++pos)
      ((DataPool *) pools_list[pos])->clear_stream();
}

int
OpenFiles::File::add_pool(DataPool * pool)
{
   GCriticalSectionLock lock(&pools_lock);
   if (!pools_list.contains(pool)) pools_list.append(pool);
   return pools_list.size();
}

int
OpenFiles::File::del_pool(DataPool * pool)
{
   GCriticalSectionLock lock(&pools_lock);
   GPosition pos;
   if (pools_list.search(pool, pos)) pools_list.del(pos);
   return pools_list.size();
}

inline OpenFiles *
OpenFiles::get(void)
{
   if (!global_ptr) global_ptr=new OpenFiles();
   return global_ptr;
}

void
OpenFiles::request_stream(const char * name_in, DataPool * pool,
			  GP<StdioByteStream> & stream,
			  GCriticalSection ** stream_lock)
{
   DEBUG_MSG("OpenFiles::request_stream(): name='" << name_in << "'\n");
   DEBUG_MAKE_INDENT(3);

   GP<File> file;

   GString name=GOS::expand_name(name_in, GOS::cwd());
   
      // Check: maybe the stream has already been open by request of
      // another DataPool
   GCriticalSectionLock lock(&files_lock);
   for(GPosition pos=files_list;pos;++pos)
      if (files_list[pos]->name==name)
      {
	 DEBUG_MSG("found existing stream\n");
	 file=files_list[pos];
	 break;
      }

      // No? Open the stream, but check, that there are not
      // too many streams open
   if (!file)
   {
      file=new File(name, pool);
      files_list.append(file);
      if (files_list.size()>MAX_OPEN_FILES)
      {
	    // Too many open files (streams). Get rid of the oldest one.
	 unsigned long oldest_time=GOS::ticks();
	 GPosition oldest_pos=files_list;
	 for(GPosition pos=files_list;pos;++pos)
	    if (files_list[pos]->open_time<oldest_time)
	    {
	       oldest_time=files_list[pos]->open_time;
	       oldest_pos=pos;
	    }
	 files_list.del(oldest_pos);
      }
   }
   
   file->add_pool(pool);
   stream=file->stream;
   *stream_lock=&file->stream_lock;
}

void
OpenFiles::stream_released(StdioByteStream * stream, DataPool * pool)
{
   GCriticalSectionLock lock(&files_lock);
   for(GPosition pos=files_list;pos;)
   {
      GP<File> f=files_list[pos];
      if (f->stream==stream && f->del_pool(pool)==0)
      {
	 GPosition this_pos=pos;
	 ++pos;
	 files_list.del(this_pos);
      } else ++pos;
   }
}

//****************************************************************************
//******************************** FCPools ***********************************
//****************************************************************************

/** This class is used to maintain a list of DataPools connected to a file.
    It's important to have this list if we want to do smth with this file
    like to modify it or just erase. Since any modifications of the file
    will break DataPools directly connected to it, it would be nice to have
    a mechanism for signaling all the related DataPools to read data into
    memory. This is precisely the purpose of this class. */
class FCPools
{
private:
   GMap<GString, const void *>	map;	// GMap<GString, GList<DataPool *>> in fact
   GCriticalSection		map_lock;

   static FCPools	* global_ptr;
public:
   static FCPools *	get(void);
      // Adds the <fname, pool> pair into the list
   void		add_pool(const char * fname, DataPool * pool);
      // Removes the <fname, pool> pair from the list
   void		del_pool(const char * fname, DataPool * pool);
      // Looks for the list of DataPools connected to 'fname' and makes
      // each of them load the contents of the file into memory
   void		load_file(const char * fname);
};

void
FCPools::add_pool(const char * name_in, DataPool * pool)
{
   GCriticalSectionLock lock(&map_lock);

   if (name_in && strlen(name_in))
   {
      GString name=GOS::expand_name(name_in, GOS::cwd());
      GList<void *> * list;
      GPosition pos;
      if (map.contains(name, pos)) list=(GList<void *> *) map[pos];
      else map[name]=list=new GList<void *>();
      if (!list->contains(pool)) list->append(pool);
   }
}

void
FCPools::del_pool(const char * name_in, DataPool * pool)
{
   GCriticalSectionLock lock(&map_lock);

   if (name_in && strlen(name_in))
   {
      GString name=GOS::expand_name(name_in, GOS::cwd());
      GPosition pos;
      if (map.contains(name, pos))
      {
	 GList<void *> * list=(GList<void *> *) map[pos];
	 GPosition list_pos;
	 while(list->search(pool, list_pos))
	    list->del(list_pos);
	 if (list->isempty())
	 {
	    delete list;
	    map.del(pos);
	 }
      }
   }
}

void
FCPools::load_file(const char * name_in)
{
   GCriticalSectionLock lock(&map_lock);

   if (name_in && strlen(name_in))
   {
      GString name=GOS::expand_name(name_in, GOS::cwd());
      GPosition pos;
      if (map.contains(name, pos))
      {
	    // We make here a copy of the list because DataPool::load_file()
	    // will call FCPools::del_pool(), which will modify the list
	 GList<void *> list=*(GList<void *> *) map[pos];
	 for(GPosition list_pos=list;list_pos;++list_pos)
	    ((DataPool *) list[list_pos])->load_file();
      }
   }
}

FCPools	* FCPools::global_ptr;

inline FCPools *
FCPools::get(void)
{
   if (!global_ptr) global_ptr=new FCPools();
   return global_ptr;
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
DataPool::BlockList::clear(void)
{
   GCriticalSectionLock lk(&lock);
   list.empty();
}

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

DataPool::DataPool(ByteStream & str)
{
   init();

      // It's nice to have IFF data analyzed in this case too.
   add_trigger(0, 32, static_trigger_cb, this);
   
   char buffer[1024];
   int length;
   while((length=str.read(buffer, 1024)))
      add_data(buffer, length);
   set_eof();
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
      GCriticalSectionLock lock(&class_stream_lock);
      OpenFiles::get()->stream_released(stream, this);
      stream=0;
   }

   if (fname.length())
      FCPools::get()->del_pool(fname, this);
   
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
   if (fname.length()) THROW("Already connected to a file.");
   if (start_in<0) THROW("Start must be non negative");

   pool=pool_in;
   start=start_in;
   length=length_in;

      // The following will work for length<0 too
   if (pool->has_data(start, length)) eof_flag=true;
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
DataPool::connect(const char * fname_in, int start_in, int length_in)
{
   DEBUG_MSG("DataPool::connect(): connecting to a file\n");
   DEBUG_MAKE_INDENT(3);
   
   if (pool) THROW("Already connected to a DataPool.");
   if (fname.length()) THROW("Already connected to another file.");
   if (start_in<0) THROW("Start must be non negative");

   if (!strcmp(fname_in, "-"))
   {
      DEBUG_MSG("This is stdin => just read the data...\n");
      char buffer[1024];
      int length;
      StdioByteStream str("-", "rb");
      while((length=str.read(buffer, 1024)))
	 add_data(buffer, length);
      set_eof();
   } else
   {
	 // Open the stream (just in this function) too see if
	 // the file is accessible. In future we will be using 'OpenFiles'
	 // to request and release streams
      GP<StdioByteStream> str=new StdioByteStream(fname_in, "rb");
      str->seek(0, SEEK_END);
      int file_size=str->tell();

      fname=fname_in;
      start=start_in;
      length=length_in;
      if (start>=file_size) length=0;
      else if (length<0 || start+length>=file_size) length=file_size-start;

      eof_flag=true;

      data=0;

      FCPools::get()->add_pool(fname, this);

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
   else if (fname.length())
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
   
   if (fname.length() || pool)
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

      // Do not undo the following two lines. The reason why we need them
      // here is the connected DataPools, which use 'length' (more exactly
      // has_data()) to see if they have all data required. So, right after
      // all data has been added to the master DataPool, but before EOF
      // is set, the master and slave DataPools disagree regarding if
      // all data is there or not. These two lines solve the problem
   GCriticalSectionLock lock(&data_lock);
   if (length>=0 && data->size()>=length) set_eof();
}

bool
DataPool::has_data(int dstart, int dlength)
{
   if (dlength<0 && length>0) dlength=length-dstart;

   if (pool) return pool->has_data(start+dstart, dlength);
   else if (fname.length()) return start+dstart+dlength<=length;
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
   Incrementor inc(active_readers);
   
   if (stop_flag)
     THROW("STOP");
   if (stop_blocked_flag && !is_eof() &&
       !has_data(offset, sz))
     THROW("STOP");
   
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
	       EXTHROW(exc);
	 } ENDCATCH;
      }
   } 
   else if (fname.length())
   {
      if (length>0 && offset+sz>length) sz=length-offset;
      if (sz<0) sz=0;
      
      GCriticalSectionLock lock1(&class_stream_lock);
      if (!stream || !stream_lock)
	 OpenFiles::get()->request_stream(fname, this, stream, &stream_lock);
      GCriticalSectionLock lock2(stream_lock);
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

	 // No data available.

	 // If there is no data and nothing else is expected, we can do
	 // two things: throw an "EOF" exception or return ZERO bytes.
	 // The exception is for the cases when the data flow has been
	 // terminated in the middle. ZERO bytes is for regular read() beyond
	 // the boundaries of legal data. The problem is to distinguish
	 // these two cases. We do it here with the help of analysis of the
	 // IFF structure of the data (which sets the 'length' variable).
	 // If we attempt to read beyond the [0, length[, ZERO bytes will be
	 // returned. Otherwise an "EOF" exception will be thrown.
      if (eof_flag)
      {
	 if (length>0 && offset<length) 
         {
           THROW("EOF");
	 }else 
         {
           return 0;
         }
      } 
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
	 EXTHROW(exc);
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
      if (pool || fname.length()) return;

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
   if (!fname.length() && !pool)
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
   if (pool)
   {
	 // This loop is necessary because there may be another thread, which
	 // is going down thru the DataPool chain and did not reach the
	 // lowest "master" DataPool yet. Since it didn't reach it yet,
	 // the "pool->restart_readers()" will not restart it. So we're going
	 // to continue issuing this command until we get rid of all
	 // "active_readers"
      while(active_readers)
      {
#if (THREADMODEL==COTHREADS) || (THREADMODEL==MACTHREADS)
	 GThread::yield();
#endif
	 pool->restart_readers();
      }
   }
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
DataPool::load_file(void)
{
   DEBUG_MSG("DataPool::load_file() called\n");
   DEBUG_MAKE_INDENT(3);

   if (pool)
   {
      DEBUG_MSG("passing the request down.\n");
      pool->load_file();
   } else if (fname.length())
   {
      DEBUG_MSG("loading the data.\n");

      GCriticalSectionLock lock1(&class_stream_lock);
      if (!stream || !stream_lock)
	 OpenFiles::get()->request_stream(fname, this, stream, &stream_lock);
      GCriticalSectionLock lock2(stream_lock);

      data=new MemoryByteStream();
      block_list.clear();
      FCPools::get()->del_pool(fname, this);
      fname="";

      stream->seek(0, SEEK_SET);
      char buffer[1024];
      int length;
      while((length=stream->read(buffer, 1024)))
	 add_data(buffer, length);
	 // No need to set EOF. It should already be set.

      OpenFiles::get()->stream_released(stream, this);
      stream=0;
   } else DEBUG_MSG("Not connected\n");
}

void
DataPool::load_file(const char * name)
{
   FCPools::get()->load_file(name);
}

void
DataPool::check_triggers(void)
      // This function is for not connected DataPools only
{
   DEBUG_MSG("DataPool::check_triggers(): calling activated trigger callbacks.\n");
   DEBUG_MAKE_INDENT(3);

   if (!pool && !fname.length())
      while(true)
      {
	 GP<Trigger> trigger;
	 
	    // First find a candidate (trigger, which needs to be called)
	    // Don't remove it from the list yet. del_trigger() should
	    // be able to find it if necessary and disable.
	 {
	    GCriticalSectionLock list_lock(&triggers_lock);
	    for(GPosition pos=triggers_list;pos;++pos)
	    {
	       GP<Trigger> t=triggers_list[pos];
	       if (is_eof() || t->length>=0 &&
		   block_list.get_bytes(t->start, t->length)==t->length)
	       {
		  trigger=t;
		  break;
	       }
	    }
	 }
	 
	 if (trigger)
	 {
	       // Now check that the trigger is not disabled
	       // and lock the trigger->disabled lock for the duration
	       // of the trigger. This will block the del_trigger() and
	       // will postpone client's destruction (usually following
	       // the call to del_trigger())
	    {
	       GMonitorLock lock(&trigger->disabled);
	       if (!trigger->disabled)
		  call_callback(trigger->callback, trigger->cl_data);
	    }
	    
	       // Finally - remove the trigger from the list.
	    GCriticalSectionLock list_lock(&triggers_lock);
	    for(GPosition pos=triggers_list;pos;++pos)
	       if (triggers_list[pos]==trigger)
	       {
		  triggers_list.del(pos);
		  break;
	       }
	 } else break;
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
      else
      {
	 if (pool)
	 {
	       // We're connected to a DataPool
	       // Just pass the triggers down remembering it in the list
	    if (tlength<0 && length>0) tlength=length-tstart;
	    GP<Trigger> trigger=new Trigger(tstart, tlength, callback, cl_data);
	    pool->add_trigger(start+tstart, tlength, callback, cl_data);
	    GCriticalSectionLock lock(&triggers_lock);
	    triggers_list.append(trigger);
	 } else if (!fname.length())
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
}

void
DataPool::del_trigger(void (* callback)(void *), void * cl_data)
{
   DEBUG_MSG("DataPool::del_trigger(): func=" << (void *) callback << "\n");
   DEBUG_MAKE_INDENT(3);

   while(true)
   {
      GP<Trigger> trigger;
      {
	 GCriticalSectionLock lock(&triggers_lock);
	 for(GPosition pos=triggers_list;pos;)
	 {
	    GP<Trigger> t=triggers_list[pos];
	    if (t->callback==callback && t->cl_data==cl_data)
	    {
	       trigger=t;
	       GPosition this_pos=pos;
	       ++pos;
	       triggers_list.del(this_pos);
	       break;
	    } else ++pos;
	 }
      }

	 // Above we removed the trigger from the list and unlocked the list
	 // Now we will disable it and will wait if necessary (if the
	 // trigger is currently being processed by check_triggers())
	 // check_triggers() locks the trigger for the duration of the
	 // trigger callback. Thus we will wait for the trigger callback
	 // to finish and avoid client's destruction.
      if (trigger) trigger->disabled=1;
      else break;
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
      if (pool->is_eof() || pool->has_data(start, length)) eof_flag=true;
   } else if (!fname.length())
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
   virtual long tell();
private:
      // Don't make data_pool GP<>. The problem is that DataPool creates
      // and soon destroys this ByteStream from the constructor. Since
      // there are no other pointers to the DataPool created yet, it becomes
      // destroyed immediately :(
   DataPool		* data_pool;
   GP<DataPool>		data_pool_lock;
   long			position;
   
   char			buffer[512];
   size_t		buffer_size;
   size_t		buffer_pos;

      // Cancel C++ default stuff
   PoolByteStream & operator=(const PoolByteStream &);
};

inline
PoolByteStream::PoolByteStream(DataPool * xdata_pool) :
   data_pool(xdata_pool), position(0), buffer_size(0), buffer_pos(0)
{
   if (!data_pool) 
       THROW("Internal error: ZERO DataPool passed as input.");

      // Secure the DataPool if possible. If we're called from DataPool
      // constructor (get_count()==0) there is no need to secure at all.
   if (data_pool->get_count()) data_pool_lock=data_pool;
}

size_t
PoolByteStream::read(void *data, size_t size)
{
  if (buffer_pos >= buffer_size) {
    if (size >= sizeof(buffer)) {
      // Direct read
      size = data_pool->get_data(data, position, size);
      position += size;
      return size;
    } else {
      // Refill buffer
      buffer_size = data_pool->get_data(buffer, position, sizeof(buffer));
      buffer_pos=0;
    }
  }
  if (buffer_pos + size >= buffer_size)
    size = buffer_size - buffer_pos;
  memcpy(data, buffer+buffer_pos, size);
  buffer_pos += size;
  position += size;
  return size;
}

size_t
PoolByteStream::write(const void *buffer, size_t size)
{
   THROW("write() is not implemented.");
   return 0;	// For compiler not to bark
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
