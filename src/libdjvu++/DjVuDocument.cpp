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
//C- $Id: DjVuDocument.cpp,v 1.42 1999-09-16 23:21:33 eaf Exp $

#ifdef __GNUC__
#pragma implementation
#endif

#include "DjVuDocument.h"
#include "IFFByteStream.h"
#include "GOS.h"
#include "debug.h"


DjVuDocument::DjVuDocument(void)
  : init_called(false), cache(0), doc_type(UNKNOWN_TYPE)
{
}

void
DjVuDocument::init(const GURL & url, GP<DjVuPort> xport,
                   DjVuFileCache * xcache)
{
   if (init_called)
      THROW("DjVuDocument is already initialized");
   if (!get_count())
      THROW("DjVuDocument is not secured by a GP<DjVuDocument>");
   DEBUG_MSG("DjVuDocument::init(): initializing class...\n");
   DEBUG_MAKE_INDENT(3);
   
      // Initialize
   cache=xcache;
   doc_type=UNKNOWN_TYPE;
   init_url=url;
   DjVuPortcaster * pcaster=get_portcaster();
   if (!xport) xport=simple_port=new DjVuSimplePort();
   pcaster->add_route(this, xport);
   pcaster->add_route(this, this);

   init_data_pool=pcaster->request_data(this, init_url);
   if (!init_data_pool) 
      THROW("Failed to get data for URL '"+init_url+"'");

      // Now we say it is ready
   init_called=true;

   init_thread_flags=STARTED;
   init_life_saver=this;
   init_thr.create(static_init_thread, this);
}

DjVuDocument::~DjVuDocument(void)
{
      // No more messages, please. We're being destroyed.
   get_portcaster()->del_port(this);
   
      // Before we proceed with destroy we must stop the initializing
      // thread (involves stopping the init_data_pool, the file
      // used to decode NDIR chunks (if any) and all unnamed files)
   if (init_data_pool) init_data_pool->stop();
   GMonitorLock lock(&init_thread_flags);
   while((init_thread_flags & STARTED) &&
	 !(init_thread_flags & FINISHED))
   {
      if (ndir_file) ndir_file->stop();
      ndir_file=0;

      {
	 GCriticalSectionLock lock(&ufiles_lock);
	 for(GPosition pos=ufiles_list;pos;++pos)
	    ufiles_list[pos]->file->stop();
	 ufiles_list.empty();
      }

      init_thread_flags.wait(500);
   }
}

void
DjVuDocument::check() const
{
  if (!init_called)
    THROW("DjVuDocument is not initialized");
}

void
DjVuDocument::static_init_thread(void * cl_data)
{
   DjVuDocument * th=(DjVuDocument *) cl_data;
   TRY {
      th->init_thread();
   } CATCH(exc) {
      th->flags=th->flags | DjVuDocument::DOC_INIT_FAILED;
      TRY {
	 get_portcaster()->notify_error(th, exc.get_cause());
      } CATCH(exc) {} ENDCATCH;
   } ENDCATCH;
      // Do not do ANYTHING below this line
}

