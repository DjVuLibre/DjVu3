//C-  -*- C++ -*-
//C- DjVu® Reference Library (v. 3.0)
//C- 
//C- Copyright © 1999-2000 LizardTech, Inc. All Rights Reserved.
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
//C- 
// 
// $Id: DjVuDocEditor.cpp,v 1.48 2000-11-09 20:15:05 jmw Exp $
// $Name:  $

#ifdef __GNUC__
#pragma implementation
#endif

#include "DjVuDocEditor.h"
#include "GOS.h"
#include "debug.h"

#include <ctype.h>

static const char octets[4]={0x41,0x54,0x41,0x25};

int        DjVuDocEditor::thumbnails_per_file=10;

// This is a structure for active files and DataPools. It may contain
// a DjVuFile, which is currently being used by someone (I check the list
// and get rid of hanging files from time to time) or a DataPool,
// which is "custom" with respect to the document (was modified or
// inserted), or both.
//
// DjVuFile is set to smth!=0 when it's created using url_to_file().
//          It's reset back to ZERO in clean_files_map() when
//	  it sees, that a given file is not used by anyone.
// DataPool is updated when a file is inserted
class DjVuDocEditor::File : public GPEnabled
{
public:
  // 'pool' below may be non-zero only if it cannot be retrieved
  // by the DjVuDocument, that is it either corresponds to a
  // modified DjVuFile or it has been inserted. Otherwise it's ZERO
  // Once someone assigns a non-zero DataPool, it remains non-ZERO
  // (may be updated if the file gets modified) and may be reset
  // only by save() or save_as() functions.
  GP<DataPool>	pool;

  // If 'file' is non-zero, it means, that it's being used by someone
  // We check for unused files from time to time and ZERO them.
  // But before we do it, we may save the DataPool in the case if
  // file has been modified.
  GP<DjVuFile>	file;
};

void
DjVuDocEditor::check(void)
{
   if (!initialized) G_THROW("DjVuDocEditor.not_init");
}

DjVuDocEditor::DjVuDocEditor(void)
{
   initialized=false;
   refresh_cb=0;
   refresh_cl_data=0;
}

DjVuDocEditor::~DjVuDocEditor(void)
{
   if (tmp_doc_name.length()) GOS::deletefile(tmp_doc_name);

   GPosition pos;
   GCriticalSectionLock lock(&thumb_lock);
   while((pos=thumb_map))
   {
      delete (TArray<char> *) thumb_map[pos];
      thumb_map.del(pos);
   }
   DataPool::close_all();
}

void
DjVuDocEditor::init(void)
{
   DEBUG_MSG("DjVuDocEditor::init() called\n");
   DEBUG_MAKE_INDENT(3);

      // If you remove this check be sure to delete thumb_map
   if (initialized) G_THROW("DjVuDocEditor.init");

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
}

