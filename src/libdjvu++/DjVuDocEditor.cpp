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
//C- $Id: DjVuDocEditor.cpp,v 1.5 1999-11-20 07:11:24 bcr Exp $

#ifdef __GNUC__
#pragma implementation
#endif

#include "DjVuDocEditor.h"
#include "GOS.h"
#include "debug.h"

void
DjVuDocEditor::check(void)
{
   if (!initialized) THROW("DjVuDocEditor should have been initialized before used.");
}

DjVuDocEditor::DjVuDocEditor(void)
{
   initialized=false;
}

DjVuDocEditor::~DjVuDocEditor(void)
{
   if (tmp_doc_name.length()) GOS::deletefile(tmp_doc_name);
}

void
DjVuDocEditor::init(void)
{
   DEBUG_MSG("DjVuDocEditor::init() called\n");
   DEBUG_MAKE_INDENT(3);
   
   doc_url=GOS::filename_to_url(GOS::expand_name("noname.djvu", GOS::cwd()));
   
   DjVmDoc doc;
   MemoryByteStream str;
   doc.write(str);
   str.seek(0, SEEK_SET);
   doc_pool=new DataPool(str);

   orig_doc_type=UNKNOWN_TYPE;
   orig_doc_pages=0;
   
   initialized=true;

   DjVuDocument::init(doc_url, this);
   wait_for_complete_init();
}

void
DjVuDocEditor::init(const char * fname)
{
   DEBUG_MSG("DjVuDocEditor::init() called: fname='" << fname << "'\n");
   DEBUG_MAKE_INDENT(3);
   
      // First - create a temporary DjVuDocument and check its type
   doc_pool=new DataPool(fname);
   doc_url=GOS::filename_to_url(fname);
   GP<DjVuDocument> tmp_doc=new DjVuDocument();
   tmp_doc->init(doc_url, this);
   tmp_doc->wait_for_complete_init();
   if (!tmp_doc->is_init_complete())
      THROW(GString("Failed to open document '")+fname+"'\n");
   
   orig_doc_type=tmp_doc->get_doc_type();
   orig_doc_pages=tmp_doc->get_pages_num();
   if (orig_doc_type==OLD_BUNDLED ||
       orig_doc_type==OLD_INDEXED ||
       orig_doc_type==SINGLE_PAGE)
   {
	 // Suxx. I need to convert it NOW.
	 // We will unlink this file in the destructor
      tmp_doc_name=tmpnam(0);
      StdioByteStream str(tmp_doc_name, "wb");
      tmp_doc->write(str, true);	// Force DJVM format
      str.flush();
      doc_pool=new DataPool(tmp_doc_name);
   }

      // OK. Now doc_pool contains data of the document in one of the
      // new formats. It will be a lot easier to insert/delete pages now.

      // 'doc_url' below of course doesn't refer to the file with the converted
      // data, but we will take care of it by redirecting the request_data().
   initialized=true;
   DjVuDocument::init(doc_url, this);
   wait_for_complete_init();
}

GP<DataPool>
DjVuDocEditor::request_data(const DjVuPort * source, const GURL & url)
{
   DEBUG_MSG("DjVuDocEditor::request_data(): url='" << url << "'\n");
   DEBUG_MAKE_INDENT(3);
   
      // Check if we have either original data or converted (to new format),
      // if all the story is about the DjVuDocument's data
   if (url==doc_url) return doc_pool;

      // Now see if we have any file matching the url
   {
      GCriticalSectionLock lock(&files_lock);
      GPosition pos;
      if (files_map.contains(url, pos))
      {
	 GP<File> f=files_map[pos];
	 if (f->file && f->file->get_init_data_pool())
	    return f->file->get_init_data_pool();// Favor DjVuFile's knowledge
	 else if (f->pool) return f->pool;
      }
   }

      // Finally let DjVuDocument cope with it. It may be a connected DataPool
      // for a BUNDLED format. Or it may be a file. Anyway, it was not
      // manually included, so it should be in the document.
   GP<DataPool> pool=DjVuDocument::request_data(source, url);

      // We do NOT update the 'File' structure, because our rule is that
      // we keep a separate copy of DataPool in 'File' only if it cannot
      // be retrieved from DjVuDocument (like it has been "inserted" or
      // corresponds to a modified file).
   return pool;
}

