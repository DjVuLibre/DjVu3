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
//C- $Id: DjVuDocument.cpp,v 1.75 1999-11-22 03:38:43 bcr Exp $

#ifdef __GNUC__
#pragma implementation
#endif

#include "DjVuDocument.h"
#include "IFFByteStream.h"
#include "GOS.h"
#include "debug.h"

const float	DjVuDocument::thumb_gamma=2.20;

static GString
get_int_prefix(void * ptr)
{
      // These NAMEs are used to enable DjVuFile sharing inside the same
      // DjVuDocument using DjVuPortcaster. Since URLs are unique to the
      // document, other DjVuDocuments cannot retrieve files until they're
      // assigned some permanent name. After '?' there should be the real
      // file's URL. Please note, that output of this function is used only
      // as name for DjVuPortcaster. Not as a URL.
   char buffer[128];
   sprintf(buffer, "document_%p?", ptr);
   return buffer;
}

DjVuDocument::DjVuDocument(void)
  : doc_type(UNKNOWN_TYPE),
    has_file_names(false),
    recover_errors(ABORT),
    verbose_eof(false),
    init_called(false),
    cache(0) 
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

      // We want to stop any DjVuFile, which has been created by us
      // and is still being decoded. We have to stop them manually because
      // they keep the "life saver" in the decoding thread and won't stop
      // when we clear the last reference to them
   {
      GCriticalSectionLock lock(&ufiles_lock);
      for(GPosition pos=ufiles_list;pos;++pos)
	 ufiles_list[pos]->file->stop(false);	// Disable any access to data
      ufiles_list.empty();
   }

//bcr: This just seems to create a nice segmentation fault.
   GPList<DjVuPort> ports=get_portcaster()->prefix_to_ports(get_int_prefix(this));
   for(GPosition pos=ports;pos;++pos)
   {
      GP<DjVuPort> port=ports[pos];
      if (port->inherits("DjVuFile"))
      {
	 DjVuFile * file=(DjVuFile *) (DjVuPort *) port;
	 file->stop(false);	// Disable any access to data
      }
   }
}

void
DjVuDocument::stop(void)
{
   DEBUG_MSG("DjVuDocument::stop(): making sure that the init thread dies.\n");
   DEBUG_MAKE_INDENT(3);

   GMonitorLock lock(&init_thread_flags);
   while((init_thread_flags & STARTED) &&
	 !(init_thread_flags & FINISHED))
   {
      if (init_data_pool) init_data_pool->stop(false);	// any operation

//bcr: I don't understand this.  Isn't ndir_file stopped above?
      if (ndir_file) ndir_file->stop(false);

      GCriticalSectionLock lock(&ufiles_lock);
      for(GPosition pos=ufiles_list;pos;++pos)
	 ufiles_list[pos]->file->stop(false);	// Disable any access to data
      ufiles_list.empty();

      init_thread_flags.wait(50);
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
   GP<DjVuDocument> life_saver=th;
   th->init_life_saver=0;
   TRY {
      th->init_thread();
   } CATCH(exc) {
      th->flags|=DjVuDocument::DOC_INIT_FAILED;
      TRY { get_portcaster()->notify_error(th, exc.get_cause()); } CATCH(exc) {} ENDCATCH;
      th->init_thread_flags|=FINISHED;
   } ENDCATCH;
}

void
DjVuDocument::init_thread(void)
      // This function is run in a separate thread.
      // The goal is to detect the document type (BUNDLED, OLD_INDEXED, etc.)
      // and decode navigation directory.
{
   DEBUG_MSG("DjVuDocument::init_thread(): guessing what we're dealing with\n");
   DEBUG_MAKE_INDENT(3);

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
	 flags|=DOC_TYPE_KNOWN | DOC_DIR_KNOWN;
	 pcaster->notify_doc_flags_changed(this, DOC_TYPE_KNOWN | DOC_DIR_KNOWN, 0);
	 check_unnamed_files();
      } else if (chkid=="DIR0")
      {
	 DEBUG_MSG("Got OLD_BUNDLED file.\n");
	 doc_type=OLD_BUNDLED;
	 flags|=DOC_TYPE_KNOWN;
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

	 flags|=DOC_DIR_KNOWN;
	 pcaster->notify_doc_flags_changed(this, DOC_DIR_KNOWN, 0);
	 check_unnamed_files();
      }
   } else // chkid!="FORM:DJVM"
   {
	 // DJVU format
      DEBUG_MSG("Got DJVU OLD_INDEXED or SINGLE_PAGE document here.\n");
      doc_type=OLD_INDEXED;

      flags|=DOC_TYPE_KNOWN;
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
	    doc_type=SINGLE_PAGE;
	    ndir=new DjVuNavDir(init_url.base()+"directory");
	    ndir->insert_page(-1, init_url.name());
	 }
      }
      else
      {
//bcr: I don't really understand why ndir_file must point to a 
//bcr: document.  But if it doesn't, then a segmentation fault
//bcr: will be generated with the destructor is called from a THROW().
//bcr: So we map ndir_file to the first page if init_url was the 
//bcr: index file.
        int page=ndir->url_to_page(init_url);
        if(page<0)
        {
          int pages=ndir->get_pages_num(),i;
          for(i=0;i<pages;i++)
          {
            TRY
            {
              ndir_file=get_djvu_file(i);
              break;
            }
            CATCH(ex)
            {
		// We will ignore this error for now.
            }
            ENDCATCH;
          }
          if(i==pages)
          {
            THROW("No valid pages found in this document.");
          }
        }
      }
      flags|=DOC_NDIR_KNOWN;
      pcaster->notify_doc_flags_changed(this, DOC_NDIR_KNOWN, 0);
      check_unnamed_files();
   }

   flags|=DOC_INIT_COMPLETE;
   pcaster->notify_doc_flags_changed(this, DOC_INIT_COMPLETE, 0);
   check_unnamed_files();

   init_thread_flags|=FINISHED;
   
   DEBUG_MSG("DOCUMENT IS FULLY INITIALIZED now: doc_type='" <<
	     (doc_type==BUNDLED ? "BUNDLED" :
	      doc_type==OLD_BUNDLED ? "OLD_BUNDLED" :
	      doc_type==INDIRECT ? "INDIRECT" :
	      doc_type==OLD_INDEXED ? "OLD_INDEXED" :
	      doc_type==SINGLE_PAGE ? "SINGLE_PAGE" :
	      "UNKNOWN") << "'\n");
}

