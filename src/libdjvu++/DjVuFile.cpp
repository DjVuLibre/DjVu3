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
//C- $Id: DjVuFile.cpp,v 1.1.2.3 1999-04-29 18:46:12 eaf Exp $

#ifdef __GNUC__
#pragma implementation
#endif

#include "DjVuFile.h"

class ProgressByteStream : public ByteStream
{
public:
   ProgressByteStream(ByteStream * xstr) : str(xstr),
      last_call_pos(0) {}
   virtual ~ProgressByteStream() {}

   virtual size_t read(void *buffer, size_t size)
   {
      int cur_pos=str->tell();
      if (progress_cb && (last_call_pos/256!=cur_pos/256))
      {
	 progress_cb(cur_pos, progress_cl_data);
	 last_call_pos=cur_pos;
      }
      return str->read(buffer, size);
   }
   virtual size_t write(const void *buffer, size_t size)
   {
      return str->write(buffer, size);
   }
   virtual void seek(long offset, int whence = SEEK_SET)
   {
      str->seek(offset, whence);
   }
   virtual long tell() { return str->tell(); }
   virtual int  is_seekable(void) const { return str->is_seekable(); }

   void		set_progress_cb(void (* xprogress_cb)(int, void *),
				void * xprogress_cl_data)
   {
      progress_cb=xprogress_cb;
      progress_cl_data=xprogress_cl_data;
   }
private:
   ByteStream	* str;
   void		* progress_cl_data;
   void		(* progress_cb)(int pos, void *);
   int		last_call_pos;
   
      // Cancel C++ default stuff
   ProgressByteStream & operator=(const ProgressByteStream &);
};

DjVuFile::DjVuFile(const GURL & xurl, DjVuPort * port,
		   GCache<GURL, DjVuFile> * xcache) :
      url(xurl), cache(xcache), status(0), simple_port(0)
{
   DEBUG_MSG("DjVuFile::DjVuFile(): url='" << url << "'\n");
   DEBUG_MAKE_INDENT(3);
   
   file_size=0;
   decode_thread=0;
   
   DjVuPortcaster * pcaster=get_portcaster();
   
      // We need it 'cause we're waiting for our own termination in stop_decode()
   pcaster->add_route(this, this);
   if (port) pcaster->add_route(this, port);
   else
   {
      simple_port=new DjVuSimplePort();
      pcaster->add_route(this, simple_port);
   }
      
   if (!(data_range=pcaster->request_data(this, url)))
      THROW("Failed get data for URL '"+url+"'");
   data_range->add_trigger(-1, static_trigger_cb, this);
}

DjVuFile::~DjVuFile(void)
{
   DEBUG_MSG("DjVuFile::~DjVuFile(): destroying...\n");
   DEBUG_MAKE_INDENT(3);
   
   stop_decode(1);
}

int
DjVuFile::get_status(void) const
{
   GCriticalSectionLock lock((GCriticalSection *) &status_lock);
   return status;
}

bool
DjVuFile::is_decoding(void) const
{
   GCriticalSectionLock lock((GCriticalSection *) &status_lock);
   return (status & DECODING)!=0;
}

bool
DjVuFile::is_decode_ok(void) const
{
   GCriticalSectionLock lock((GCriticalSection *) &status_lock);
   return (status & DECODE_OK)!=0;
}

bool
DjVuFile::is_decode_failed(void) const
{
   GCriticalSectionLock lock((GCriticalSection *) &status_lock);
   return (status & DECODE_FAILED)!=0;
}

bool
DjVuFile::is_decode_stopped(void) const
{
   GCriticalSectionLock lock((GCriticalSection *) &status_lock);
   return (status & DECODE_STOPPED)!=0;
}

bool
DjVuFile::is_data_present(void) const
{
   GCriticalSectionLock lock((GCriticalSection *) &status_lock);
   return (status & DATA_PRESENT)!=0;
}

bool
DjVuFile::is_all_data_present(void) const
{
   GCriticalSectionLock lock((GCriticalSection *) &status_lock);
   return (status & ALL_DATA_PRESENT)!=0;
}

unsigned int
DjVuFile::get_memory_usage(void) const
{
   unsigned int size=sizeof(*this);
   if (info) size+=info->get_memory_usage();
   if (anno) size+=anno->get_memory_usage();
   if (bg44) size+=bg44->get_memory_usage();
   if (fgjb) size+=fgjb->get_memory_usage();
   if (fgpm) size+=fgpm->get_memory_usage();
   if (dir) size+=dir->get_memory_usage();
   return size;
}

GPList<DjVuFile>
DjVuFile::get_included_files(void)
{
   GCriticalSectionLock lock(&inc_files_lock);

   GPList<DjVuFile> list=inc_files_list;	// Get a copy when locked
   return list;
}

void
DjVuFile::wait_for_chunk(void)
      // Will return after a chunk has been decoded
{
   DEBUG_MSG("DjVuFile::wait_for_chunk() called\n");
   DEBUG_MAKE_INDENT(3);
   
   chunk_mon.enter();
   chunk_mon.wait();
}

