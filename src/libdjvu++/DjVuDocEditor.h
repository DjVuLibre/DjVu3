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
//C- $Id: DjVuDocEditor.h,v 1.4 1999-11-22 21:10:51 eaf Exp $
 
#ifndef _DJVUDOCEDITOR_H
#define _DJVUDOCEDITOR_H

#ifdef __GNUC__
#pragma interface
#endif

#include "DjVuDocument.h"

/** @name DjVuDocEditor.h
    Files #"DjVuDocument.h"# and #"DjVuDocument.cpp"# contain implementation
    of the \Ref{DjVuDocument} class - the ideal tool for opening, decoding
    and saving DjVu single page and multi page documents.

    @memo DjVu document class.
    @author Andrei Erofeev <eaf@geocities.com>, L\'eon Bottou <leonb@research.att.com>
    @version #$Id: DjVuDocEditor.h,v 1.4 1999-11-22 21:10:51 eaf Exp $#
*/

//@{

/** #DjVuDocument# provides convenient interface for opening, decoding
    and saving back DjVu documents of any format.

    {\bf Conversion.} Since #DjVuDocument# can open and save DjVu documents
    in any of two supported formats (all-in-one-file documents and
    each-page-in-separate-file documents), it can be used to do conversion
    between these two formats.

    {\bf Decoding.} #DjVuDocument# provides convenient interface for obtaining
    \Ref{DjVuImage} corresponding to any page of the document. It uses
    \Ref{GCache} to do caching thus avoiding unnecessary multiple decoding of
    the same page. The real decoding though is accomplished by \Ref{DjVuFile}.

    {\bf Modifying.} With #DjVuDocument# you can open an existing document
    for editing. Editing functionality includes insertion and deletion of
    pages (\Ref{DjVuFile}s). Using capabilities provided by \Ref{DjVuFile}
    it's possible to create DjVu documents of any desired structure: with
    deep inclusion, shared dictionaries, annotations, etc.

    {\bf Messenging.} Being derived from \Ref{DjVuPort}, #DjVuDocument#
    takes active part in exchanging messages (requests and notifications)
    between different parties involved in decoding. It reports (relays)
    errors, progress information, handles some requests for data (when
    these requests deal with local files. */
    
class DjVuDocEditor : public DjVuDocument
{
public:
   DjVuDocEditor(void);

   void		init(void);
   void		init(const char * fname);
   
   virtual ~DjVuDocEditor(void);

   int		get_orig_doc_type(void) const;
   bool		can_be_saved(void) const;
   int		get_save_doc_type(void) const;

   void		save(void);
   
   virtual void	save_as(const char * where, bool bundled);
   
   GString	insert_file(const char * fname, const char * parent_id,
			    int chunk_num=1);
      /** Inserts the referenced file into this DjVu document.

	  @param fname Name of the top-level file containing the image of
	  	 the page to be inserted. This file must be a DjVu file and
		 may include one or more other DjVu files.

		 If it include other DjVu files, the function will try to
		 insert them into the document too. Should this attempt fail,
		 the corresponding #INCL# chunk will be removed from the
		 referencing file and an exception will be thrown.

		 When inserting a file, the function may modify its name
		 to be unique in the DjVu document.
	  @param page_num Position where the new page should be inserted at.
	  	 Negative value means "append" */
   void		insert_page(const char * fname, int page_num=-1);
      /** Inserts a group of pages into this DjVu document.
	  
	  Like \Ref{insert_page}() it will insert every page into the document.
	  The main advantage of calling this function once for the whole
	  group instead of calling \Ref{insert_page}() for every page is
	  the processing of included files:

	  The group of files may include one or more files, which are thus
	  shared by them. If you call \Ref{insert_page}() for every page,
	  this shared file will be inserted into the document more than once
	  though under different names. This is how \Ref{insert_page}() works:
	  whenever it inserts something, it checks for duplicate names with
	  only one purpose: invent a new name if a given one is already in
	  use.

	  On the other hand, if you call #insert_group#(), it will insert
	  shared included files only once. This is because it can analyze
	  the group of files before inserting them and figure out what files
	  are shared and thus should be inserted only once.

	  @param fname_list List of top-level files for the pages to be inserted
	  @param page_num Position where the new pages should be inserted at.
	  	 Negative value means "append" */
   void		insert_group(const GList<GString> & fname_list, int page_num=-1);
   void		generate_thumbnails(int thumb_size, int images_per_file,
				    void (* cb)(int page_num, void *)=0,
				    void * cl_data=0);
   
      /** Returns TRUE if #class_name# is #"DjVuDocEditor"#,
	  #"DjVuDocument"# or #"DjVuPort"# */
   virtual bool		inherits(const char * class_name) const;
   virtual GP<DataPool>	request_data(const DjVuPort * source, const GURL & url);
protected:
   virtual GP<DjVuFile>	url_to_file(const GURL & url, bool dont_create);
private:
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
   class File : public GPEnabled
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
   bool		initialized;
   GURL		doc_url;
   GP<DataPool>	doc_pool;
   GString	tmp_doc_name;
   int		orig_doc_type;
   int		orig_doc_pages;

   GPMap<GURL, File>	files_map;
   GCriticalSection	files_lock;

   void		check(void);
   GString	find_unique_id(const char * id);
   GP<DataPool>	strip_incl_chunks(const GP<DataPool> & pool);
   void		clean_files_map(void);
   void		insert_file(const char * file_name, bool is_page,
			    int & file_pos, GMap<GString, GString> & name2id);
};

inline bool
DjVuDocEditor::inherits(const char * class_name) const
{
   return
      !strcmp("DjVuDocEditor", class_name) ||
      DjVuDocument::inherits(class_name);
}

inline int
DjVuDocEditor::get_orig_doc_type(void) const
{
   return orig_doc_type;
}

inline bool
DjVuDocEditor::can_be_saved(void) const
{
   return !(orig_doc_type==UNKNOWN_TYPE ||
	    orig_doc_type==OLD_INDEXED);
}

inline int
DjVuDocEditor::get_save_doc_type(void) const
{
   if (orig_doc_type==SINGLE_PAGE)
      if (get_pages_num()==1) return SINGLE_PAGE;
      else return BUNDLED;
   else if (orig_doc_type==INDIRECT) return INDIRECT;
   else if (orig_doc_type==OLD_BUNDLED || orig_doc_type==BUNDLED) return BUNDLED;
   else return UNKNOWN_TYPE;
}

//@}

#endif
