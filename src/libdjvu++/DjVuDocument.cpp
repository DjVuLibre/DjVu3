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
//C- $Id: DjVuDocument.cpp,v 1.32 1999-09-10 21:52:36 eaf Exp $

#ifdef __GNUC__
#pragma implementation
#endif

#include "DjVuDocument.h"
#include "IFFByteStream.h"
#include "GOS.h"
#include "debug.h"


DjVuDocument::DjVuDocument(void)
  : initialized(false), cache(0), doc_type(BUNDLED), dummy_ndir(false)
{
}


void
DjVuDocument::init(const GURL & url, GP<DjVuPort> xport,
                   DjVuFileCache * xcache)
{
   if (initialized)
     THROW("DjVuDocument is already initialized");
   if (!get_count())
     THROW("DjVuDocument is not secured by a GP<DjVuDocument>");
   DEBUG_MSG("DjVuDocument::init(): initializing class...\n");
   DEBUG_MAKE_INDENT(3);
   // Initialize
   cache = xcache;
   doc_type = BUNDLED;
   dummy_ndir = true;
   init_url = url;
   DjVuPortcaster * pcaster=get_portcaster();
   if (!xport) 
     xport = simple_port = new DjVuSimplePort();
   pcaster->add_route(this, xport);
   pcaster->add_route(this, this);
   detect_doc_type(url);
   // Now we say it is ready
   initialized = true;
}

DjVuDocument::~DjVuDocument(void)
{
}


void
DjVuDocument::check() const
{
  if (!initialized)
    THROW("DjVuDocument is not initialized");
}


void
DjVuDocument::detect_doc_type(const GURL & doc_url)
{
   DEBUG_MSG("DjVuDocument::detect_doc_type(): guessing what we're dealing with\n");
   DEBUG_MAKE_INDENT(3);

   DjVuPortcaster * pcaster=get_portcaster();
   GP<DataPool> data_pool=pcaster->request_data(this, doc_url);
   if (!data_pool) 
      THROW("Failed to get data for URL '"+doc_url+"'");

   init_data_pool=data_pool;
   
   GP<ByteStream> stream=data_pool->get_stream();
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
      } else if (chkid=="DIR0")
      {
	 DEBUG_MSG("Got OLD_BUNDLED file.\n");
	 doc_type=OLD_BUNDLED;
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
	 GString first_page_name;
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

	    // Create dummy DjVuNavDir and insert the first page into it
	 ndir=new DjVuNavDir(init_url+"directory");
	 ndir->insert_page(-1, first_page_name);
      }
   } else // chkid!="FORM:DJVM"
   {
	 // DJVU format
      DEBUG_MSG("Got DJVU INDEXED document here.\n");
      doc_type=INDEXED;

	 // Create dummy DjVuNavDir and insert this page into it
      ndir=new DjVuNavDir(init_url.base()+"directory");
      ndir->insert_page(-1, init_url.name());
   }
}

void
DjVuDocument::decode_doc_structure(void)
{
   DEBUG_MSG("DjVuDocument::decode_doc_structure(): learning doc structure.\n");
   DEBUG_MAKE_INDENT(3);

   if (get_doc_type()==OLD_BUNDLED ||
       get_doc_type()==INDEXED)
      if (!ndir || dummy_ndir)
	 decode_ndir();
}

void
DjVuDocument::decode_ndir(void)
{
   DEBUG_MSG("DjVuDocument::decode_ndir(): decoding the navigation directory...\n");
   DEBUG_MAKE_INDENT(3);

   if (doc_type==BUNDLED || doc_type==INDIRECT)
      THROW("The document is in the new format, which doesn't use NDIR chunks.");

   if (!ndir || dummy_ndir)
   {
      GP<DjVuNavDir> dir=get_djvu_file(-1)->decode_ndir();
      if (dir)
      {
	 ndir=dir; dummy_ndir=false;
      }
   }
}

int
DjVuDocument::get_pages_num(void)
{
  if (!initialized)
    return 0;
   if (doc_type==BUNDLED || doc_type==INDIRECT)
      return djvm_dir->get_pages_num();
   else return ndir->get_pages_num();
}

GString
DjVuDocument::page_to_id(int page_num)
{
   check();
   if (page_num<0) THROW("Page number may not be negative.");
   if (page_num>=get_pages_num()) THROW("Page number is too big.");
   if (doc_type==BUNDLED || doc_type==INDIRECT)
      return djvm_dir->page_to_file(page_num)->id;
   else return ndir->page_to_name(page_num);
}