bool
DjVuFile::wait_for_finish(bool self)
      // if self==TRUE, will block until decoding of this file is over
      // if self==FALSE, will block until decoding of a child is over
      // Will return FALSE if there is nothing to wait for. TRUE otherwise
{
#ifdef DEBUG_MSG
   DEBUG_MSG("DjVuFile::wait_for_finish() called\n");
   if (self) DEBUG_MSG("Waiting for self termination, btw\n");
   DEBUG_MAKE_INDENT(3);
#endif
   
      // By locking the monitor, we guarantee that situation doesn't change
      // between the moments when we check for pending finish events
      // and when we actually run wait(). If we don't lock, the last child
      // may terminate in between, and we'll wait forever.
      //
      // Locking is required by GMonitor interface too, btw.
   GMonitorLock lock(&finish_mon);
   if (self && is_decoding())
   {
      finish_mon.wait();
      DEBUG_MSG("got it\n");
      return 1;
   };
   if (!self)
   {
      GP<DjVuFile> file;
      {
	 GCriticalSectionLock lock(&inc_files_lock);
	 for(GPosition pos=inc_files_list;pos;++pos)
	 {
	    GP<DjVuFile> & f=inc_files_list[pos];
	    if (f->is_decoding())
	    {
	       file=f; break;
	    }
	 }
      }
      if (file)
      {
	 finish_mon.wait();
	 DEBUG_MSG("got it\n");
	 return 1;
      };
   };
   DEBUG_MSG("nothing to wait for\n");
   return 0;
}

void
DjVuFile::notify_chunk_done(const DjVuPort *, const char *)
{
   GMonitorLock lock(&chunk_mon);
   chunk_mon.broadcast();
}

void
DjVuFile::notify_file_done(const DjVuPort * src)
{
   if (src!=this)
   {
      GCriticalSectionLock inc_lock(&inc_files_lock);
      GPosition pos;
      for(pos=inc_files_list;pos;++pos)
	 if (inc_files_list[pos]==src) break;
      if (!pos) return;
   }
   GMonitorLock lock(&finish_mon);
   finish_mon.broadcast();
}

void
DjVuFile::notify_file_stopped(const DjVuPort * src)
{
   DjVuFile::notify_file_done(src);
}

void
DjVuFile::notify_file_failed(const DjVuPort * src)
{
   DjVuFile::notify_file_done(src);
}

void
DjVuFile::static_decode_func(void * cl_data)
{
   DjVuFile * th=(DjVuFile *) cl_data;
   th->decode_func();
}

void
DjVuFile::decode_func(void)
{
   DEBUG_MSG("DjVuFile::decode_func() called, url='" << url << "'\n");
   DEBUG_MAKE_INDENT(3);
   
      // In case if all reference to this file are suddenly lost
   GP<DjVuFile> life_saver=this;
   decode_life_saver=0;

   DjVuPortcaster * pcaster=get_portcaster();
   ByteStream * decode_stream=0;
   
   TRY {
      decode_data_range=new DataRange(*data_range);
      decode_stream=decode_data_range->get_stream();
      ProgressByteStream pstr(decode_stream);
      pstr.set_progress_cb(progress_cb, this);
      decode(pstr);

	 // Wait for all child files to finish
      while(wait_for_finish(0));

	 // Check for termination status
      GCriticalSectionLock lock(&inc_files_lock);
      for(GPosition pos=inc_files_list;pos;++pos)
      {
	 GP<DjVuFile> & f=inc_files_list[pos];
	 if (f->is_decode_failed()) THROW("Decode of an included file failed.");
	 if (f->is_decode_stopped()) THROW("STOP");
	 if (!f->is_decode_ok())
	    THROW("Internal error: an included file has not finished yet.");
      };
   } CATCH(exc) {
      GCriticalSectionLock lock(&status_lock);
      delete decode_stream; decode_stream=0;
      status&=~DECODING;
      if (strstr(exc.get_cause(), "STOP"))
      {
	 status|=DECODE_STOPPED;
	 pcaster->notify_status(this, GString(url)+" STOPPED");
	 pcaster->notify_file_stopped(this);
      } else
      {
	 status|=DECODE_FAILED;
	 pcaster->notify_status(this, GString(url)+" FAILED");
	 pcaster->notify_error(this, exc.get_cause());
	 pcaster->notify_file_failed(this);
      };
   } ENDCATCH;

   delete decode_stream; decode_stream=0;

   TRY {
      GCriticalSectionLock lock(&status_lock);
      if (is_decoding())
      {
	 status|=DECODE_OK;
	 status&=~DECODING;
	 pcaster->notify_file_done(this);
      };
   } CATCH(exc) {} ENDCATCH;
}

