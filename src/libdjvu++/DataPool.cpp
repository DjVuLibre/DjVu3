//C-  -*- C++ -*-
//C- DjVu� Reference Library (v. 3.0)
//C- 
//C- Copyright � 1999-2001 LizardTech, Inc. All Rights Reserved.
//C- The DjVu Reference Library is protected by U.S. Pat. No.
//C- 6,058,214 and patents pending.
//C- 
//C- This software is subject to, and may be distributed under, the
//C- GNU General Public License, Version 2. The license should have
//C- accompanied the software or you may obtain a copy of the license
//C- from the Free Software Foundation at http://www.fsf.org .
//C- 
//C- The computer code originally released by LizardTech under this
//C- license and unmodified by other parties is deemed the "LizardTech
//C- Original Code."
//C- 
//C- With respect to the LizardTech Original Code ONLY, and subject
//C- to any third party intellectual property claims, LizardTech
//C- grants recipient a worldwide, royalty-free, non-exclusive license
//C- under patent claims now or hereafter owned or controlled by
//C- LizardTech that are infringed by making, using, or selling
//C- LizardTech Original Code, but solely to the extent that any such
//C- patent(s) is/are reasonably necessary to enable you to make, have
//C- made, practice, sell, or otherwise dispose of LizardTech Original
//C- Code (or portions thereof) and not to any greater extent that may
//C- be necessary to utilize further modifications or combinations.
//C- 
//C- The LizardTech Original Code is provided "AS IS" WITHOUT WARRANTY
//C- OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED
//C- TO ANY WARRANTY OF NON-INFRINGEMENT, OR ANY IMPLIED WARRANTY OF
//C- MERCHANTIBILITY OR FITNESS FOR A PARTICULAR PURPOSE.
// 
// $Id: DataPool.cpp,v 1.68 2001-02-20 19:36:50 fcrary Exp $
// $Name:  $


#ifdef __GNUC__
#pragma implementation
#endif

#include "DataPool.h"
#include "IFFByteStream.h"
#include "GString.h"
#include "GOS.h"
#include "debug.h"

#if defined(macintosh) //MCW can't compile
#else
#ifndef UNDER_CE 
#include <sys/types.h>
#endif
#endif

static void
// call_callback(void (* callback)(GP<GPEnabled> &), GP<GPEnabled> cl_data)
call_callback(void (* callback)(void *), void *cl_data)
{
   G_TRY
   {
      if (callback)
        callback(cl_data);
   } G_CATCH_ALL {} G_ENDCATCH;
}


//****************************************************************************
//****************************** OpenFiles ***********************************
//****************************************************************************

#define MAX_OPEN_FILES	15

/** The purpose of this class is to limit the number of files open by
    connected DataPools. Now, when a DataPool is connected to a file, it
    doesn't necessarily has it open. Every time it needs access to data
    it's supposed to ask this file for the ByteStream. It should
    also inform the class when it's going to die (so that the file can
    be closed). OpenFiles makes sure, that the number of open files
    doesn't exceed MAX_OPEN_FILES. When it does, it looks for the oldest
    file, closes it and asks all DataPools working with it to ZERO
    their GP<> pointers. */
   class DataPool::OpenFiles_File : public GPEnabled
   {
   public:
      GString			name;
      GP<ByteStream>	        stream;		// Stream connected to 'name'
      GCriticalSection		stream_lock;
      GPList<DataPool>		pools_list;	// List of pools using this stream
      GCriticalSection		pools_lock;
      unsigned long		open_time;	// Time when stream was open

      int	add_pool(GP<DataPool> &pool);
      int	del_pool(GP<DataPool> &pool);
      
      OpenFiles_File(const char * name, GP<DataPool> &pool);
      virtual ~OpenFiles_File(void);
   };
class DataPool::OpenFiles : public GPEnabled
{
private:
   static OpenFiles	* global_ptr;

   GPList<DataPool::OpenFiles_File>		files_list;
   GCriticalSection	files_lock;
public:
   static OpenFiles	* get(void);

