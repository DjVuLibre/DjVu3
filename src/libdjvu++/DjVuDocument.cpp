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
//C- $Id: DjVuDocument.cpp,v 1.1.2.2 1999-04-26 19:20:46 eaf Exp $

#ifdef __GNUC__
#pragma implementation
#endif

#include "DjVuDocument.h"
#include "IFFByteStream.h"
#include "GOS.h"
#include "DjVmFile.h"
#include "debug.h"

DjVuDocument::DjVuDocument(const GURL & url, bool xreadonly,
			   DjVuPort * xport, GCache<GURL, DjVuFile> * xcache):
      cache(xcache), simple_port(0), readonly(1), djvm(0)
{
   dir_url=url.baseURL();

   if (!xreadonly)
   {
	 // We want to keep loaded files in an unlimited private cache
	 // :(( Drawbacks of the design. It's hard to combine features
	 // useful for the plugin and the editor in one class
      cache=new GCache<GURL, DjVuFile>(-1);
   }
   
   DjVuPortcaster * pcaster=get_portcaster();
   if (xport) pcaster->add_route(this, xport);
   else
   {
      simple_port=new DjVuSimplePort();
      pcaster->add_route(this, simple_port);
   };

   detect_doc_type(url);
   
      // Create fake directory (with only one URL in it)
   GURL decode_url;
   if (is_djvm()) decode_url=djvm_get_first_page_url();
   else decode_url=url;
   dir=new DjVuNavDir(decode_url.baseURL()+"directory");
   dir->insert_page(-1, decode_url.fileURL());

   if (!xreadonly)
   {
	 // Get the REAL directory by decoding page #0
      GP<DjVuImage> dimg=get_page(0);
      dimg->get_djvu_file()->wait_for_finish();

	 // Now load each and every file into the cache
      int page;
      for(page=0;page<dir->get_pages_num();page++)
	 get_djvu_file(page);

      check_nav_structure();
   }

   readonly=xreadonly;
}

DjVuDocument::~DjVuDocument(void)
{
   delete simple_port; simple_port=0;
   if (!readonly) delete cache; cache=0;
}

void
DjVuDocument::detect_doc_type(const GURL & doc_url)
{
   DEBUG_MSG("DjVuDocument::detect_doc_type(): guessing what we're dealing with\n");
   DEBUG_MAKE_INDENT(3);
   
   DjVuPortcaster * pcaster=get_portcaster();
   GP<DataRange> data_range=pcaster->request_data(this, doc_url);
   if (!data_range) THROW("Failed to get data for URL '"+doc_url+"'");

   ByteStream * stream=0;

   TRY {
      stream=data_range->get_stream();
      IFFByteStream iff(*stream);
      GString chkid;
      int size;
      if ((size=iff.get_chunk(chkid)))
      {
	 if (size<0 || size>10*1024*1024)
	    THROW("The main stream is not IFF stream.");
	 if (chkid=="FORM:DJVM")
	 {
	    DEBUG_MSG("Got DJVM document here\n");
	    DEBUG_MAKE_INDENT(3);

	    djvm=1;
	    djvm_pool=data_range->get_pool();
	    djvm_doc_url=doc_url;

	       // Get the DJVM directory and offset to the 1st DJVU page
	    int first_page_offset=0;
	    while(1)
	    {
	       int offset;
	       size=iff.get_chunk(chkid, &offset);
	       if (chkid=="DIR0")
	       {
		  DEBUG_MSG("Got DIR0 chunk\n");
		  djvm_dir.read(&iff);
		  if (first_page_offset) break;
	       };
	       if (!first_page_offset &&
		   (chkid=="FORM:DJVU" ||
		    chkid=="FORM:PM44" ||
		    chkid=="FORM:BM44"))
	       {
		  DEBUG_MSG("Got 1st page offset=" << offset << "\n");
		  first_page_offset=offset;
		  if (djvm_dir.get_files_num()) break;
	       };
	       iff.close_chunk();
	    };
	 
	    if (!djvm_dir.get_files_num())
	       THROW("Didn't manage to find DIR0 chunk in given DJVM stream.");
	    if (!first_page_offset)
	       THROW("This document does not contain any DJVU pages.");

	       // Now get the name for the first DJVU page using the directory
	    for(int i=0;i<djvm_dir.get_files_num();i++)
	    {
	       DjVmDir0::FileRec & file=*djvm_dir.get_file(i);
	       if (file.offset==first_page_offset)
	       {
		  djvm_first_page_name=file.name;
		  break;
	       };
	    };
	    if (!djvm_first_page_name.length())
	       THROW("Failed to guess the name of the first DJVU page in this document.");
	 } else
	 {
	       // DJVU format
	    DEBUG_MSG("Got DJVU document here.\n");
	    djvm=0;
	 };
      } else THROW("Corrupt document file: not in IFF format.");
   } CATCH(exc) {
      delete stream; stream=0;
      if (!strstr(exc.get_cause(), "EOF")) RETHROW;
   } ENDCATCH;
   
   delete stream; stream=0;
}