void
DjVuDocEditor::clean_files_map(void)
{
   DEBUG_MSG("DjVuDocEditor::clean_files_map() called\n");
   DEBUG_MAKE_INDENT(3);

   GCriticalSectionLock lock(&files_lock);

      // See if there are too old items in the "cache", which are
      // not referenced by anyone. If the corresponding DjVuFile has been
      // modified, obtain the new data and replace the 'pool'. Clear the
      // DjVuFile anyway. If both DataPool and DjVuFile are zero, remove
      // the entry.
   for(GPosition pos=files_map;pos;)
   {
      GP<File> f=files_map[pos];
      if (f->file && f->file->get_count()==1)
      {
	 DEBUG_MSG("ZEROing file '" << f->file->get_url() << "'\n");
	 if (f->file->is_modified())
	    f->pool=f->file->get_djvu_data(false, true);
	 f->file=0;
      }
      if (!f->file && !f->pool)
      {
	 DEBUG_MSG("Removing record '" << files_map.key(pos) << "'\n");
	 GPosition this_pos=pos;
	 ++pos;
	 files_map.del(this_pos);
      } else ++pos;
   }
}

GP<DjVuFile>
DjVuDocEditor::url_to_file(const GURL & url, bool dont_create)
{
   DEBUG_MSG("DjVuDocEditor::url_to_file(): url='" << url << "'\n");
   DEBUG_MAKE_INDENT(3);
   
      // Check if have a DjVuFile with this url cached (created before
      // and either still active or left because it has been modified)
   {
      GCriticalSectionLock lock(&files_lock);
      GPosition pos;
      if (files_map.contains(url, pos))
      {
	 GP<File> f=files_map[pos];
	 if (f->file) return f->file;
      }
   }

   clean_files_map();

      // We don't have the file cached. Let DjVuDocument create the file.
   GP<DjVuFile> file=DjVuDocument::url_to_file(url, dont_create);

      // And add it to our private "cache"
   if (file)
   {
      GCriticalSectionLock lock(&files_lock);
      GPosition pos;
      if (files_map.contains(url, pos))
	 files_map[url]->file=file;
      else
      {
	 GP<File> f=new File();
	 f->file=file;
	 files_map[url]=f;
      }
   }
   
   return file;
}

GString
DjVuDocEditor::find_unique_id(const char * id_in)
{
   GP<DjVmDir> dir=get_djvm_dir();

   GString base, ext;
   const char * dot=strrchr(id_in, '.');
   if (dot)
   {
      base=GString(id_in, dot-id_in);
      ext=dot+1;
   } else base=id_in;

   GString id=id_in;
   int cnt=0;
   while(1)
   {
      if (!dir->id_to_file(id) &&
	  !dir->name_to_file(id) &&
	  !dir->title_to_file(id)) break;
      cnt++;
      id=base+"_"+GString(cnt);
      if (ext.length()) id+="."+ext;
   }
   return id;
}

GP<DataPool>
DjVuDocEditor::strip_incl_chunks(const GP<DataPool> & pool_in)
{
   DEBUG_MSG("DjVuDocEditor::strip_incl_chunks() called\n");
   DEBUG_MAKE_INDENT(3);
   
   GP<ByteStream> str_in=pool_in->get_stream();
   IFFByteStream iff_in(*str_in);

   MemoryByteStream str_out;
   IFFByteStream iff_out(str_out);

   bool have_incl=false;
   
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
	 } else have_incl=true;
	 iff_in.close_chunk();
      }
      iff_out.close_chunk();
   }

   if (have_incl)
   {
      str_out.seek(0, SEEK_SET);
      return new DataPool(str_out);
   } else return pool_in;
}