GString
DjVuDocument::page_to_name(int page_num)
{
   check();
   if (page_num<0) THROW("Page number may not be negative.");
   if (page_num>=get_pages_num()) THROW("Page number is too big.");
   if (doc_type==BUNDLED || doc_type==INDIRECT)
      return djvm_dir->page_to_file(page_num)->name;
   else return ndir->page_to_name(page_num);
}

GString
DjVuDocument::page_to_title(int page_num)
{
   check();
   if (page_num<0) THROW("Page number may not be negative.");
   if (page_num>=get_pages_num()) THROW("Page number is too big.");
   if (doc_type==BUNDLED || doc_type==INDIRECT)
      return djvm_dir->page_to_file(page_num)->title;
   else return ndir->page_to_name(page_num);
}

GURL
DjVuDocument::page_to_url(int page_num)
{
   check();
   if (page_num<0) THROW("Page number may not be negative.");
   if (page_num>=get_pages_num()) THROW("Page number is too big.");

   GURL url;
   switch(doc_type)
   {
      case OLD_BUNDLED:
      case INDEXED:
      {
	 url=ndir->page_to_url(page_num);
	 break;
      }
      case BUNDLED:
      {
	 url=init_url+djvm_dir->page_to_file(page_num)->name;
	 break;
      }
      case INDIRECT:
      {
	 url=init_url.base()+djvm_dir->page_to_file(page_num)->name;
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
   int page_num=-1;
   switch(doc_type)
   {
      case OLD_BUNDLED:
      case INDEXED:
      {
	 page_num=ndir->url_to_page(url);
	 break;
      }
      case BUNDLED:
      {
	 GP<DjVmDir::File> file;
	 if (url.base()==init_url) file=djvm_dir->name_to_file(url.name());
	 if (file) page_num=file->get_page_num();
	 break;
      }
      case INDIRECT:
      {
	 GP<DjVmDir::File> file;
	 if (url.base()==init_url.base()) file=djvm_dir->name_to_file(url.name());
	 if (file) page_num=file->get_page_num();
	 break;
      }
      default:
	 THROW("Unknown document type.");
   };
   return page_num;
}

static void
add_to_cache(const GP<DjVuFile> & f, GMap<GURL, void *> & map,
	     DjVuFileCache * cache)
{
   GURL url=f->get_url();
   
   if (!map.contains(url))
   {
      map[url]=0;
      cache->add_file(f);	// Force overwrite even if item exists
      
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

GURL
DjVuDocument::id_to_url(const char * id)
{
   check();
   DEBUG_MSG("DjVuDocument::id_to_url(): translating ID='" << id << "' to URL\n");
   DEBUG_MAKE_INDENT(3);

   switch(doc_type)
   {
      case BUNDLED:
      {
	 GP<DjVmDir::File> file=djvm_dir->id_to_file(id);
	 if (file) return init_url+file->name;
	 else break;
      }
      case INDIRECT:
      {
	 GP<DjVmDir::File> file=djvm_dir->id_to_file(id);
	 if (file) return init_url.base()+file->name;
	 else break;
      }
      case OLD_BUNDLED:
	 return init_url+id;
      case INDEXED:
	 return init_url.base()+id;
   }
   return GURL();
}

GURL
DjVuDocument::id_to_url(const DjVuPort * source, const char * id)
{
   return id_to_url(id);
}

GPBase
DjVuDocument::get_cached_file(const DjVuPort * source, const GURL & url)
      /* There are two caches in use:
	   1. One of them is global. It's passed from outside and is used
	      to cache fully decoded pages, shared between all documents.
	   2. The other one is local. Any file created inside this document
	      is added to it.
	      
	 The difference between these two caches is that global cache
	 *holds* the files, that is it keeps GP<> pointers and prevents
	 even unused DjVuFiles from destruction (as long as the cache is
	 not full). The local cache just keeps track of still alive files,
	 which have been created inside this document. As soon as a file
	 is destroyed, it's removed from the local cache too.
	 Thus we manage to reuse most of the files created inside this
	 or another document. */
{
   DEBUG_MSG("DjVuDocument::get_cached_file(): url='" << url << "'\n");
   DEBUG_MAKE_INDENT(3);

   GP<DjVuFile> file;
   
      // First - check if there is a file with this URL globally registered
   GP<DjVuPort> file_port=get_portcaster()->name_to_port(url);
   if (file_port && file_port->inherits("DjVuFile"))
      file=(DjVuFile *) (DjVuPort *) file_port;
   DEBUG_MSG("found file in the global cache=" << (file!=0) << "\n");

   if (!file)
   {
         // Next - check files, that have ever been created inside this
	 // document, and are still alive.
      GCriticalSectionLock lock(&active_files_lock);
      GPosition pos;
      if (active_files.contains(url, pos)) file=(DjVuFile *) active_files[pos];
      DEBUG_MSG("found file in the local cache=" << (file!=0) << "\n");
   } else
   {
	 // Heh. Add the file retrieved from the global cache to the local one.
      GCriticalSectionLock lock(&active_files_lock);
      if (!active_files.contains(file->get_url()))
      {
	 file->add_destroy_cb(static_destroy_cb, this);
	 active_files[file->get_url()]=file;
      }
   }

   return (DjVuFile *) file;
}

void
DjVuDocument::cache_djvu_file(const DjVuPort * source, DjVuFile * file)
{
   if (cache && file->is_decode_ok()) add_to_cache(file);

   GCriticalSectionLock lock(&active_files_lock);
   if (!active_files.contains(file->get_url()))
   {
      file->add_destroy_cb(static_destroy_cb, this);
      active_files[file->get_url()]=file;
   }
}

GP<DataPool>
DjVuDocument::request_data(const DjVuPort * source, const GURL & url)
{
   DEBUG_MSG("DjVuDocument::request_data(): seeing if we can do it\n");
   DEBUG_MAKE_INDENT(3);

   if (source==this) return 0;
   
   switch(doc_type)
   {
      case OLD_BUNDLED:
      {
	 DEBUG_MSG("The document is in OLD_BUNDLED format\n");
	 if (url.base()!=init_url)
	    THROW("URL '"+url+"' points outside of the bundled document.");
	 
	 GP<DjVmDir0::FileRec> file=djvm_dir0->get_file(url.name());
	 if (!file) THROW("File '"+url.name()+"' is not in this bundle.");
	 return new DataPool(init_data_pool, file->offset, file->size);
      }
      case BUNDLED:
      {
	 DEBUG_MSG("The document is in new BUNDLED format\n");
	 if (url.base()!=init_url)
	    THROW("URL '"+url+"' points outside of the bundled document.");
	 
	 GP<DjVmDir::File> file=djvm_dir->name_to_file(url.name());
	 if (!file) THROW("File '"+url.name()+"' is not in this bundle.");
	 return new DataPool(init_data_pool, file->offset, file->size);
      }
      case INDEXED:
      case INDIRECT:
      {
	 DEBUG_MSG("The document is in INDEXED or INDIRECT format\n");
	 if (url.base()!=init_url.base())
	    THROW("URL '"+url+"' points outside of the document's directory.");
	 if (doc_type==INDIRECT && !djvm_dir->name_to_file(url.name()))
	    THROW("URL '"+url+"' points outside of the INDIRECT document.");
	 
	 if (url.is_local_file_url())
	 {
	    GString fname=GOS::url_to_filename(url);
	    if (GOS::basename(fname)=="-") fname="-";
	    DEBUG_MSG("fname=" << fname << "\n");

	    GP<DataPool> pool=new DataPool();
	    StdioByteStream str(fname, "rb");
	    char buffer[1024];
	    int length;
	    while((length=str.read(buffer, 1024)))
	       pool->add_data(buffer, length);
	    pool->set_eof();
	    return pool;
	 }
      }
   }
   DEBUG_MSG("Oops. Can't return the stream. Ask smb else.\n");
   return 0;
}

void
DjVuDocument::notify_chunk_done(const DjVuPort * source, const char * name)
{
   if (doc_type==OLD_BUNDLED || doc_type==INDEXED)
      if (!strcmp(name, "NDIR") && (!ndir || dummy_ndir))
	 if (source->inherits("DjVuFile"))
	 {
	    GP<DjVuNavDir> dir=((DjVuFile *) source)->find_ndir();
	    if (dir)
	    {
	       DEBUG_MSG("DjVuDocument::notify_chunk_done(): updating nav. dir.\n");
	       ndir=dir;
	       dummy_ndir=false;
	    }
	 }
}

void
DjVuDocument::notify_file_flags_changed(const DjVuFile * source,
					long set_mask, long clr_mask)
{
   if (set_mask & DjVuFile::ALL_DATA_PRESENT)
      if (doc_type==OLD_BUNDLED || doc_type==INDEXED)
	 if (!ndir || dummy_ndir)
	 {
	    DEBUG_MSG("DjVuDocument::notify_file_flags_changed(): ALL_DATA_PRESENT: updating nav. dir.\n");
	    GP<DjVuNavDir> dir=((DjVuFile *) source)->find_ndir();
	    if (dir)
	    {
	       ndir=dir;
	       dummy_ndir=false;
	    }
	 }

   if (set_mask & DjVuFile::DECODE_OK)
      get_portcaster()->set_name(source, source->get_url());
}

GP<DjVuFile>
DjVuDocument::get_djvu_file(const GURL & url)
{
   check();
   DEBUG_MSG("DjVuDocument::get_djvu_file(): request for '" << url << "'\n");
   DEBUG_MAKE_INDENT(3);

   DjVuPortcaster * pcaster=get_portcaster();
   
   GPBase tmpfile=pcaster->get_cached_file(this, url);
   GP<DjVuFile> file=(DjVuFile *) tmpfile.get();
   if (!file) 
     {
       file= new DjVuFile;
       file->init(url, this);
     }
   else
   {
      pcaster->add_route(file, this);
      if (!ndir || dummy_ndir)
	 if (file->is_all_data_present())	// Check to avoid deadlocks
	 {
	    GP<DjVuNavDir> dir=file->find_ndir();
	    if (dir)
	    {
	       DEBUG_MSG("updating nav. directory from the cached file.\n");
	       ndir=dir;
	       dummy_ndir=false;
	    }
	 }
   }
   
   return file;
}

GP<DjVuFile>
DjVuDocument::get_djvu_file(int page_num)
{
   DEBUG_MSG("DjVuDocument::get_djvu_file(): request for page " << page_num << "\n");
   DEBUG_MAKE_INDENT(3);

   GURL url;
   if (page_num>=0) url=page_to_url(page_num);
   else
   {
      if (doc_type==BUNDLED || doc_type==INDIRECT) url=page_to_url(0);
      else if (doc_type==OLD_BUNDLED) url=ndir->page_to_url(0);
      else url=init_url;
   }
   return get_djvu_file(url);
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

void
DjVuDocument::static_destroy_cb(const DjVuFile * file, void * cl_data)
{
   DjVuDocument * th=(DjVuDocument *) cl_data;
   if (get_portcaster()->is_port_alive(th)) th->file_destroyed(file);
}

void
DjVuDocument::file_destroyed(const DjVuFile * file)
{
   DEBUG_MSG("DjVuDocument::file_destroyed(): file '" << file->get_url() << "' is about to destroy\n");
   DEBUG_MAKE_INDENT(3);

   GCriticalSectionLock lock(&active_files_lock);
   GPosition pos;
   if (active_files.contains(file->get_url(), pos)) active_files.del(pos);
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
{
   check();
   DEBUG_MSG("DjVuDocument::get_djvm_doc(): creating the DjVmDoc\n");
   DEBUG_MAKE_INDENT(3);

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
	 GP<DjVuFile> file=get_djvu_file(id_to_url(f->id));
	 doc->insert_file(f, file->get_data_pool());
      }
   } else
   {
      DEBUG_MSG("Converting: the document is in an old format.\n");

      GMap<GURL, void *> map_add;
      if (dummy_ndir) decode_ndir();
      for(int page_num=0;page_num<ndir->get_pages_num();page_num++)
      {
	 GP<DjVuFile> file=get_djvu_file(ndir->page_to_url(page_num));
	 GMap<GURL, void *> map_del;
	 add_file_to_djvm(file, true, *doc, map_add);
      }
   }
   return doc;
}

void
DjVuDocument::write(ByteStream & str)
{
   GP<DjVmDoc> doc=get_djvm_doc();
   doc->write(str);
}

void
DjVuDocument::expand(const char * dir_name, const char * idx_name)
{
   GP<DjVmDoc> doc=get_djvm_doc();
   doc->expand(dir_name, idx_name);
}