void
DjVuDocument::init_thread(void)
      // This function is run in a separate thread.
      // The goal is to detect the document type (BUNDLED, OLD_INDEXED, etc.)
      // and decode navigation directory.
{
   DEBUG_MSG("DjVuDocument::init_thread(): guessing what we're dealing with\n");
   DEBUG_MAKE_INDENT(3);

      // No *local* life savers, please. Otherwise this object is very
      // likely to be never destroyed (data stops flowing into a DataPool,
      // the thread is waiting for it, the last external reference to
      // DjVuDocument is lost, but this local would stay => we will hang
      // forever)
   init_life_saver=0;
   
   DjVuPortcaster * pcaster=get_portcaster();
      
   GP<ByteStream> stream=init_data_pool->get_stream();
   IFFByteStream iff(*stream);
   GString chkid;
   int size=iff.get_chunk(chkid);
   if (size==0) THROW("EOF");
   if (size<0 || size>10*1024*1024)
      THROW("The main stream is not IFF stream.");

   if (chkid=="FORM:DJVM")
   {
      DEBUG_MSG("Got DJVM document here\n");
      DEBUG_MAKE_INDENT(3);

      size=iff.get_chunk(chkid);
      if (chkid=="DIRM")
      {
	 djvm_dir=new DjVmDir();
	 djvm_dir->decode(iff);
	 iff.close_chunk();
	 if (djvm_dir->is_bundled())
	 {
	    DEBUG_MSG("Got BUNDLED file.\n");
	    doc_type=BUNDLED;
	 } else
	 {
	    DEBUG_MSG("Got INDIRECT file.\n");
	    doc_type=INDIRECT;
	 }
	 flags=flags | DOC_TYPE_KNOWN | DOC_DIR_KNOWN;
	 pcaster->notify_doc_flags_changed(this, DOC_TYPE_KNOWN | DOC_DIR_KNOWN, 0);
	 check_unnamed_files();
      } else if (chkid=="DIR0")
      {
	 DEBUG_MSG("Got OLD_BUNDLED file.\n");
	 doc_type=OLD_BUNDLED;
	 flags=flags | DOC_TYPE_KNOWN;
	 pcaster->notify_doc_flags_changed(this, DOC_TYPE_KNOWN, 0);
	 check_unnamed_files();
      } else THROW("Unknown document format. Can't decode.");

      if (doc_type==OLD_BUNDLED)
      {
	    // Read the DjVmDir0 directory. We are unable to tell what
	    // files are pages and what are included at this point.
	    // We only know that the first file with DJVU (BM44 or PM44)
	    // form *is* the first page. The rest will become known
	    // after we decode DjVuNavDir
	 djvm_dir0=new DjVmDir0();
	 djvm_dir0->decode(iff);
	 iff.close_chunk();

	    // Get offset to the first DJVU, PM44 or BM44 chunk
	 int first_page_offset=0;
	 while(!first_page_offset)
	 {
	    int offset;
	    size=iff.get_chunk(chkid, &offset);
	    if (size==0) THROW("Failed to find any page in this document.");
	    if (chkid=="FORM:DJVU" || chkid=="FORM:PM44" || chkid=="FORM:BM44")
	    {
	       DEBUG_MSG("Got 1st page offset=" << offset << "\n");
	       first_page_offset=offset;
	    }
	    iff.close_chunk();
	 }

	    // Now get the name of this file
	 int file_num;
	 for(file_num=0;file_num<djvm_dir0->get_files_num();file_num++)
	 {
	    DjVmDir0::FileRec & file=*djvm_dir0->get_file(file_num);
	    if (file.offset==first_page_offset)
	    {
	       first_page_name=file.name;
	       break;
	    }
	 }
	 if (!first_page_name.length())
	    THROW("Failed to find any page in this document.");

	 flags=flags | DOC_DIR_KNOWN;
	 pcaster->notify_doc_flags_changed(this, DOC_DIR_KNOWN, 0);
	 check_unnamed_files();
      }
   } else // chkid!="FORM:DJVM"
   {
	 // DJVU format
      DEBUG_MSG("Got DJVU OLD_INDEXED document here.\n");
      doc_type=OLD_INDEXED;

      flags=flags | DOC_TYPE_KNOWN;
      pcaster->notify_doc_flags_changed(this, DOC_TYPE_KNOWN, 0);
      check_unnamed_files();
   }

   if (doc_type==OLD_BUNDLED || doc_type==OLD_INDEXED)
   {
      DEBUG_MSG("Searching for NDIR chunks...\n");
      ndir_file=get_djvu_file(-1);
      ndir=ndir_file->decode_ndir();
      if (!ndir)
      {
	    // Seems to be 1-page old-style document. Create dummy NDIR
	 if (doc_type==OLD_BUNDLED)
	 {
	    ndir=new DjVuNavDir(init_url+"directory");
	    ndir->insert_page(-1, first_page_name);
	 } else
	 {
	    ndir=new DjVuNavDir(init_url.base()+"directory");
	    ndir->insert_page(-1, init_url.name());
	 }
      }
      flags=flags | DOC_NDIR_KNOWN;
      pcaster->notify_doc_flags_changed(this, DOC_NDIR_KNOWN, 0);
      check_unnamed_files();
   }

   flags=flags | DOC_INIT_COMPLETE;
   pcaster->notify_doc_flags_changed(this, DOC_INIT_COMPLETE, 0);
   check_unnamed_files();

   init_thread_flags=init_thread_flags | FINISHED;
      // Nothing else after this point, please. The object may already
      // be destroyed.
   
   DEBUG_MSG("DOCUMENT IS FULLY INITIALIZED now: doc_type='" <<
	     (doc_type==BUNDLED ? "BUNDLED" :
	      doc_type==OLD_BUNDLED ? "OLD_BUNDLED" :
	      doc_type==INDIRECT ? "INDIRECT" :
	      doc_type==OLD_INDEXED ? "OLD_INDEXED" :
	      "UNKNOWN") << "'\n");
}

