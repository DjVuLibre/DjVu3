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
//C- $Id: DjVuFile.cpp,v 1.95 1999-12-02 22:37:31 eaf Exp $

#ifdef __GNUC__
#pragma implementation
#endif

#include "debug.h"
#include "DjVuFile.h"
#include "BSByteStream.h"
#include "MMRDecoder.h"
#ifdef NEED_JPEG_DECODER
#include "JPEGDecoder.h"
#endif

#define STRINGIFY(x) STRINGIFY_(x)
#define STRINGIFY_(x) #x


#define REPORT_EOF(x) \
  {TRY{THROW("EOF");}CATCH(ex){report_error(ex,(x));}ENDCATCH;}

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
   virtual int seek(long offset, int whence = SEEK_SET, bool nothrow=false)
   {
     return str->seek(offset, whence);
   }
   virtual long tell() { return str->tell(); }

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
  : file_size(0), recover_errors(ABORT), verbose_eof(false), chunks_number(-1),
    initialized(false)
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
   dir  = 0; 
   description = ""; 
   mimetype = "";
}

unsigned int
DjVuFile::get_memory_usage(void) const
{
   unsigned int size=sizeof(*this);
   if (info) size+=info->get_memory_usage();
   if (bg44) size+=bg44->get_memory_usage();
   if (fgjb) size+=fgjb->get_memory_usage();
   if (fgpm) size+=fgpm->get_memory_usage();
   if (anno) size+=anno->size();
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
   th->decode_life_saver=0;
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
   while(incl_str.length()>0 && incl_str[(int)incl_str.length()-1]=='\n')
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
      if(recover_errors != ABORT) file->set_recover_errors(recover_errors);
      if(verbose_eof) file->set_verbose_eof(verbose_eof);
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
DjVuFile::report_error
(const GException &ex,bool throw_errors)
{
  const char eof_msg[]="Unexpected End Of File Encountered Reading:\n\t%0.1024s";
  if((!verbose_eof)||strcmp(ex.get_cause(),"EOF"))
  {
    if(throw_errors)
    {
      EXTHROW(ex);
    }else
    {
      get_portcaster()->notify_error(this,ex.get_cause());
    }
  }else
  {
    char buffer[sizeof(eof_msg)+1020];
    const char *url=get_url();
    strcpy(buffer,eof_msg); 
    sprintf(buffer,eof_msg,url);
    if(throw_errors)
    {
      EXTHROW(ex,buffer);
    }else
    {
      get_portcaster()->notify_error(this,buffer);
    }
  }
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
   GString chkid;
   IFFByteStream iff(*str);
   if (iff.get_chunk(chkid))
   {
      int chunks=0;
      int last_chunk=0;
      TRY
      {
         int chunks_left=(recover_errors>SKIP_PAGES)?chunks_number:(-1);
         int chksize;
         for(;(chunks_left--)&&(chksize=iff.get_chunk(chkid));last_chunk=chunks)
         {
            chunks++;
	    if (chkid=="INCL") process_incl_chunk(iff, incl_cnt++);
	    iff.seek_close_chunk();
         }
         if (chunks_number < 0) chunks_number=last_chunk;
      }
      CATCH(ex)
      {	
         if (chunks_number < 0)
           chunks_number=(recover_errors>SKIP_CHUNKS)?chunks:last_chunk;
         report_error(ex,(recover_errors <= SKIP_PAGES));
      }
      ENDCATCH;
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

int
DjVuFile::get_dpi(int w, int h)
{
   int dpi=0, red=1;
   if (info)
   {
      for(red=1; red<=12; red++)
	 if ((info->width+red-1)/red==w)
	    if ((info->height+red-1)/red==h)
	       break;
      if (red>12)
	 THROW("DjVu Decoder: Corrupted data (Incorrect size in BG44 chunk)\n");
      dpi=info->dpi;
   }
   return (dpi ? dpi : 300)/red;
}


static inline bool
is_annotation(GString chkid)
{
  if (chkid=="ANTa" ||
      chkid=="ANTz" ||
      chkid=="TXTa" ||
      chkid=="TXTz" ||
      chkid=="FORM:ANNO" ) 
    return true;
  return false;
}


GString
DjVuFile::decode_chunk(const char *id, ByteStream &iff, bool djvi, bool djvu, bool iw44)
{
  check();

     // If this object is referenced by only one GP<> pointer, this
     // pointer should be the "life_saver" created by the decoding thread.
     // If it is the only GP<> pointer, then nobody is interested in the
     // results of the decoding and we can abort now with "STOP"
  if (get_count()==1) THROW("STOP");
  
  GString desc = "Unrecognized chunk";
  GString chkid = id;
  DEBUG_MSG("DjVuFile::decode_chunk() : decoding " << id << "\n");
  
  // INFO  (information chunk for djvu page)
  if (chkid == "INFO" && (djvu || djvi))
    {
      if (DjVuFile::info)
        THROW("DjVu Decoder: Corrupted file (Duplicate INFO chunk)");
      if (djvi)
        THROW("DjVu Decoder: Corrupted file (Found INFO chunk in FORM:DJVI)");
      // DjVuInfo::decode no longer throws version exceptions
      GP<DjVuInfo> info=new DjVuInfo();
      info->decode(iff);
      DjVuFile::info = info;
      desc.format("Page information");
      // Consistency checks (previously in DjVuInfo::decode)
      if (info->width<0 || info->height<0)
        THROW("DjVu Decoder: Corrupted file (image size is zero)");
      if (info->version >= DJVUVERSION_TOO_NEW)
        THROW("DjVu Decoder: Cannot decode DjVu files with version >= "
              STRINGIFY(DJVUVERSION_TOO_NEW) );
    }

  // INCL (inclusion chunk)
  else if (chkid == "INCL" && (djvi || djvu))
    {
      GP<DjVuFile> file=process_incl_chunk(iff);
      if (file)
        {
          int decode_was_already_started = 1;
          {
            GMonitorLock lock(&file->flags);
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
      // ---- begin hack
      if (info && info->version <=18)
        fgjb->reproduce_old_bug = true;
      // ---- end hack
      fgjb->decode(iff, static_get_fgjd, (void*)this);
      DjVuFile::fgjb = fgjb;
      desc.format("JB2 foreground mask (%dx%d)", fgjb->get_width(), fgjb->get_height());
    }
 
  // Smmr (MMR-G4 encoded mask)
  else if (chkid=="Smmr" && (djvu || djvi))
    {
      if (DjVuFile::fgjb)
        THROW("DjVu Decoder: Corrupted data (Duplicate Sxxx chunk)");
      DjVuFile::fgjb = MMRDecoder::decode(iff);
      desc.format("G4/MMR encoded mask (%dx%d)", fgjb->get_width(), fgjb->get_height());
    }
  
  // BG44 (background wavelets)
  else if (chkid == "BG44" && (djvu || djvi))
    {
      if (!bg44)
        {
          if (bgpm)
            THROW("DjVu Decoder: Corrupted data (Duplicate background layer)");
          // First chunk
          GP<IWPixmap> bg44=new IWPixmap();
          bg44->decode_chunk(iff);
          DjVuFile::bg44=bg44;
          desc.format("IW44 background (%dx%d, %d dpi)",
		      bg44->get_width(), bg44->get_height(),
		      get_dpi(bg44->get_width(), bg44->get_height()));
        } 
      else
        {
          // Refinement chunks
          bg44->decode_chunk(iff);
          desc.format("IW44 background (part %d, %d dpi)",
		      bg44->get_serial(), get_dpi(bg44->get_width(), bg44->get_height()));
        }
    }

  // FG44 (foreground wavelets)
  else if (chkid == "FG44" && (djvu || djvu))
    {
      if (fgpm || fgbc)
        THROW("DjVu Decoder: Corrupted data (Duplicate foreground color info)");
      IWPixmap fg44;
      fg44.decode_chunk(iff);
      fgpm=fg44.get_pixmap();
      desc.format("IW44 foreground colors (%dx%d, %d dpi)",
		  fg44.get_width(), fg44.get_height(),
		  get_dpi(fg44.get_width(), fg44.get_height()));
    } 

  // BGjp (background JPEG)
  else if (chkid == "BGjp" && (djvu || djvi))
    {
      if (bg44 || bgpm)
        THROW("DjVu Decoder: Corrupted data (Duplicate background layer)");
#ifdef NEED_JPEG_DECODER
      DjVuFile::bgpm = JPEGDecoder::decode(iff);
      desc.format("JPEG background (%dx%d, %d dpi)",
		  bgpm->columns(), bgpm->rows(),
		  get_dpi(bgpm->columns(), bgpm->rows()));
#else
      desc.format("JPEG background (Unimplemented)");
#endif
    } 
  
  // FGjp (foreground JPEG)
  else if (chkid == "FGjp" && (djvu || djvi))
    {
      if (fgpm || fgbc)
        THROW("DjVu Decoder: Corrupted data (Duplicate foreground color info)");
#ifdef NEED_JPEG_DECODER
      DjVuFile::fgpm = JPEGDecoder::decode(iff);
      desc.format("JPEG foreground colors (%dx%d, %d dpi)",
		  fgpm->columns(), fgpm->rows(),
		  get_dpi(fgpm->columns(), fgpm->rows()));
#else
      desc.format("JPEG foreground colors (Unimplemented)");
#endif
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
      if (fgpm || fgbc)
        THROW("DjVu Decoder: Corrupted data (Duplicate foreground color info)");
      desc.format("JPEG-2000 foreground colors (Unimplemented)");
    } 

  // FGbz (foreground color vector)
  else if (chkid == "FGbz" && (djvu || djvi))
    {
      if (fgpm || fgbc)
        THROW("DjVu Decoder: Corrupted data (Duplicate foreground color info)");
      GP<DjVuPalette> fgbc = new DjVuPalette;
      fgbc->decode(iff);
      DjVuFile::fgbc = fgbc;
      desc.format("JB2 foreground colors (%d colors, %d ccs)", 
                  fgbc->size(), fgbc->colordata.size());
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
          desc.format("IW44 data (%dx%d, %d dpi)",
		      bg44->get_width(), bg44->get_height(),
		      get_dpi(bg44->get_width(), bg44->get_height()));
        } 
      else
        {
          // Refinement chunks
          bg44->decode_chunk(iff);
          desc.format("IW44 data (part %d, %d dpi)",
		      bg44->get_serial(),
		      get_dpi(bg44->get_width(), bg44->get_height()));
        }
    }

  // NDIR (obsolete navigation chunk)
  else if (chkid == "NDIR")
    {
      GP<DjVuNavDir> dir=new DjVuNavDir(url);
      dir->decode(iff);
      DjVuFile::dir=dir;
      desc.format("Navigation directory (obsolete)");
    }
  
  // FORM:ANNO (obsolete) (must be before other annotations)
  else if (chkid == "FORM:ANNO") 
    {
      GCriticalSectionLock lock(&anno_lock);
      if (! anno) anno = new MemoryByteStream;
      anno->seek(0,SEEK_END);
      if (anno->tell() & 1)  anno->write((void*)"", 1);
      // Copy data
      anno->copy(iff);
      desc.format("Annotations (bundled)");
    }
  
  // ANTa/ANTx/TXTa/TXTz annotations
  else if (is_annotation(chkid))  // but not FORM:ANNO
    {
      GCriticalSectionLock lock(&anno_lock);
      if (! anno) anno = new MemoryByteStream;
      anno->seek(0,SEEK_END);
      if (anno->tell() & 1) anno->write((const void*)"", 1);
      // Recreate chunk header
      IFFByteStream iffout(*anno);
      iffout.put_chunk(id);
      iffout.copy(iff);
      iffout.close_chunk();
      desc.format("Annotations");
      if (chkid == "ANTa" || chkid == "ANTz")
        desc = desc + " (hyperlinks, etc.)";
      else if (chkid == "TXTa" || chkid == "TXTz")
        desc = desc + " (text, etc.)";
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
   GString chkid;
   IFFByteStream iff(str);
   if (!iff.get_chunk(chkid)) 
     REPORT_EOF(true)

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
   int size_so_far=iff.tell();
   int chunks=0;
   int last_chunk=0;
   TRY
   {
      int chunks_left=(recover_errors>SKIP_PAGES)?chunks_number:(-1);
      int chksize;
      for(;(chunks_left--)&&(chksize = iff.get_chunk(chkid));last_chunk=chunks)
        {
          chunks++;
          // Decode
          GString str = decode_chunk(chkid, iff, djvi, djvu, iw44);
          // Update description and notify
          GString desc;
          desc.format(" %0.1f Kb\t'%s'\t%s.\n", chksize/1024.0, 
                   (const char*)chkid, (const char*)str );
          description=description+desc;
          pcaster->notify_chunk_done(this, chkid);
          // Close chunk
          iff.seek_close_chunk();
          // Record file size
          size_so_far=iff.tell();
        }
      if (chunks_number < 0) chunks_number=last_chunk;
   }
   CATCH(ex)
   {
     if(!strcmp(ex.get_cause(),"EOF"))
     {
       if (chunks_number < 0)
         chunks_number=(recover_errors>SKIP_CHUNKS)?chunks:last_chunk;
       report_error(ex,(recover_errors <= SKIP_PAGES));
     }else
     {
       report_error(ex,true);
     }
   }
   ENDCATCH;

   // Record file size
   file_size=size_so_far;
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

	    // We want to create it right here to be able to stop the
	    // decoding thread even before its function is called (it starts)
	 decode_data_pool=new DataPool(data_pool);
	 decode_life_saver=this;
	 
	 delete decode_thread; decode_thread=0;
	 decode_thread=new GThread();
	 decode_thread->create(static_decode_func, this);
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
      
      GString chkid;
      IFFByteStream iff(*str);
      if (!iff.get_chunk(chkid)) 
        REPORT_EOF(true)

      int chunks=0;
      int last_chunk=0;
      TRY
      {
         int chunks_left=(recover_errors>SKIP_PAGES)?chunks_number:(-1);
         int chksize;
         for(;(chunks_left--)&&(chksize=iff.get_chunk(chkid));last_chunk=chunks)
         {
	    chunks++;
	    if (chkid=="NDIR")
	    {
	       GP<DjVuNavDir> d=new DjVuNavDir(url);
	       d->decode(iff);
	       dir=d;
	       break;
	    }
	    iff.seek_close_chunk();
         }
         if ((!dir)&&(chunks_number < 0)) chunks_number=last_chunk;
      }
      CATCH(ex)
      {
         if(!strcmp(ex.get_cause(),"EOF"))
         {
           if (chunks_number < 0)
             chunks_number=(recover_errors>SKIP_CHUNKS)?chunks:last_chunk;
           report_error(ex,(recover_errors<=SKIP_PAGES));
         }else
         {
           report_error(ex,true);
         }
      }
      ENDCATCH;

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
DjVuFile::get_merged_anno(const GP<DjVuFile> & file,
			  ByteStream & str_out,
			  GMap<GURL, void *> & map)
{
   GURL url=file->get_url();
   if (!map.contains(url))
   {
      map[url]=0;

      if (!file->is_data_present() || file->is_modified())
      {
	    // Return smth using the 'anno' and partially decoded
	    // list of included files. If the file has not been modified
	    // and we're here due to the lack of data, the result will be
	    // only an approximation due to two things:
	    //    1. 'anno' may not contain all data
	    //    2. annotations are merged regardless of where a child
	    //       DjVuFile is included.
	 if (file->anno && file->anno->size())
	 {
	    GCriticalSectionLock lock(&file->anno_lock);
	    if (str_out.tell() & 1) str_out.write((void *) "", 1);
	    file->anno->seek(0);
	    str_out.copy(*file->anno);
	 }
	 GPList<DjVuFile> list=file->get_included_files();
	 for(GPosition pos=list;pos;++pos)
	    get_merged_anno(list[pos], str_out, map);
      } else
      {
	    // Process the DjVuFile's data by decoding the annotations
	    // and included annotations from lower-level files where necessary
	    // Note, that using 'anno' and included files list is not
	    // a good idea because we want to insert annotations of the
	    // included files into correct places
	    // The file is not modified so we don't care about decoded 'anno'
	 GP<ByteStream> str=file->data_pool->get_stream();
	 IFFByteStream iff(*str);
	 GString chkid;
	 if (iff.get_chunk(chkid))
	    while(iff.get_chunk(chkid))
	    {
	       if (chkid=="INCL")
	       {
		  GP<DjVuFile> inc_file=file->process_incl_chunk(iff);
		  if (inc_file) get_merged_anno(inc_file, str_out, map);
	       } 
               else if (chkid=="FORM:ANNO")
	       {
		  if (str_out.tell() & 1) str_out.write((void *) "", 1);
		  str_out.copy(iff);
	       } 
               else if (is_annotation(chkid)) // but not FORM:ANNO
	       {
		  if (str_out.tell() & 1) str_out.write((void *) "", 1);
		  IFFByteStream iff_out(str_out);
		  iff_out.put_chunk(chkid);
		  iff_out.copy(iff);
		  iff_out.close_chunk();
	       }
	       iff.close_chunk();
	    }
      }
   }
}
   
GP<MemoryByteStream>
DjVuFile::get_merged_anno(void)
      // Will go down the DjVuFile's hierarchy and decode all DjVuAnno even
      // when the DjVuFile is not fully decoded yet. To avoid correlations
      // with DjVuFile::decode(), we do not modify DjVuFile::anno data.
      // NOTE! This function guarantees correct results only if the
      // DjVuFile as all data
{
   GP<MemoryByteStream> str=new MemoryByteStream;
   GMap<GURL, void *> map;
   get_merged_anno(this, *str, map);
   if (str->tell()==0) str=0;
   else str->seek(0);
   return str;
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
   if(chunks_number < 0)
   {
     GP<ByteStream> str=data_pool->get_stream();
     GString chkid;
     IFFByteStream iff(*str);
     if (!iff.get_chunk(chkid))
       REPORT_EOF(true)

     int chunks=0;
     int last_chunk=0;
     TRY
     {
       int chksize;
       for(;(chksize=iff.get_chunk(chkid));last_chunk=chunks)
       {
          chunks++;
          iff.seek_close_chunk();
       }
       chunks_number=last_chunk;
     }
     CATCH(ex)
     {
       chunks_number=(recover_errors>SKIP_CHUNKS)?chunks:last_chunk;
       report_error(ex,(recover_errors<=SKIP_PAGES));
     }
     ENDCATCH;
   }
   return chunks_number;
}

GString
DjVuFile::get_chunk_name(int chunk_num)
{
   if(chunk_num < 0)
   {
     THROW("Illegal chunk number");
   }
   if((chunks_number >= 0)&&(chunk_num > chunks_number))
   {
     THROW("Too few chunks");
   }
   check();

   GString name;
   GP<ByteStream> str=data_pool->get_stream();
   GString chkid;
   IFFByteStream iff(*str);
   if (!iff.get_chunk(chkid)) 
     REPORT_EOF(true)

   int chunks=0;
   int last_chunk=0;
   TRY
   {
     int chunks_left=(recover_errors>SKIP_PAGES)?chunks_number:(-1);
     int chksize;
     for(;(chunks_left--)&&(chksize=iff.get_chunk(chkid));last_chunk=chunks)
     {
        if (chunks++==chunk_num) { name=chkid; break; }
        iff.seek_close_chunk();
     }
   }
   CATCH(ex)
   {
     if (chunks_number < 0)
       chunks_number=(recover_errors>SKIP_CHUNKS)?chunks:last_chunk;
     report_error(ex,(recover_errors <= SKIP_PAGES));
   }
   ENDCATCH;
   if (!name.length())
   {
     if (chunks_number < 0) chunks_number=chunks;
     THROW("Too few chunks.");
   }
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
   GString chkid;
   IFFByteStream iff(*str);
   if (!iff.get_chunk(chkid)) 
     REPORT_EOF((recover_errors<=SKIP_PAGES))

   int chunks=0;
   int last_chunk=0;
   TRY
   {
     int chunks_left=(recover_errors>SKIP_PAGES)?chunks_number:(-1);
     int chksize;
     for(;(chunks_left--)&&(chksize=iff.get_chunk(chkid));last_chunk=chunks)
     {
       chunks++;
       if (chkid==chunk_name) { contains=1; break; }
       iff.seek_close_chunk();
     }
     if (!contains &&(chunks_number < 0)) chunks_number=last_chunk;
   }
   CATCH(ex)
   {
     if (chunks_number < 0)
       chunks_number=(recover_errors>SKIP_CHUNKS)?chunks:last_chunk;
     report_error(ex,(recover_errors <= SKIP_PAGES));
   }
   ENDCATCH;
   return contains;
}

//*****************************************************************************
//****************************** Save routines ********************************
//*****************************************************************************

static void
copy_chunks(ByteStream *from, IFFByteStream &ostr)
{
  from->seek(0);
  IFFByteStream iff(*from);
  GString chkid;
  while (iff.get_chunk(chkid))
    {
      ostr.put_chunk(chkid);
      ostr.copy(iff);
      ostr.close_chunk();
      iff.close_chunk();
    }
}


void
DjVuFile::add_djvu_data(IFFByteStream & ostr, GMap<GURL, void *> & map,
			bool included_too, bool no_ndir)
{
   check();
   if (map.contains(url)) return;
   bool top_level = !map.size();
   map[url]=0;
   bool processed_annotation = false;
     
   GP<ByteStream> str=data_pool->get_stream();
   GString chkid;
   IFFByteStream iff(*str);
   if (!iff.get_chunk(chkid)) 
     REPORT_EOF(true)

   // Open toplevel form
   if (top_level) 
     ostr.put_chunk(chkid);
   // Process chunks
   int chunks=0;
   int last_chunk=0;
   TRY
   {
     int chunks_left=(recover_errors>SKIP_PAGES)?chunks_number:(-1);
     int chksize;
     for(;(chunks_left--)&&(chksize = iff.get_chunk(chkid));last_chunk=chunks)
     {
      chunks++;
      if (chkid=="INCL" && included_too)
        {
          GP<DjVuFile> file = process_incl_chunk(iff);
          if (file)
          {
            if(recover_errors!=ABORT) file->set_recover_errors(recover_errors);
            if(verbose_eof) file->set_verbose_eof(verbose_eof);
            file->add_djvu_data(ostr, map, included_too, no_ndir);
          }
        } 
      else if (is_annotation(chkid) && anno && anno->size())
        {
          if (!processed_annotation)
            {
              processed_annotation = true;
              GCriticalSectionLock lock(&anno_lock);
              copy_chunks(anno, ostr);
            }
        }
      else if (chkid=="NDIR" && dir && !no_ndir)
        {
#ifndef NEED_DECODER_ONLY   
          // Decoder should never generate old NDIR chunks
          if (dir && !no_ndir)
            {
              ostr.put_chunk(chkid);
              dir->encode(ostr);
              ostr.close_chunk();
            }
#endif
        } 
      else if (chkid!="NDIR" || !no_ndir)
        {
          ostr.put_chunk(chkid);
          int ochksize=ostr.copy(iff);
          ostr.close_chunk();
          if(ochksize != chksize)
          {
            iff.seek_close_chunk();
            if (chunks_number < 0)
              chunks_number=(recover_errors>SKIP_CHUNKS)?chunks:last_chunk;
            THROW("EOF");
          }
        }
      iff.seek_close_chunk();
     }
     if (chunks_number < 0) chunks_number=last_chunk;
   }
   CATCH(ex)
   {
     if(!strcmp(ex.get_cause(),"EOF"))
     {
       if (chunks_number < 0)
         chunks_number=(recover_errors>SKIP_CHUNKS)?chunks:last_chunk;
       report_error(ex,(recover_errors<=SKIP_PAGES));
     }else
     {
       report_error(ex,true);
     }
   }
   ENDCATCH;

   // Otherwise, writes annotation at the end (annotations could be big)
   if (!processed_annotation && anno && anno->size())
     {
       processed_annotation = true;
       GCriticalSectionLock lock(&anno_lock);
       copy_chunks(anno, ostr);
     }
   // Close iff
   if (top_level) 
       ostr.close_chunk();
}


GP<MemoryByteStream>  
DjVuFile::get_djvu_bytestream(bool included_too, bool no_ndir)
{
   check();
   DEBUG_MSG("DjVuFile::get_djvu_bytestream(): creating DjVu raw file\n");
   DEBUG_MAKE_INDENT(3);
   GP<MemoryByteStream> pbs = new MemoryByteStream;
   IFFByteStream iff(*pbs);
   GMap<GURL, void *> map;
   add_djvu_data(iff, map, included_too, no_ndir);
   iff.flush();
   pbs->seek(0, SEEK_SET);
   return pbs;
}

GP<DataPool>
DjVuFile::get_djvu_data(bool included_too, bool no_ndir)
{
   GP<MemoryByteStream> pbs = get_djvu_bytestream(included_too, no_ndir);
   return new DataPool(*pbs);
}

void
DjVuFile::merge_anno(MemoryByteStream &out)
{
      // Reuse get_merged_anno(), which is better than the previous
      // implementation due to three things:
      //  1. It works even before the file is completely decoded
      //  2. It merges annotations taking into account where a child DjVuFile
      //     is included.
      //  3. It handles loops in DjVuFile's hierarchy
   
   GP<MemoryByteStream> str=get_merged_anno();
   if (str)
   {
      str->seek(0);
      if (out.tell() & 1) out.write((void *) "", 1);
      out.copy(*str);
   }
}



//****************************************************************************
//******************************* Modifying **********************************
//****************************************************************************

// Do NOT comment this function out. It's used by DjVuDocument to convert
// old-style DjVu documents to BUNDLED format.

GP<DataPool>
DjVuFile::unlink_file(const GP<DataPool> & data, const char * name)
      // Will process contents of data[] and remove any INCL chunk
      // containing 'name'
{
   MemoryByteStream str_out;
   IFFByteStream iff_out(str_out);

   GP<ByteStream> str_in=data->get_stream();
   IFFByteStream iff_in(*str_in);

   int chksize;
   GString chkid;
   if (!iff_in.get_chunk(chkid)) return data;

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
	 while(incl_str.length()>0 && incl_str[(int)incl_str.length()-1]=='\n')
	    incl_str.setat(incl_str.length()-1, 0);
	    
	 if (incl_str!=name)
	 {
	    iff_out.put_chunk(chkid);
	    iff_out.writall((const char*)incl_str, incl_str.length());
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
   iff_out.flush();
   str_out.seek(0, SEEK_SET);
   return new DataPool(str_out);
}

#ifndef NEED_DECODER_ONLY
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
   GString chkid;
   if (iff_in.get_chunk(chkid))
   {
      iff_out.put_chunk(chkid);
      int chksize;
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
#endif

#ifndef NEED_DECODER_ONLY
void
DjVuFile::unlink_file(const char * id)
{
   DEBUG_MSG("DjVuFile::insert_file(): id='" << id << "'\n");
   DEBUG_MAKE_INDENT(3);

      // Remove the file from the list of included files
   {
      GURL url=DjVuPort::get_portcaster()->id_to_url(this, id);
      if (url.is_empty()) url=DjVuFile::url.base()+id;
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

   GString chkid;
   if (iff_in.get_chunk(chkid))
   {
      iff_out.put_chunk(chkid);
      int chksize;
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
	    while(incl_str.length()>0 && incl_str[(int)incl_str.length()-1]=='\n')
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
#endif