GString
DjVuDocument::djvm_url2name(const GURL & url) const
{
   if (!is_djvm()) THROW("Internal error: the document format is not DJVM.");
   
   GString name;
   if (!strncmp(url, djvm_doc_url, strlen(djvm_doc_url)))
   {
      name=(const char *) url+strlen(djvm_doc_url);
      while(name[0]=='/') { GString tmp=(const char *) name+1; name=tmp; };
   }
   return name;
}

GURL
DjVuDocument::djvm_name2url(const char * name) const
{
   if (!is_djvm()) THROW("Internal error: the document format is not DJVM.");
   
   return djvm_doc_url+name;
}

bool
DjVuDocument::djvm_contains(const GURL & url) const
{
   if (!is_djvm()) THROW("Internal error: the document format is not DJVM.");

   DEBUG_MSG("url='" << url << "'\n");
   DEBUG_MSG("djvm_doc_url='" << djvm_doc_url << "'\n");
   return djvm && !strncmp(url, djvm_doc_url, strlen(djvm_doc_url));
}

GURL
DjVuDocument::djvm_get_first_page_url(void) const
{
   if (!is_djvm()) THROW("Internal error: the document format is not DJVM.");
   
   return djvm_doc_url+djvm_first_page_name;
}

static void
add_to_cache(const GP<DjVuFile> & f, GMap<GURL, void *> & map,
	     GCache<GURL, DjVuFile> * cache)
{
   GURL url=f->get_url();
   
   if (!map.contains(url))
   {
      map[url]=0;
      cache->add_item(url, f);	// Force overwrite even if item exists
      
      GPList<DjVuFile> list;
      for(GPosition pos=list;pos;++pos)
	 add_to_cache(list[pos], map, cache);
   }
}

void
DjVuDocument::add_to_cache(const GP<DjVuFile> & f)
{
   GMap<GURL, void *> map;
   ::add_to_cache(f, map, cache);
}

static void
unlink_empty_files(const GP<DjVuFile> & f, GMap<GURL, void *> & map,
		   GCache<GURL, DjVuFile> * cache)
{
   if (!map.contains(f->get_url()))
   {
      map[f->get_url()]=0;

      GPosition pos;
      GPList<DjVuFile> files=f->get_included_files();
      for(pos=files;pos;++pos)
	 unlink_empty_files(files[pos], map, cache);

      for(pos=files;pos;++pos)
      {
	 GP<DjVuFile> file=files[pos];
	 if (file->get_chunks_number()==0)
	 {
	    f->unlink_file(file->get_url().fileURL());
	    cache->del_item(file->get_url());
	 }
      }
   }
}

void
DjVuDocument::unlink_empty_files(void)
{
   DEBUG_MSG("DjVuDocument::unlink_empty_files(): getting rid of empty files\n");
   DEBUG_MAKE_INDENT(3);
   
   GMap<GURL, void *> map;
   for(int page=0;page<dir->get_pages_num();page++)
      ::unlink_empty_files(get_djvu_file(page), map, cache);
}

static void
get_shared_files(const GP<DjVuFile> & f, GMap<GURL, void *> & map)
{
   GURL url=f->get_url();
   
   if (!map.contains(url))
   {
      map[url]=0;
      GPList<DjVuFile> list=f->get_included_files();
      for(GPosition pos=list;pos;++pos)
	 get_shared_files(list[pos], map);
   }
}