void
DjVuDocument::set_file_aliases(const DjVuFile * file)
{
   DEBUG_MSG("DjVuDocument::set_file_aliases(): setting global aliases for file '"
	     << file->get_url() << "'\n");
   DEBUG_MAKE_INDENT(3);

   DjVuPortcaster * pcaster=DjVuPort::get_portcaster();
   
   GMonitorLock lock(&((DjVuFile *) file)->get_safe_flags());
   pcaster->clear_aliases(file);
   if (file->is_decode_ok())
   {
      pcaster->add_alias(file, file->get_url());
      if (is_init_complete())
      {
	 int page_num=url_to_page(file->get_url());
	 if (page_num==0) pcaster->add_alias(file, init_url+"#-1");
	 pcaster->add_alias(file, init_url+"#"+GString(page_num));
      }
      pcaster->add_alias(file, file->get_url()+"#-1");
   } else pcaster->add_alias(file, get_int_prefix(this)+file->get_url());
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
	    set_file_aliases(ufile->file);
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
DjVuDocument::get_pages_num(void) const
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
DjVuDocument::page_to_url(int page_num) const
{
   check();
   DEBUG_MSG("DjVuDocument::page_to_url(): page_num=" << page_num << "\n");
   DEBUG_MAKE_INDENT(3);
   
   GURL url;
   if (flags & DOC_TYPE_KNOWN)
      switch(doc_type)
      {
	 case SINGLE_PAGE:
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
DjVuDocument::url_to_page(const GURL & url) const
{
   check();
   DEBUG_MSG("DjVuDocument::url_to_page(): url='" << url << "'\n");
   DEBUG_MAKE_INDENT(3);

   int page_num=0;
   if (flags & DOC_TYPE_KNOWN)
      switch(doc_type)
      {
	 case SINGLE_PAGE:
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
DjVuDocument::id_to_url(const char * id) const
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
	 case SINGLE_PAGE:
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
DjVuDocument::url_to_file(const GURL & url, bool dont_create)
      // This function is private and is called from two places:
      // id_to_file() and get_djvu_file() ONLY when the structure is known
{
   check();
   DEBUG_MSG("DjVuDocument::url_to_file(): url='" << url << "'\n");
   DEBUG_MAKE_INDENT(3);

      // Try DjVuPortcaster to find existing files.
   DjVuPortcaster * pcaster=DjVuPort::get_portcaster();
   GP<DjVuPort> port;
   
      // First - fully decoded files
   port=pcaster->alias_to_port(url);
   if (port && port->inherits("DjVuFile"))
   {
      DEBUG_MSG("found fully decoded file using DjVuPortcaster\n");
      return (DjVuFile *) (DjVuPort *) port;
   }

      // Second - internal files
   port=pcaster->alias_to_port(get_int_prefix(this)+url);
   if (port && port->inherits("DjVuFile"))
   {
      DEBUG_MSG("found internal file using DjVuPortcaster\n");
      return (DjVuFile *) (DjVuPort *) port;
   }

   GP<DjVuFile> file;
   
   if (!dont_create)
   {
      DEBUG_MSG("creating a new file\n");
      file=new DjVuFile();
      file->set_recover_errors(recover_errors);
      file->set_verbose_eof(verbose_eof);
      file->init(url, this);
      set_file_aliases(file);
   }

   return file;
}

GP<DjVuFile>
DjVuDocument::get_djvu_file(int page_num, bool dont_create)
{
   check();
   DEBUG_MSG("DjVuDocument::get_djvu_file(): request for page " << page_num << "\n");
   DEBUG_MAKE_INDENT(3);

   DjVuPortcaster * pcaster=DjVuPort::get_portcaster();
   
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
	 DEBUG_MSG("Structure is not known => check <doc_url>#<page_num> alias...\n");
	 GP<DjVuPort> port=pcaster->alias_to_port(init_url+"#"+GString(page_num));
	 if (!port || !port->inherits("DjVuFile"))
	 {
	    DEBUG_MSG("failed => invent dummy URL and proceed\n");
	 
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
            file->set_recover_errors(recover_errors);
            file->set_verbose_eof(verbose_eof);
	    file->init(url, this);
	    ufile->file=file;
	    return file;
	 } else url=((DjVuFile *) (DjVuPort *) port)->get_url();
      }
   }
   
   GP<DjVuFile> file=url_to_file(url, dont_create);
   if (file) 
     pcaster->add_route(file, this);
   return file;
}

GP<DjVuFile>
DjVuDocument::get_djvu_file(const char * id, bool dont_create)
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
         file->set_recover_errors(recover_errors);
         file->set_verbose_eof(verbose_eof);
	 file->init(url, this);
	 ufile->file=file;
	 return file;
      }
   }

   GP<DjVuFile> file=url_to_file(url, dont_create);
   if (file)
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

void
DjVuDocument::process_threqs(void)
      // Will look thru threqs_list and try to fulfil every request
{
   GCriticalSectionLock lock(&threqs_lock);
   for(GPosition pos=threqs_list;pos;)
   {
      GP<ThumbReq> req=threqs_list[pos];
      bool remove=false;
      if (req->thumb_file)
      {
	 TRY {
	       // There supposed to be a file with thumbnails
	    if (req->thumb_file->is_data_present())
	    {
		  // Cool we can extract the thumbnail now
	       GP<ByteStream> str=req->thumb_file->get_init_data_pool()->get_stream();
	       IFFByteStream iff(*str);
	       GString chkid;
	       if (!iff.get_chunk(chkid) || chkid!="FORM:THUM")
		  THROW("Corrupted thumbnails");
	       
	       for(int i=0;i<req->thumb_chunk;i++)
	       {
		  if (!iff.get_chunk(chkid)) THROW("Corrupted thumbnails");
		  iff.close_chunk();
	       }
	       if (!iff.get_chunk(chkid) || chkid!="TH44")
		  THROW("Corrupted thumbnails");

		  // Copy the data
	       char buffer[1024];
	       int length;
	       while((length=iff.read(buffer, 1024)))
		  req->data_pool->add_data(buffer, length);
	       req->data_pool->set_eof();

		  // Also add this file to cache so that we won't have
		  // to download it next time
	       add_to_cache(req->thumb_file);

	       req->thumb_file=0;
	       req->image_file=0;
	       remove=true;
	    }
	 } CATCH(exc) {
	    GString msg="Failed to extract predecoded thumbnails:\n";
	    msg+=exc.get_cause();
	    get_portcaster()->notify_error(this, msg);
	       // Switch this request to the "decoding" mode
	    req->image_file=get_djvu_file(req->page_num);
	    req->thumb_file=0;
	 } ENDCATCH;
      } // if (req->thumb_file)
      
      if (req->image_file)
      {
	 TRY {
	       // Decode the file if necessary. Or just used predecoded image.
	    GSafeFlags & file_flags=req->image_file->get_safe_flags();
	    {
	       GMonitorLock lock(&file_flags);
	       if (!req->image_file->is_decoding())
	       {
		  if (req->image_file->is_decode_ok())
		  {
			// We can generate it now
		     GP<DjVuImage> dimg=new DjVuImage;
		     dimg->connect(req->image_file);
      
		     GRect rect(0, 0, 160, dimg->get_height()*160/dimg->get_width());
		     GP<GPixmap> pm=dimg->get_pixmap(rect, rect, thumb_gamma);
		     if (!pm)
		     {
			GP<GBitmap> bm=dimg->get_bitmap(rect, rect, sizeof(int));
			pm=new GPixmap(*bm);
		     }
		     if (!pm) THROW("Unable to render page "+GString(req->page_num));
		     
			// Store and compress the pixmap
		     GP<IWPixmap> iwpix=new IWPixmap(pm);
		     GP<MemoryByteStream> str=new MemoryByteStream;
		     IWEncoderParms parms;
		     parms.slices=97;
		     parms.bytes=0;
		     parms.decibels=0;
		     iwpix->encode_chunk(*str, parms);
		     TArray<char> data=str->get_data();

		     req->data_pool->add_data((const char *) data, data.size());
		     req->data_pool->set_eof();
      
		     req->thumb_file=0;
		     req->image_file=0;
		     remove=true;
		  } else if (req->image_file->is_decode_failed())
		  {
			// Unfortunately we cannot decode it
		     req->thumb_file=0;
		     req->image_file=0;
		     remove=true;
		  } else req->image_file->start_decode();
	       }
	    }
	 } CATCH(exc) {
	    GString msg="Failed to decode thumbnails:\n";
	    msg+=exc.get_cause();
	    get_portcaster()->notify_error(this, msg);
	    
	       // Get rid of this request
	    req->image_file=0;
	    req->thumb_file=0;
	    remove=true;
	 } ENDCATCH;
      }

      if (remove)
      {
	 GPosition this_pos=pos;
	 ++pos;
	 threqs_list.del(this_pos);
      } else ++pos;
   }
}

GP<DjVuDocument::ThumbReq>
DjVuDocument::add_thumb_req(const GP<ThumbReq> & thumb_req)
      // Will look through the list of pending requests for thumbnails
      // and try to add the specified request. If a duplicate is found,
      // it will be returned and the list will not be modified
{
   GCriticalSectionLock lock(&threqs_lock);
   for(GPosition pos=threqs_list;pos;++pos)
   {
      GP<ThumbReq> req=threqs_list[pos];
      if (req->page_num==thumb_req->page_num)
	 return req;
   }
   threqs_list.append(thumb_req);
   return thumb_req;
}

GP<DataPool>
DjVuDocument::get_thumbnail(int page_num, bool dont_decode)
{
   DEBUG_MSG("DjVuDocument::get_thumbnail(): page_num=" << page_num << "\n");
   DEBUG_MAKE_INDENT(3);

   if (!is_init_complete()) return 0;
   
   {
	 // See if we already have request for this thumbnail pending
      GCriticalSectionLock lock(&threqs_lock);
      for(GPosition pos=threqs_list;pos;++pos)
      {
	 GP<ThumbReq> req=threqs_list[pos];
	 if (req->page_num==page_num)
	    return req->data_pool;	// That's it. Just return it.
      }
   }

      // No pending request for this page... Create one
   GP<ThumbReq> thumb_req=new ThumbReq(page_num, new DataPool());
   
      // First try to find predecoded thumbnail
   if (get_doc_type()==INDIRECT || get_doc_type()==BUNDLED)
   {
	 // Predecoded thumbnails exist for new formats only
      GPList<DjVmDir::File> files_list=djvm_dir->get_files_list();
      GP<DjVmDir::File> thumb_file;
      int thumb_start=0;
      int page_cnt=-1;
      for(GPosition pos=files_list;pos;++pos)
      {
	 GP<DjVmDir::File> f=files_list[pos];
	 if (f->is_thumbnails())
	 {
	    thumb_file=f;
	    thumb_start=page_cnt+1;
	 } else if (f->is_page()) page_cnt++;
	 if (page_cnt==page_num) break;
      }
      if (thumb_file)
      {
	    // That's the file with the desired thumbnail image
	 thumb_req->thumb_file=get_djvu_file(thumb_file->id);
	 thumb_req->thumb_chunk=page_num-thumb_start;
	 thumb_req=add_thumb_req(thumb_req);
	 process_threqs();
	 return thumb_req->data_pool;
      }
   }

      // Apparently we're out of luck and need to decode the requested
      // page (unless it's already done and if it's allowed) and render
      // it into the thumbnail. If dont_decode is true, do not attempt
      // to create this file (because this will result in a request for data)
   GP<DjVuFile> file=get_djvu_file(page_num, dont_decode);
   if (file)
   {
      thumb_req->image_file=file;

	 // I'm locking the flags here to make sure, that DjVuFile will not
	 // change its state in between of the checks.
      GSafeFlags & file_flags=file->get_safe_flags();
      {
	 GMonitorLock lock(&file_flags);
	 if (thumb_req->image_file->is_decode_ok() || !dont_decode)
	 {
	       // Just add it to the list and call process_threqs(). It
	       // will start decoding if necessary
	    thumb_req=add_thumb_req(thumb_req);
	    process_threqs();
	 } else
	 {
	       // Nothing can be done return ZERO
	    thumb_req=0;
	 }
      }
   } else thumb_req=0;
   
   if (thumb_req) return thumb_req->data_pool;
   else return 0;
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
   {
      set_file_aliases(source);
      if (cache) add_to_cache((DjVuFile *) source);
      process_threqs();
   }
   
   if (set_mask & DjVuFile::DATA_PRESENT)
      process_threqs();		// May be we can extract thumbnails now
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

   if (url==init_url) return init_data_pool;

   check();	// Don't put it before 'init_data_pool'

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
	 case SINGLE_PAGE:
	 case OLD_INDEXED:
	 case INDIRECT:
	 {
	    DEBUG_MSG("The document is in SINGLE_PAGE or OLD_INDEXED or INDIRECT format\n");
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

	       data_pool=new DataPool(fname);
	    }
	 }
      }
   return data_pool;
}