      // Opend the specified file if necessary (or finds an already open one)
      // and returns it. The caller (pool) is stored in the list associated
      // with the stream. Whenever OpenFiles decides, that this stream
      // had better be closed, it will order every pool from the list to
      // ZERO their references to it
   GP<DataPool::OpenFiles_File>		request_stream(const char * name, GP<DataPool> pool);
      // If there are more than MAX_STREAM_FILES open, close the oldest.
   void		prune(void);
      // Removes the pool from the list associated with the stream.
      // If there is nobody else using this stream, the stream will
      // be closed too.
   void		stream_released(GP<ByteStream> &stream, GP<DataPool> pool);

   void 	close_all(void);
};

DataPool::OpenFiles * DataPool::OpenFiles::global_ptr;

DataPool::OpenFiles_File::OpenFiles_File(const char * xname, GP<DataPool> &pool) : name(xname)
{
   DEBUG_MSG("DataPool::OpenFiles_File::OpenFiles_File(): Opening file '" << name << "'\n");
   DEBUG_MAKE_INDENT(3);
   
   open_time=GOS::ticks();
   stream=ByteStream::create(name,"rb");
   add_pool(pool);
}

DataPool::OpenFiles_File::~OpenFiles_File(void)
{
   DEBUG_MSG("DataPool::OpenFiles_File::~OpenFiles_File(): Closing file '" << name << "'\n");
   DEBUG_MAKE_INDENT(3);

      // Make all DataPools using this stream release it (so that
      // the stream can actually be closed)
   GCriticalSectionLock lock(&pools_lock);
   for(GPosition pos=pools_list;pos;++pos)
      pools_list[pos]->clear_stream();
}

int
DataPool::OpenFiles_File::add_pool(GP<DataPool> &pool)
{
   DEBUG_MSG("DataPool::OpenFiles_File::add_pool: pool=" << (void *) pool << "\n");
   DEBUG_MAKE_INDENT(3);
   GCriticalSectionLock lock(&pools_lock);
   if (!pools_list.contains(pool))
     pools_list.append(pool);
   return pools_list.size();
}

int
DataPool::OpenFiles_File::del_pool(GP<DataPool> &pool)
{
   DEBUG_MSG("DataPool::OpenFiles_File::del_pool: pool=" << (void *) pool << "\n");
   DEBUG_MAKE_INDENT(3);
   GCriticalSectionLock lock(&pools_lock);
   GPosition pos;
   if (pools_list.search(pool, pos))
     pools_list.del(pos);
   return pools_list.size();
}

inline DataPool::OpenFiles *
DataPool::OpenFiles::get(void)
{
   DEBUG_MSG("DataPool::OpenFiles::get()\n");
   DEBUG_MAKE_INDENT(3);
   if (!global_ptr)
     global_ptr=new OpenFiles();
   return global_ptr;
}

void
DataPool::OpenFiles::prune(void)
{
  DEBUG_MSG("DataPool::OpenFiles::prune(void): " << files_list.size() << "\n");
  DEBUG_MAKE_INDENT(3);
  while(files_list.size()>MAX_OPEN_FILES)
  {
    // Too many open files (streams). Get rid of the oldest one.
    unsigned long oldest_time=GOS::ticks();
    GPosition oldest_pos=files_list;
    for(GPosition pos=files_list;pos;++pos)
    {
      if (files_list[pos]->open_time<oldest_time)
      {
        oldest_time=files_list[pos]->open_time;
        oldest_pos=pos;
      }
    }
    files_list.del(oldest_pos);
  }
}

//			  GP<ByteStream> & stream,
//			  GCriticalSection ** stream_lock)
GP<DataPool::OpenFiles_File>
DataPool::OpenFiles::request_stream(const char * name_in, GP<DataPool> pool)
{
   DEBUG_MSG("DataPool::OpenFiles::request_stream(): name='" << name_in << "'\n");
   DEBUG_MAKE_INDENT(3);

   GP<DataPool::OpenFiles_File> file;

   GString name=GOS::expand_name(name_in, GOS::cwd());
   
      // Check: maybe the stream has already been open by request of
      // another DataPool
   GCriticalSectionLock lock(&files_lock);
   for(GPosition pos=files_list;pos;++pos)
   {
      if (files_list[pos]->name==name)
      {
	 DEBUG_MSG("found existing stream\n");
	 file=files_list[pos];
	 break;
      }
   }

      // No? Open the stream, but check, that there are not
      // too many streams open
   if (!file)
   {
      file=new DataPool::OpenFiles_File(name, pool);
      files_list.append(file);
      prune();
   }
   
   file->add_pool(pool);
   return file;
}