GP<DjVuFile>
DjVuFile::process_incl_chunk(ByteStream & str)
{
   GString incl_str;
   char buffer[1024];
   int length;
   while((length=str.read(buffer, 1024)))
      incl_str+=GString(buffer, length);
   
      // Eat '\n' in the beginning and at the end
   while(incl_str.length() && incl_str[0]=='\n')
   {
      GString tmp=((const char *) incl_str)+1; incl_str=tmp;
   };
   while(incl_str.length()>0 && incl_str[incl_str.length()-1]=='\n')
      incl_str.setat(incl_str.length()-1, 0);

   if (incl_str.length()>0)
   {
      if (strchr(incl_str, '/'))
	 THROW("Malformed INCL chunk. No directories allowed.");
      
      GURL incl_url=url.baseURL()+incl_str;
      GCriticalSectionLock lock(&inc_files_lock);
      GPosition pos;
      for(pos=inc_files_list;pos;++pos)
	 if (inc_files_list[pos]->url==incl_url) break;
      if (!pos)
      {
	 GP<DjVuFile> file;
	 if (cache) file=cache->get_item(incl_url);
	 if (!file)
	 {
	    file=new DjVuFile(incl_url, this, cache);
	    if (cache) cache->add_item(incl_url, file);
	 };
	 get_portcaster()->add_route(file, this);
	 inc_files_list.append(file);
	 return file;
      } else return inc_files_list[pos];
   };
   return 0;
}