static GP<DataPool>
unlink_file(const GP<DataPool> & data, const char * name)
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
	 GP<DataPool> data=file->get_djvu_data(false, true);
	 for(pos=files_list;pos;++pos)
	 {
	    GP<DjVuFile> f=files_list[pos];
	    if (f->contains_chunk("NDIR"))
	       data=unlink_file(data, f->get_url().name());
	 }
	 
	    // Finally add it to the document
	 GString name=file->get_url().name();
	 GP<DjVmDir::File> file_rec=new DjVmDir::File(name, name, name,
						      page ? DjVmDir::File::PAGE :
						      DjVmDir::File::INCLUDE);
	 doc.insert_file(file_rec, data, -1);

	    // And repeat for all included files
	 for(pos=files_list;pos;++pos)
	    add_file_to_djvm(files_list[pos], false, doc, map);
      }
   }
}

static void
local_get_file_names
(DjVuFile * f,const GMap<GURL, void *> & map,GMap<GURL,void *> &tmpmap)
{
   GURL url=f->get_url();
   if (!map.contains(url) && !tmpmap.contains(url))
   {
      tmpmap[url]=0;
      f->process_incl_chunks();
      GPList<DjVuFile> files_list=f->get_included_files(false);
      for(GPosition pos=files_list;pos;++pos)
         local_get_file_names(files_list[pos], map, tmpmap);
   }
}