void
DataPool::OpenFiles::stream_released(GP<ByteStream> &stream, GP<DataPool> pool)
{
   DEBUG_MSG("DataPool::OpenFiles::stream_release: stream=" << (void *)stream << " pool=" << (void *)pool << "\n");
   DEBUG_MAKE_INDENT(3);
   GCriticalSectionLock lock(&files_lock);
   for(GPosition pos=files_list;pos;)
   {
      GP<DataPool::OpenFiles_File> f=files_list[pos];
      if ((ByteStream *)(f->stream)==(ByteStream *)stream && f->del_pool(pool)==0)
      {
	 GPosition this_pos=pos;
	 ++pos;
	 files_list.del(this_pos);
      } else ++pos;
   }
}

// This isn't really an accurate name.  The files are not really
// closed.  Instead they are dereferenced from the data pool.  If
// a there is another reference to the respective bytestream, it
// will remain open until dereferenced.
void
DataPool::OpenFiles::close_all(void)
{
  DEBUG_MSG("DataPool::OpenFiles::close_all\n");
  DEBUG_MAKE_INDENT(3);
  GCriticalSectionLock lock(&files_lock);
  files_list.empty();
}

//****************************************************************************
//******************************** FCPools ***********************************
//****************************************************************************

/** This class is used to maintain a list of DataPools connected to a file.
    It's important to have this list if we want to do something with this file
    like to modify it or just erase. Since any modifications of the file
    will break DataPools directly connected to it, it would be nice to have
    a mechanism for signaling all the related DataPools to read data into
    memory. This is precisely the purpose of this class. */
class FCPools
{
private:
   GMap<GString, GPList<DataPool> >	map;	// GMap<GString, GPList<DataPool>> in fact
   GCriticalSection		map_lock;

   static FCPools	* global_ptr;
public:
   static FCPools *	get(void);
      // Adds the <fname, pool> pair into the list
   void		add_pool(const char * fname, GP<DataPool> pool);
      // Removes the <fname, pool> pair from the list
   void		del_pool(const char * fname, GP<DataPool> pool);
      // Looks for the list of DataPools connected to 'fname' and makes
      // each of them load the contents of the file into memory
   void		load_file(const char * fname);
};

void
FCPools::add_pool(const char * name_in, GP<DataPool> pool)
{
  DEBUG_MSG("FCPools::add_pool: name_in='" << name_in << "' pool=" << (void *)pool << "\n");
  DEBUG_MAKE_INDENT(3);
   GCriticalSectionLock lock(&map_lock);

   if (name_in && strlen(name_in))
   {
      GString name=GOS::expand_name(name_in, GOS::cwd());
      GPList<DataPool> list;
      GPosition pos;
      if (!map.contains(name, pos))
      {
        map[name]=list;
        pos=map.contains(name);
      }
      GPList<DataPool> &plist=map[pos];
      if (!plist.contains(pool))
        plist.append(pool);
   }
}

void
FCPools::del_pool(const char * name_in, GP<DataPool> pool)
{
  DEBUG_MSG("FCPools::del_pool: name_in='" << name_in << "' pool=" << (void *)pool << "\n");
  DEBUG_MAKE_INDENT(3);
   GCriticalSectionLock lock(&map_lock);

   if (name_in && strlen(name_in))
   {
      GString name=GOS::expand_name(name_in, GOS::cwd());
      GPosition pos;
      if (map.contains(name, pos))
      {
	 GPList<DataPool> &list=map[pos];
	 GPosition list_pos;
	 while(list.search(pool, list_pos))
	    list.del(list_pos);
	 if (list.isempty())
	 {
	    map.del(pos);
	 }
      }
   }
}

