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
//C- $Id: DjVuDocument.cpp,v 1.1.2.1 1999-04-12 16:48:21 eaf Exp $

#ifdef __GNUC__
#pragma implementation
#endif

#include "DjVuDocument.h"
#include "IFFByteStream.h"
#include "GOS.h"
#include "DjVmFile.h"
#include "debug.h"

DjVuDocument::DjVuDocument(const GURL & xurl, DjVuPort * xport,
			   GCache<GURL, DjVuFile> * xcache):
      init_url(xurl), cache(xcache), simple_port(0), djvm(0)
{
   DjVuPortcaster * pcaster=get_portcaster();
   if (xport) pcaster->add_route(this, xport);
   else
   {
      simple_port=new DjVuSimplePort();
      pcaster->add_route(this, simple_port);
   };

   detect_doc_type();
   
      // Create fake directory (with only one URL in it)
   GURL decode_url;
   if (is_djvm()) decode_url=djvm_get_first_page_url();
   else decode_url=init_url;
   dir=new DjVuNavDir(decode_url.baseURL()+"directory");
   dir->insert_page(-1, decode_url.fileURL());
}

DjVuDocument::~DjVuDocument(void)
{
   delete simple_port;
}

void
DjVuDocument::detect_doc_type(void)
{
   DEBUG_MSG("DjVuDocument::detect_doc_type(): guessing what we're dealing with\n");
   DEBUG_MAKE_INDENT(3);
   
   DjVuPortcaster * pcaster=get_portcaster();
   GP<DataRange> data_range=pcaster->request_data(this, init_url);

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
	    djvu_pool=data_range->get_pool();
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
   if (!strncmp(url, init_url, strlen(init_url)))
   {
      name=(const char *) url+strlen(init_url);
      while(name[0]=='/') { GString tmp=(const char *) name+1; name=tmp; };
   }
   return name;
}

GURL
DjVuDocument::djvm_name2url(const char * name) const
{
   if (!is_djvm()) THROW("Internal error: the document format is not DJVM.");
   
   return init_url+name;
}

bool
DjVuDocument::djvm_contains(const GURL & url) const
{
   if (!is_djvm()) THROW("Internal error: the document format is not DJVM.");
   
   return djvm && !strncmp(url, init_url, strlen(init_url));
}

GURL
DjVuDocument::djvm_get_first_page_url(void) const
{
   if (!is_djvm()) THROW("Internal error: the document format is not DJVM.");
   
   return init_url+djvm_first_page_name;
}

GP<DataRange>
DjVuDocument::request_data(const DjVuPort * source, const GURL & url)
{
   DEBUG_MSG("DjVuDocument::request_data(): seeing if we can do it\n");
   DEBUG_MAKE_INDENT(3);
   
   if (is_djvm() && djvm_contains(url))
   {
      GString name=djvm_url2name(url);
      
      DEBUG_MSG("Yep. It's DjVm document and file name='" << name << "'\n");
      
      GP<DjVmDir0::FileRec> file=djvm_dir.get_file(name);
      if (!file) THROW(GString("File '")+name+"' is not in this DjVm document.");

      DEBUG_MSG("found file at offset=" << file->offset << ", size=" << file->size << "\n");
      return new DataRange(djvm_pool, file->offset, file->size);
   } else
   {
      DEBUG_MSG("The document type is DJVU.\n");
      if (url==init_url) return new DataRange(djvu_pool);
      else if (url.isLocal())
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
   };
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
   djvm_file->write(data);
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