static void
local_get_file_names
(DjVuFile * f, GMap<GURL, void *> & map)
{
   GMap<GURL,void *> tmpmap;
   local_get_file_names(f,map,tmpmap);
   for(GPosition pos=tmpmap;pos;++pos)
     map[tmpmap.key(pos)]=0;
}

GList<GString>
DjVuDocument::get_file_names(void) 
{
  check();

  GCriticalSectionLock lock(&file_names_lock);
  if(has_file_names) return file_names;

  GMap<GURL, void *> map;
  int i;
  if (doc_type==BUNDLED || doc_type==INDIRECT || doc_type==SINGLE_PAGE)
  {
    GPList<DjVmDir::File> files_list=djvm_dir->get_files_list();
    for(GPosition pos=files_list;pos;++pos)
    {
      GURL url=id_to_url(files_list[pos]->id);
      map[url]=0;
    }
  }else
  {
    int pages_num=get_pages_num();
    for(i=0;i<pages_num;i++)
    {
      TRY
      {
        local_get_file_names(get_djvu_file(i), map);
      }
      CATCH(ex)
      {
        TRY { 
          get_portcaster()->notify_error(this, ex.get_cause()); 
          static const char emsg[]="Excluding page %d form the file list due to errors.\n";
          char buf[sizeof(emsg)+20];
          sprintf(buf,emsg,i+1);
          get_portcaster()->notify_error(this,buf); 
        }
        CATCH(exc)
        {
          RETHROW;
        }
        ENDCATCH;
      }
      ENDCATCH;
    }
  }
  for(GPosition j=map;j;++j)
  {
    if (map.key(j).is_local_file_url())
    {
      file_names.append(GOS::url_to_filename(map.key(j)));
    }
  }
  has_file_names=true;
  return file_names;
}