void
DjVuFile::decode(ByteStream & str)
{
   DEBUG_MSG("DjVuFile::decode(), url='" << url << "'\n");
   DEBUG_MAKE_INDENT(3);
   
   DjVuPortcaster * pcaster=get_portcaster();

   int chksize;
   GString chkid;
   
   IFFByteStream iff(str);
   if (!iff.get_chunk(chkid)) THROW("EOF");

   bool djvi=(chkid=="FORM:DJVI");

   if (chkid=="FORM:DJVU" || djvi)
   {
      GString desc;
      mimetype="image/djvu";
      while((chksize=iff.get_chunk(chkid)))
      {
	 DEBUG_MSG("decoding chunk '" << chkid << "'\n");
	 
	 if (chkid=="INFO")
	 {
	    if (DjVuFile::info)
	       THROW("DjVu Decoder: Corrupted file (Duplicate INFO chunk)");
	    GP<DjVuInfo> info=new DjVuInfo();
	    info->decode(iff);
	    DjVuFile::info=info;
	    pcaster->notify_relayout(this);
	    desc.format(" %0.1f Kb\t'%s'\tPage information.\n", 
			chksize/1024.0, (const char*)chkid );
              
	 } else if (chkid=="ANTa")
	 {
	    if (!anno)
	    {
	       anno=new DjVuAnno();
	       anno->decode(iff);
	    } else anno->merge(iff);
	    desc.format(" %0.1f Kb\t'%s'\tPage annotation.\n",
			chksize/1024.0, (const char*)chkid);
	 } else if (chkid=="INCL")
	 {
	    GP<DjVuFile> file=process_incl_chunk(iff);
	    if (file &&
		!file->is_decoding() &&
		!file->is_decode_ok() &&
		!file->is_decode_failed())
	       file->start_decode();
	    desc.format(" %0.1f Kb\t'%s'\tIndirection chunk.\n",
			chksize/1024.0, (const char*)chkid);
	 } else if (chkid=="NDIR")
	 {
	    GP<DjVuNavDir> dir=new DjVuNavDir(url);
	    dir->decode(iff);
	    DjVuFile::dir=dir;
	    desc.format(" %0.1f Kb\t'%s'\tNavigation chunk.\n",
			chksize/1024.0, (const char*)chkid);
	 } else if (chkid=="BG44")
	 {
	    if (!bg44)
	    {
		  // First chunk
	       GP<IWPixmap> bg44=new IWPixmap();
	       bg44->decode_chunk(iff);
	       DjVuFile::bg44=bg44;
	       pcaster->notify_redisplay(this);
	       desc.format(" %0.1f Kb\t'%s'\tIW44 background (%dx%d)\n",
			   chksize/1024.0, (const char*)chkid,
			   bg44->get_width(), bg44->get_height());
	    } else
	    {
		  // Refinement chunks
	       bg44->decode_chunk(iff);
	       pcaster->notify_redisplay(this);
	       desc.format(" %0.1f Kb\t'%s'\tIW44 background (part %d).\n",
			   chksize/1024.0, (const char*)chkid,
			   bg44->get_serial());
                  
	    }
	 } else if (chkid=="Sjbz")
	 {
	    if (DjVuFile::fgjb)
	       THROW("DjVu Decoder: Corrupted data (Duplicate FGxx chunk)");
	    GP<JB2Image> fgjb=new JB2Image();
	    fgjb->decode(iff);
	    DjVuFile::fgjb=fgjb;
	    pcaster->notify_redisplay(this);
	    desc.format(" %0.1f Kb\t'%s'\tJB2 foreground mask (%dx%d)\n",
			chksize/1024.0, (const char*)chkid,
			fgjb->get_width(), fgjb->get_height());
	 } else if (chkid=="FG44")
	 {
	    if (fgpm)
	       THROW("DjVu Decoder: Corrupted data (Duplicate foreground layer)");
	    IWPixmap fg44;
	    fg44.decode_chunk(iff);
	    fgpm=fg44.get_pixmap();
	    pcaster->notify_redisplay(this);
	    desc.format(" %0.1f Kb\t'%s'\tIW44 foreground colors (%dx%d)\n",
			chksize/1024.0, (const char*)chkid,
			fg44.get_width(), fg44.get_height());
	 } else if (chkid=="BGjp")
	 {
	    if (bg44)
	       THROW("DjVu Decoder: Corrupted data (Duplicate background layer)");
	    desc.format(" %0.1f Kb\t'%s'\tObsolete JPEG background (Ignored).\n", 
			chksize/1024.0, (const char*)chkid);
	 } else if (chkid=="FGjp")
	 {
	    if (fgpm)
	       THROW("DjVu Decoder: Corrupted data (Duplicate foreground layer)");
	    desc.format(" %0.1f Kb\t'%s'\tObsolete JPEG foreground colors (Ignored).\n", 
			chksize/1024.0, (const char*)chkid);
	 } else 
	 {
	    desc.format(" %0.1f Kb\t'%s'\tUnknown chunk (Ignored).\n",
			chksize/1024.0, (const char*)chkid);
	 };
	    // Update description and notify
	 description=description+desc;
	 pcaster->notify_chunk_done(this, chkid);
	 
	    // Close chunk
	 iff.close_chunk();
      }

	 // Record file size
      file_size=iff.tell();
	 // Close BG44 codec
      if (bg44) bg44->close_codec();
	 // Complete description
      if (info)
      {
	 desc.format("DJVU Image (%dx%d) version %d:\n\n", 
		     info->width, info->height, info->version);
	 description=desc+description;
	 int rawsize=info->width*info->height*3;
	 desc.format("\nCompression ratio: %0.f (%0.1f Kb)\n",
		     (double)rawsize/file_size, file_size/1024.0 );
	 description=description+desc;
      } else if (!djvi)
	 THROW("DjVu Decoder: Corrupted data (Missing INFO chunk)");
   } else if (chkid=="FORM:PM44" || chkid=="FORM:BM44")
   {
      GString desc;
      mimetype="image/iw44";
      while((chksize=iff.get_chunk(chkid)))
      {
	 DEBUG_MSG("decoding chunk '" << chkid << "'\n");
	 
	 if (chkid=="ANTa")
	 {
	    if (!anno)
	    {
	       anno=new DjVuAnno();
	       anno->decode(iff);
	    } else anno->merge(iff);
	    desc.format(" %0.1f Kb\t'%s'\tPage annotation.\n",
			chksize/1024.0, (const char*)chkid);
	 } else if (chkid=="INCL")
	 {
	    GP<DjVuFile> file=process_incl_chunk(iff);
	    if (file &&
		!file->is_decoding() &&
		!file->is_decode_ok() &&
		!file->is_decode_failed())
	       file->start_decode();
	    desc.format(" %0.1f Kb\t'%s'\tIndirection chunk (Unsupported).\n",
			chksize/1024.0, (const char*)chkid);
	 } else if (chkid=="NDIR")
	 {
	    GP<DjVuNavDir> dir=new DjVuNavDir(url);
	    dir->decode(iff);
	    DjVuFile::dir=dir;
	    desc.format(" %0.1f Kb\t'%s'\tNavigation chunk (Unsupported).\n",
			chksize/1024.0, (const char*)chkid);
	 } else if (chkid=="PM44" || chkid=="BM44")
	 {
	    if (!bg44)
	    {
                  // First chunk
	       GP<IWPixmap> bg44=new IWPixmap();
	       bg44->decode_chunk(iff);
	       GP<DjVuInfo> info=new DjVuInfo();
	       info->width=bg44->get_width();
	       info->height=bg44->get_height();
	       info->dpi=100;
	       DjVuFile::bg44=bg44;
	       DjVuFile::info=info;
	       pcaster->notify_relayout(this);
	    } else
	    {
                  // Refinement chunks
	       bg44->decode_chunk(iff);
	       pcaster->notify_redisplay(this);
	    }
	    desc.format(" %0.1f Kb\t'%s'\tIW44 wavelet data (part %d)\n", 
			chksize/1024.0, (const char*)chkid, 
			bg44->get_serial());
	 } else
	 {
	    desc.format(" %0.1f Kb\t'%s'\tUnknown chunk (Ignored).\n",
			chksize/1024.0, (const char*)chkid);
	 }
	    // Update description and notify
	 description=description+desc;
	 pcaster->notify_chunk_done(this, chkid);
	 
	    // Close chunk
	 iff.close_chunk();
      }
	 // Record file size
      file_size=iff.tell();
	 // Close BG44
      if (bg44) bg44->close_codec();
	 // Complete description
      if (info)
      {
	 desc.format("IW44 Image (%dx%d) :\n\n", 
		     info->width, info->height);
	 description=desc+description;
	 int rawsize=info->width*info->height*3;
	 desc.format("\nCompression ratio: %0.f (%0.1f Kb)\n",
		     (double)rawsize/file_size, file_size/1024.0);
	 description=description+desc;
      } else
	 THROW("DjVu Decoder: Corrupted data (Missing IW44 data chunks)");
   } else
   {
      THROW("DejaVu decoder: a DJVU or IW44 image was expected");
   }
}