GPList<DjVuFile>
DjVuDocument::get_shared_files(void)
      // Will return the list of files included into every page
{
   GMap<GURL, void *> total;
   for(int page=0;page<dir->get_pages_num();page++)
   {
      GMap<GURL, void *> map;
      GP<DjVuFile> file=get_djvu_file(page);
      ::get_shared_files(file, map);

      for(GPosition pos=map;pos;++pos)
      {
	 GURL url=map.key(pos);
	 GPosition tpos;
	 if (!total.contains(url, tpos)) total[url]=(void *) 1;
	 else total[tpos]=(void *) ((int) total[tpos]+1);
      }
   }
   GPList<DjVuFile> list;
   for(GPosition pos=total;pos;++pos)
   {
      GURL url=total.key(pos);
      if ((int) total[pos]==dir->get_pages_num())
	 list.append(get_djvu_file(url));
   }
   return list;
}

static void
delete_chunks(const GP<DjVuFile> & f, const char * chunk_name)
{
   GPList<DjVuFile> list=f->get_included_files();
   for(GPosition pos=list;pos;++pos)
      delete_chunks(list[pos], chunk_name);

   f->delete_chunks(chunk_name);
}

void
DjVuDocument::check_nav_structure(void)
      // The function will make sure, that there is one and only one file
      // included into every page containing one and only one NDIR chunk
{
   DEBUG_MSG("DjVuDocument::check_nav_structure() called\n");
   DEBUG_MAKE_INDENT(3);
   
   if (dir->get_pages_num()==1)
   {
	 // Get rid of all NDIR chunks anywhere
      DEBUG_MSG("only one page found => kill any NDIR chunks\n");
      dir_file=0;
      delete_chunks(get_djvu_file(0), "NDIR");
      unlink_empty_files();
   } else
   {
      DEBUG_MSG("more than one page exist => update NDIR\n");
      
	 // Get list of files included into every page
      GPList<DjVuFile> shared_files=get_shared_files();
      DEBUG_MSG("got " << shared_files.size() << " shared files\n");
   
	 // One of them may have NDIR chunk in it
      GPosition pos;
      for(pos=shared_files;pos;++pos)
	 if (shared_files[pos]->contains_chunk("NDIR")) break;
   
      if (pos)
      {
	    // Shared file with directory has been found
	    // Restore NDIR chunk in the file we found
	 DEBUG_MSG("found a shared file with NDIR chunk\n");

	 DEBUG_MSG("killing NDIR chunks everywhere\n");
	 for(int page=0;page<dir->get_pages_num();page++)
	    delete_chunks(get_djvu_file(page), "NDIR");
	 
	 DEBUG_MSG("updating NDIR chunk in the shared file '" << shared_files[pos]->get_url() << "'\n");
	 MemoryByteStream str;
	 dir->encode(str);
	 dir_file=shared_files[pos];
	 dir_file->insert_chunk(1, "NDIR", str.get_data());
	 dir_file->dir=dir;

	 unlink_empty_files();
      } else
      {
	    // No shared file with directory
	 DEBUG_MSG("no shared files with NDIR chunk found.\n");
	 
	 DEBUG_MSG("killing NDIR chunks everywhere\n");
	 for(int page=0;page<dir->get_pages_num();page++)
	    delete_chunks(get_djvu_file(page), "NDIR");

	 unlink_empty_files();
	 
	    // Create new directory file.
	 DEBUG_MSG("creating new shared file.\n");
	 GString name=tmpnam(0);
	 StdioByteStream str(name, "wb");
	 IFFByteStream iff(str);
	 iff.put_chunk("FORM:DJVI");
	 iff.put_chunk("NDIR");
	 dir->encode(iff);
	 iff.close_chunk();
	 iff.close_chunk();
	 dir_file=new DjVuFile(GOS::filename_to_url(name));
	 unlink(name);

	    // Now assign it a decent name
	 GURL url=dir->page_to_url(0);
	 char tst_name[128];
	 for(int i=0;;i++)
	 {
	    sprintf(tst_name, "dir%d", i);
	    if (!cache->get_item(url.baseURL()+tst_name)) break;
	 };
	 dir_file->set_name(tst_name);
	 
	    // Include it into every page
	 for(int page=0;page<dir->get_pages_num();page++)
	    get_djvu_file(page)->include_file(dir_file, 1);

	    // We don't want to add the file to the cache anywhere before
	    // 'cause its URL becomes OK only at this point
	 dir_file->change_cache(cache);
	 add_to_cache(dir_file);
      }
   }
}