GP<DjVmDoc>
DjVuDocument::get_djvm_doc()
      // This function may block for data
{
   check();
   DEBUG_MSG("DjVuDocument::get_djvm_doc(): creating the DjVmDoc\n");
   DEBUG_MAKE_INDENT(3);

   if (!is_init_complete()) THROW("Document has not been completely initialized yet.");

   GP<DjVmDoc> doc=new DjVmDoc();

   if (doc_type==BUNDLED || doc_type==INDIRECT || doc_type==SINGLE_PAGE)
   {
      DEBUG_MSG("Trivial: the document is either INDIRECT or BUNDLED: follow DjVmDir.\n");

      GPList<DjVmDir::File> files_list=djvm_dir->get_files_list();
      for(GPosition pos=files_list;pos;++pos)
      {
	 GP<DjVmDir::File> f=new DjVmDir::File(*files_list[pos]);
	 GP<DjVuFile> file=url_to_file(id_to_url(f->id));
	 GP<DataPool> data;
	 if (file->is_modified()) data=file->get_djvu_data(false, true);
	 else data=file->get_init_data_pool();
	 doc->insert_file(f, data);
      }
   } else
   {
      DEBUG_MSG("Converting: the document is in an old format.\n");

      GMap<GURL, void *> map_add;
      if(recover_errors == ABORT)
      {
        for(int page_num=0;page_num<ndir->get_pages_num();page_num++)
        {
          GP<DjVuFile> file=url_to_file(ndir->page_to_url(page_num));
          add_file_to_djvm(file, true, *doc, map_add);
        }
      }else
      {
        for(int page_num=0;page_num<ndir->get_pages_num();page_num++)
        {
          TRY
          {
            GP<DjVuFile> file=url_to_file(ndir->page_to_url(page_num));
            add_file_to_djvm(file, true, *doc, map_add);
          }
          CATCH(ex)
          {
            TRY { 
              get_portcaster()->notify_error(this, ex.get_cause()); 
              static const char emsg[]="Skipping page %d due to errors.\n";
              char buf[sizeof(emsg)+20];
              sprintf(buf,emsg,page_num+1);
              get_portcaster()->notify_error(this,buf); 
            }
            CATCH(exc)
            {
              RETHROW;
            }
            ENDCATCH;
          }
          ENDCATCH;
        }
      }
   }
   return doc;
}