void
DjVuFile::start_decode(void)
{
   DEBUG_MSG("DjVuFile::start_decode(), url='" << url << "'\n");
   DEBUG_MAKE_INDENT(3);
   
   GCriticalSectionLock st_lock(&status_lock);
   TRY {
      if (!is_decoding())
      {
	 status&=~(DECODE_OK | DECODE_STOPPED | DECODE_FAILED);
	 status|=DECODING;
      
	 delete decode_thread; decode_thread=0;
	 decode_thread=new GThread();
	 decode_life_saver=this;	// To prevent unexpected destruction
	 decode_thread->create(static_decode_func, this);
      };
   } CATCH(exc) {
      status&=~DECODING;
      status|=DECODE_FAILED;
      get_portcaster()->notify_file_failed(this);
      RETHROW;
   } ENDCATCH;
}

void
DjVuFile::stop_decode(bool sync)
{
   DEBUG_MSG("DjVuFile::stop_decode(), url='" << url <<
	     "', sync=" << (int) sync << "\n");
   DEBUG_MAKE_INDENT(3);
   
      // Don't stop SYNCHRONOUSLY from the thread where the decoding is going!!!
   {
	 // First - ask every included child to stop in async mode
      GCriticalSectionLock lock(&inc_files_lock);
      for(GPosition pos=inc_files_list;pos;++pos)
	 inc_files_list[pos]->stop_decode(0);

      if (decode_data_range) decode_data_range->stop();
   }

   if (sync)
   {
      while(1)
      {
	 GP<DjVuFile> file;
	 {
	    GCriticalSectionLock lock(&inc_files_lock);
	    for(GPosition pos=inc_files_list;pos;++pos)
	    {
	       GP<DjVuFile> & f=inc_files_list[pos];
	       if (f->is_decoding())
	       {
		  file=f; break;
	       };
	    };
	 }
	 if (!file) break;

	 file->stop_decode(1);
      };

      wait_for_finish(1);	// Wait for self termination
   }
}

void
DjVuFile::process_incl_chunks(void)
{
   ByteStream * str=data_range->get_stream();

   TRY {
      int chksize;
      GString chkid;
      IFFByteStream iff(*str);
      if (!iff.get_chunk(chkid)) THROW("File does not appear to be in IFF format.");

      while((chksize=iff.get_chunk(chkid)))
      {
	 if (chkid=="INCL") process_incl_chunk(iff);
	 iff.close_chunk();
      }
   } CATCH(exc) {
      delete str; str=0;
      RETHROW;
   } ENDCATCH;
   
   delete str; str=0;
   status|=INCL_FILES_CREATED;
}

void
DjVuFile::notify_all_data_received(const DjVuPort * src)
{
#ifdef DEBUG_MSG
   DEBUG_MSG("Intercept a notify_all_data_received()\n");
   DEBUG_MAKE_INDENT(3);
   if (src!=this) { DEBUG_MSG("it comes from an included file\n"); }
   else DEBUG_MSG("and it comes from us\n");
   if (status & INCL_FILES_CREATED) { DEBUG_MSG("and we HAVE all files included\n"); }
   else DEBUG_MSG("and we do NOT have all files included\n");
#endif

   if ((status & INCL_FILES_CREATED) && src!=this)
   {
	 // Check if all children have data
      GCriticalSectionLock lock(&inc_files_lock);
      GPosition pos;
      for(pos=inc_files_list;pos;++pos)
	 if (!inc_files_list[pos]->is_all_data_present()) break;
      if (!pos)
      {
	 DEBUG_MSG("Just got ALL data for '" << url << "'\n");
	 status|=ALL_DATA_PRESENT;
	 get_portcaster()->notify_all_data_received(this);
      }
   }
}

void
DjVuFile::static_trigger_cb(void * cl_data)
{
   DjVuFile * th=(DjVuFile *) cl_data;
   th->trigger_cb();
}

void
DjVuFile::trigger_cb(void)
{
   DEBUG_MSG("DjVuFile::trigger_cb(): got data for '" << url << "'\n");
   DEBUG_MAKE_INDENT(3);

   file_size=data_range->get_length();
   status|=DATA_PRESENT;
   get_portcaster()->notify_file_data_received(this);

   process_incl_chunks();

   GCriticalSectionLock flock(&inc_files_lock);
   GPosition pos;
   for(pos=inc_files_list;pos;++pos)
      if (!inc_files_list[pos]->is_all_data_present()) break;
   if (!pos)
   {
      DEBUG_MSG("It appears, that we have ALL data for '" << url << "'\n");
      status|=ALL_DATA_PRESENT;
      get_portcaster()->notify_all_data_received(this);
   }
}

void
DjVuFile::progress_cb(int pos, void * cl_data)
{
   DEBUG_MSG("DjVuFile::progress_cb() called\n");
   DEBUG_MAKE_INDENT(3);

   DjVuFile * th=(DjVuFile *) cl_data;

   int length=th->decode_data_range->get_length();
   if (length>0)
   {
      float progress=(float) pos/length;
      DEBUG_MSG("progress=" << progress << "\n");
      get_portcaster()->notify_decode_progress(th, progress);
   } else
   {
      DEBUG_MSG("DataRange size is still unknown => ignoring\n");
   };
}