void
DjVuDocument::insert_page(const GP<DjVuFile> & file, int page_num)
{
   DEBUG_MSG("DjVuDocument::insert_page(): page_num=" << page_num << "\n");
   DEBUG_MAKE_INDENT(3);

   if (readonly) THROW("The document has been created in readonly mode.");
   
   GString name=file->get_url().fileURL();
   if (dir->name_to_page(name)>=0)
      THROW("Can't insert page '"+name+"': already exists.");
   
   file->move(dir->page_to_url(0).baseURL());
   file->change_cache(cache);
   get_portcaster()->add_route(file, this);

   dir->insert_page(page_num, name);
   add_to_cache(file);

   if (dir_file) file->include_file(dir_file, 1);

   check_nav_structure();
}

void
DjVuDocument::delete_page(int page_num)
{
   DEBUG_MSG("DjVuDocument::delete_page(): page_num=" << page_num << "\n");
   DEBUG_MAKE_INDENT(3);

   if (readonly) THROW("The document has been created in readonly mode.");

   if (dir->get_pages_num()==1) THROW("Can't delete the last page.");

   GURL url=dir->page_to_url(page_num);
   dir->delete_page(page_num);
   cache->del_item(url);
   check_nav_structure();
}

GP<DataRange>
DjVuDocument::request_data(const DjVuPort * source, const GURL & url)
{
   DEBUG_MSG("DjVuDocument::request_data(): seeing if we can do it\n");
   DEBUG_MAKE_INDENT(3);

   if (readonly)
   {
      if (is_djvm())
      {
	 DEBUG_MSG("The document type is DJVM.\n");
	 if (djvm_contains(url))
	 {
	    GString name=djvm_url2name(url);
      
	    DEBUG_MSG("Yep. It's DjVm document and file name='" << name << "'\n");
      
	    GP<DjVmDir0::FileRec> file=djvm_dir.get_file(name);
	    if (!file) THROW(GString("File '")+name+"' is not in this DjVm document.");

	    DEBUG_MSG("found file at offset=" << file->offset << ", size=" << file->size << "\n");
	    return new DataRange(djvm_pool, file->offset, file->size);
	 }
      } else
      {
	 DEBUG_MSG("The document type is DJVU.\n");
	 if (url.isLocal())
	 {
	    GString fname=GOS::url_to_filename(url);
	    DEBUG_MSG("fname=" << fname << "\n");

	    GP<DataPool> pool=new DataPool();
	    StdioByteStream str(fname, "rb");
	    char buffer[1024];
	    int length;
	    while((length=str.read(buffer, 1024)))
	       pool->add_data(buffer, length);
	    pool->set_eof();
	    return new DataRange(pool);
	 };
      }
   } else
   {
      if (djvm)	// The document has originally been DjVm
      {
	 GP<DjVmDir0::FileRec> file=djvm_dir.get_file(url.fileURL());
	 if (file) return new DataRange(djvm_pool, file->offset, file->size);
      };
      if (url.isLocal())
      {
	 GString fname=GOS::url_to_filename(url);
	 DEBUG_MSG("fname=" << fname << "\n");

	 GP<DataPool> pool=new DataPool();
	 StdioByteStream str(fname, "rb");
	 char buffer[1024];
	 int length;
	 while((length=str.read(buffer, 1024)))
	    pool->add_data(buffer, length);
	 pool->set_eof();
	 return new DataRange(pool);
      }
   }
   DEBUG_MSG("Oops. Can't return the stream. Ask smb else.\n");
   return 0;
}