void
DjVuDocument::check_unnamed_files(void)
{
   DEBUG_MSG("DjVuDocument::check_unnamed_files(): Seeing if we can fix some...\n");
   DEBUG_MAKE_INDENT(3);

   if ((flags & DOC_TYPE_KNOWN)==0) return;
   
      // See the list of unnamed files (created when there was insufficient
      // information about DjVuDocument structure) and try to fix those,
      // which can be fixed at this time
   while(1)
   {
      DjVuPortcaster * pcaster=get_portcaster();

      GP<UnnamedFile> ufile;
      GURL new_url;
      
      TRY {
	 {
	    GCriticalSectionLock lock(&ufiles_lock);
	    for(GPosition pos=ufiles_list;pos;)
	    {
	       TRY {
		  GP<UnnamedFile> f=ufiles_list[pos];
		  if (f->id_type==UnnamedFile::ID) new_url=id_to_url(f->id);
		  else new_url=page_to_url(f->page_num);
		  if (!new_url.is_empty())
		  {
		     ufile=f;
			// Don't take it off the list. We want to be
			// able to stop the init from ~DjVuDocument();
			//
			// ufiles_list.del(pos);
		     break;
		  }
		  ++pos;
	       } CATCH(exc) {
		  pcaster->notify_error(this, exc.get_cause());
		  GPosition this_pos=pos;
		  ++pos;
		  ufiles_list.del(this_pos);
	       } ENDCATCH;
	    }
	 }

	 if (ufile && !new_url.is_empty())
	 {
	    DEBUG_MSG("Fixing file: '" << ufile->url << "'=>'" << new_url << "'\n");
	    
	       // Now, once we know its real URL we can request a real DataPool and
	       // can connect the DataPool owned by DjVuFile to that real one
	       // Note, that now request_data() will not play fool because
	       // we have enough information
	    if (ufile->data_pool)
	    {
	       GP<DataPool> new_pool=pcaster->request_data(ufile->file, new_url);
	       if (!new_pool) THROW("Failed to get data for URL '"+new_url+"'");
	       ufile->data_pool->connect(new_pool);
	    }
	    
	    ufile->file->set_name(new_url.name());
	    ufile->file->move(new_url.base());
	    pcaster->set_name(ufile->file, new_url);
	 } else break;
      } CATCH(exc) {
	 pcaster->notify_error(this, exc.get_cause());
      } ENDCATCH;
      
	 // Remove the 'ufile' from the list
      GCriticalSectionLock lock(&ufiles_lock);
      for(GPosition pos=ufiles_list;pos;++pos)
	 if (ufiles_list[pos]==ufile)
	 {
	    ufiles_list.del(pos);
	    break;
	 }
   } // while(1)
}