void
DjVuFile::unlink_file(const char * name)
{
   DEBUG_MSG("DjVuFile::unlink_file(): name='" << name << "'\n");
   DEBUG_MAKE_INDENT(3);

   process_incl_chunks();

   bool done=0;
   MemoryByteStream str_out;
   IFFByteStream iff_out(str_out);
   
   ByteStream * str_in=0;
   TRY {
      str_in=data_range->get_stream();

      int chksize;
      GString chkid;
      IFFByteStream iff_in(*str_in);
      if (!iff_in.get_chunk(chkid)) THROW("File does not appear to be in IFF format.");

      iff_out.put_chunk(chkid);

      while((chksize=iff_in.get_chunk(chkid)))
      {
	 if (chkid=="INCL")
	 {
	    GString incl_str;
	    char buffer[1024];
	    int length;
	    while((length=iff_in.read(buffer, 1024)))
	       incl_str+=GString(buffer, length);

	       // Eat '\n' in the beginning and at the end
	    while(incl_str.length() && incl_str[0]=='\n')
	    {
	       GString tmp=((const char *) incl_str)+1; incl_str=tmp;
	    }
	    while(incl_str.length()>0 && incl_str[incl_str.length()-1]=='\n')
	       incl_str.setat(incl_str.length()-1, 0);
	    
	    if (incl_str==name) done=1;
	    else
	    {
	       iff_out.put_chunk("INCL");
	       iff_out.writall(incl_str, incl_str.length());
	       iff_out.close_chunk();
	    }
	 } else
	 {
	    iff_out.put_chunk(chkid);
	    char buffer[1024];
	    int length;
	    while((length=iff_in.read(buffer, 1024)))
	       iff_out.writall(buffer, length);
	    iff_out.close_chunk();
	 }
	 iff_in.close_chunk();
      }
      iff_out.close_chunk();
   } CATCH(exc) {
      delete str_in; str_in=0;
      RETHROW;
   } ENDCATCH;
   delete str_in; str_in=0;

   if (done)
   {
	 // Replace data_range with the new one
      GP<DataPool> data_pool=new DataPool();
      str_out.seek(0, SEEK_SET);
      char buffer[1024];
      int length;
      while((length=str_out.read(buffer, 1024)))
	 data_pool->add_data(buffer, length);
      data_pool->set_eof();
      data_range=new DataRange(data_pool);
   }

   GCriticalSectionLock lock(&inc_files_lock);
   GPosition pos=inc_files_list;
   while(pos)
   {
      GP<DjVuFile> f=inc_files_list[pos];
      if (f->get_url().fileURL()==name)
      {
	 GPosition this_pos=pos;
	 ++pos;
	 inc_files_list.del(this_pos);
      } else ++pos;
   }
}

void
DjVuFile::include_file(const GP<DjVuFile> & file, int chunk_pos)
{
   DEBUG_MSG("DjVuFile::include_file(): incl_url='" << file->get_url() << "'\n");
   DEBUG_MAKE_INDENT(3);

      // Prepare the new file's url
   GString file_name=file->get_url().fileURL();
   GURL file_url=url.baseURL()+file_name;
   
      // See if the file is already included
   process_incl_chunks();
   {
      GCriticalSectionLock lock(&inc_files_lock);
      for(GPosition pos=inc_files_list;pos;++pos)
	 if (inc_files_list[pos]->get_url()==file_url)
	    THROW("File with name '"+file_name+"' is already included.");
   }

   file->move(url.baseURL());

      // Set info route
   file->disable_standard_port();
   get_portcaster()->add_route(file, this);

      // Insert INCL chunk
   MemoryByteStream str_out;
   IFFByteStream iff_out(str_out);
   
   ByteStream * str_in=0;
   TRY {
      str_in=data_range->get_stream();

      int chksize;
      GString chkid;
      IFFByteStream iff_in(*str_in);
      if (!iff_in.get_chunk(chkid)) THROW("File does not appear to be in IFF format.");

      iff_out.put_chunk(chkid);

      int chunk_num=0, inc_chunk_num=0;
      bool stored=0;
      while((chksize=iff_in.get_chunk(chkid)))
      {
	 if (chunk_num==chunk_pos)
	 {
	    iff_out.put_chunk("INCL");
	    iff_out.write(file_name, file_name.length());
	    iff_out.close_chunk();
	    stored=1;

	    GCriticalSectionLock lock(&inc_files_lock);
	    GPosition pos;
	    if (inc_files_list.nth(inc_chunk_num, pos))
	       inc_files_list.insert_before(pos, file);
	    else inc_files_list.append(file);
	 }
	 iff_out.put_chunk(chkid);
	 char buffer[1024];
	 int length;
	 while((length=iff_in.read(buffer, 1024)))
	    iff_out.writall(buffer, length);
	 iff_in.close_chunk();
	 iff_out.close_chunk();
	 chunk_num++;
	 if (chkid=="INCL") inc_chunk_num++;
      }

      if (!stored)
      {
	 iff_out.put_chunk("INCL");
	 iff_out.write(file_name, file_name.length());
	 iff_out.close_chunk();

	 GCriticalSectionLock lock(&inc_files_lock);
	 inc_files_list.append(file);
      }
      iff_out.close_chunk();
   } CATCH(exc) {
      delete str_in; str_in=0;
      RETHROW;
   } ENDCATCH;
   delete str_in; str_in=0;

      // Replace data_range with the new one
   GP<DataPool> data_pool=new DataPool();
   str_out.seek(0, SEEK_SET);
   char buffer[1024];
   int length;
   while((length=str_out.read(buffer, 1024)))
      data_pool->add_data(buffer, length);
   data_pool->set_eof();
   data_range=new DataRange(data_pool);
}