GString
DjVuDocEditor::insert_file(const char * file_name, const char * parent_id,
			   int chunk_num)
{
   DEBUG_MSG("DjVuDocEditor::insert_file(): fname='" << file_name <<
	     "', parent_id='" << parent_id << "'\n");
   DEBUG_MAKE_INDENT(3);
   
   GP<DjVmDir> dir=get_djvm_dir();

      // Create DataPool and see if the file exists
   GP<DataPool> file_pool=new DataPool(file_name);

      // Strip any INCL chunks
   file_pool=strip_incl_chunks(file_pool);
   
      // Check if parent ID is valid
   GP<DjVmDir::File> parent_frec=dir->id_to_file(parent_id);
   if (!parent_frec) parent_frec=dir->name_to_file(parent_id);
   if (!parent_frec) parent_frec=dir->title_to_file(parent_id);
   if (!parent_frec)
      THROW(GString("There is no file with ID '")+parent_id+"' in this document.");
   GP<DjVuFile> parent_file=get_djvu_file(parent_id);
   if (!parent_file) THROW(GString("Failed to create file with ID '")+parent_id+"'\n");

      // Now obtain ID for the new file
   GString id=find_unique_id(GOS::basename(file_name));

      // Add it into the directory
   GP<DjVmDir::File> frec=new DjVmDir::File(id, id, id, DjVmDir::File::INCLUDE);
   int pos=dir->get_file_pos(parent_frec);
   if (pos>=0) ++pos;
   dir->insert_file(frec, pos);

      // Add it to the list
   {
      GURL file_url=id_to_url(id);
      GP<File> f=new File;
      f->pool=file_pool;
      GCriticalSectionLock lock(&files_lock);
      files_map[file_url]=f;
   }

      // And insert it into the parent DjVuFile
   parent_file->insert_file(id, chunk_num);

   return id;
}

GString
DjVuDocEditor::insert_page(const char * file_name, int page_num)
{
   DEBUG_MSG("DjVuDocEditor::insert_page(): fname='" << file_name << "'\n");
   DEBUG_MAKE_INDENT(3);
   
   GP<DjVmDir> dir=get_djvm_dir();

      // Create DataPool and see if the file exists
   GP<DataPool> file_pool=new DataPool(file_name);

      // Strip any INCL chunks
   file_pool=strip_incl_chunks(file_pool);

      // Choose an ID, which is not in the directory yet
   GString id=find_unique_id(GOS::basename(file_name));

      // Create a file record with the chosen ID
   GP<DjVmDir::File> file=new DjVmDir::File(id, id, id, DjVmDir::File::PAGE);

      // And insert it into the directory
   if (page_num<0 || page_num>=dir->get_pages_num())
      dir->insert_file(file, -1);
   else
   {
      int file_num=dir->get_page_pos(page_num);
      dir->insert_file(file, file_num);
   }

      // Now add the File record (containing the file URL and DataPool)
   GURL file_url=id_to_url(id);
   GP<File> f=new File;
   f->pool=file_pool;
   GCriticalSectionLock lock(&files_lock);
   files_map[file_url]=f;

      // At this moment the page is considered "added" because DjVuDocument
      // knows about it from DjVmDir, and we will provide data for it
      // as soon as somebody requests it.
   return id;
}