int
DjVuDocument::get_pages_num(void)
{
   check();
   if (flags & DOC_TYPE_KNOWN)
      if (doc_type==BUNDLED || doc_type==INDIRECT)
	 return djvm_dir->get_pages_num();
      else if (flags & DOC_NDIR_KNOWN)
	 return ndir->get_pages_num();
   return 1;
}

GURL
DjVuDocument::page_to_url(int page_num)
{
   check();
   DEBUG_MSG("DjVuDocument::page_to_url(): page_num=" << page_num << "\n");
   DEBUG_MAKE_INDENT(3);
   
   GURL url;
   if (flags & DOC_TYPE_KNOWN)
      switch(doc_type)
      {
	 case OLD_INDEXED:
	 {
	    if (page_num<0) url=init_url;
	    else if (flags & DOC_NDIR_KNOWN) url=ndir->page_to_url(page_num);
	    break;
	 }
	 case OLD_BUNDLED:
	 {
	    if (page_num<0) page_num=0;
	    if (page_num==0 && (flags & DOC_DIR_KNOWN))
	       url=init_url+first_page_name;
	    else if (flags & DOC_NDIR_KNOWN)
	       url=ndir->page_to_url(page_num);
	    break;
	 }
	 case BUNDLED:
	 {
	    if (page_num<0) page_num=0;
	    if (flags & DOC_DIR_KNOWN)
	    {
	       GP<DjVmDir::File> file=djvm_dir->page_to_file(page_num);
	       if (!file) THROW("Page number is too big.");
	       url=init_url+file->name;
	    }
	    break;
	 }
	 case INDIRECT:
	 {
	    if (page_num<0) page_num=0;
	    if (flags & DOC_DIR_KNOWN)
	    {
	       GP<DjVmDir::File> file=djvm_dir->page_to_file(page_num);
	       if (!file) THROW("Page number is too big.");
	       url=init_url.base()+file->name;
	    }
	    break;
	 }
	 default:
	    THROW("Unknown document type.");
      };
   return url;
}

int
DjVuDocument::url_to_page(const GURL & url)
{
   check();
   DEBUG_MSG("DjVuDocument::url_to_page(): url='" << url << "'\n");
   DEBUG_MAKE_INDENT(3);

   int page_num=0;
   if (flags & DOC_TYPE_KNOWN)
      switch(doc_type)
      {
	 case OLD_BUNDLED:
	 case OLD_INDEXED:
	 {
	    if (flags & DOC_NDIR_KNOWN) page_num=ndir->url_to_page(url);
	    break;
	 }
	 case BUNDLED:
	 {
	    if (flags & DOC_DIR_KNOWN)
	    {
	       GP<DjVmDir::File> file;
	       if (url.base()==init_url) file=djvm_dir->name_to_file(url.name());
	       if (file) page_num=file->get_page_num();
	    }
	    break;
	 }
	 case INDIRECT:
	 {
	    if (flags & DOC_DIR_KNOWN)
	    {
	       GP<DjVmDir::File> file;
	       if (url.base()==init_url.base()) file=djvm_dir->name_to_file(url.name());
	       if (file) page_num=file->get_page_num();
	    }
	    break;
	 }
	 default:
	    THROW("Unknown document type.");
      };
   return page_num;
}

GURL
DjVuDocument::id_to_url(const char * id)
{
   check();
   DEBUG_MSG("DjVuDocument::id_to_url(): translating ID='" << id << "' to URL\n");
   DEBUG_MAKE_INDENT(3);

   if (flags & DOC_TYPE_KNOWN)
      switch(doc_type)
      {
	 case BUNDLED:
	    if (flags & DOC_DIR_KNOWN)
	    {
	       GP<DjVmDir::File> file=djvm_dir->id_to_file(id);
	       if (!file) file=djvm_dir->name_to_file(id);
	       if (!file) file=djvm_dir->title_to_file(id);
	       if (file) return init_url+file->name;
	    }
	    break;
	 case INDIRECT:
	    if (flags & DOC_DIR_KNOWN)
	    {
	       GP<DjVmDir::File> file=djvm_dir->id_to_file(id);
	       if (!file) file=djvm_dir->name_to_file(id);
	       if (!file) file=djvm_dir->title_to_file(id);
	       if (file) return init_url.base()+file->name;
	    }
	    break;
	 case OLD_BUNDLED:
	    return init_url+id;
	 case OLD_INDEXED:
	    return init_url.base()+id;
      }
   return GURL();
}

