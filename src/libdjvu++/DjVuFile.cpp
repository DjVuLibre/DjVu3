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
//C- $Id: DjVuFile.cpp,v 1.61 1999-09-24 18:46:34 leonb Exp $

#ifdef __GNUC__
#pragma implementation
#endif

#include "DjVuFile.h"
#include "BSByteStream.h"
#include "debug.h"

class ProgressByteStream : public ByteStream
{
public:
   ProgressByteStream(const GP<ByteStream> & xstr) : str(xstr),
      last_call_pos(0) {}
   virtual ~ProgressByteStream() {}

   virtual size_t read(void *buffer, size_t size)
   {
      int rc=0;
	 // TRY {} CATCH; block here is merely to avoid egcs internal error
      TRY {
	 int cur_pos=str->tell();
	 if (progress_cb && (last_call_pos/256!=cur_pos/256))
	 {
	    progress_cb(cur_pos, progress_cl_data);
	    last_call_pos=cur_pos;
	 }
	 rc=str->read(buffer, size);
      } CATCH(exc) {
	 RETHROW;
      } ENDCATCH;
      return rc;
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
   GP<ByteStream> str;
   void		* progress_cl_data;
   void		(* progress_cb)(int pos, void *);
   int		last_call_pos;
   
      // Cancel C++ default stuff
   ProgressByteStream & operator=(const ProgressByteStream &);
};


DjVuFile::DjVuFile()
  : file_size(0), initialized(false)
{
}

void
DjVuFile::check() const
{
  if (!initialized)
    THROW("DjVuFile is not initialized");
}

void 
DjVuFile::init(ByteStream & str)
{
   DEBUG_MSG("DjVuFile::DjVuFile(): ByteStream constructor\n");
   DEBUG_MAKE_INDENT(3);

   if (initialized)
     THROW("DjVuFile is already initialized");
   if (!get_count())
     THROW("DjVuFile is not secured by a GP<DjVuFile>");

   file_size=0;
   decode_thread=0;

      // Read the data from the stream
   data_pool=new DataPool(str);

      // Construct some dummy URL
   char buffer[1024];
   sprintf(buffer, "djvufile:/%p.djvu", this);
   url=buffer;

      // Set it here because trigger will call other DjVuFile's functions
   initialized=true;
   
      // Add (basically - call) the trigger
   data_pool->add_trigger(-1, static_trigger_cb, this);
}

void
DjVuFile::init(const GURL & xurl, GP<DjVuPort> port) 
{
   DEBUG_MSG("DjVuFile::init(): url='" << xurl << "'\n");
   DEBUG_MAKE_INDENT(3);

   if (initialized)
     THROW("DjVuFile is already initialized");
   if (!get_count())
     THROW("DjVuFile is not secured by a GP<DjVuFile>");
   url = xurl;
   file_size=0;
   decode_thread=0;
   
   DjVuPortcaster * pcaster=get_portcaster();
   
      // We need it 'cause we're waiting for our own termination in stop_decode()
   pcaster->add_route(this, this);
   if (!port)
     port = simple_port = new DjVuSimplePort();
   pcaster->add_route(this, port);

      // Set it here because trigger will call other DjVuFile's functions
   initialized=true;
   
   if (!(data_pool=new DataPool(pcaster->request_data(this, url))))
      THROW("Failed get data for URL '"+url+"'");
   data_pool->add_trigger(-1, static_trigger_cb, this);
}

DjVuFile::~DjVuFile(void)
{
   DEBUG_MSG("DjVuFile::~DjVuFile(): destroying...\n");
   DEBUG_MAKE_INDENT(3);

      // No more messages. They may result in adding this file to a cache
      // which will be very-very bad as we're being destroyed
   get_portcaster()->del_port(this);

      // Unregister the trigger (we don't want it to be called and attempt
      // to access the destroyed object)
   if (data_pool) data_pool->del_trigger(static_trigger_cb, this);

      // We don't have to wait for decoding to finish here. It's already
      // finished (we know it because there is a "life saver" in the
      // thread function)
}

void
DjVuFile::reset(void)
{
   info = 0; 
   anno = 0; 
   bg44 = 0; 
   fgjb = 0; 
   fgjd = 0;
   fgpm = 0;
   txtz = 0;
   dir  = 0; 
   description = ""; 
   mimetype = "";
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
DjVuFile::get_included_files(bool only_created)
{
   check();
   if (!only_created && !are_incl_files_created())
      process_incl_chunks();

   GCriticalSectionLock lock(&inc_files_lock);
   GPList<DjVuFile> list=inc_files_list;	// Get a copy when locked
   return list;
}

void
DjVuFile::wait_for_chunk(void)
      // Will return after a chunk has been decoded
{
   check();
   DEBUG_MSG("DjVuFile::wait_for_chunk() called\n");
   DEBUG_MAKE_INDENT(3);
   chunk_mon.enter();
   chunk_mon.wait();
   chunk_mon.leave();
}

bool
DjVuFile::wait_for_finish(bool self)
      // if self==TRUE, will block until decoding of this file is over
      // if self==FALSE, will block until decoding of a child (direct
      // or indirect) is over.
      // Will return FALSE if there is nothing to wait for. TRUE otherwise
{
#ifdef DEBUG_MSG
   DEBUG_MSG("DjVuFile::wait_for_finish() called\n");
   if (self) DEBUG_MSG("Waiting for self termination, btw\n");
   DEBUG_MAKE_INDENT(3);
#endif
   check();
   
   if (self)
   {
	 // It's best to check for self termination using flags. The reason
	 // is that finish_mon is updated in a DjVuPort function, which
	 // will not be called if the object is being destroyed
      GMonitorLock lock(&flags);
      if (is_decoding())
      {
	 while(is_decoding()) flags.wait();
	 DEBUG_MSG("got it\n");
	 return 1;
      }
   } else
   {
	 // By locking the monitor, we guarantee that situation doesn't change
	 // between the moments when we check for pending finish events
	 // and when we actually run wait(). If we don't lock, the last child
	 // may terminate in between, and we'll wait forever.
	 //
	 // Locking is required by GMonitor interface too, btw.
      GMonitorLock lock(&finish_mon);
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
      }
   }
   DEBUG_MSG("nothing to wait for\n");
   return 0;
}

void
DjVuFile::notify_chunk_done(const DjVuPort *, const char *)
{
   check();
   chunk_mon.enter();
   chunk_mon.broadcast();
   chunk_mon.leave();
}

void
DjVuFile::notify_file_flags_changed(const DjVuFile * src,
				    long set_mask, long clr_mask)
{
   check();
   if (set_mask & (DECODE_OK | DECODE_FAILED | DECODE_STOPPED))
   {
	 // Signal threads waiting for file termination
      finish_mon.enter();
      finish_mon.broadcast();
      finish_mon.leave();
   
	 // In case a thread is still waiting for a chunk
      chunk_mon.enter();
      chunk_mon.broadcast();
      chunk_mon.leave();
   }
   
   if ((set_mask & ALL_DATA_PRESENT) && src!=this &&
       are_incl_files_created() && is_data_present())
   {
      if (src!=this && are_incl_files_created() && is_data_present())
      {
	    // Check if all children have data
	 bool all=true;
	 {
	    GCriticalSectionLock lock(&inc_files_lock);
	    for(GPosition pos=inc_files_list;pos;++pos)
	       if (!inc_files_list[pos]->is_all_data_present())
	       {
		  all=false;
		  break;
	       }
	 }
	 if (all)
	 {
	    DEBUG_MSG("Just got ALL data for '" << url << "'\n");
	    flags|=ALL_DATA_PRESENT;
	    get_portcaster()->notify_file_flags_changed(this, ALL_DATA_PRESENT, 0);
	 }
      }
   }
}

void
DjVuFile::static_decode_func(void * cl_data)
{
   DjVuFile * th=(DjVuFile *) cl_data;
   
      /* Please do not undo this life saver. If you do then try to resolve the
         following conflict first:
            1. Decoding starts and there is only one external reference
	       to the DjVuFile.
	    2. Decoding proceeds and calls DjVuPortcaster::notify_error(),
	       which creates inside a temporary GP<DjVuFile>.
	    3. While notify_error() is running, the only external reference
	       is lost, but the DjVuFile is still alive (remember the
	       temporary GP<>?)
	    4. The notify_error() returns, the temporary GP<> gets destroyed
	       and the DjVuFile is attempting to destroy right in the middle
	       of the decoding thread. This is either a dead block (waiting
	       for the termination of the decoding from the ~DjVuFile() called
	       from the decoding thread) or coredump. */
   GP<DjVuFile> life_saver=th;
   TRY {
      th->decode_func();
   } CATCH(exc) {
   } ENDCATCH;
}

void
DjVuFile::decode_func(void)
{
   check();
   DEBUG_MSG("DjVuFile::decode_func() called, url='" << url << "'\n");
   DEBUG_MAKE_INDENT(3);

   DjVuPortcaster * pcaster=get_portcaster();

   TRY {
      decode_data_pool=new DataPool(data_pool);
      GP<ByteStream> decode_stream=decode_data_pool->get_stream();
      GP<ProgressByteStream> pstr=new ProgressByteStream(decode_stream);
      pstr->set_progress_cb(progress_cb, this);
      
      decode(*pstr);

	 // Wait for all child files to finish
      while(wait_for_finish(0));

      DEBUG_MSG("waiting for children termination\n");
	 // Check for termination status
      GCriticalSectionLock lock(&inc_files_lock);
      for(GPosition pos=inc_files_list;pos;++pos)
      {
	 GP<DjVuFile> & f=inc_files_list[pos];
	 if (f->is_decode_failed()) THROW("Decode of an included file failed.");
	 if (f->is_decode_stopped()) THROW("STOP");
	 if (!f->is_decode_ok())
	 {
	    DEBUG_MSG("this_url='" << url << "'\n");
	    DEBUG_MSG("incl_url='" << f->get_url() << "'\n");
	    DEBUG_MSG("decoding=" << f->is_decoding() << "\n");
	    DEBUG_MSG("status='" << f->get_flags() << "\n");
	    THROW("Internal error: an included file has not finished yet.");
	 }
      }
   } CATCH(exc) {
      TRY {
	 if (!strcmp(exc.get_cause(), "STOP"))
	 {
	    flags.enter();
	    flags=flags & ~DECODING | DECODE_STOPPED;
	    flags.leave();
	    pcaster->notify_status(this, GString(url)+" STOPPED");
	    pcaster->notify_file_flags_changed(this, DECODE_STOPPED, DECODING);
	 } else
	 {
	    flags.enter();
	    flags=flags & ~DECODING | DECODE_FAILED;
	    flags.leave();
	    pcaster->notify_status(this, GString(url)+" FAILED");
	    pcaster->notify_error(this, exc.get_cause());
	    pcaster->notify_file_flags_changed(this, DECODE_FAILED, DECODING);
	 }
      } CATCH(exc) {
	 DEBUG_MSG("******* Oops. Almost missed an exception\n");
      } ENDCATCH;
   } ENDCATCH;

   TRY {
      if (flags.test_and_modify(DECODING, 0, DECODE_OK | INCL_FILES_CREATED, DECODING))
	 pcaster->notify_file_flags_changed(this, DECODE_OK | INCL_FILES_CREATED, DECODING);
   } CATCH(exc) {} ENDCATCH;
   DEBUG_MSG("decoding thread for url='" << url << "' ended\n");
}

GP<DjVuFile>
DjVuFile::process_incl_chunk(ByteStream & str, int file_num)
{
   check();
   DEBUG_MSG("DjVuFile::process_incl_chunk(): processing INCL chunk...\n");
   DEBUG_MAKE_INDENT(3);

   DjVuPortcaster * pcaster=get_portcaster();
   
   GString incl_str;
   char buffer[1024];
   int length;
   while((length=str.read(buffer, 1024)))
      incl_str+=GString(buffer, length);
   
      // Eat '\n' in the beginning and at the end
   while(incl_str.length() && incl_str[0]=='\n')
   {
      GString tmp=((const char *) incl_str)+1; incl_str=tmp;
   }
   while(incl_str.length()>0 && incl_str[incl_str.length()-1]=='\n')
      incl_str.setat(incl_str.length()-1, 0);

   if (incl_str.length()>0)
   {
      if (strchr(incl_str, '/'))
	 THROW("Malformed INCL chunk. No slashes allowed.");

      DEBUG_MSG("incl_str='" << incl_str << "'\n");

      GURL incl_url=pcaster->id_to_url(this, incl_str);
      if (incl_url.is_empty())	// Fallback. Should never be used.
	 incl_url=url.base()+incl_str;

	 // Now see if there is already a file with this *name* created
      {
	 GCriticalSectionLock lock(&inc_files_lock);
	 GPosition pos;
	 for(pos=inc_files_list;pos;++pos)
	    if (inc_files_list[pos]->url.name()==incl_url.name()) break;
	 if (pos) return inc_files_list[pos];
      }
      
	 // No. We have to request a new file
      GP<DjVuFile> file=(DjVuFile *) pcaster->id_to_file(this, incl_str).get();
      if (!file) THROW("Internal error: id_to_file() didn't create any file.");
      pcaster->add_route(file, this);
      
	 // We may have been stopped. Make sure the child will be stopped too.
      if (flags & STOPPED) file->stop(false);
      if (flags & BLOCKED_STOPPED) file->stop(true);

	 // Lock the list again and check if the file has already been
	 // added by someone else
      {
	 GCriticalSectionLock lock(&inc_files_lock);
	 GPosition pos;
	 for(pos=inc_files_list;pos;++pos)
	    if (inc_files_list[pos]->url.name()==incl_url.name()) break;
	 if (pos) file=inc_files_list[pos];
	 else if (file_num<0 || !(pos=inc_files_list.nth(file_num)))
	    inc_files_list.append(file);
	 else inc_files_list.insert_before(pos, file);
		
      }
      return file;
   }
   return 0;
}

void
DjVuFile::process_incl_chunks(void)
      // This function may block for data
      // NOTE: It may be called again when INCL_FILES_CREATED is set.
      // It happens in insert_file() when it has modified the data
      // and wants to create the actual file
{
   check();

   int incl_cnt=0;
   
   GP<ByteStream> str=data_pool->get_stream();
   int chksize;
   GString chkid;
   IFFByteStream iff(*str);
   if (iff.get_chunk(chkid))
   {
      while((chksize=iff.get_chunk(chkid)))
      {
	 if (chkid=="INCL") process_incl_chunk(iff, incl_cnt++);
	 iff.close_chunk();
      }
   }
   flags|=INCL_FILES_CREATED;
}

GP<JB2Dict>
DjVuFile::static_get_fgjd(void *arg)
{
   DjVuFile *file = (DjVuFile*)arg;
   return file->get_fgjd(1);
}

GP<JB2Dict>
DjVuFile::get_fgjd(int block)
{
   check();
   
      // Simplest case
   if (DjVuFile::fgjd)
      return DjVuFile::fgjd;
      // Check wether included files
   chunk_mon.enter();
   for(;;)
   {
      int active = 0;
      GPList<DjVuFile> incs = get_included_files();
      for (GPosition pos=incs.firstpos(); pos; ++pos)
      {
	 GP<DjVuFile> file = incs[pos];
	 if (file->is_decoding())
            active = 1;
	 GP<JB2Dict> fgjd = file->get_fgjd();
	 if (fgjd)
	 {
	    chunk_mon.leave();
	    return fgjd;
	 }
      }
	 // Exit if non-blocking mode
      if (! block) break;
	 // Exit if there is no decoding activity
      if (! active) break;
	 // Wait until a new chunk gets decoded
      wait_for_chunk();
   }
   chunk_mon.leave();
   return 0;
}

GString
DjVuFile::decode_chunk(const char *id, ByteStream &iff, bool djvi, bool djvu, bool iw44)
{
  check();
   
  GString chkid = id;
  GString desc = "Unrecognized chunk";
  DjVuPortcaster * pcaster=get_portcaster();
  DEBUG_MSG("DjVuFile::decode_chunk() : decoding " << id << "\n");
  
  // INFO  (information chunk for djvu page)
  
  if (chkid == "INFO" && (djvu || djvi))
    {
      if (DjVuFile::info)
        THROW("DjVu Decoder: Corrupted file (Duplicate INFO chunk)");
      if (djvi)
        THROW("DjVu Decoder: Corrupted file (Found INFO chunk in FORM:DJVI)");
      GP<DjVuInfo> info=new DjVuInfo();
      info->decode(iff);
      DjVuFile::info = info;
      desc.format("Page information");
    }

  // ANTa (annotation)

  else if (chkid == "ANTa")
    {
      if (DjVuFile::anno)
        {
          anno->merge(iff);
        }
      else 
        {
          GP<DjVuAnno> anno=new DjVuAnno();
          anno->decode(iff);
          DjVuFile::anno = anno;
        }
      desc.format("Page annotation");
    }

  // ANTz (compressed annotation)

  else if (chkid == "ANTz")
    {
      BSByteStream bsiff(iff);
      if (DjVuFile::anno)
        {
          anno->merge(bsiff);
        }
      else 
        {
          GP<DjVuAnno> anno=new DjVuAnno();
          anno->decode(bsiff);
          DjVuFile::anno = anno;
        }
      desc.format("Page annotation (compressed)");
    }
  
  // NDIR (obsolete navigation chunk)

  else if (chkid == "NDIR")
    {
      GP<DjVuNavDir> dir=new DjVuNavDir(url);
      dir->decode(iff);
      DjVuFile::dir=dir;
      desc.format("Navigation directory (obsolete)");
    }

  // TXTz (text information)

  else if (chkid == "TXTz")
    {
      if (DjVuFile::txtz)
        THROW("DjVu Decoder: Corrupted file (Duplicate TXTz chunk)");
      GP<DjVuText> txtz = new DjVuText;
      txtz->decode_text(iff);
      DjVuFile::txtz = txtz;
    }
  
  // TZNz (text zone hierarchy)
  
  else if (chkid == "TZNz")
    {
      if (! DjVuFile::txtz)
        THROW("DjVu Decoder: Corrupted file (Found TZNz without preceding TXTz chunk)");
      if (txtz->has_valid_zones())
        THROW("DjVu Decoder: Corrupted file (Duplicate TZNz chunk)");
      txtz->decode_zones(iff);
    }

  // INCL (inclusion chunk)

  else if (chkid == "INCL" && (djvi || djvu))
    {
      GP<DjVuFile> file=process_incl_chunk(iff);
      if (file)
        {
          int decode_was_already_started = 1;
          {
            GMonitorLock(&file->flags);
            if (!file->is_decoding() &&
                !file->is_decode_ok() &&
                !file->is_decode_failed() )
              {
                // Start decoding
                decode_was_already_started = 0;
                file->start_decode();
              }
          }
          // Send file notifications if previously started
          if (decode_was_already_started)
            {
              // May send duplicate notifications...
              if (file->is_decode_ok())
                get_portcaster()->notify_file_flags_changed(file, DECODE_OK, 0);
              else if (file->is_decode_failed())
                get_portcaster()->notify_file_flags_changed(file, DECODE_FAILED, 0);
            }
	  desc.format("Indirection chunk ("+file->get_url().name()+")");
        } else desc.format("Indirection chunk");
    }

  // Djbz (JB2 Dictionary)

  else if (chkid == "Djbz" && (djvu || djvi))
    {
      if (DjVuFile::fgjd)
        THROW("DjVu Decoder: Corrupted data (Duplicate Dxxx chunk)");
      if (DjVuFile::fgjd)
        THROW("DjVu Decoder: Corrupted data (Dxxx chunk found after Sxxx chunk)");
      GP<JB2Dict> fgjd = new JB2Dict();
      fgjd->decode(iff);
      DjVuFile::fgjd = fgjd;
      desc.format("JB2 shape dictionary (%d shapes)", fgjd->get_shape_count());
    } 

  // Sjbz (JB2 encoded mask)

  else if (chkid=="Sjbz" && (djvu || djvi))
    {
      if (DjVuFile::fgjb)
        THROW("DjVu Decoder: Corrupted data (Duplicate Sxxx chunk)");
      GP<JB2Image> fgjb=new JB2Image();
      fgjb->decode(iff, static_get_fgjd, (void*)this);
      DjVuFile::fgjb = fgjb;
      desc.format("JB2 foreground mask (%dx%d)", fgjb->get_width(), fgjb->get_height());
    }
 
  // Smmr (MMR-G4 encoded mask)

  else if (chkid=="Smmr" && (djvu || djvi))
    {
      if (DjVuFile::fgjb)
        THROW("DjVu Decoder: Corrupted data (Duplicate Sxxx chunk)");
      desc.format("MMR-G4 encoded mask (Unimplemented)");
    }
  
  // BG44 (background wavelets)
  
  else if (chkid == "BG44" && (djvu || djvi))
    {
      if (!bg44)
        {
          // First chunk
          GP<IWPixmap> bg44=new IWPixmap();
          bg44->decode_chunk(iff);
          DjVuFile::bg44=bg44;
          desc.format("IW44 background (%dx%d)", bg44->get_width(), bg44->get_height());
        } 
      else
        {
          // Refinement chunks
          bg44->decode_chunk(iff);
          desc.format("IW44 background (part %d)", bg44->get_serial());
        }
    }

  // FG44 (foreground wavelets)

  else if (chkid == "FG44" && (djvu || djvu))
    {
      if (fgpm)
        THROW("DjVu Decoder: Corrupted data (Duplicate foreground layer)");
      IWPixmap fg44;
      fg44.decode_chunk(iff);
      fgpm=fg44.get_pixmap();
      desc.format("IW44 foreground colors (%dx%d)", fg44.get_width(), fg44.get_height());
    } 

  // BGjp (background JPEG)

  else if (chkid == "BGjp" && (djvu || djvi))
    {
      if (bg44)
        THROW("DjVu Decoder: Corrupted data (Duplicate background layer)");
      desc.format("JPEG background (Unimplemented)");
    } 

  // FGjp (foreground JPEG)
  
  else if (chkid == "FGjp" && (djvu || djvi))
    {
      if (fgpm)
        THROW("DjVu Decoder: Corrupted data (Duplicate foreground layer)");
      desc.format("JPEG foreground colors (Unimplemented)");
    } 
  
  // BG2k (background JPEG-2000) Note: JPEG2K bitstream not finalized.

  else if (chkid == "BG2k" && (djvu || djvi))
    {
      if (bg44)
        THROW("DjVu Decoder: Corrupted data (Duplicate background layer)");
      desc.format("JPEG-2000 background (Unimplemented)");
    } 

  // FG2k (foreground JPEG-2000) Note: JPEG2K bitstream not finalized.
  
  else if (chkid == "FG2k" && (djvu || djvi))
    {
      if (fgpm)
        THROW("DjVu Decoder: Corrupted data (Duplicate foreground layer)");
      desc.format("JPEG-2000 foreground colors (Unimplemented)");
    } 

  // BM44/PM44 (IW44 data)

  else if ((chkid == "PM44" || chkid=="BM44") && iw44)
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
          desc.format("IW44 data (%dx%d)", bg44->get_width(), bg44->get_height());
        } 
      else
        {
          // Refinement chunks
          bg44->decode_chunk(iff);
          desc.format("IW44 data (part %d)", bg44->get_serial());
        }
    }

  // Return description
  return desc;
}