void
DjVuDocEditor::generate_thumbnails(int thumb_size, int images_per_file,
				   void (* cb)(int page_num, void *),
				   void * cl_data)
{
   DEBUG_MSG("DjVuDocEditor::generate_thumbnails(): doing it\n");
   DEBUG_MAKE_INDENT(3);

   GP<DjVmDir> djvm_dir=get_djvm_dir();
   
   DEBUG_MSG("removing any existing thumbnails\n");
   GPList<DjVmDir::File> xfiles_list=djvm_dir->get_files_list();
   for(GPosition pos=xfiles_list;pos;++pos)
   {
      GP<DjVmDir::File> f=xfiles_list[pos];
      if (f->is_thumbnails()) djvm_dir->delete_file(f->id);
   }

   DEBUG_MSG("creating new thumbnails\n");
   int page_num, pages_num=djvm_dir->get_pages_num();
   GPArray<MemoryByteStream> thumb_str(pages_num-1);
   for(page_num=0;page_num<pages_num;page_num++)
   {
      if (cb) cb(page_num, cl_data);
      
      GP<DjVuImage> dimg=get_page(page_num);
      dimg->wait_for_complete_decode();
      
      GRect rect(0, 0, thumb_size, dimg->get_height()*thumb_size/dimg->get_width());
      GP<GPixmap> pm=dimg->get_pixmap(rect, rect, get_thumbnails_gamma());
      if (!pm)
      {
	 GP<GBitmap> bm=dimg->get_bitmap(rect, rect, sizeof(int));
	 pm=new GPixmap(*bm);
      }
      if (!pm) THROW("Unable to render image of page "+GString(page_num));
      
	 // Store and compress the pixmap
      GP<IWPixmap> iwpix=new IWPixmap(pm);
      GP<MemoryByteStream> str=new MemoryByteStream;
      IWEncoderParms parms;
      parms.slices=97;
      parms.bytes=0;
      parms.decibels=0;
      iwpix->encode_chunk(*str, parms);
      str->seek(0);
      thumb_str[page_num]=str;
   }

   DEBUG_MSG("creating DjVuFiles to contain new thumbnails...\n");
   
      // The first thumbnail file always contains only one thumbnail
   int ipf=1;
   int image_num=0;
   page_num=0;
   GP<MemoryByteStream> str=new MemoryByteStream;
   GP<IFFByteStream> iff=new IFFByteStream(*str);
   iff->put_chunk("FORM:THUM");
   while(true)
   {
      iff->put_chunk("TH44");
      iff->copy(*thumb_str[page_num]);
      iff->close_chunk();
      image_num++;
      page_num++;
      if (image_num>=ipf || page_num>=pages_num)
      {
	    // Get unique ID for this file
	 GString id=find_unique_id("thumb");

	    // Create a file record with the chosen ID
	 GP<DjVmDir::File> file=new DjVmDir::File(id, id, id, DjVmDir::File::THUMBNAILS);

	    // Set correct file position (so that it will cover the next
	    // ipf pages)
	 int file_pos=djvm_dir->get_page_pos(page_num-image_num);
	 djvm_dir->insert_file(file, file_pos);

	    // Now add the File record (containing the file URL and DataPool)
	    // After we do it a simple save_as() will save the document
	    // with the thumbnails. This is because DjVuDocument will see
	    // the file in DjVmDir and will ask for data. We will intercept
	    // the request for data and will provide this DataPool
	 iff->close_chunk();
	 str->seek(0);
	 GP<DataPool> file_pool=new DataPool(*str);
	 GURL file_url=id_to_url(id);
	 GP<File> f=new File;
	 f->pool=file_pool;
	 GCriticalSectionLock lock(&files_lock);
	 files_map[file_url]=f;

	    // And create new streams
	 str=new MemoryByteStream;
	 iff=new IFFByteStream(*str);
	 iff->put_chunk("FORM:THUM");
	 image_num=0;

	    // Reset ipf to correct value (after we stored first
	    // "exceptional" file with thumbnail for the first page)
	 if (page_num==1) ipf=images_per_file;
	 if (page_num>=pages_num) break;
      }
   }
}

void
DjVuDocEditor::save(void)
{
   DEBUG_MSG("DjVuDocEditor::save(): saving the file\n");
   DEBUG_MAKE_INDENT(3);

   if (!can_be_saved()) THROW("Can't save the document. Use 'Save As'");

   save_as(0, orig_doc_type!=INDIRECT);
}