GURL
DjVuDocument::id_to_url(const DjVuPort * source, const char * id)
{
   return id_to_url(id);
}

GP<DjVuFile>
DjVuDocument::url_to_file(const GURL & url)
      // This function is private and is called from two places:
      // id_to_file() and get_djvu_file() ONLY when the structure is known
{
   check();
   DEBUG_MSG("DjVuDocument::url_to_file(): url='" << url << "'\n");
   DEBUG_MAKE_INDENT(3);

      // Check files registered with DjVuPortcaster
   GP<DjVuPort> port=get_portcaster()->name_to_port(url);
   if (port && port->inherits("DjVuFile"))
   {
      DEBUG_MSG("found file using DjVuPortcaster\n");
      GP<DjVuFile> file=(DjVuFile *) (DjVuPort *) port;
      if (file->is_decode_ok()) return (DjVuFile *) file;
      DEBUG_MSG("but it's not fully decoded.\n");
   }

   DEBUG_MSG("creating a new file\n");
   GP<DjVuFile> file=new DjVuFile();
   file->init(url, this);
   get_portcaster()->set_name(file, url);

   return file;
}

GP<DjVuFile>
DjVuDocument::get_djvu_file(int page_num)
{
   check();
   DEBUG_MSG("DjVuDocument::get_djvu_file(): request for page " << page_num << "\n");
   DEBUG_MAKE_INDENT(3);

   GURL url;
   {
	 // I'm locking the flags because depending on what page_to_url()
	 // returns me, I'll be creating DjVuFile in different ways.
	 // And I don't want the situation to change between the moment I call
	 // id_to_url() and I actually create DjVuFile
      GMonitorLock lock(&flags);
      url=page_to_url(page_num);
      if (url.is_empty())
      {
	 DEBUG_MSG("Structure is not known => inventing dummy URL.\n");
	 
	    // Invent some dummy temporary URL. I don't care what it will
	    // be. I'll remember the page_num and will generate the correct URL
	    // after I learn what the document is
	 char buffer[128];
	 sprintf(buffer, "djvufileurl://%p/page%d.djvu", this, page_num);
	 url=buffer;

	 GCriticalSectionLock lock(&ufiles_lock);
	 for(GPosition pos=ufiles_list;pos;++pos)
	 {
	    GP<UnnamedFile> f=ufiles_list[pos];
	    if (f->url==url) return f->file;
	 }
	 GP<UnnamedFile> ufile=new UnnamedFile(UnnamedFile::PAGE_NUM, 0,
					       page_num, url, 0);

	    // We're adding the record to the list before creating the DjVuFile
	    // because DjVuFile::init() will call request_data(), and the
	    // latter should be able to find the record.
	    //
	    // We also want to keep ufiles_lock to make sure that when
	    // request_data() is called, the record is still there
	 ufiles_list.append(ufile);
      
	 GP<DjVuFile> file=new DjVuFile();
	 file->init(url, this);
	 ufile->file=file;
	 return file;
      }
   }
   
   GP<DjVuFile> file=url_to_file(url);
   get_portcaster()->add_route(file, this);
   
   return file;
}