void
DjVuFile::decode(ByteStream & str)
{
   check();
   DEBUG_MSG("DjVuFile::decode(), url='" << url << "'\n");
   DEBUG_MAKE_INDENT(3);
   DjVuPortcaster * pcaster=get_portcaster();

   // Get form chunk
   int chksize;
   GString chkid;
   IFFByteStream iff(str);
   if (!iff.get_chunk(chkid)) 
     THROW("EOF");

   // Check file format
   bool djvi = (chkid=="FORM:DJVI");
   bool djvu = (chkid=="FORM:DJVU");
   bool iw44 = ((chkid=="FORM:PM44") || (chkid=="FORM:BM44"));
   if (djvi || djvu)
     mimetype = "image/djvu";
   else if (iw44)
     mimetype = "image/iw44";
   else
     THROW("DejaVu decoder: a DJVU or IW44 image was expected");
   
   // Process chunks
   while((chksize = iff.get_chunk(chkid)))
     {
       // Decode
       GString str = decode_chunk(chkid, iff, djvi, djvu, iw44);
       // Update description and notify
       GString desc;
       desc.format(" %0.1f Kb\t'%s'\t%s.\n", chksize/1024.0, 
                   (const char*)chkid, (const char*)str );
       description=description+desc;
       pcaster->notify_chunk_done(this, chkid);
       // Close chunk
       iff.close_chunk();
     }
   // Record file size
   file_size=iff.tell();
   // Close form chunk
   iff.close_chunk();
   // Close BG44 codec
   if (bg44) 
     bg44->close_codec();

   // Complete description
   if (djvu && !info)
     THROW("DjVu Decoder: Corrupted data (Missing INFO chunk)");
   if (iw44 && !info)
     THROW("DjVu Decoder: Corrupted data (Missing IW44 data chunks)");
   if (info)
     {
       GString desc;
       if (djvu || djvi)
         desc.format("DJVU Image (%dx%d) version %d:\n\n", 
                     info->width, info->height, info->version);
       else if (iw44)
         desc.format("IW44 Image (%dx%d) :\n\n", 
                     info->width, info->height);
       description=desc+description;
       int rawsize=info->width*info->height*3;
       desc.format("\nCompression ratio: %0.f (%0.1f Kb)\n",
                   (double)rawsize/file_size, file_size/1024.0 );
       description=description+desc;
     }
}