void
DjVuDocEditor::save_as(const char * where, bool bundled)
{
   DEBUG_MSG("DjVuDocEditor::save_as(): where='" << where << "'\n");
   DEBUG_MAKE_INDENT(3);

   GURL save_doc_url;
   GString save_doc_name;
   
   if (!where || !strlen(where))
   {
	 // Assume, that we just want to 'save'. Check, that it's possible
	 // and proceed.
      bool can_be_saved_bundled=orig_doc_type==BUNDLED ||
				orig_doc_type==OLD_BUNDLED ||
				orig_doc_type==SINGLE_PAGE ||
				orig_doc_type==OLD_INDEXED && orig_doc_pages==1;
      bool can_be_saved_indirect=orig_doc_type==INDIRECT;
      if ((bundled ^ can_be_saved_bundled)!=0)
	 THROW("Can't 'Save' the document in the requested format. Use 'Save As'");
      save_doc_url=doc_url;
      save_doc_name=GOS::url_to_filename(save_doc_url);
   } else
   {
      save_doc_name=GOS::expand_name(where, GOS::cwd());
      save_doc_url=GOS::filename_to_url(save_doc_name);
   }

   int save_doc_type=bundled ? BUNDLED : INDIRECT;
   
   clean_files_map();
   
   GCriticalSectionLock lock(&files_lock);

   DjVuPortcaster * pcaster=DjVuPort::get_portcaster();
   
      // First: save each page in a separate file, if applies
   if (get_pages_num()==1)
   {
	 // Here 'bundled' has no effect: we will save it as one page.
      DEBUG_MSG("saving one page...\n");
      GURL file_url=page_to_url(0);
      GP<DataPool> file_pool;
      GPosition pos;
      if (files_map.contains(file_url, pos))
      {
	 GP<File> file_rec=files_map[pos];
	 if (file_rec->pool && (!file_rec->file ||
				!file_rec->file->is_modified()))
	    file_pool=file_rec->pool;
	 else if (file_rec->file) file_pool=file_rec->file->get_djvu_data(false, true);
      }
	 // Even if file has not been modified (pool==0) we still want
	 // to save it.
      if (!file_pool) file_pool=pcaster->request_data(this, file_url);
      if (file_pool)
      {
	 DEBUG_MSG("Saving '" << file_url << "' to '" << save_doc_url << "'\n");
	 DataPool::load_file(save_doc_name);
	 StdioByteStream str_out(save_doc_name, "wb");
	 str_out.writall("AT&T", 4);
	 GP<ByteStream> str_in=file_pool->get_stream();
	 str_out.copy(*str_in);
      }

	 // Update the document's DataPool (to save memory)
      GP<DjVmDoc> doc=get_djvm_doc();
      MemoryByteStream str;	// One page: we can do it in the memory
      doc->write(str);
      str.seek(0, SEEK_SET);
      GP<DataPool> pool=new DataPool(str);
      doc_pool=pool;
      init_data_pool=pool;
      if (doc_url!=save_doc_url)
      {
	    // Also update document's URL (we moved, didn't we?)
	 doc_url=save_doc_url;
	 init_url=save_doc_url;
      }
	 // Also update DjVmDir (to reflect changes in offsets)
      djvm_dir=doc->get_djvm_dir();
   } else if (save_doc_type==INDIRECT)
   {
      DEBUG_MSG("Saving in INDIRECT format to '" << save_doc_url << "'\n");
      int pages_num=get_pages_num();
      for(int page_num=0;page_num<pages_num;page_num++)
      {
	 GURL file_url=page_to_url(page_num);
	 GP<DataPool> file_pool;
	 GPosition pos;
	 if (files_map.contains(file_url, pos))
	 {
	    GP<File> file_rec=files_map[pos];
	    if (file_rec->pool && (!file_rec->file ||
				   !file_rec->file->is_modified()))
	       file_pool=file_rec->pool;
	    else if (file_rec->file) file_pool=file_rec->file->get_djvu_data(false, true);
	 }
	    // At this moment pool may be ZERO, which means, that the
	    // data for this file has not been modified and is not
	    // worth saving. Still, if we save not to the original location,
	    // or the document type changed we still must save the file
	 if (!file_pool && (save_doc_url!=doc_url || save_doc_type!=orig_doc_type))
	    file_pool=pcaster->request_data(this, file_url);
	 if (file_pool)
	 {
	    GURL save_url=save_doc_url.base()+file_url.name();
	    GString save_name=GOS::url_to_filename(save_url);
	    DEBUG_MSG("Saving '" << file_url << "' to '" << save_url << "'\n");
	    DataPool::load_file(save_name);
	    StdioByteStream str_out(save_name, "wb");
	    str_out.writall("AT&T", 4);
	    GP<ByteStream> str_in=file_pool->get_stream();
	    str_out.copy(*str_in);
	 }
      }

	 // Save the top-level index file
      GP<DjVmDir> dir=get_djvm_dir();
      GPList<DjVmDir::File> xfiles_list=dir->get_files_list();
      for(GPosition pos=xfiles_list;pos;++pos)
      {
	 GP<DjVmDir::File> file=xfiles_list[pos];
	 file->offset=0;
	 file->size=0;
      }
      DataPool::load_file(save_doc_name);
      StdioByteStream str(save_doc_name, "wb");
      IFFByteStream iff(str);

      iff.put_chunk("FORM:DJVM", 1);
      iff.put_chunk("DIRM");
      dir->encode(iff);
      iff.close_chunk();
      iff.close_chunk();
      iff.flush();

	 // Update the document data pool (not required, but will save memory)
      doc_pool=new DataPool(save_doc_name);
      init_data_pool=doc_pool;
      if (doc_url!=save_doc_url)
      {
	    // Also update the document's URL. We moved, didn't we?
	 doc_url=save_doc_url;
	 init_url=save_doc_url;
      }
	 // No reason to update DjVmDir as for this format it doesn't
	 // contain DJVM offsets
   } else if (save_doc_type==BUNDLED || save_doc_type==OLD_BUNDLED)
   {
      DEBUG_MSG("Saving in BUNDLED format to '" << save_doc_url << "'\n");
      
	 // Can't be very smart here. Simply overwrite the file.
      GP<DjVmDoc> doc=get_djvm_doc();
      DataPool::load_file(save_doc_name);
      StdioByteStream str(save_doc_name, "wb");
      doc->write(str);
      str.flush();

	 // Update the document data pool (not required, but will save memory)
      doc_pool=new DataPool(save_doc_name);
      init_data_pool=doc_pool;
      if (doc_url!=save_doc_url)
      {
	    // Also update the document's URL.
	 doc_url=save_doc_url;
	 init_url=save_doc_url;
      }
	 // Also update DjVmDir (to reflect changes in offsets)
      djvm_dir=doc->get_djvm_dir();
   } else THROW("Can't save the document. Use 'Save As'");

      // Now, after we have saved the document w/o any error, detach DataPools,
      // which are in the 'File's list to save memory. Detach everything.
      // Even in the case when File->file is non-zero. If File->file is zero,
      // remove the item from the list at all. If it's non-zero, it has
      // to stay there because by definition files_map[] contains the list
      // of all active files and customized DataPools
      //
      // In addition to it, look thru all active files and change their URLs
      // to reflect changes in the document's URL (if there was a change)
      // Another reason why file's URLs must be changed is that we may have
      // saved the document in a different format, which changes the rules
      // of file url composition.
   for(GPosition pos=files_map;pos;)
   {
      GP<File> file_rec=files_map[pos];
      file_rec->pool=0;
      if (file_rec->file==0)
      {
	 GPosition this_pos=pos;
	 ++pos;
	 files_map.del(this_pos);
      } else
      {
	    // Change the file's url;
	 if (doc_url!=save_doc_url ||
	     orig_doc_type!=save_doc_type)
	    if (save_doc_type==BUNDLED)
	       file_rec->file->move(save_doc_url);
	    else file_rec->file->move(save_doc_url.base());
	 ++pos;
      }
   }

   orig_doc_type=save_doc_type;
   doc_type=save_doc_type;
}