GP<DjVuFile>
DjVuDocument::get_djvu_file(const char * id)
{
   check();
   DEBUG_MSG("DjVuDocument::get_djvu_file(): ID='" << id << "'\n");
   DEBUG_MAKE_INDENT(3);

   if (!id || !strlen(id)) return get_djvu_file(-1);
   char * ptr;
   int page_num=strtol(id, &ptr, 10);
   if (!*ptr) return get_djvu_file(page_num);

   GURL url;
   {
	 // I'm locking the flags because depending on what id_to_url()
	 // returns me, I'll be creating DjVuFile in different ways.
	 // And I don't want the situation to change between the moment I call
	 // id_to_url() and I actually create DjVuFile
      GMonitorLock lock(&flags);
      url=id_to_url(id);
      if (url.is_empty())
      {
	    // Invent some dummy temporary URL. I don't care what it will
	    // be. I'll remember the ID and will generate the correct URL
	    // after I learn what the document is
	 char buffer[128];
	 sprintf(buffer, "djvufileurl://%p/%s", this, id);
	 url=buffer;
	 DEBUG_MSG("Invented url='" << url << "'\n");
	 
	 GCriticalSectionLock lock(&ufiles_lock);
	 for(GPosition pos=ufiles_list;pos;++pos)
	 {
	    GP<UnnamedFile> f=ufiles_list[pos];
	    if (f->url==url) return f->file;
	 }
	 GP<UnnamedFile> ufile=new UnnamedFile(UnnamedFile::ID, id, 0, url, 0);

	    // We're adding the record to the list before creating the DjVuFile
	    // because DjVuFile::init() will call request_data(), and the
	    // latter should be able to find the record.
	    //
	    // We also want to keep ufiles_lock to make sure that when
	    // request_data() is called, the record is still there
	 ufiles_list.append(ufile);
      
	 GP<DjVuFile> file=new DjVuFile();
	 file->init(url, this);
	 ufile->file=file;
	 return file;
      }
   }

   GP<DjVuFile> file=url_to_file(url);
   get_portcaster()->add_route(file, this);
   
   return file;
}

GP<DjVuImage>
DjVuDocument::get_page(int page_num, DjVuPort * port)
{
   check();
   DEBUG_MSG("DjVuDocument::get_page(): request for page " << page_num << "\n");
   DEBUG_MAKE_INDENT(3);
   
   GP<DjVuFile> file=get_djvu_file(page_num);
   GP<DjVuImage> dimg=new DjVuImage();
   dimg->connect(file);
   if (port) DjVuPort::get_portcaster()->add_route(dimg, port);
   
   if (!file->is_decoding() &&
       !file->is_decode_ok() &&
       !file->is_decode_failed())
      file->start_decode();

   return dimg;
}

GP<DjVuImage>
DjVuDocument::get_page(const char * id, DjVuPort * port)
{
   check();
   DEBUG_MSG("DjVuDocument::get_page(): ID='" << id << "'\n");
   DEBUG_MAKE_INDENT(3);

   GP<DjVuFile> file=get_djvu_file(id);
   GP<DjVuImage> dimg=new DjVuImage();
   dimg->connect(file);
   if (port) DjVuPort::get_portcaster()->add_route(dimg, port);
   
   if (!file->is_decoding() &&
       !file->is_decode_ok() &&
       !file->is_decode_failed())
      file->start_decode();

   return dimg;
}

static void
add_to_cache(const GP<DjVuFile> & f, GMap<GURL, void *> & map,
	     DjVuFileCache * cache)
{
   GURL url=f->get_url();
   DEBUG_MSG("DjVuDocument::add_to_cache(): url='" << url << "'\n");
   DEBUG_MAKE_INDENT(3);
   
   if (!map.contains(url))
   {
      map[url]=0;
      cache->add_file(f);
      
      GPList<DjVuFile> list;
      for(GPosition pos=list;pos;++pos)
	 add_to_cache(list[pos], map, cache);
   }
}

void
DjVuDocument::add_to_cache(const GP<DjVuFile> & f)
{
   if (cache)
   {
      GMap<GURL, void *> map;
      ::add_to_cache(f, map, cache);
   }
}