void
DjVuFile::start_decode(void)
{
   check();
   DEBUG_MSG("DjVuFile::start_decode(), url='" << url << "'\n");
   DEBUG_MAKE_INDENT(3);

   flags.enter();
   TRY {
      if (!(flags & DONT_START_DECODE) && !is_decoding())
      {
	 if (flags & DECODE_STOPPED) reset();
	 flags&=~(DECODE_OK | DECODE_STOPPED | DECODE_FAILED);
	 flags|=DECODING;
      
	 delete decode_thread; decode_thread=0;
	 decode_thread=new GThread();
	 decode_thread->create(static_decode_func, this);

	    // We want to wait until the other thread actually starts.
	    // One of the reasons is that if somebody tries to terminate the
	    // decoding before its thread actually starts, it will NOT be
	    // terminated. The other is that we want it to initialize the
	    // local life_saver
	 while(!decode_data_pool) GThread::yield();
      }
   } CATCH(exc) {
      flags&=~DECODING;
      flags|=DECODE_FAILED;
      flags.leave();
      get_portcaster()->notify_file_flags_changed(this, DECODE_FAILED, DECODING);
      RETHROW;
   } ENDCATCH;
   flags.leave();
}

void
DjVuFile::stop_decode(bool sync)
{
   check();
   
   DEBUG_MSG("DjVuFile::stop_decode(), url='" << url <<
	     "', sync=" << (int) sync << "\n");
   DEBUG_MAKE_INDENT(3);

   TRY {
      flags|=DONT_START_DECODE;

	 // Don't stop SYNCHRONOUSLY from the thread where the decoding is going!!!
      {
	    // First - ask every included child to stop in async mode
	 GCriticalSectionLock lock(&inc_files_lock);
	 for(GPosition pos=inc_files_list;pos;++pos)
	    inc_files_list[pos]->stop_decode(0);

	 if (decode_data_pool) decode_data_pool->stop();
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
	 }

	 wait_for_finish(1);	// Wait for self termination

	 delete decode_thread; decode_thread=0;
      }
      flags&=~DONT_START_DECODE;
   } CATCH(exc) {
      flags&=~DONT_START_DECODE;
      RETHROW;
   } ENDCATCH;
}