void
DjVuDocument::notify_chunk_done(const DjVuPort * source, const char * name)
{
   if (!strcmp(name, "NDIR"))
      if (source->inherits("DjVuFile"))
      {
	 DjVuFile * file=(DjVuFile *) source;
	 if (dir->get_pages_num()==1 && file->dir)
	 {
	    DEBUG_MSG("DjVuDocument::notify_chunk_done(): updating nav. directory\n");
	    dir=file->dir;
	 };
      };
}

GP<DjVuFile>
DjVuDocument::get_djvu_file(const GURL & url)
{
   DEBUG_MSG("DjVuDocument::get_djvu_file(): request for '" << url << "'\n");
   DEBUG_MAKE_INDENT(3);
   
   GP<DjVuFile> file;
   if (cache) file=cache->get_item(url);
   if (!file)
   {
      file=new DjVuFile(url, this, cache);
      if (cache) cache->add_item(url, file);
   };
   get_portcaster()->add_route(file, this);

   return file;
}

GP<DjVuFile>
DjVuDocument::get_djvu_file(int page_num)
{
   DEBUG_MSG("DjVuDocument::get_djvu_file(): request for page " << page_num << "\n");
   DEBUG_MAKE_INDENT(3);

   return get_djvu_file(dir->page_to_url(page_num));
}

GP<DjVuImage>
DjVuDocument::get_page(int page_num)
{
   DEBUG_MSG("DjVuDocument::get_page(): request for page " << page_num << "\n");
   DEBUG_MAKE_INDENT(3);
   
   GP<DjVuFile> file=get_djvu_file(page_num);
   if (!file->is_decoding() &&
       !file->is_decode_ok() &&
       !file->is_decode_failed())
      file->start_decode();
   
   return new DjVuImage(file);
}

GP<DjVmFile>
DjVuDocument::get_djvm_file(void)
{
   if (dir->get_pages_num()==1)
   {
	 // It's very likely, that we didn't decode the nav. directory yet.
      GP<DjVuFile> file=get_djvu_file(0);
      TArray<char> data=file->get_djvu_data(1, 0);

      MemoryByteStream str(data, data.size());
      IFFByteStream iff(str);
      
      int chksize;
      GString chkid;
      if (!iff.get_chunk(chkid)) THROW("File does not appear to be in IFF format.");

      while((chksize=iff.get_chunk(chkid)))
      {
	 if (chkid=="NDIR")
	 {
	    GP<DjVuNavDir> dir=new DjVuNavDir(file->get_url());
	    dir->decode(iff);
	    DjVuDocument::dir=dir;
	    break;
	 }
	 iff.close_chunk();
      }
   }

   GP<DjVmFile> djvm_file=new DjVmFile();
   
   for(int page=0;page<dir->get_pages_num();page++)
   {
      GP<DjVuFile> file=get_djvu_file(page);
      file->add_to_djvm(*djvm_file);
   }

   return djvm_file;
}

TArray<char>
DjVuDocument::get_djvm_data(void)
{
   DEBUG_MSG("DjVuDocument::get_djvm_data(): creating DJVM doc\n");
   DEBUG_MAKE_INDENT(3);

   GP<DjVmFile> djvm_file=get_djvm_file();

   TArray<char> data;
   GP<DjVmDir0> djvm_dir=djvm_file->get_djvm_dir();
   if (djvm_dir->get_files_num()==1)
   {
      TArray<char> tmp_data=djvm_file->get_file(djvm_dir->get_file(0)->name);
      data.resize(tmp_data.size()+3);
      memcpy(data, "AT&T", 4);
      memcpy((char *) data+4, tmp_data, tmp_data.size());
   } else djvm_file->write(data);
   
   return data;
}

void
DjVuDocument::save_as_djvm(const char * file_name)
{
   TArray<char> data=get_djvm_data();

   StdioByteStream str(file_name, "wb");
   str.writall(data, data.size());
}

void
DjVuDocument::save_as_djvu(const char * dir_name)
{
   DEBUG_MSG("DjVuDocument::save_as_djvu(): Saving into dir='" << dir_name << "'\n");
   DEBUG_MAKE_INDENT(3);

   GP<DjVmFile> djvm_file=get_djvm_file();

   djvm_file->expand(dir_name);
}