void
DjVuDocument::notify_file_flags_changed(const DjVuFile * source,
					long set_mask, long clr_mask)
{
   check();
   if (set_mask & DjVuFile::DECODE_OK)
      if (cache) add_to_cache((DjVuFile *) source);
}

GPBase
DjVuDocument::id_to_file(const DjVuPort * source, const char * id)
{
   return (DjVuFile *) get_djvu_file(id);
}

GP<DataPool>
DjVuDocument::request_data(const DjVuPort * source, const GURL & url)
{
   DEBUG_MSG("DjVuDocument::request_data(): seeing if we can do it\n");
   DEBUG_MAKE_INDENT(3);

   if (source==this) return 0;

   check();

   if (url==init_url) return init_data_pool;

   {
	 // See if there is a file in the "UnnamedFiles" list.
	 // If it's there, then create an empty DataPool and store its
	 // pointer in the list. The "init thread" will eventually
	 // do smth with it.
      GCriticalSectionLock lock(&ufiles_lock);
      for(GPosition pos=ufiles_list;pos;++pos)
      {
	 GP<UnnamedFile> f=ufiles_list[pos];
	 if (f->url==url)
	 {
	    DEBUG_MSG("Found tmp unnamed DjVuFile. Return empty DataPool\n");
	       // Remember the DataPool. We will connect it to the
	       // actual data after the document structure becomes known
	    f->data_pool=new DataPool();
	    return f->data_pool;
	 }
      }
   }

      // Well, the url is not in the "UnnamedFiles" list, but it doesn't
      // mean, that it's not "artificial". Stay alert!
   GP<DataPool> data_pool;
   if (flags & DOC_TYPE_KNOWN)
      switch(doc_type)
      {
	 case OLD_BUNDLED:
	 {
	    if (flags & DOC_DIR_KNOWN)
	    {
	       DEBUG_MSG("The document is in OLD_BUNDLED format\n");
	       if (url.base()!=init_url)
		  THROW("URL '"+url+"' points outside of the bundled document.");
	 
	       GP<DjVmDir0::FileRec> file=djvm_dir0->get_file(url.name());
	       if (!file) THROW("File '"+url.name()+"' is not in this bundle.");
	       data_pool=new DataPool(init_data_pool, file->offset, file->size);
	    }
	    break;
	 }
	 case BUNDLED:
	 {
	    if (flags & DOC_DIR_KNOWN)
	    {
	       DEBUG_MSG("The document is in new BUNDLED format\n");
	       if (url.base()!=init_url)
		  THROW("URL '"+url+"' points outside of the bundled document.");
	 
	       GP<DjVmDir::File> file=djvm_dir->name_to_file(url.name());
	       if (!file) THROW("File '"+url.name()+"' is not in this bundle.");
	       data_pool=new DataPool(init_data_pool, file->offset, file->size);
	    }
	    break;
	 }
	 case OLD_INDEXED:
	 case INDIRECT:
	 {
	    DEBUG_MSG("The document is in OLD_INDEXED or INDIRECT format\n");
	    if (url.base()!=init_url.base())
	       THROW("URL '"+url+"' points outside of the document's directory.");
	    if (flags & DOC_DIR_KNOWN)
	       if (doc_type==INDIRECT && !djvm_dir->name_to_file(url.name()))
		  THROW("URL '"+url+"' points outside of the INDIRECT document.");
	 
	    if (url.is_local_file_url())
	    {
	       GString fname=GOS::url_to_filename(url);
	       if (GOS::basename(fname)=="-") fname="-";
	       DEBUG_MSG("fname=" << fname << "\n");

	       data_pool=new DataPool();
	       StdioByteStream str(fname, "rb");
	       char buffer[1024];
	       int length;
	       while((length=str.read(buffer, 1024)))
		  data_pool->add_data(buffer, length);
	       data_pool->set_eof();
	    }
	 }
      }
   return data_pool;
}