void
DjVuDocument::write(ByteStream & str, bool force_djvm)
{
   DEBUG_MSG("DjVuDocument::write(): storing DjVmDoc into ByteStream\n");
   DEBUG_MAKE_INDENT(3);
   
   GP<DjVmDoc> doc=get_djvm_doc();
   GP<DjVmDir> dir=doc->get_djvm_dir();
   if (force_djvm || dir->get_files_num()>1) doc->write(str);
   else
   {
      GPList<DjVmDir::File> files_list=dir->get_files_list();
      GP<DataPool> pool=doc->get_data(files_list[files_list]->id);
      GP<ByteStream> pool_str=pool->get_stream();
      str.copy(*pool_str);
   }
}

void
DjVuDocument::expand(const char * dir_name, const char * idx_name)
{
   DEBUG_MSG("DjVuDocument::expand(): dir_name='" << dir_name << "'\n");
   DEBUG_MAKE_INDENT(3);
   
   GP<DjVmDoc> doc=get_djvm_doc();
   doc->expand(dir_name, idx_name);
}

void
DjVuDocument::save_as
(const char where[], const bool bundled)
{
   DEBUG_MSG("DjVuDocument::save_as(): where='" << where <<
	     "', bundled=" << bundled << "\n");
   DEBUG_MAKE_INDENT(3);
   
   GString full_name=GOS::expand_name(where, GOS::cwd());
   
   if (bundled)
   {
      DataPool::load_file(full_name);
      StdioByteStream str(full_name, "wb");
      write(str);
   } else expand(GOS::dirname(full_name), GOS::basename(full_name));
}