void
DjVuFile::stop(bool only_blocked)
      // This is a one-way function. There is no way to undo the stop()
      // command.
{
   DEBUG_MSG("DjVuFile::stop(): Stopping everything\n");
   DEBUG_MAKE_INDENT(3);

   flags|=only_blocked ? BLOCKED_STOPPED : STOPPED;
   if (data_pool) data_pool->stop(only_blocked);
   GCriticalSectionLock lock(&inc_files_lock);
   for(GPosition pos=inc_files_list;pos;++pos)
      inc_files_list[pos]->stop(only_blocked);
}

GP<DjVuNavDir>
DjVuFile::find_ndir(GMap<GURL, void *> & map)
{
   check();
   
   DEBUG_MSG("DjVuFile::find_ndir(): looking for NDIR in '" << url << "'\n");
   DEBUG_MAKE_INDENT(3);
   
   if (dir) return dir;

   if (!map.contains(url))
   {
      map[url]=0;

      GPList<DjVuFile> list=get_included_files(false);
      for(GPosition pos=list;pos;++pos)
      {
	 GP<DjVuNavDir> d=list[pos]->find_ndir(map);
	 if (d) return d;
      }
   }
   return 0;
}

GP<DjVuNavDir>
DjVuFile::find_ndir(void)
{
   GMap<GURL, void *> map;
   return find_ndir(map);
}