//*****************************************************************************
//****************************** Data routines ********************************
//*****************************************************************************

int
DjVuFile::get_chunks_number(void)
{
   int chunks=0;
   ByteStream * str=data_range->get_stream();
   TRY {
      int chksize;
      GString chkid;
      IFFByteStream iff(*str);
      if (!iff.get_chunk(chkid)) THROW("File does not appear to be in IFF format.");

      while((chksize=iff.get_chunk(chkid)))
      {
	 chunks++;
	 iff.close_chunk();
      }
   } CATCH(exc) {
      delete str; str=0;
      RETHROW;
   } ENDCATCH;
   delete str; str=0;
   return chunks;
}

GString
DjVuFile::get_chunk_name(int chunk_num)
{
   GString name;
   ByteStream * str=data_range->get_stream();
   TRY {
      int chksize;
      GString chkid;
      IFFByteStream iff(*str);
      if (!iff.get_chunk(chkid)) THROW("File does not appear to be in IFF format.");

      int chunk=0;
      while((chksize=iff.get_chunk(chkid)))
      {
	 if (chunk++==chunk_num) { name=chkid; break; }
	 iff.close_chunk();
      }
   } CATCH(exc) {
      delete str; str=0;
      RETHROW;
   } ENDCATCH;
   delete str; str=0;
   if (!name.length()) THROW("Too few chunks.");
   return name;
}

bool
DjVuFile::contains_chunk(const char * chunk_name)
{
   DEBUG_MSG("DjVuFile::contains_chunk(): url='" << url << "', chunk_name='" <<
	     chunk_name << "'\n");
   DEBUG_MAKE_INDENT(3);
   
   bool contains=0;
   ByteStream * str=data_range->get_stream();
   TRY {
      int chksize;
      GString chkid;
      IFFByteStream iff(*str);
      if (!iff.get_chunk(chkid)) THROW("File does not appear to be in IFF format.");

      while((chksize=iff.get_chunk(chkid)))
      {
	 if (chkid==chunk_name) { contains=1; break; }
	 iff.close_chunk();
      }
   } CATCH(exc) {
      delete str; str=0;
      RETHROW;
   } ENDCATCH;
   delete str; str=0;
   return contains;
}

void
DjVuFile::delete_chunks(const char * chunk_name)
{
   DEBUG_MSG("DjVuFile::delete_chunks(): chunk_name='" << chunk_name << "'\n");
   DEBUG_MAKE_INDENT(3);

   if (chunk_name=="INCL") THROW("Can't delete INCL chunks. Use unlink_file() instead.");
   
   bool done=0;
   MemoryByteStream str_out;
   IFFByteStream iff_out(str_out);
   
   ByteStream * str_in=0;
   TRY {
      str_in=data_range->get_stream();

      int chksize;
      GString chkid;
      IFFByteStream iff_in(*str_in);
      if (!iff_in.get_chunk(chkid)) THROW("File does not appear to be in IFF format.");

      iff_out.put_chunk(chkid);

      while((chksize=iff_in.get_chunk(chkid)))
      {
	 if (chkid!=chunk_name)
	 {
	    iff_out.put_chunk(chkid);
	    char buffer[1024];
	    int length;
	    while((length=iff_in.read(buffer, 1024)))
	       iff_out.writall(buffer, length);
	    iff_out.close_chunk();
	 } else done=1;
	 iff_in.close_chunk();
      }

      iff_out.close_chunk();
   } CATCH(exc) {
      delete str_in; str_in=0;
      RETHROW;
   } ENDCATCH;
   delete str_in; str_in=0;

   if (done)
   {
	 // Replace data_range with the new one
      GP<DataPool> data_pool=new DataPool();
      str_out.seek(0, SEEK_SET);
      char buffer[1024];
      int length;
      while((length=str_out.read(buffer, 1024)))
	 data_pool->add_data(buffer, length);
      data_pool->set_eof();
      data_range=new DataRange(data_pool);
   }
}