void
DjVuDocEditor::init(const char * fname)
{
   DEBUG_MSG("DjVuDocEditor::init() called: fname='" << fname << "'\n");
   DEBUG_MAKE_INDENT(3);

      // If you remove this check be sure to delete thumb_map
   if (initialized) G_THROW("DjVuDocEditor.init");

      // First - create a temporary DjVuDocument and check its type
   doc_pool=new DataPool(fname);
   doc_url=GOS::filename_to_url(fname);
   GP<DjVuDocument> tmp_doc=new DjVuDocument();
   tmp_doc->init(doc_url, this);
   if (!tmp_doc->is_init_ok())
      G_THROW(GString("DjVuDocEditor.open_fail\t")+fname);

   orig_doc_type=tmp_doc->get_doc_type();
   orig_doc_pages=tmp_doc->get_pages_num();
   if (orig_doc_type==OLD_BUNDLED ||
       orig_doc_type==OLD_INDEXED ||
       orig_doc_type==SINGLE_PAGE)
   {
         // Suxx. I need to convert it NOW.
         // We will unlink this file in the destructor
#ifndef UNDER_CE
      tmp_doc_name=tmpnam(0);
#else
      tmp_doc_name = "tempFileForDjVu" ;
#endif
      StdioByteStream str(tmp_doc_name, "wb");
      tmp_doc->write(str, true);        // Force DJVM format
      str.flush();
      doc_pool=new DataPool(tmp_doc_name);
   }

      // OK. Now doc_pool contains data of the document in one of the
      // new formats. It will be a lot easier to insert/delete pages now.

      // 'doc_url' below of course doesn't refer to the file with the converted
      // data, but we will take care of it by redirecting the request_data().
   initialized=true;
   DjVuDocument::init(doc_url, this);

      // Cool. Now extract the thumbnails...
   GCriticalSectionLock lock(&thumb_lock);
   int pages_num=get_pages_num();
   for(int page_num=0;page_num<pages_num;page_num++)
   {
	 // Call DjVuDocument::get_thumbnail() here to bypass logic
	 // of DjVuDocEditor::get_thumbnail(). init() is the only safe
	 // place where we can still call DjVuDocument::get_thumbnail();
      GP<DataPool> pool=DjVuDocument::get_thumbnail(page_num, true);
      if (pool)
      {
	 TArray<char> * data=new TArray<char>(pool->get_size()-1);
	 pool->get_data(*data, 0, data->size());
	 thumb_map[page_to_id(page_num)]=data;
      }
   }
      // And remove then from DjVmDir so that DjVuDocument
      // does not try to use them
   unfile_thumbnails();
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
   GP<DjVmDir::File> frec=djvm_dir->name_to_file(url.fname());
   if (frec)
   {
      GCriticalSectionLock lock(&files_lock);
      GPosition pos;
      if (files_map.contains(frec->id, pos))
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
      // Will go thru the map of files looking for unreferenced
      // files or records w/o DjVuFile and DataPool.
      // These will be modified and/or removed.
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
   GP<DjVmDir::File> frec;
   if((const DjVmDir *)djvm_dir)
     frec=djvm_dir->name_to_file(url.fname());
   if (frec)
   {
      GCriticalSectionLock lock(&files_lock);
      GPosition pos;
      if (files_map.contains(frec->id, pos))
      {
         GP<File> f=files_map[pos];
         if (f->file) return f->file;
      }
   }

   clean_files_map();

      // We don't have the file cached. Let DjVuDocument create the file.
   GP<DjVuFile> file=DjVuDocument::url_to_file(url, dont_create);

      // And add it to our private "cache"
   if (file && frec)
   {
      GCriticalSectionLock lock(&files_lock);
      GPosition pos;
      if (files_map.contains(frec->id, pos))
      {
         files_map[frec->id]->file=file;
      }else
      {
         GP<File> f=new File();
         f->file=file;
         files_map[frec->id]=f;
      }
   }

   return file;
}

GString
DjVuDocEditor::page_to_id(int page_num) const
{
   GP<DjVmDir::File> f;
   if (page_num<0 || page_num>=get_pages_num() ||
       (f=djvm_dir->page_to_file(page_num))==0)
      G_THROW("DjVuDocEditor.page_num\t"+GString(page_num));

   return f->id;
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
DjVuDocEditor::strip_incl_chunks(GP<DataPool> & pool_in)
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
      // Will open the 'file_name' and insert it into an existing DjVuFile
      // with ID 'parent_id'. Will insert the INCL chunk at position chunk_num
      // Will NOT process ANY files included into the file being inserted.
      // Moreover it will strip out any INCL chunks in that file...
{
   DEBUG_MSG("DjVuDocEditor::insert_file(): fname='" << file_name <<
             "', parent_id='" << parent_id << "'\n");
   DEBUG_MAKE_INDENT(3);
   GP<DjVmDir> dir=get_djvm_dir();

      // Create DataPool and see if the file exists
   GP<DataPool> file_pool=new DataPool(file_name);
   if(file_pool && file_name && DjVuDocument::djvu_import_codec)
   {
     (*DjVuDocument::djvu_import_codec)(file_pool,(const char *)file_name,needs_compression_flag,can_compress_flag);
   }

      // Strip any INCL chunks
   file_pool=strip_incl_chunks(file_pool);

      // Check if parent ID is valid
   GP<DjVmDir::File> parent_frec=dir->id_to_file(parent_id);
   if (!parent_frec) parent_frec=dir->name_to_file(parent_id);
   if (!parent_frec) parent_frec=dir->title_to_file(parent_id);
   if (!parent_frec)
      G_THROW(GString("DjVuDocEditor.no_file\t")+parent_id);
   GP<DjVuFile> parent_file=get_djvu_file(parent_id);
   if (!parent_file) G_THROW(GString("DjVuDocEditor.create_fail\t")+parent_id);

      // Now obtain ID for the new file
   GString id=find_unique_id(GOS::basename(file_name));

      // Add it into the directory
   GP<DjVmDir::File> frec=new DjVmDir::File(id, id, id, DjVmDir::File::INCLUDE);
   int pos=dir->get_file_pos(parent_frec);
   if (pos>=0) ++pos;
   dir->insert_file(frec, pos);

      // Add it to our "cache"
   {
      GP<File> f=new File;
      f->pool=file_pool;
      GCriticalSectionLock lock(&files_lock);
      files_map[id]=f;
   }

      // And insert it into the parent DjVuFile
   parent_file->insert_file(id, chunk_num);

   return id;
}

bool
DjVuDocEditor::insert_file(const char * file_name, bool is_page,
                           int & file_pos, GMap<GString, GString> & name2id)
      // First it will insert the 'file_name' at position 'file_pos'.
      //
      // Then it will process all the INCL chunks in the file and try to do
      // the same thing with the included files. If insertion of an included
      // file fails, it will proceed with other INCL chunks until it does
      // them all. In the very end we will throw exception to let the caller
      // know about problems with included files.
      //
      // If the name of a file being inserted conflicts with some other
      // name, which has been in DjVmDir prior to call to this function,
      // it will be modified. name2id is the translation table to
      // keep track of these modifications.
      //
      // Also, if a name is in name2id, we will not insert that file again.
      //
      // Will return TRUE if the file has been successfully inserted.
      // FALSE, if the file contains NDIR chunk and has been skipped.
{
   GString errors;

   if (refresh_cb)
      refresh_cb(refresh_cl_data);

      // We do not want to insert the same file twice (important when
      // we insert a group of files at the same time using insert_group())
      // So we check if we already did that and return if so.
   if (name2id.contains(GOS::basename(file_name))) return true;

   G_TRY {
      GP<DjVmDir> dir=get_djvm_dir();

      GP<DataPool> file_pool=new DataPool(file_name);
         // Create DataPool and see if the file exists
      if(file_pool && file_name && DjVuDocument::djvu_import_codec)
      {
        (*DjVuDocument::djvu_import_codec)(file_pool,(const char *)file_name,needs_compression_flag,can_compress_flag);
      }

         // Oh. It does exist... Check that it has IFF structure
      {
         GP<ByteStream> str;
         str=file_pool->get_stream();
         IFFByteStream iff(*str);
         GString chkid;
         int length;
         length=iff.get_chunk(chkid);
//         if (length>100*1024*1024)
//            G_THROW("File '"+GString(file_name)+"' is not a DjVu file");
         if (chkid!="FORM:DJVI" && chkid!="FORM:DJVU" &&
             chkid!="FORM:BM44" && chkid!="FORM:PM44")
            G_THROW("DjVuDocEditor.not_1_page\t"+GString(file_name));

            // Wonderful. It's even a DjVu file. Scan for NDIR chunks.
            // If NDIR chunk is found, ignore the file
         while(iff.get_chunk(chkid))
         {
            if (chkid=="NDIR") return false;
            iff.close_chunk();
         }
      }

         // Now get a unique name for this file.
         // Check the name2id first...
      GString id, name=GOS::basename(file_name);
      if (name2id.contains(name)) id=name2id[name];
      else
      {
            // Otherwise create a new unique ID and remember the translation
         id=find_unique_id(name);
         name2id[name]=id;
      }

         // Good. Before we continue with the included files we want to
         // complete insertion of this one. Notice, that insertion of
         // children may fail, in which case we will have to modify
         // data for this file to get rid of invalid INCL

         // Create a file record with the chosen ID
      GP<DjVmDir::File> file;
      file=new DjVmDir::File(id, id, id, is_page ? DjVmDir::File::PAGE :
                             DjVmDir::File::INCLUDE);

         // And insert it into the directory
      dir->insert_file(file, file_pos);
      if (file_pos>=0) file_pos++;

         // And add the File record (containing the file URL and DataPool)
      {
         GP<File> f=new File;
         f->pool=file_pool;
         GCriticalSectionLock lock(&files_lock);
         files_map[id]=f;
      }

         // The file has been added. If it doesn't include anything else,
         // that will be enough. Otherwise repeat what we just did for every
         // included child. Don't forget to modify the contents of INCL
         // chunks due to name2id translation.
         // We also want to include here our file with shared annotations,
         // if it exists.
      GString chkid;
      GP<ByteStream> str_in=file_pool->get_stream();
      IFFByteStream iff_in(*str_in);
      MemoryByteStream str_out;
      IFFByteStream iff_out(str_out);

      GP<DjVmDir::File> shared_frec=djvm_dir->get_shared_anno_file();

      iff_in.get_chunk(chkid);
      iff_out.put_chunk(chkid);
      while(iff_in.get_chunk(chkid))
      {
         if (chkid!="INCL")
         {
            iff_out.put_chunk(chkid);
            iff_out.copy(iff_in);
            iff_in.close_chunk();
            iff_out.close_chunk();
            if (shared_frec && chkid=="INFO")
            {
               iff_out.put_chunk("INCL");
               iff_out.writall((const char *) shared_frec->id,
                               shared_frec->id.length());
               iff_out.close_chunk();
            }
         } else
         {
            GString name;
            char buffer[1024];
            int length;
            while((length=iff_in.read(buffer, 1024)))
               name+=GString(buffer, length);
            while(isspace(name[0])) { GString tmp=(const char *) name+1; name=tmp; }
            while(isspace(name[(int)name.length()-1])) name.setat(name.length()-1, 0);
            GString full_name=GOS::expand_name(name, GOS::dirname(file_name));
            iff_in.close_chunk();

            G_TRY {
               if (insert_file(full_name, false, file_pos, name2id))
               {
                     // If the child file has been inserted (doesn't
                     // contain NDIR chunk), add INCL chunk.
                  GString id=name2id[name];
                  iff_out.put_chunk("INCL");
                  iff_out.write((const char *) id, id.length());
                  iff_out.close_chunk();
               }
            } G_CATCH(exc) {
                  // Should an error occur, we move on. INCL chunk will
                  // not be copied.
               if (errors.length()) errors+="\n\n";
               errors+=exc.get_cause();
            } G_ENDCATCH;
         }
      } // while(iff_in.get_chunk(chkid))
      iff_out.close_chunk();

         // We have just inserted every included file. We may have modified
         // contents of the INCL chunks. So we need to update the DataPool...
      str_out.seek(0);
      GP<DataPool> new_file_pool=new DataPool(str_out);
      {
            // It's important, that we replace the pool here anyway.
            // By doing this we load the file into memory. And this is
            // exactly what insert_group() wants us to do because
            // it creates temporary files.
         GCriticalSectionLock lock(&files_lock);
         files_map[id]->pool=new_file_pool;
      }
   } G_CATCH(exc) {
      if (errors.length()) errors+="\n\n";
      errors+=exc.get_cause();
      G_THROW(errors);
   } G_ENDCATCH;

      // The only place where we intercept exceptions is when we process
      // included files. We want to process all of them even if we failed to
      // process one. But here we need to let the exception propagate...
   if (errors.length()) G_THROW(errors);

   return true;
}

void
DjVuDocEditor::insert_group(const GList<GString> & file_names, int page_num,
                             void (* _refresh_cb)(void *), void * _cl_data)
      // The function will insert every file from the list at position
      // corresponding to page_num. If page_num is negative, concatenation
      // will occur. Included files will be processed as well
{
   refresh_cb=_refresh_cb;
   refresh_cl_data=_cl_data;

   G_TRY {

         // First translate the page_num to file_pos.
      GP<DjVmDir> dir=get_djvm_dir();
      int file_pos;
      if (page_num<0 || page_num>=dir->get_pages_num()) file_pos=-1;
      else file_pos=dir->get_page_pos(page_num);

         // Now call the insert_file() for every page. We will remember the
         // name2id translation table. Thus insert_file() will remember IDs
         // it assigned to shared files
      GMap<GString, GString> name2id;

      GString errors;
      for(GPosition pos=file_names;pos;++pos)
      {
         GString fname=file_names[pos];
         G_TRY {
               // Check if it's a multipage document...
            GP<DataPool> xdata_pool=new DataPool(fname);
            if(xdata_pool && (const char *)fname && ((const char *)fname)[0] && DjVuDocument::djvu_import_codec)
            {
              (*DjVuDocument::djvu_import_codec)(xdata_pool,(const char *)fname,needs_compression_flag,can_compress_flag);
            }
            GP<ByteStream> str;
            str=xdata_pool->get_stream();
            IFFByteStream iff(*str);
            GString chkid;
            iff.get_chunk(chkid);
            str=0;
            if (chkid=="FORM:DJVM")
            {
                  // Hey, it really IS a multipage document.
                  // Open it, expand to a tmp directory and add pages
                  // one after another
               GP<DjVuDocument> doc=new DjVuDocument();
               doc->init(GOS::filename_to_url(fname));
#ifndef UNDER_CE
	       GString dirname=tmpnam(0);
#else
               GString dirname="tempFileForDjVu" ;
#endif
               if (GOS::mkdir(dirname)<0)
                  G_THROW(GString("DjVuDocEditor.dir_fail\t")+dirname);
               G_TRY {
                  doc->expand(dirname, GOS::basename(fname));
                  int pages_num=doc->get_pages_num();
                  for(int page_num=0;page_num<pages_num;page_num++)
                  {
                     GString name=doc->page_to_url(page_num).fname();
                     name=GOS::expand_name(name, dirname);
                     insert_file(name, true, file_pos, name2id);
                  }
                  GOS::cleardir(dirname);
                  GOS::deletefile(dirname);
                  dirname.empty();
               } G_CATCH_ALL {
                  if (dirname.length())
                  {
                     GOS::cleardir(dirname);
                     GOS::deletefile(dirname);
                  }
                  G_RETHROW;
               } G_ENDCATCH;
            } else insert_file(fname, true, file_pos, name2id);
         } G_CATCH(exc) {
            if (errors.length()) errors+="\n\n";
            errors+=exc.get_cause();
         } G_ENDCATCH;
      }
      if (errors.length()) G_THROW(errors);
   } G_CATCH_ALL {
      refresh_cb=0;
      refresh_cl_data=0;
      G_RETHROW;
   } G_ENDCATCH;
   refresh_cb=0;
   refresh_cl_data=0;
}

void
DjVuDocEditor::insert_page(const char * file_name, int page_num)
{
   DEBUG_MSG("DjVuDocEditor::insert_page(): fname='" << file_name << "'\n");
   DEBUG_MAKE_INDENT(3);

   GList<GString> list;
   list.append(file_name);

   insert_group(list, page_num);
}

void
DjVuDocEditor::insert_page(GP<DataPool> & _file_pool,
			   const char * file_name, int page_num)
      // Use _file_pool as source of data, create a new DjVuFile
      // with name file_name, and insert it as page number page_num
{
   DEBUG_MSG("DjVuDocEditor::insert_page(): pool size='" <<
	     _file_pool->get_size() << "'\n");
   DEBUG_MAKE_INDENT(3);

   GP<DjVmDir> dir=get_djvm_dir();

      // Strip any INCL chunks (we do not allow to insert hierarchies
      // using this function)
   GP<DataPool> file_pool=strip_incl_chunks(_file_pool);
   
      // Now obtain ID for the new file
   GString id=find_unique_id(GOS::basename(file_name));

      // Add it into the directory
   GP<DjVmDir::File> frec=new DjVmDir::File(id, id, id, DjVmDir::File::PAGE);
   int pos=dir->get_page_pos(page_num);
   dir->insert_file(frec, pos);

      // Add it to our "cache"
   {
      GP<File> f=new File;
      f->pool=file_pool;
      GCriticalSectionLock lock(&files_lock);
      files_map[id]=f;
   }
}

void
DjVuDocEditor::generate_ref_map(const GP<DjVuFile> & file,
				GMap<GString, void *> & ref_map,
				GMap<GURL, void *> & visit_map)
      // This private function is used to generate a list (implemented as map)
      // of files referencing the given file. To get list of all parents
      // for file with ID 'id' iterate map obtained as
      // *((GMap<GString, void *> *) ref_map[id])
{
   GURL url=file->get_url();
   GString id=djvm_dir->name_to_file(url.fname())->id;
   if (!visit_map.contains(url))
   {
      visit_map[url]=0;

      GPList<DjVuFile> files_list=file->get_included_files(false);
      for(GPosition pos=files_list;pos;++pos)
      {
         GP<DjVuFile> child_file=files_list[pos];
            // First: add the current file to the list of parents for
            // the child being processed
         GURL child_url=child_file->get_url();
         GString child_id=djvm_dir->name_to_file(child_url.fname())->id;
         GMap<GString, void *> * parents=0;
         if (ref_map.contains(child_id))
            parents=(GMap<GString, void *> *) ref_map[child_id];
         else
            ref_map[child_id]=parents=new GMap<GString, void *>();
         (*parents)[id]=0;
            // Second: go recursively
         generate_ref_map(child_file, ref_map, visit_map);
      }
   }
}

void
DjVuDocEditor::remove_file(const char * id, bool remove_unref,
                           GMap<GString, void *> & ref_map)
      // Private function, which will remove file with ID id.
      //
      // If will also remove all INCL chunks in parent files pointing
      // to this one
      //
      // Finally, if remove_unref is TRUE, we will go down the files
      // hierarchy removing every file, which becomes unreferenced.
      //
      // ref_map will be used to find out list of parents referencing
      // this file (required when removing INCL chunks)
{
      // First get rid of INCL chunks in parents
   GMap<GString, void *> * parents=(GMap<GString, void *> *) ref_map[id];
   if (parents)
   {
      for(GPosition pos=*parents;pos;++pos)
      {
         GString parent_id=(*parents).key(pos);
         GP<DjVuFile> parent=get_djvu_file(parent_id);
         if (parent) parent->unlink_file(id);
      }
      delete parents; parents=0;
      ref_map.del(id);
   }

      // We will accumulate errors here.
   GString errors;

      // Now modify the ref_map and process children if necessary
   GP<DjVuFile> file=get_djvu_file(id);
   if (file)
   {
      G_TRY {
         GPList<DjVuFile> files_list=file->get_included_files(false);
         for(GPosition pos=files_list;pos;++pos)
         {
            GP<DjVuFile> child_file=files_list[pos];
            GURL child_url=child_file->get_url();
            GString child_id=djvm_dir->name_to_file(child_url.fname())->id;
            GMap<GString, void *> * parents=(GMap<GString, void *> *) ref_map[child_id];
            if (parents) parents->del(id);

            if (remove_unref && (!parents || !parents->size()))
               remove_file(child_id, remove_unref, ref_map);
         }
      } G_CATCH(exc) {
         if (errors.length()) errors+="\n\n";
         errors+=exc.get_cause();
      } G_ENDCATCH;
   }

      // Finally remove this file from the directory.
   djvm_dir->delete_file(id);

      // And get rid of its thumbnail, if any
   GCriticalSectionLock lock(&thumb_lock);
   GPosition pos;
   if (thumb_map.contains(id, pos))
   {
      delete (TArray<char> *) thumb_map[pos];
      thumb_map.del(pos);
   }

   if (errors.length()) G_THROW(errors);
}

void
DjVuDocEditor::remove_file(const char * id, bool remove_unref)
{
   DEBUG_MSG("DjVuDocEditor::remove_file(): id='" << id << "'\n");
   DEBUG_MAKE_INDENT(3);

   if (!djvm_dir->id_to_file(id))
      G_THROW("DjVuDocEditor.no_file\t"+GString(id));

      // First generate a map of references (containing the list of parents
      // including this particular file. This will speed things up
      // significatly.
   GMap<GString, void *> ref_map;        // GMap<GString, GMap<GString, void *> *> in fact
   GMap<GURL, void *> visit_map;        // To avoid loops

   int pages_num=djvm_dir->get_pages_num();
   for(int page_num=0;page_num<pages_num;page_num++)
      generate_ref_map(get_djvu_file(page_num), ref_map, visit_map);

      // Now call the function, which will do the removal recursively
   remove_file(id, remove_unref, ref_map);

      // And clear the ref_map
   GPosition pos;
   while((pos=ref_map))
   {
      GMap<GString, void *> * parents=(GMap<GString, void *> *) ref_map[pos];
      delete parents;
      ref_map.del(pos);
   }
}

void
DjVuDocEditor::remove_page(int page_num, bool remove_unref)
{
   DEBUG_MSG("DjVuDocEditor::remove_page(): page_num=" << page_num << "\n");
   DEBUG_MAKE_INDENT(3);

      // Translate the page_num to ID
   GP<DjVmDir> djvm_dir=get_djvm_dir();
   if (page_num<0 || page_num>=djvm_dir->get_pages_num())
      G_THROW("DjVuDocEditor.bad_page\t"+GString(page_num));

      // And call general remove_file()
   remove_file(djvm_dir->page_to_file(page_num)->id, remove_unref);
}

void
DjVuDocEditor::remove_pages(const GList<int> & page_list, bool remove_unref)
{
   DEBUG_MSG("DjVuDocEditor::remove_pages() called\n");
   DEBUG_MAKE_INDENT(3);

      // First we need to translate page numbers to IDs (they will
      // obviously be changing while we're removing pages one after another)
   GP<DjVmDir> djvm_dir=get_djvm_dir();
   GPosition pos ;
   if (djvm_dir)
   {
      GList<GString> id_list;
      for(pos=page_list;pos;++pos)
      {
         GP<DjVmDir::File> frec=djvm_dir->page_to_file(page_list[pos]);
         if (frec)
            id_list.append(frec->id);
      }

      for(pos=id_list;pos;++pos)
      {
         GP<DjVmDir::File> frec=djvm_dir->id_to_file(id_list[pos]);
         if (frec)
            remove_page(frec->get_page_num(), remove_unref);
      }
   }
}

void
DjVuDocEditor::move_file(const char * id, int & file_pos,
                         GMap<GString, void *> & map)
      // NOTE! file_pos here is the desired position in DjVmDir *after*
      // the record with ID 'id' is removed.
{
   if (!map.contains(id))
   {
      map[id]=0;

      GP<DjVmDir::File> file_rec=djvm_dir->id_to_file(id);
      if (file_rec)
      {
         file_rec=new DjVmDir::File(*file_rec);
         djvm_dir->delete_file(id);
         djvm_dir->insert_file(file_rec, file_pos);

         if (file_pos>=0)
         {
            file_pos++;
        
               // We care to move included files only if we do not append
               // This is because the only reason why we move included
               // files is to made them available sooner than they would
               // be available if we didn't move them. By appending files
               // we delay the moment when the data for the file becomes
               // available, of course.
            GP<DjVuFile> djvu_file=get_djvu_file(id);
            if (djvu_file)
            {
               GPList<DjVuFile> files_list=djvu_file->get_included_files(false);
               for(GPosition pos=files_list;pos;++pos)
               {
                  GString name=files_list[pos]->get_url().fname();
                  GP<DjVmDir::File> child_frec=djvm_dir->name_to_file(name);

                     // If the child is positioned in DjVmDir AFTER the
                     // file being processed (position is file_pos or greater),
                     // move it to file_pos position
                  if (child_frec)
                     if (djvm_dir->get_file_pos(child_frec)>file_pos)
                        move_file(child_frec->id, file_pos, map);
               }
            }
         }
      }
   }
}

void
DjVuDocEditor::move_page(int page_num, int new_page_num)
{
   DEBUG_MSG("DjVuDocEditor::move_page(): page_num=" << page_num <<
             ", new_page_num=" << new_page_num << "\n");
   DEBUG_MAKE_INDENT(3);

   if (page_num==new_page_num) return;

   int pages_num=get_pages_num();
   if (page_num<0 || page_num>=pages_num)
      G_THROW("DjVuDocEditor.bad_page\t"+GString(page_num));

   GString id=page_to_id(page_num);
   int file_pos=-1;
   if (new_page_num>=0 && new_page_num<pages_num)
      if (new_page_num>page_num)        // Moving toward the end
      {
         if (new_page_num<pages_num-1)
            file_pos=djvm_dir->get_page_pos(new_page_num+1)-1;
      } else
         file_pos=djvm_dir->get_page_pos(new_page_num);

   GMap<GString, void *> map;
   move_file(id, file_pos, map);
}
#ifdef _WIN32_WCE_EMULATION         // Work around odd behavior under WCE Emulation
#define CALLINGCONVENTION __cdecl
#else
#define CALLINGCONVENTION  /* */
#endif

static int
CALLINGCONVENTION
cmp(const void * ptr1, const void * ptr2)
{
   int num1=*(int *) ptr1;
   int num2=*(int *) ptr2;
   return num1<num2 ? -1 : num1>num2 ? 1 : 0;
}

static GList<int>
sortList(const GList<int> & list)
{
   GArray<int> a(list.size()-1);
   int cnt;
   GPosition pos;
   for(pos=list, cnt=0;pos;++pos, cnt++)
      a[cnt]=list[pos];

   qsort((int *) a, a.size(), sizeof(int), cmp);

   GList<int> l;
   for(int i=0;i<a.size();i++)
      l.append(a[i]);

   return l;
}

void
DjVuDocEditor::move_pages(const GList<int> & _page_list, int shift)
{
   if (!shift) return;

   GList<int> page_list=sortList(_page_list);

   GList<GString> id_list;
   for(GPosition pos=page_list;pos;++pos)
   {
      GP<DjVmDir::File> frec=djvm_dir->page_to_file(page_list[pos]);
      if (frec)
         id_list.append(frec->id);
   }

   if (shift<0)
   {
         // We have to start here from the smallest page number
         // We will move it according to the 'shift', and all
         // further moves are guaranteed not to affect its page number.

         // We will be changing the 'min_page' to make sure that
         // pages moved beyond the document will still be in correct order
      int min_page=0;
      for(GPosition pos=id_list;pos;++pos)
      {
         GP<DjVmDir::File> frec=djvm_dir->id_to_file(id_list[pos]);
         if (frec)
         {
            int page_num=frec->get_page_num();
            int new_page_num=page_num+shift;
            if (new_page_num<min_page)
               new_page_num=min_page++;
            move_page(page_num, new_page_num);
         }
      }
   } else
   {
         // We have to start here from the biggest page number
         // We will move it according to the 'shift', and all
         // further moves will not affect its page number.

         // We will be changing the 'max_page' to make sure that
         // pages moved beyond the document will still be in correct order
      int max_page=djvm_dir->get_pages_num()-1;
      for(GPosition pos=id_list.lastpos();pos;--pos)
      {
         GP<DjVmDir::File> frec=djvm_dir->id_to_file(id_list[pos]);
         if (frec)
         {
            int page_num=frec->get_page_num();
            int new_page_num=page_num+shift;
            if (new_page_num>max_page)
               new_page_num=max_page--;
            move_page(page_num, new_page_num);
         }
      }
   }
}

void
DjVuDocEditor::set_file_name(const char * id, const char * name)
{
   DEBUG_MSG("DjVuDocEditor::set_file_name(), id='" << id << "', name='" << name << "'\n");
   DEBUG_MAKE_INDENT(3);

      // It's important to get the URL now, because later (after we
      // change DjVmDir) id_to_url() will be returning a modified value
   GURL url=id_to_url(id);

      // Change DjVmDir. It will check if the name is unique
   djvm_dir->set_file_name(id, name);

      // Now find DjVuFile (if any) and rename it
   GPosition pos;
   if (files_map.contains(id, pos))
   {
      GP<File> file=files_map[pos];
      GP<DataPool> pool=file->pool;
      if (pool) pool->load_file();
      GP<DjVuFile> djvu_file=file->file;
      if (djvu_file) djvu_file->set_name(name);
   }
}

void
DjVuDocEditor::set_page_name(int page_num, const char * name)
{
   DEBUG_MSG("DjVuDocEditor::set_page_name(), page_num='" << page_num << "'\n");
   DEBUG_MAKE_INDENT(3);

   if (page_num<0 || page_num>=get_pages_num())
      G_THROW("DjVuDocEditor.bad_page\t"+GString(page_num));

   set_file_name(page_to_id(page_num), name);
}

void
DjVuDocEditor::set_file_title(const char * id, const char * title)
{
   DEBUG_MSG("DjVuDocEditor::set_file_title(), id='" << id << "', title='" << title << "'\n");
   DEBUG_MAKE_INDENT(3);

      // Just change DjVmDir. It will check if the title is unique
   djvm_dir->set_file_title(id, title);
}

void
DjVuDocEditor::set_page_title(int page_num, const char * title)
{
   DEBUG_MSG("DjVuDocEditor::set_page_title(), page_num='" << page_num << "'\n");
   DEBUG_MAKE_INDENT(3);

   if (page_num<0 || page_num>=get_pages_num())
      G_THROW("DjVuDocEditor.bad_page\t"+GString(page_num));

   set_file_title(page_to_id(page_num), title);
}

//****************************************************************************
//************************** Shared annotations ******************************
//****************************************************************************

void
DjVuDocEditor::simplify_anno(void (* progress_cb)(float progress, void *),
                             void * cl_data)
      // It's important that no decoding is done while this function
      // is running. Otherwise the DjVuFile's decoding routines and
      // this function may attempt to decode/modify a file's
      // annotations at the same time.
{
      // Get the name of the SHARED_ANNO file. We will not
      // touch that file (will not move annotations from it)
   GP<DjVmDir::File> shared_file=djvm_dir->get_shared_anno_file();
   GString shared_id;
   if (shared_file)
      shared_id=shared_file->id;

   GList<GURL> ignore_list;
   if (shared_id.length())
      ignore_list.append(id_to_url(shared_id));

      // First, for every page get merged (or "flatten" or "projected")
      // annotations and store them inside the top-level page file
   int pages_num=djvm_dir->get_pages_num();
   for(int page_num=0;page_num<pages_num;page_num++)
   {
      GP<DjVuFile> djvu_file=get_djvu_file(page_num);
      if (!djvu_file)
         G_THROW("DjVuDocEditor.page_fail\t"+page_num);
      int max_level=0;
      GP<ByteStream> anno;
      anno=djvu_file->get_merged_anno(ignore_list, &max_level);
      if (anno && max_level>0)
      {
            // This is the moment when we try to modify DjVuFile's annotations
            // Make sure, that it's not being decoded
         GSafeFlags & file_flags=djvu_file->get_safe_flags();
         GMonitorLock lock(&file_flags);
         while(file_flags & DjVuFile::DECODING)
            file_flags.wait();
        
            // Merge all chunks in one by decoding and encoding DjVuAnno
         GP<DjVuAnno> dec_anno=new DjVuAnno;
         dec_anno->decode(*anno);
         MemoryByteStream *mbs=new MemoryByteStream;
         GP<ByteStream> new_anno=mbs;
         dec_anno->encode(*mbs);
         mbs->seek(0);

            // And store it in the file
         djvu_file->anno=new_anno;
         djvu_file->rebuild_data_pool();
         if ((file_flags & (DjVuFile::DECODE_OK |
                            DjVuFile::DECODE_FAILED |
                            DjVuFile::DECODE_STOPPED))==0)
            djvu_file->anno=0;
      }
      if (progress_cb)
    progress_cb((float)(page_num/2.0/pages_num), cl_data);
   }

      // Now remove annotations from every file except for
      // the top-level page files and SHARED_ANNO file.
      // Unlink empty files too.
   GPList<DjVmDir::File> files_list=djvm_dir->get_files_list();
   int cnt;
   GPosition pos;
   for(pos=files_list, cnt=0;pos;++pos, cnt++)
   {
      GP<DjVmDir::File> frec=files_list[pos];
      if (!frec->is_page() && frec->id!=shared_id)
      {
         GP<DjVuFile> djvu_file=get_djvu_file(frec->id);
         if (djvu_file)
         {
            djvu_file->remove_anno();
            if (djvu_file->get_chunks_number()==0)
               remove_file(frec->id, true);
         }
      }
      if (progress_cb)
         progress_cb((float)(0.5+cnt/2.0/files_list.size()), cl_data);
   }
}

void
DjVuDocEditor::create_shared_anno_file(void (* progress_cb)(float progress, void *),
                                       void * cl_data)
{
   if (djvm_dir->get_shared_anno_file())
      G_THROW("DjVuDocEditor.share_fail");

      // Prepare file with ANTa chunk inside
   MemoryByteStream str;
   IFFByteStream iff(str);
   iff.put_chunk("FORM:DJVI");
   iff.put_chunk("ANTa");
   iff.close_chunk();
   iff.close_chunk();
   str.flush();
   str.seek(0);
   GP<DataPool> file_pool=new DataPool(str);

      // Get a unique ID for the new file
   GString id=find_unique_id("shared_anno.iff");

      // Add it into the directory
   GP<DjVmDir::File> frec=new DjVmDir::File(id, id, id, DjVmDir::File::SHARED_ANNO);
   djvm_dir->insert_file(frec, 1);

      // Add it to our "cache"
   {
      GP<File> f=new File;
      f->pool=file_pool;
      GCriticalSectionLock lock(&files_lock);
      files_map[id]=f;
   }

      // Now include this shared file into every top-level page file
   int pages_num=djvm_dir->get_pages_num();
   for(int page_num=0;page_num<pages_num;page_num++)
   {
      GP<DjVuFile> djvu_file=get_djvu_file(page_num);
      djvu_file->insert_file(id, 1);

      if (progress_cb)
         progress_cb((float) page_num/pages_num, cl_data);
   }
}

GP<DjVuFile>
DjVuDocEditor::get_shared_anno_file(void)
{
   GP<DjVuFile> djvu_file;

   GP<DjVmDir::File> frec=djvm_dir->get_shared_anno_file();
   if (frec)
      djvu_file=get_djvu_file(frec->id);

   return djvu_file;
}

GP<DataPool>
DjVuDocEditor::get_thumbnail(int page_num, bool dont_decode)
      // We override DjVuDocument::get_thumbnail() here because
      // pages may have been shuffled and those "thumbnail file records"
      // from the DjVmDir do not describe things correctly.
      //
      // So, first we will check the thumb_map[] if we have a predecoded
      // thumbnail for the given page. If this is the case, we will
      // return it. Otherwise we will ask DjVuDocument to generate
      // this thumbnail for us.
{
   GString id=page_to_id(page_num);

   GPosition pos;
   GCriticalSectionLock lock(&thumb_lock);
   if (thumb_map.contains(id, pos))
   {
         // Get the image from the map
      TArray<char> & data=*(TArray<char> *) thumb_map[pos];
      GP<DataPool> pool=new DataPool;
      pool->add_data((const char *) data, data.size());
      pool->set_eof();
      return pool;
   } else
   {
      unfile_thumbnails();
      return DjVuDocument::get_thumbnail(page_num, dont_decode);
   }
}

int
DjVuDocEditor::get_thumbnails_num(void) const
{
   GCriticalSectionLock lock((GCriticalSection *) &thumb_lock);

   int cnt=0;
   int pages_num=get_pages_num();
   for(int page_num=0;page_num<pages_num;page_num++)
      if (thumb_map.contains(page_to_id(page_num))) cnt++;
   return cnt;
}

int
DjVuDocEditor::get_thumbnails_size(void) const
{
   DEBUG_MSG("DjVuDocEditor::remove_thumbnails(): doing it\n");
   DEBUG_MAKE_INDENT(3);

   GCriticalSectionLock lock((GCriticalSection *) &thumb_lock);

   GPosition pos;
   int pages_num=get_pages_num();
   for(int page_num=0;page_num<pages_num;page_num++)
      if (thumb_map.contains(page_to_id(page_num), pos))
      {
         TArray<char> & data=*(TArray<char> *) thumb_map[pos];
         MemoryByteStream str;
         str.writall((const char *) data, data.size());
         str.seek(0);
         GP<IWPixmap> iwpix=new IWPixmap;
         iwpix->decode_chunk(str);
        
         int width=iwpix->get_width();
         int height=iwpix->get_height();
         return width<height ? width : height;
      }
   return -1;
}

void
DjVuDocEditor::remove_thumbnails(void)
{
   DEBUG_MSG("DjVuDocEditor::remove_thumbnails(): doing it\n");
   DEBUG_MAKE_INDENT(3);

   unfile_thumbnails();

   DEBUG_MSG("clearing thumb_map\n");
   GPosition pos;
   GCriticalSectionLock lock(&thumb_lock);
   while((pos=thumb_map))
   {
      delete (TArray<char> *) thumb_map[pos];
      thumb_map.del(pos);
   }
}

void
DjVuDocEditor::unfile_thumbnails(void)
      // Will erase all "THUMBNAILS" files from DjVmDir.
      // This function is useful when filing thumbnails (to get rid of
      // those files, which currently exist: they need to be replaced
      // anyway) and when calling DjVuDocument::get_thumbnail() to
      // be sure, that it will not use wrong information from DjVmDir
{
   DEBUG_MSG("DjVuDocEditor::unfile_thumbnails(): updating DjVmDir\n");
   DEBUG_MAKE_INDENT(3);

   {
     GCriticalSectionLock lock(&threqs_lock);
     threqs_list.empty();
   }
   if((const DjVmDir *)djvm_dir)
   {
     GPList<DjVmDir::File> xfiles_list=djvm_dir->get_files_list();
     for(GPosition pos=xfiles_list;pos;++pos)
     {
       GP<DjVmDir::File> f=xfiles_list[pos];
       if (f->is_thumbnails()) djvm_dir->delete_file(f->id);
     }
   }
}

void
DjVuDocEditor::file_thumbnails(void)
      // The purpose of this function is to create files containing
      // thumbnail images and register them in DjVmDir.
      // If some of the thumbnail images are missing, they'll
      // be generated with generate_thumbnails()
{
   DEBUG_MSG("DjVuDocEditor::file_thumbnails(): updating DjVmDir\n");
   DEBUG_MAKE_INDENT(3);
   unfile_thumbnails();

      // Generate thumbnails if they're missing due to some reason.
   int thumb_num=get_thumbnails_num();
   int size=thumb_num>0 ? get_thumbnails_size() : 128;
   if (thumb_num!=get_pages_num())
   {
     generate_thumbnails(size);
   }

   DEBUG_MSG("filing thumbnails\n");

   GCriticalSectionLock lock(&thumb_lock);

      // The first thumbnail file always contains only one thumbnail
   int ipf=1;
   int image_num=0;
   int page_num=0, pages_num=djvm_dir->get_pages_num();
   MemoryByteStream *mbs=new MemoryByteStream;
   GP<ByteStream> str=mbs;
   GP<IFFByteStream> iff=new IFFByteStream(*mbs);
   iff->put_chunk("FORM:THUM");
   while(true)
   {
      GPosition pos;
      GString id=page_to_id(page_num);

      if (!thumb_map.contains(id, pos))
      {
         G_THROW("DjVuDocEditor.no_thumb\t"+GString(page_num));
      }
      TArray<char> & data=*(TArray<char> *) thumb_map[pos];
      iff->put_chunk("TH44");
      iff->write((const char *) data, data.size());
      iff->close_chunk();
      image_num++;
      page_num++;
      if (image_num>=ipf || page_num>=pages_num)
      {
         int i=id.rsearch('.');
         if(i<=0)
         {
           i=id.length();
         }
         id=id.substr(0,i)+".thumb";
            // Get unique ID for this file
         id=find_unique_id(id);

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
         mbs->seek(0);
         GP<DataPool> file_pool=new DataPool(*mbs);
         GP<File> f=new File;
         f->pool=file_pool;
         GCriticalSectionLock lock(&files_lock);
         files_map[id]=f;

            // And create new streams
         str=mbs=new MemoryByteStream;
         iff=new IFFByteStream(*mbs);
         iff->put_chunk("FORM:THUM");
         image_num=0;

            // Reset ipf to correct value (after we stored first
            // "exceptional" file with thumbnail for the first page)
         if (page_num==1) ipf=thumbnails_per_file;
         if (page_num>=pages_num) break;
      }
   }
}

int
DjVuDocEditor::generate_thumbnails(int thumb_size, int page_num)
{
   DEBUG_MSG("DjVuDocEditor::generate_thumbnails(): doing it\n");
   DEBUG_MAKE_INDENT(3);

   if(page_num<(djvm_dir->get_pages_num()))
   {
      GString id=page_to_id(page_num);
      if (!thumb_map.contains(id))
      {
         GP<DjVuImage> dimg=get_page(page_num, true);

         GRect rect(0, 0, thumb_size, dimg->get_height()*thumb_size/dimg->get_width());
         GP<GPixmap> pm=dimg->get_pixmap(rect, rect, get_thumbnails_gamma());
         if (!pm)
         {
            GP<GBitmap> bm=dimg->get_bitmap(rect, rect, sizeof(int));
            pm=new GPixmap(*bm);
         }
         if (!pm) G_THROW("DjVuDocEditor.render\t"+GString(page_num));

            // Store and compress the pixmap
         GP<IWPixmap> iwpix=new IWPixmap(pm);
         MemoryByteStream *mbs=new MemoryByteStream;
         GP<ByteStream> str=mbs;
         IWEncoderParms parms;
         parms.slices=97;
         parms.bytes=0;
         parms.decibels=0;
         iwpix->encode_chunk(*mbs, parms);
         thumb_map[id]=new TArray<char>(mbs->get_data());
      }
      ++page_num;
   }else
   {
      page_num=(-1);
   }
   return page_num;
}

void
DjVuDocEditor::generate_thumbnails(int thumb_size,
                                   bool (* cb)(int page_num, void *),
                                   void * cl_data)
{
   int page_num=0;
   do
   {
     page_num=generate_thumbnails(thumb_size,page_num);
     if (cb) if (cb(page_num, cl_data)) return;
   } while(page_num>=0);
}

static void
store_file(const GP<DjVmDir> & src_djvm_dir, const GP<DjVmDoc> & djvm_doc,
           GP<DjVuFile> & djvu_file, GMap<GURL, void *> & map)
{
   GURL url=djvu_file->get_url();
   if (!map.contains(url))
   {
      map[url]=0;

         // Store included files first
      GPList<DjVuFile> djvu_files_list=djvu_file->get_included_files(false);
      for(GPosition pos=djvu_files_list;pos;++pos)
         store_file(src_djvm_dir, djvm_doc, djvu_files_list[pos], map);

         // Now store contents of this file
      GP<DataPool> file_data=djvu_file->get_djvu_data(false, true);
      GP<DjVmDir::File> frec=src_djvm_dir->name_to_file(url.name());
      if (frec)
      {
         frec=new DjVmDir::File(*frec);
         djvm_doc->insert_file(frec, file_data, -1);
      }
   }
}

void
DjVuDocEditor::save_pages_as(ByteStream & str, const GList<int> & _page_list)
{
   GList<int> page_list=sortList(_page_list);

   GP<DjVmDoc> djvm_doc=new DjVmDoc;
   GMap<GURL, void *> map;
   for(GPosition pos=page_list;pos;++pos)
   {
      GP<DjVmDir::File> frec=djvm_dir->page_to_file(page_list[pos]);
      if (frec)
      {
         GP<DjVuFile> djvu_file=get_djvu_file(frec->id);
         if (djvu_file)
            store_file(djvm_dir, djvm_doc, djvu_file, map);
      }
   }
   djvm_doc->write(str);
}

void
DjVuDocEditor::save_file(const char * file_id, const char * save_dir,
                         bool only_modified, GMap<GString, void *> & map)
{
   DEBUG_MSG("DjVuDocEditor::save_file(): ID='" << file_id << "'\n");
   DEBUG_MAKE_INDENT(3);

   if (!map.contains(file_id))
   {
      map[file_id]=0;
      GString file_name=djvm_dir->id_to_file(file_id)->name;

      GP<DataPool> file_pool;
      GPosition pos;
      if (files_map.contains(file_id, pos))
      {
         GP<File> file_rec=files_map[pos];
         bool file_modified=file_rec->pool || file_rec->file &&
                            file_rec->file->is_modified();
         if (file_rec->file)
            file_pool=file_rec->file->get_djvu_data(false, true);
         else
            file_pool=file_rec->pool;
         if (!file_modified && only_modified)
            file_pool=0;
      }

      if (!file_pool && !only_modified)
      {
         DjVuPortcaster * pcaster=DjVuPort::get_portcaster();
         file_pool=pcaster->request_data(this, id_to_url(file_id));
      }

      if (file_pool)
      {
         GString save_name=GOS::expand_name(file_name, save_dir);
         DEBUG_MSG("Saving '" << file_id << "' to '" << save_name << "'\n");
         DataPool::load_file(save_name);
         StdioByteStream str_out(save_name, "wb");
         str_out.writall(octets, 4);
         GP<ByteStream> str_in=file_pool->get_stream();
         str_out.copy(*str_in);

         GP<ByteStream> str=file_pool->get_stream();
         IFFByteStream iff(*str);
         GString chkid;
         if (iff.get_chunk(chkid))
         {
            while(iff.get_chunk(chkid))
            {
               if (chkid=="INCL")
               {
                  GString incl_str;
                  char buffer[1024];
                  int length;
                  while((length=iff.read(buffer, 1024)))
                     incl_str+=GString(buffer, length);

                     // Eat '\n' in the beginning and at the end
                  while(incl_str.length() && incl_str[0]=='\n')
                  {
                     GString tmp=((const char *) incl_str)+1; incl_str=tmp;
                  }
                  while(incl_str.length()>0 && incl_str[(int)incl_str.length()-1]=='\n')
                     incl_str.setat(incl_str.length()-1, 0);

                  save_file(incl_str, save_dir, only_modified, map);
               }
               iff.close_chunk();
            }
         }
      }
   }
}

void
DjVuDocEditor::save(void)
{
   DEBUG_MSG("DjVuDocEditor::save(): saving the file\n");
   DEBUG_MAKE_INDENT(3);

   if (!can_be_saved())
     G_THROW("DjVuDocEditor.cant_save");
   save_as(0, orig_doc_type!=INDIRECT);
}

void
DjVuDocEditor::save_as(const char * where, bool bundled)
{
   DEBUG_MSG("DjVuDocEditor::save_as(): where='" << where << "'\n");
   DEBUG_MAKE_INDENT(3);

      // First see if we need to generate (or just reshuffle) thumbnails...
      // If we have an icon for every page, we will just call
      // file_thumbnails(), which will update DjVmDir and will create
      // the actual bundles with thumbnails (very fast)
      // Otherwise we will remove the thumbnails completely because
      // we really don't want to deal with documents, which have only
      // some of their pages thumbnailed.
   if (get_thumbnails_num()==get_pages_num())
   {
     file_thumbnails();
   }else
   { 
     remove_thumbnails();
   }

   GURL save_doc_url;
   GString save_doc_name;

   if (!where || !where[0])
   {
         // Assume, that we just want to 'save'. Check, that it's possible
         // and proceed.
      bool can_be_saved_bundled=orig_doc_type==BUNDLED ||
                                orig_doc_type==OLD_BUNDLED ||
                                orig_doc_type==SINGLE_PAGE ||
                                orig_doc_type==OLD_INDEXED && orig_doc_pages==1;
      if ((bundled ^ can_be_saved_bundled)!=0)
         G_THROW("DjVuDocEditor.cant_save2");
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

      // First consider saving in SINGLE_FILE format (one file)
   if(needs_compression())
   {
     DEBUG_MSG("Compressing on output\n");
     remove_thumbnails();
     if(! djvu_compress_codec)
     {
       G_THROW("DjVuDocEditor.no_codec");
     }
     GP<DjVmDoc> doc=get_djvm_doc();
     GP<ByteStream> mbs=new MemoryByteStream();
     doc->write(*mbs);
     mbs->flush();
     mbs->seek(0,SEEK_SET);
     djvu_compress_codec(mbs,save_doc_name,(!(const DjVmDir *)djvm_dir)||(djvm_dir->get_files_num()==1)||(save_doc_type!=INDIRECT));
     files_map.empty();
     doc_url="";
   }else
   {
     if (djvm_dir->get_files_num()==1)
     {
         // Here 'bundled' has no effect: we will save it as one page.
        DEBUG_MSG("saving one file...\n");
        GURL file_url=page_to_url(0);
        GString file_id=djvm_dir->page_to_file(0)->id;
        GP<DataPool> file_pool;
        GPosition pos;
        if (files_map.contains(file_id, pos))
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
         str_out.writall(octets, 4);
         GP<ByteStream> str_in=file_pool->get_stream();
         str_out.copy(*str_in);
        }

         // Update the document's DataPool (to save memory)
        GP<DjVmDoc> doc=get_djvm_doc();
        MemoryByteStream str;        // One page: we can do it in the memory
        doc->write(str);
        str.seek(0, SEEK_SET);
        GP<DataPool> pool=new DataPool(str);
        doc_pool=pool;
        init_data_pool=pool;

         // Also update DjVmDir (to reflect changes in offsets)
        djvm_dir=doc->get_djvm_dir();
     } else if (save_doc_type==INDIRECT)
     {
        DEBUG_MSG("Saving in INDIRECT format to '" << save_doc_url << "'\n");
        bool save_only_modified=!(save_doc_url!=doc_url || save_doc_type!=orig_doc_type);
        GString save_dir=GOS::dirname(save_doc_name);
        int pages_num=djvm_dir->get_pages_num();
        GMap<GString, void *> map;
         // First go thru the pages
        for(int page_num=0;page_num<pages_num;page_num++)
        {
         GString id=djvm_dir->page_to_file(page_num)->id;
         save_file(id, save_dir, save_only_modified, map);
        }
         // Next go thru thumbnails and similar stuff
        GPList<DjVmDir::File> xfiles_list=djvm_dir->get_files_list();
        GPosition pos;
        for(pos=xfiles_list;pos;++pos)
         save_file(xfiles_list[pos]->id, save_dir, save_only_modified, map);

         // Finally - save the top-level index file
        for(pos=xfiles_list;pos;++pos)
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
        djvm_dir->encode(iff);
        iff.close_chunk();
        iff.close_chunk();
        iff.flush();

         // Update the document data pool (not required, but will save memory)
        doc_pool=new DataPool(save_doc_name);
        init_data_pool=doc_pool;

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

         // Also update DjVmDir (to reflect changes in offsets)
        djvm_dir=doc->get_djvm_dir();
     } else
     {
       G_THROW("DjVuDocEditor.cant_save");
     }

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

   }
   orig_doc_type=save_doc_type;
   doc_type=save_doc_type;

   if (doc_url!=save_doc_url)
   {
     // Also update document's URL (we moved, didn't we?)
     doc_url=save_doc_url;
     init_url=save_doc_url;
   }
}