GP<DjVuNavDir>
DjVuFile::decode_ndir(GMap<GURL, void *> & map)
{
   check();
   
   DEBUG_MSG("DjVuFile::decode_ndir(): decoding for NDIR in '" << url << "'\n");
   DEBUG_MAKE_INDENT(3);
   
   if (dir) return dir;
   
   if (!map.contains(url))
   {
      map[url]=0;
      
      GP<ByteStream> str=data_pool->get_stream();
      
      int chksize;
      GString chkid;
      IFFByteStream iff(*str);
      if (!iff.get_chunk(chkid)) 
	 THROW("EOF");

      while((chksize=iff.get_chunk(chkid)))
      {
	 if (chkid=="NDIR")
	 {
	    GP<DjVuNavDir> d=new DjVuNavDir(url);
	    d->decode(iff);
	    dir=d;
	    break;
	 }
	 iff.close_chunk();
      }

      if (dir) return dir;

      GPList<DjVuFile> list=get_included_files(false);
      for(GPosition pos=list;pos;++pos)
      {
	 GP<DjVuNavDir> d=list[pos]->decode_ndir(map);
	 if (d) return d;
      }
   }
   return 0;
}

GP<DjVuNavDir>
DjVuFile::decode_ndir(void)
{
   GMap<GURL, void *> map;
   return decode_ndir(map);
}