void
FCPools::load_file(const char * name_in)
{
  DEBUG_MSG("FCPools::load_file: name_in='" << name_in << "'\n");
  DEBUG_MAKE_INDENT(3);
   GCriticalSectionLock lock(&map_lock);

   if (name_in && strlen(name_in))
   {
      GString name=GOS::expand_name(name_in, GOS::cwd());
      GPosition pos;
      if (map.contains(name, pos))
      {
	    // We make here a copy of the list because DataPool::load_file()
	    // will call FCPools::del_pool(), which will modify the list
	 GPList<DataPool> list=map[pos];
	 for(GPosition list_pos=list;list_pos;++list_pos)
	    list[list_pos]->load_file();
      }
   }
}

FCPools	* FCPools::global_ptr;

inline FCPools *
FCPools::get(void)
{
   if (!global_ptr)
     global_ptr=new FCPools();
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

class DataPool::BlockList
{
         // See comments in .cpp file.
private:
   GCriticalSection  lock;
   GList<int>        list;
public:
   BlockList() {};
   void              clear(void);
   void              add_range(int start, int length);
   int               get_bytes(int start, int length) const;
   int               get_range(int start, int length) const;
friend class DataPool;
};

void
DataPool::BlockList::clear(void)
{
  DEBUG_MSG("DataPool::BlockList::clear()\n");
  DEBUG_MAKE_INDENT(3);
   GCriticalSectionLock lk(&lock);
   list.empty();
}

void
DataPool::BlockList::add_range(int start, int length)
      // Adds range of known data.
{
  DEBUG_MSG("DataPool::BlockList::add_range: start=" << start << " length=" << length << "\n");
  DEBUG_MAKE_INDENT(3);
   if (start<0)
     G_THROW("DataPool.neg_start");        //  The start offset of the range may not be negative.
   if (length<=0)
     G_THROW("DataPool.bad_length");     //  The length must be positive.
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
  DEBUG_MSG("DataPool::BlockList::get_bytes: start=" << start << " length=" << length << "\n");
  DEBUG_MAKE_INDENT(3);

   if (length<0)
     G_THROW("DataPool.bad_length");        //  The length must be positive.

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
  DEBUG_MSG("DataPool::BlockList::get_range: start=" << start << " length=" << length << "\n");
  DEBUG_MAKE_INDENT(3);
   if (start<0)
     G_THROW("DataPool.neg_start");    //  The start offset of the range may not be negative.
   if (length<=0)
      G_THROW("DataPool.bad_length"); //  The length must be positive.

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

class DataPool::Reader : public GPEnabled
{
public:
   GEvent event;
   bool reenter_flag;
   int  offset;
   int  size;
   Reader() : reenter_flag(false), offset(0), size(-1){};
   Reader(int offset_in, int size_in=-1) :
   reenter_flag(false), offset(offset_in), size(size_in) {};
   virtual ~Reader() {};
};

class DataPool::Trigger : public GPEnabled
{
public:
   GSafeFlags disabled;
   int  start, length;
//   void (* callback)(GP<GPEnabled> &);
   void (* callback)(void *);
//   GP<GPEnabled> cl_data;
   void *cl_data;

   Trigger() : start(0), length(-1), callback(0), cl_data(0) {};
   Trigger(int xstart, int xlength,
//   void (* xcallback)(GP<GPEnabled> &), GP<GPEnabled> xcl_data) :
   void (* xcallback)(void *), void *xcl_data) :
      start(xstart), length(xlength), callback(xcallback), cl_data(xcl_data) {};
   virtual ~Trigger() {};
};

class DataPool::Counter
{
private:
   int               counter;
   GCriticalSection  lock;
public:
   Counter() : counter(0) {};
   operator int(void) const;
   void              inc(void);
   void              dec(void);
};

#define DATAPOOL_INIT eof_flag(false),stop_flag(false), \
    stop_blocked_flag(false), \
    add_at(0),start(0),length(-1)

void
DataPool::init(void)
{
  DEBUG_MSG("DataPool::init(): Initializing\n");
  DEBUG_MAKE_INDENT(3);
  start=0; length=-1; add_at=0;
  eof_flag=false;
  stop_flag=false;
  stop_blocked_flag=false;

  active_readers=new Counter;
  block_list=0;
  G_TRY
  {   
    block_list=new BlockList;
    data=ByteStream::create();
  }
  G_CATCH_ALL
  {
    delete block_list;
    block_list=0;
    delete active_readers;
    active_readers=0;
    G_RETHROW;
  }
  G_ENDCATCH;
}

DataPool::DataPool(void) : DATAPOOL_INIT {}

GP<DataPool>
DataPool::create(void)
{
  DEBUG_MSG("DataPool::DataPool()\n");
  DEBUG_MAKE_INDENT(3);
  DataPool *pool=new DataPool();

  GP<DataPool> retval=pool;
  pool->init();

      // If we maintain the data ourselves, we want to interpret its
      // IFF structure to predict its length
  pool->add_trigger(0, 32, static_trigger_cb, pool);
  return retval;
}

GP<DataPool> 
DataPool::create(ByteStream &str)
{
  DEBUG_MSG("DataPool::create: str=" << (void *)&str << "\n");
  DEBUG_MAKE_INDENT(3);
  DataPool *pool=new DataPool();
  GP<DataPool> retval=pool;
  pool->init();

      // It's nice to have IFF data analyzed in this case too.
  pool->add_trigger(0, 32, static_trigger_cb, pool);
   
  char buffer[1024];
  int length;
  while((length=str.read(buffer, 1024)))
     pool->add_data(buffer, length);
  pool->set_eof();
  return retval;
}

GP<DataPool>
DataPool::create(const GP<DataPool> & pool, int start, int length)
{
  DEBUG_MSG("DataPool::DataPool: pool=" << (void *)((DataPool *)pool) << " start=" << start << " length= " << length << "\n");
  DEBUG_MAKE_INDENT(3);

  DataPool *xpool=new DataPool();
  GP<DataPool> retval=xpool;
  xpool->init();
  xpool->connect(pool, start, length);
  return retval;
}

GP<DataPool>
DataPool::create(const char * fname, int start, int length)
{
  DEBUG_MSG("DataPool::DataPool: fname='" << fname << "' start=" << start << " length= " << length << "\n");
  DEBUG_MAKE_INDENT(3);

  DataPool *pool=new DataPool();
  GP<DataPool> retval=pool;
  pool->init();
  pool->connect(fname, start, length);
  return retval;
}

void
DataPool::clear_stream(const bool release)
{
  DEBUG_MSG("DataPool::clear_stream()\n");
  DEBUG_MAKE_INDENT(3);
  if(fstream)
  {
    GCriticalSectionLock lock1(&class_stream_lock);
    GP<OpenFiles_File> f=fstream;
    if(f)
    {
      GCriticalSectionLock lock2(&(f->stream_lock));
      fstream=0;
      if(release)
        OpenFiles::get()->stream_released(f->stream, this);
    }
  }
}

DataPool::~DataPool(void)
{
  DEBUG_MSG("DataPool::~DataPool()\n");
  DEBUG_MAKE_INDENT(3);

  clear_stream(true);

  if (fname.length()) 
  {
    FCPools::get()->del_pool(fname, this);
  }
   
  {
	 // Wait until the static_trigger_cb() exits
      GCriticalSectionLock lock(&trigger_lock);
      if (pool)
        pool->del_trigger(static_trigger_cb, this);
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
  delete block_list;
  delete active_readers;
}

void
DataPool::connect(const GP<DataPool> & pool_in, int start_in, int length_in)
{
   DEBUG_MSG("DataPool::connect(): connecting to another DataPool\n");
   DEBUG_MAKE_INDENT(3);
   
   if (pool) G_THROW("DataPool.connected1");            //  Already connected to another DataPool.
   if (fname.length()) G_THROW("DataPool.connected2");  //  Already connected to a file.
   if (start_in<0) G_THROW("DataPool.neg_start");       //  The start offset of the range may not be negative.

   pool=pool_in;
   start=start_in;
   length=length_in;

      // The following will work for length<0 too
   if (pool->has_data(start, length))
     eof_flag=true;
   else
     pool->add_trigger(start, length, static_trigger_cb, this);

   data=0;

   wake_up_all_readers();
   
      // Pass registered trigger callbacks to the DataPool
   GCriticalSectionLock lock(&triggers_lock);
   for(GPosition pos=triggers_list;pos;++pos)
   {
      GP<Trigger> t=triggers_list[pos];
      int tlength=t->length;
      if (tlength<0 && length>0)
        tlength=length-t->start;
      pool->add_trigger(start+t->start, tlength, t->callback, t->cl_data);
   }
}

void
DataPool::connect(const char * fname_in, int start_in, int length_in)
{
   DEBUG_MSG("DataPool::connect(): connecting to a file\n");
   DEBUG_MAKE_INDENT(3);
   
   if (pool)
     G_THROW("DataPool.connected1");              //  Already connected to another DataPool.
   if (fname.length())
     G_THROW("DataPool.connected2");    //  Already connected to a file.
   if (start_in<0)
     G_THROW("DataPool.neg_start");         //  The start offset of the range may not be negative.


   if (fname_in[0] == '-' && !fname_in[1])
   {
      DEBUG_MSG("This is stdin => just read the data...\n");
      DEBUG_MAKE_INDENT(3);
      char buffer[1024];
      int length;
      GP<ByteStream> gstr=ByteStream::create("-", "rb");
      ByteStream &str=*gstr;
      while((length=str.read(buffer, 1024)))
	 add_data(buffer, length);
      set_eof();
   } else
   {
	 // Open the stream (just in this function) too see if
	 // the file is accessible. In future we will be using 'OpenFiles'
	 // to request and release streams
      GP<ByteStream> str=ByteStream::create(fname_in,"rb");
      str->seek(0, SEEK_END);
      int file_size=str->tell();

      fname=fname_in;
      start=start_in;
      length=length_in;
      if (start>=file_size)
        length=0;
      else if (length<0 || start+length>=file_size)
        length=file_size-start;

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
   int retval=(-1);
   if (length>=0) 
   {
     retval=length;
   }else if (pool)
   {
      int plength=pool->get_length();
      if (plength>=0)
        retval=plength-start;
   }
   return retval;
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
      return (dlength<0)?0:(block_list->get_bytes(dstart, dlength));
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
      G_THROW("DataPool.add_data");     //  Function DataPool::add_data() may not be called for connected DataPools.
   
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
   block_list->add_range(offset, size);
   
      // Wake up all threads, which may be waiting for this data
   {
      GCriticalSectionLock lock(&readers_lock);
      for(GPosition pos=readers_list;pos;++pos)
      {
	 GP<Reader> reader=readers_list[pos];
	 if (block_list->get_bytes(reader->offset, 1))
	 {
	    DEBUG_MSG("waking up reader: offset=" << reader->offset <<
		      ", size=" << reader->size << "\n");
            DEBUG_MAKE_INDENT(3);
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
   if (length>=0 && data->size()>=length)
     set_eof();
}

bool
DataPool::has_data(int dstart, int dlength)
{
   if (dlength<0 && length>0)
     dlength=length-dstart;
   return (pool?(pool->has_data(start+dstart, dlength))
     :((fname.length())?(start+dstart+dlength<=length)
       :((dlength<0)?is_eof()
         :(block_list->get_bytes(dstart, dlength)==dlength))));
}

int
DataPool::get_data(void * buffer, int offset, int sz)
{
   return get_data(buffer, offset, sz, 0);
}

class DataPool::Incrementor
{
private:
   Counter      & counter;
public:
   Incrementor(Counter & xcounter) : counter(xcounter) {counter.inc();}
   ~Incrementor() {counter.dec();}
};

int
DataPool::get_data(void * buffer, int offset, int sz, int level)
{
   DEBUG_MSG("DataPool::get_data()\n");
   DEBUG_MAKE_INDENT(3);
   Incrementor inc(*active_readers);
   
   if (stop_flag)
     G_THROW("STOP");
   if (stop_blocked_flag && !is_eof() &&
       !has_data(offset, sz))
     G_THROW("STOP");
   
   if (sz < 0)
     G_THROW("DataPool.bad_size");        //  Size must be non negative

   if (! sz)
     return 0;

   if (pool)
   {
      DEBUG_MSG("DataPool::get_data(): from pool\n");
      DEBUG_MAKE_INDENT(3);
      int retval=0;
      if (length>0 && offset+sz>length)
        sz=length-offset;
      if (sz<0)
        sz=0;
      for(;;)
      {
	    // Ask the underlying (master) DataPool for data. Note, that
	    // master DataPool may throw the "DATA_POOL_REENTER" exception
	    // demanding all readers to restart. This happens when
	    // a DataPool in the chain of DataPools stops. All readers
	    // should return to the most upper level and then reenter the
	    // DataPools hierarchy. Some of them will be stopped by "STOP"
	    // exception.
	 G_TRY
         {
	    if(stop_flag||stop_blocked_flag&&!is_eof()&&!has_data(offset, sz))
              G_THROW("STOP");
	    retval=pool->get_data(buffer, start+offset, sz, level+1);
	 } G_CATCH(exc) {
            pool->clear_stream();
	    if (strcmp(exc.get_cause(), "DataPool.reenter") || level)
	      G_RETHROW;
	 } G_ENDCATCH;
         pool->clear_stream();
         return retval;
      }
   } 
   else if (fname.length())
   {
      DEBUG_MSG("DataPool::get_data(): from file\n");
      DEBUG_MAKE_INDENT(3);
      if (length>0 && offset+sz>length)
        sz=length-offset;
      if (sz<0)
        sz=0;
      
      GP<OpenFiles_File> f=fstream;
      if (!f)
      {
        GCriticalSectionLock lock(&class_stream_lock);
        f=fstream;
        if(!f)
        {
          fstream=f=OpenFiles::get()->request_stream(fname, this);
        }
      }
      GCriticalSectionLock lock2(&(f->stream_lock));
      f->stream->seek(start+offset, SEEK_SET); 
      return f->stream->readall(buffer, sz);
   } 
   else
   {
      DEBUG_MSG("DataPool::get_data(): direct\n");
      DEBUG_MAKE_INDENT(3);
	 // We're not connected to anybody => handle the data
      int size=block_list->get_range(offset, sz);
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
           G_THROW("EOF");
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
      G_TRY {
	       {
	          GCriticalSectionLock slock(&readers_lock);
	          readers_list.append(reader);
	       }
	       wait_for_data(reader);
      } G_CATCH_ALL {
	       {
	          GCriticalSectionLock slock(&readers_lock);
	          GPosition pos;
	          if (readers_list.search(reader, pos)) readers_list.del(pos);
	       }
	       G_RETHROW;
      } G_ENDCATCH;
   
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
   G_THROW("DataPool.no_threadless");  // Internal error. This function can't be used in threadless mode.
#else
   for(;;)
   {
      if (stop_flag)
        G_THROW("STOP");
      if (reader->reenter_flag)
        G_THROW("DataPool.reenter");    //  DATA_POOL_REENTER
      if (eof_flag || block_list->get_bytes(reader->offset, 1))
        return;
      if (pool || fname.length())
        return;

      if (stop_blocked_flag)
        G_THROW("STOP");

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
      while(*active_readers)
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
      
   if (pool)
     pool->restart_readers();
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
      DEBUG_MSG("loading the data from \"" << fname << "\".\n");

      GCriticalSectionLock lock1(&class_stream_lock);
      GP<OpenFiles_File> f=fstream;
      if (!f)
      {
        fstream=f=OpenFiles::get()->request_stream(fname, this);
      }
      {  // Scope to de-allocate lock2 before stream gets released
         GCriticalSectionLock lock2(&(f->stream_lock));

         data=ByteStream::create();
         block_list->clear();
         FCPools::get()->del_pool(fname, this);
         fname="";

         f->stream->seek(0, SEEK_SET);
         char buffer[1024];
         int length;
         while((length=f->stream->read(buffer, 1024)))
	         add_data(buffer, length);
	      // No need to set EOF. It should already be set.
        OpenFiles::get()->stream_released(f->stream, this);
      }
      fstream=0;
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
            block_list->get_bytes(t->start, t->length)==t->length)
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
// DataPool::add_trigger(int thresh, void (* callback)(GP<GPEnabled> &), GP<GPEnabled> cl_data)
DataPool::add_trigger(int thresh, void (* callback)(void *), void * cl_data)
{
  if (thresh>=0)
    add_trigger(0, thresh+1, callback, cl_data);
  else
    add_trigger(0, -1, callback, cl_data);
}

void
DataPool::add_trigger(int tstart, int tlength,
//		      void (* callback)(GP<GPEnabled> &), GP<GPEnabled> cl_data)
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
	    if (tlength>=0 && block_list->get_bytes(tstart, tlength)==tlength)
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
// DataPool::del_trigger(void (* callback)(GP<GPEnabled> &), GP<GPEnabled> cl_data)
DataPool::del_trigger(void (* callback)(void *), void * cl_data)
{
   DEBUG_MSG("DataPool::del_trigger(): func=" << (void *) callback << "\n");
   DEBUG_MAKE_INDENT(3);

   for(;;)
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
	    } else
              ++pos;
	 }
      }

	 // Above we removed the trigger from the list and unlocked the list
	 // Now we will disable it and will wait if necessary (if the
	 // trigger is currently being processed by check_triggers())
	 // check_triggers() locks the trigger for the duration of the
	 // trigger callback. Thus we will wait for the trigger callback
	 // to finish and avoid client's destruction.
      if (trigger)
        trigger->disabled=1;
      else
        break;
   }

   if (pool)
     pool->del_trigger(callback, cl_data);
}

void
// DataPool::static_trigger_cb(GP<GPEnabled> &cl_data)
DataPool::static_trigger_cb(void *cl_data)
{
//  GP<DataPool> d=(DataPool *)(GPEnabled *)cl_data;
  GP<DataPool> d=(DataPool *)cl_data;
  d->trigger_cb();
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
   
   GP<IFFByteStream> giff=IFFByteStream::create(str);
   IFFByteStream &iff=*giff;
   GString chkid;
   int size;
   if ((size=iff.get_chunk(chkid)) && size>=0)
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
   PoolByteStream(GP<DataPool> data_pool);
   virtual ~PoolByteStream() {};

   virtual size_t read(void *buffer, size_t size);
   virtual size_t write(const void *buffer, size_t size);
   virtual long tell(void) const ;
   virtual int seek(long offset, int whence = SEEK_SET, bool nothrow=false);
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
PoolByteStream::PoolByteStream(GP<DataPool> xdata_pool) :
   data_pool(xdata_pool), position(0), buffer_size(0), buffer_pos(0)
{
   if (!data_pool) 
       G_THROW("DataPool.zero_DataPool");   //  Internal error: ZERO DataPool passed as input.

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
   G_THROW("not_implemented_n\tPoolByteStream::write()");   //  PoolByteStream::write() is not implemented.
   return 0;	// For compiler not to bark
}

long
PoolByteStream::tell(void) const
{
   return position;
}

int
PoolByteStream::seek(long offset, int whence, bool nothrow)
{
  int retval=(-1);
  switch(whence)
  {
    case SEEK_CUR:
      offset+=position;
      // fallthrough;
    case SEEK_SET:
      if(offset<position)
      {
        if((int)(offset+buffer_pos)>=(int)position)
        {
          buffer_pos-=position-offset;
        }else
        {
          buffer_size=0;
        }
        position=offset;
      }else if(offset>position)
      {
        buffer_pos+=(offset-position)-1;
        position=offset-1;
        unsigned char c;
        if(read(&c,1)<1)
        {
          G_THROW("EOF");
        }
      }
      retval=0;
      break;
    case SEEK_END:
      if(! nothrow)
        G_THROW("DataPool.seek_backward");  //  Seeking backwards from EOF is not supported by this ByteStream
      break;
   }
   return retval;
}

void
DataPool::close_all(void)
{
  OpenFiles::get()->close_all();
}


inline GP<ByteStream>
DataPool::get_stream(void)
{
   return new PoolByteStream(this);
}

#if 0
void
DataPool::clear_stream(void)
{
  fstream=0;
}
#endif

inline
DataPool::Counter::operator int(void) const
{
   GCriticalSectionLock lk((GCriticalSection *) &lock);
   int cnt=counter;
   return cnt;
}

inline void
DataPool::Counter::inc(void)
{
   GCriticalSectionLock lk(&lock);
   counter++;
}

inline void
DataPool::Counter::dec(void)
{
   GCriticalSectionLock lk(&lock);
   counter--;
}