static TArray<char>
unlink_file(const TArray<char> & data, const char * name)
      // Will process contents of data[] and remove any INCL chunk
      // containing 'name'
{
   MemoryByteStream str_out;
   IFFByteStream iff_out(str_out);

   MemoryByteStream str_in(data, data.size());
   IFFByteStream iff_in(str_in);

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
	 while(incl_str.length()>0 && incl_str[incl_str.length()-1]=='\n')
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
   return str_out.get_data();
}

static void
add_file_to_djvm(const GP<DjVuFile> & file, bool page,
		 DjVmDoc & doc, GMap<GURL, void *> & map)
      // This function is used only for obsolete formats.
      // For new formats there is no need to process files recursively.
      // All information is already available from the DJVM chunk
{
   GURL url=file->get_url();

   if (!map.contains(url))
   {
      map[url]=0;

      if (file->get_chunks_number()>0 && !file->contains_chunk("NDIR"))
      {
	    // Get the data and unlink any file containing NDIR chunk.
	    // Yes. We're lazy. We don't check if those files contain
	    // anything else.
	 GPosition pos;
	 GPList<DjVuFile> files_list=file->get_included_files(false);
	 TArray<char> data=file->get_djvu_data(false, true);
	 for(pos=files_list;pos;++pos)
	 {
	    GP<DjVuFile> f=files_list[pos];
	    if (f->contains_chunk("NDIR"))
	       data=unlink_file(data, f->get_url().name());
	 }
	 
	    // Finally add it to the document
	 GString name=file->get_url().name();
	 GP<DjVmDir::File> file_rec=new DjVmDir::File(name, name, name, page);
	 doc.insert_file(file_rec, data, -1);

	    // And repeat for all included files
	 for(pos=files_list;pos;++pos)
	    add_file_to_djvm(files_list[pos], false, doc, map);
      }
   }
}

GP<DjVmDoc>
DjVuDocument::get_djvm_doc(void)
      // This function may block for data
{
   check();
   DEBUG_MSG("DjVuDocument::get_djvm_doc(): creating the DjVmDoc\n");
   DEBUG_MAKE_INDENT(3);

   if (!is_init_complete()) THROW("Document has not been initialized complete yet.");

   GP<DjVmDoc> doc=new DjVmDoc();
   if (doc_type==BUNDLED)
   {
      DEBUG_MSG("Trivial: the document is BUNDLED: just pass the pool.\n");
      doc->read(init_data_pool);
   } else if (doc_type==INDIRECT)
   {
      DEBUG_MSG("Not difficult: the document is INDIRECT => follow DjVmDir.\n");
      
      GPList<DjVmDir::File> files_list=djvm_dir->get_files_list();
      for(GPosition pos=files_list;pos;++pos)
      {
	 GP<DjVmDir::File> f=new DjVmDir::File(*files_list[pos]);
	 GP<DjVuFile> file=url_to_file(id_to_url(f->id));
	 doc->insert_file(f, file->get_data_pool());
      }
   } else
   {
      DEBUG_MSG("Converting: the document is in an old format.\n");

      GMap<GURL, void *> map_add;
      for(int page_num=0;page_num<ndir->get_pages_num();page_num++)
      {
	 GP<DjVuFile> file=url_to_file(ndir->page_to_url(page_num));
	 GMap<GURL, void *> map_del;
	 add_file_to_djvm(file, true, *doc, map_add);
      }
   }
   return doc;
}

void
DjVuDocument::write(ByteStream & str)
{
   DEBUG_MSG("DjVuDocument::write(): storing DjVmDoc into ByteStream\n");
   DEBUG_MAKE_INDENT(3);
   
   GP<DjVmDoc> doc=get_djvm_doc();
   doc->write(str);
}

void
DjVuDocument::expand(const char * dir_name, const char * idx_name)
{
   DEBUG_MSG("DjVuDocument::expand(): dir_name='" << dir_name << "'\n");
   DEBUG_MAKE_INDENT(3);
   
   GP<DjVmDoc> doc=get_djvm_doc();
   doc->expand(dir_name, idx_name);
}