void
DjVuFile::static_trigger_cb(void * cl_data)
{
   DjVuFile * th=(DjVuFile *) cl_data;
   TRY {
      th->trigger_cb();
   } CATCH(exc) {
      TRY {
	 get_portcaster()->notify_error(th, exc.get_cause());
      } CATCH(exc) {} ENDCATCH;
   } ENDCATCH;
}

void
DjVuFile::trigger_cb(void)
{
   GP<DjVuFile> life_saver=this;
   
   DEBUG_MSG("DjVuFile::trigger_cb(): got data for '" << url << "'\n");
   DEBUG_MAKE_INDENT(3);

   file_size=data_pool->get_length();
   flags|=DATA_PRESENT;
   get_portcaster()->notify_file_flags_changed(this, DATA_PRESENT, 0);

   if (!are_incl_files_created()) process_incl_chunks();

   bool all=true;
   {
      GCriticalSectionLock flock(&inc_files_lock);
      for(GPosition pos=inc_files_list;pos;++pos)
	 if (!inc_files_list[pos]->is_all_data_present())
	 {
	    all=false; break;
	 }
   }
   if (all)
   {
      DEBUG_MSG("It appears, that we have ALL data for '" << url << "'\n");
      flags|=ALL_DATA_PRESENT;
      get_portcaster()->notify_file_flags_changed(this, ALL_DATA_PRESENT, 0);
   }
}