void
DjVuFile::insert_chunk(int pos, const char * chunk_name,
		       const TArray<char> & data)
{
   DEBUG_MSG("DjVuFile::insert_chunk(): chunk_name='" << chunk_name << "'\n");
   DEBUG_MAKE_INDENT(3);

   if (chunk_name=="INCL") THROW("Can't insert INCL chunk. Use include_file() instead.");
   
   MemoryByteStream str_out;
   IFFByteStream iff_out(str_out);
   
   ByteStream * str_in=0;
   TRY {
      str_in=data_range->get_stream();

      int chksize;
      GString chkid;
      IFFByteStream iff_in(*str_in);
      if (!iff_in.get_chunk(chkid)) THROW("File does not appear to be in IFF format.");

      iff_out.put_chunk(chkid);

      bool done=0;
      int chunk=0;
      while((chksize=iff_in.get_chunk(chkid)))
      {
	 if (pos>=0 && chunk==pos)
	 {
	    iff_out.put_chunk(chunk_name);
	    iff_out.writall(data, data.size());
	    iff_out.close_chunk();
	    done=1;
	 }
	 iff_out.put_chunk(chkid);
	 char buffer[1024];
	 int length;
	 while((length=iff_in.read(buffer, 1024)))
	    iff_out.writall(buffer, length);
	 iff_out.close_chunk();
	 iff_in.close_chunk();
	 chunk++;
      }
      if (!done)
      {
	 iff_out.put_chunk(chunk_name);
	 iff_out.writall(data, data.size());
	 iff_out.close_chunk();
      }

      iff_out.close_chunk();
   } CATCH(exc) {
      delete str_in; str_in=0;
      RETHROW;
   } ENDCATCH;
   delete str_in; str_in=0;

      // Replace data_range with the new one
   GP<DataPool> data_pool=new DataPool();
   str_out.seek(0, SEEK_SET);
   char buffer[1024];
   int length;
   while((length=str_out.read(buffer, 1024)))
      data_pool->add_data(buffer, length);
   data_pool->set_eof();
   data_range=new DataRange(data_pool);
}

//*****************************************************************************
//****************************** Save routines ********************************
//*****************************************************************************

void
DjVuFile::add_djvu_data(IFFByteStream & ostr, GMap<GURL, void *> & map,
			bool included_too, bool no_ndir)
{
   if (map.contains(url)) return;

   bool top_level=map.size()==0;
   
   map[url]=0;

   ByteStream * str=data_range->get_stream();

   TRY {
      int chksize;
      GString chkid;
      IFFByteStream iff(*str);
      if (!iff.get_chunk(chkid)) THROW("File does not appear to be in IFF format.");

      if (top_level) ostr.put_chunk(chkid);
      
      while((chksize=iff.get_chunk(chkid)))
      {
	 if (chkid=="INCL" && included_too)
	 {
	    GP<DjVuFile> file=process_incl_chunk(iff);
	    file->add_djvu_data(ostr, map, included_too, no_ndir);
	 } else if (chkid=="ANTa" && anno)
	 {
	    ostr.put_chunk(chkid);
	    anno->encode(ostr);
	    ostr.close_chunk();
	 } else if (chkid=="NDIR" && dir && !no_ndir)
	 {
	    ostr.put_chunk(chkid);
	    dir->encode(ostr);
	    ostr.close_chunk();
	 } else if (!no_ndir || chkid!="NDIR")
	 {
	    ostr.put_chunk(chkid);
	    char buffer[1024];
	    int length;
	    while((length=iff.read(buffer, 1024)))
	       ostr.writall(buffer, length);
	    ostr.close_chunk();
	 }
	 iff.close_chunk();
      }

      if (top_level) ostr.close_chunk();
   } CATCH(exc) {
      delete str; str=0;
      RETHROW;
   } ENDCATCH;

   delete str; str=0;
}

TArray<char>
DjVuFile::get_djvu_data(bool included_too, bool no_ndir)
{
   DEBUG_MSG("DjVuFile::get_djvu_data(): creating DjVu raw file\n");
   DEBUG_MAKE_INDENT(3);
   
   MemoryByteStream str;
   IFFByteStream iff(str);
   GMap<GURL, void *> map;

   add_djvu_data(iff, map, included_too, no_ndir);

   return str.get_data();
}

void
DjVuFile::add_to_djvm(DjVmFile & djvm_file, GMap<GURL, void *> & map)
{
   if (map.contains(url)) return;

   map[url]=0;

   TArray<char> data=get_djvu_data(0, 0);
   djvm_file.add_file(url.fileURL(), data, -1);

   process_incl_chunks();
   
   GCriticalSectionLock lock(&inc_files_lock);
   for(GPosition pos=inc_files_list;pos;++pos)
      inc_files_list[pos]->add_to_djvm(djvm_file, map);
}

void
DjVuFile::add_to_djvm(DjVmFile & djvm_file)
{
   GMap<GURL, void *> map;
   add_to_djvm(djvm_file, map);
}

void
DjVuFile::move(GMap<GURL, void *> & map, const GURL & dir_url)
{
   if (!map.contains(url))
   {
      map[url]=0;

      url=dir_url+url.fileURL();

      process_incl_chunks();

      GCriticalSectionLock lock(&inc_files_lock);
      for(GPosition pos=inc_files_list;pos;++pos)
	 inc_files_list[pos]->move(map, dir_url);
   }
}

void
DjVuFile::move(const GURL & dir_url)
{
   GMap<GURL, void *> map;
   move(map, dir_url);
}

void
DjVuFile::change_cache(GMap<GURL, void *> & map,
		       GCache<GURL, DjVuFile> * xcache)
{
   if (map.contains(url)) return;

   map[url]=0;

   cache=xcache;
   if (xcache) xcache->add_item(url, this);

   process_incl_chunks();

   GCriticalSectionLock lock(&inc_files_lock);
   for(GPosition pos=inc_files_list;pos;++pos)
      inc_files_list[pos]->change_cache(map, xcache);
}

void
DjVuFile::change_cache(GCache<GURL, DjVuFile> * cache)
{
   GMap<GURL, void *> map;
   change_cache(map, cache);
}