void
DjVuFile::progress_cb(int pos, void * cl_data)
{
   DEBUG_MSG("DjVuFile::progress_cb() called\n");
   DEBUG_MAKE_INDENT(3);

   DjVuFile * th=(DjVuFile *) cl_data;

   int length=th->decode_data_pool->get_length();
   if (length>0)
   {
      float progress=(float) pos/length;
      DEBUG_MSG("progress=" << progress << "\n");
      get_portcaster()->notify_decode_progress(th, progress);
   } else
   {
      DEBUG_MSG("DataPool size is still unknown => ignoring\n");
   }
}

//*****************************************************************************
//******************************** Utilities **********************************
//*****************************************************************************

void
DjVuFile::move(GMap<GURL, void *> & map, const GURL & dir_url)
      // This function may block for data.
{
   if (!map.contains(url))
   {
      map[url]=0;

      url=dir_url+url.name();

      if (!are_incl_files_created()) process_incl_chunks();

      GPList<DjVuFile> list=get_included_files(false);
      for(GPosition pos=list;pos;++pos)
	 list[pos]->move(map, dir_url);
   }
}

void
DjVuFile::move(const GURL & dir_url)
      // This function may block for data.
{
   check();
   DEBUG_MSG("DjVuFile::move(): dir_url='" << dir_url << "'\n");
   DEBUG_MAKE_INDENT(3);
   
   GMap<GURL, void *> map;
   move(map, dir_url);
}

void
DjVuFile::set_name(const char * name)
{
   DEBUG_MSG("DjVuFile::set_name(): name='" << name << "'\n");
   DEBUG_MAKE_INDENT(3);
   url=url.base()+name;
}

//*****************************************************************************
//****************************** Data routines ********************************
//*****************************************************************************

int
DjVuFile::get_chunks_number(void)
{
   check();
   int chunks=0;
   GP<ByteStream> str=data_pool->get_stream();
   int chksize;
   GString chkid;
   IFFByteStream iff(*str);
   if (!iff.get_chunk(chkid)) 
      THROW("EOF");
   while((chksize=iff.get_chunk(chkid)))
   {
      chunks++;
      iff.close_chunk();
   }
   return chunks;
}

GString
DjVuFile::get_chunk_name(int chunk_num)
{
   check();
   GString name;
   GP<ByteStream> str=data_pool->get_stream();
   int chksize;
   GString chkid;
   IFFByteStream iff(*str);
   if (!iff.get_chunk(chkid)) 
      THROW("EOF");
   int chunk=0;
   while((chksize=iff.get_chunk(chkid)))
   {
      if (chunk++==chunk_num) { name=chkid; break; }
      iff.close_chunk();
   }
   if (!name.length()) THROW("Too few chunks.");
   return name;
}

bool
DjVuFile::contains_chunk(const char * chunk_name)
{
   check();
   DEBUG_MSG("DjVuFile::contains_chunk(): url='" << url << "', chunk_name='" <<
	     chunk_name << "'\n");
   DEBUG_MAKE_INDENT(3);
   
   bool contains=0;
   GP<ByteStream> str=data_pool->get_stream();
   int chksize;
   GString chkid;
   IFFByteStream iff(*str);
   if (!iff.get_chunk(chkid)) 
      THROW("EOF");
   while((chksize=iff.get_chunk(chkid)))
   {
      if (chkid==chunk_name) { contains=1; break; }
      iff.close_chunk();
   }
   return contains;
}

//*****************************************************************************
//****************************** Save routines ********************************
//*****************************************************************************

void
DjVuFile::add_djvu_data(IFFByteStream & ostr, GMap<GURL, void *> & map,
			bool included_too, bool no_ndir)
{
   check();
   if (map.contains(url)) return;

   bool top_level=map.size()==0;
   
   map[url]=0;

   bool have_anta=contains_chunk("ANTa");
   
   GP<ByteStream> str=data_pool->get_stream();
   int chksize;
   GString chkid;
   IFFByteStream iff(*str);
   if (!iff.get_chunk(chkid)) 
      THROW("EOF");

   if (top_level) ostr.put_chunk(chkid);

   int chunk_num=0;
   while((chksize=iff.get_chunk(chkid)))
   {
      if (chkid=="INCL" && included_too)
      {
	 GP<DjVuFile> file=process_incl_chunk(iff);
	 if (file) file->add_djvu_data(ostr, map, included_too, no_ndir);
      } else if (chkid=="ANTa" && anno && !anno->is_empty())
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
	 ostr.copy(iff);
	 ostr.close_chunk();
      }
      iff.close_chunk();

      if (!have_anta && chunk_num==0 &&
	  anno && !anno->is_empty())
      {
	 ostr.put_chunk("ANTa");
	 anno->encode(ostr);
	 ostr.close_chunk();
      }
      chunk_num++;
   }

   if (top_level) ostr.close_chunk();
}

GP<DataPool>
DjVuFile::get_djvu_data(bool included_too, bool no_ndir)
{
   check();
   
   DEBUG_MSG("DjVuFile::get_djvu_data(): creating DjVu raw file\n");
   DEBUG_MAKE_INDENT(3);
   
   MemoryByteStream str;
   IFFByteStream iff(str);
   GMap<GURL, void *> map;

   add_djvu_data(iff, map, included_too, no_ndir);

   iff.flush();
   str.seek(0, SEEK_SET);
   return new DataPool(str);
}

//****************************************************************************
//******************************* Modifying **********************************
//****************************************************************************

void
DjVuFile::insert_file(const char * id, int chunk_num)
{
   DEBUG_MSG("DjVuFile::insert_file(): id='" << id << "', chunk_num="
	     << chunk_num << "\n");
   DEBUG_MAKE_INDENT(3);

      // First: create new data
   GP<ByteStream> str_in=data_pool->get_stream();
   IFFByteStream iff_in(*str_in);

   MemoryByteStream str_out;
   IFFByteStream iff_out(str_out);

   int chunk_cnt=0;
   bool done=false;
   int chksize;
   GString chkid;
   if (iff_in.get_chunk(chkid))
   {
      iff_out.put_chunk(chkid);
      while((chksize=iff_in.get_chunk(chkid)))
      {
	 if (chunk_cnt++==chunk_num)
	 {
	    iff_out.put_chunk("INCL");
	    iff_out.writall(id, strlen(id));
	    iff_out.close_chunk();
	    done=true;
	 }
	 iff_out.put_chunk(chkid);
	 iff_out.copy(iff_in);
	 iff_out.close_chunk();
	 iff_in.close_chunk();
      }
      if (!done)
      {
	 iff_out.put_chunk("INCL");
	 iff_out.writall(id, strlen(id));
	 iff_out.close_chunk();
      }
      iff_out.close_chunk();
   }
   str_out.seek(0, SEEK_SET);
   data_pool=new DataPool(str_out);

      // Second: create missing DjVuFiles
   process_incl_chunks();

   flags|=MODIFIED;
}

void
DjVuFile::unlink_file(const char * id)
{
   DEBUG_MSG("DjVuFile::insert_file(): id='" << id << "'\n");
   DEBUG_MAKE_INDENT(3);

      // Remove the file from the list of included files
   {
      GCriticalSectionLock lock(&inc_files_lock);
      for(GPosition pos=inc_files_list;pos;)
	 if (inc_files_list[pos]->get_url()==url)
	 {
	    GPosition this_pos=pos;
	    ++pos;
	    inc_files_list.del(this_pos);
	 } else ++pos;
   }

      // And update the data.
   GP<ByteStream> str_in=data_pool->get_stream();
   IFFByteStream iff_in(*str_in);

   MemoryByteStream str_out;
   IFFByteStream iff_out(str_out);

   int chksize;
   GString chkid;
   if (iff_in.get_chunk(chkid))
   {
      iff_out.put_chunk(chkid);
      while((chksize=iff_in.get_chunk(chkid)))
      {
	 if (chkid!="INCL")
	 {
	    iff_out.put_chunk(chkid);
	    iff_out.copy(iff_in);
	    iff_out.close_chunk();
	 } else
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
	    if (incl_str!=id)
	    {
	       iff_out.put_chunk("INCL");
	       iff_out.writall((const char *) incl_str, incl_str.length());
	       iff_out.close_chunk();
	    }
	 }
	 iff_in.close_chunk();
      }
      iff_out.close_chunk();
   }

   str_out.seek(0, SEEK_SET);
   data_pool=new DataPool(str_out);

   flags|=MODIFIED;
}
