//C-  -*- C++ -*-
//C- DjVu� Reference Library (v. 3.0)
//C- 
//C- Copyright � 1999-2000 LizardTech, Inc. All Rights Reserved.
//C- 
//C- This software (the "Original Code") is subject to, and may be
//C- distributed under, the GNU General Public License, Version 2.
//C- The license should have accompanied the Original Code or you
//C- may obtain a copy of the license from the Free Software
//C- Foundation at http://www.fsf.org .
//C- 
//C- With respect to the Original Code, and subject to any third
//C- party intellectual property claims, LizardTech grants recipient
//C- a worldwide, royalty-free, non-exclusive license under patent
//C- claims infringed by making, using, or selling Original Code
//C- which are now or hereafter owned or controlled by LizardTech,
//C- but solely to the extent that any such patent is reasonably
//C- necessary to enable you to make, have made, practice, sell, or 
//C- otherwise dispose of Original Code (or portions thereof) and
//C- not to any greater extent that may be necessary to utilize
//C- further modifications or combinations.
//C- 
//C- The Original Code is provided "AS IS" WITHOUT WARRANTY OF ANY
//C- KIND, EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO
//C- ANY WARRANTY OF NON-INFRINGEMENT, OR ANY IMPLIED WARRANTY OF 
//C- MERCHANTIBILITY OF FITNESS FOR A PARTICULAR PURPOSE.
// 
// $Id: DjVuDocEditor.h,v 1.27 2000-11-03 02:08:36 bcr Exp $
// $Name:  $

#ifndef _DJVUDOCEDITOR_H
#define _DJVUDOCEDITOR_H

#ifdef __GNUC__
#pragma interface
#endif

#include "DjVuDocument.h"

/** @name DjVuDocEditor.h
    Files #"DjVuDocEditor.h"# and #"DjVuDocEditor.cpp"# contain extension
    of \Ref{DjVuDocument} class, which can create and modify existing
    DjVu document, generate thumbnails, etc. It does {\bf not} do
    compression though.

    @memo DjVu document editor class.
    @author Andrei Erofeev <eaf@geocities.com>
    @version #$Id: DjVuDocEditor.h,v 1.27 2000-11-03 02:08:36 bcr Exp $#
*/

//@{

/** #DjVuDocEditor# is an extension of \Ref{DjVuDocument} class with
    additional capabilities for editing the document contents.

    It can be used to:
    \begin{enumerate}
       \item Create (compose) new multipage DjVu documents using single
             page DjVu documents. The class does {\bf not} do compression.
       \item Insert and remove different pages of multipage DjVu documents.
       \item Change attributes ({\em names}, {\em IDs} and {\em titles})
             of files composing the DjVu document.
       \item Generate thumbnail images and integrate them into the document.
    \end{enumerate}
*/

class DjVuDocEditor : public DjVuDocument
{
public:
   static int	thumbnails_per_file;

      /// Default constructor
   DjVuDocEditor(void);

      /** Initialization function. Initializes an empty document.

	  {\bf Note}: You must call either of the two
	  available \Ref{init}() function before you start doing
	  anything else with the #DjVuDocEditor#. */
   void		init(void);

      /** Initialization function. Opens document with name #fname#.

	  {\bf Note}: You must call either of the two
	  available \Ref{init}() function before you start doing
	  anything else with the #DjVuDocEditor#. */
   void		init(const char * fname);

      /// Destructor
   virtual ~DjVuDocEditor(void);

      /** Returns type of open document. #DjVuDocEditor# silently
	  converts any open DjVu document to #BUNDLED# format (see
	  \Ref{DjVuDocument}. Thus, \Ref{DjVuDocument::get_doc_type}()
	  will always be returning #BUNDLED#. Use this function to
	  learn the original format of the document being edited. */
   int		get_orig_doc_type(void) const;

      /** Returns #TRUE# if the document can be "saved" (sometimes
	  the only possibility is to do a "save as"). The reason why
	  we have this function is that #DjVuDocEditor# can save
	  documents in new formats only (#BUNDLED# and #INDIRECT#).
	  At the same time it recognizes all DjVu formats (#OLD_BUNDLED#,
	  #OLD_INDEXED#, #BUNDLED#, and #INDIRECT#).

	  #OLD_BUNDLED# and #BUNDLED# documents occupy only one file,
	  so in this case "saving" involves the automatic conversion
	  to #BUNDLED# format and storing data into the same file.

	  #OLD_INDEXED# documents, on the other hand, occupy more
	  than one file. They could be converted to #INDIRECT# format
	  if these two formats had the same set of files. Unfortunately,
	  these formats are too different, and the best thing to do
	  is to use "save as" capability. */
   bool		can_be_saved(void) const;

      /** Returns type of the document, which can be created by
	  \Ref{save}() function. Can be #INDIRECT#, #BUNDLED#,
	  #SINGLE_PAGE#, or #UNKNOWN_TYPE#. The latter indicates,
	  that \Ref{save}() will fail, and that \Ref{save_as}()
	  should be used instead */
   int		get_save_doc_type(void) const;

      /** Saves the document. May generate exception if the document
	  can not be saved, and \Ref{save_as}() should be used.
	  See \Ref{can_be_saved}() for details. */
   void		save(void);

      /** Saves the document. */
   virtual void	save_as(const char * where, bool bundled);

      /** Saves the specified pages in DjVu #BUNDLED# multipage document. */
   void		save_pages_as(ByteStream & str, const GList<int> & page_list);

      /** Translates page number #page_num# to ID. If #page_num# is invalid,
	  an exception is thrown. */
   GString	page_to_id(int page_num) const;
   
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
   /** Inserts a new page with data inside the #data_pool# as page
       number #page_num.

	  @param data_pool \Ref{DataPool} with data for this page.
	  @param file_name Name, which will be assigned to this page.
	  	 If you try to save the document in #INDIRECT# format,
		 a file with this name will be created to hold the
		 page's data. If there is already a file in the document
		 with the same name, the function will derive a new
		 unique name from file_name, which will be assigned
		 to the page.
	  @param page_num Describes where the page should be inserted.
	  	 Negative number means "append". */
   void		insert_page(GP<DataPool> & file_pool,
			    const char * fname, int page_num=-1);
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
   void		insert_group(const GList<GString> & fname_list, int page_num=-1,
			     void (* refresh_cb)(void *)=0, void * cl_data=0);
      /** Removes the specified page from the document. If #remove_unref#
	  is #TRUE#, the function will also remove from the document any file,
	  which became unreferenced due to the page's removal */
   void		remove_page(int page_num, bool remove_unref=true);
      /** Removes the specified pages from the document. If #remove_unref#
	  is #TRUE#, the function will also remove from the document any file,
	  which became unreferenced due to the pages' removal */
   void		remove_pages(const GList<int> & page_list, bool remove_unref=true);
      /** Removes a DjVu file with the specified #id#.

	  If some other files include this file, the corresponding #INCL#
	  chunks will be removed to avoid dead links.

	  If #remove_unref# is #TRUE#, the function will also remove every
	  file, which will become unreferenced after the removal of this file. */
   void		remove_file(const char * id, bool remove_unref=true);
      /** Makes page number #page_num# to be #new_page_num#. If #new_page_num#
	  is negative or too big, the function will move page #page_num# to
	  the end of the document. */
   void		move_page(int page_num, int new_page_num);
      /** Shifts all pags from the #page_list# according to the #shift#.
	  The #shift# can be positive (shift toward the end of the document)
	  or negative (shift toward the beginning of the document).

	  It is OK to make #shift# too big in value. Pages will just be
	  moved to the end (or to the beginning, depending on the #shift#
	  sign) of the document. */
   void		move_pages(const GList<int> & page_list, int shift);
   
      /** Changes the name of the file with ID #id# to #name#.
	  Refer to \Ref{DjVmDir} for the explanation of {\em IDs},
          {\em names} and {\em titles}. */
   void		set_file_name(const char * id, const char * name);
      /** Changes the name of the page #page_num# to #name#.
	  Refer to \Ref{DjVmDir} for the explanation of {\em IDs},
          {\em names} and {\em titles}. */
   void		set_page_name(int page_num, const char * name);
      /** Changes the title of the file with ID #id# to #title#.
	  Refer to \Ref{DjVmDir} for the explanation of {\em IDs},
          {\em names} and {\em titles}. */
   void		set_file_title(const char * id, const char * title);
      /** Changes the title of the page #page_num# to #title#.
	  Refer to \Ref{DjVmDir} for the explanation of {\em IDs},
          {\em names} and {\em titles}. */
   void		set_page_title(int page_num, const char * title);

      /** @name Thumbnails */
      //@{
      /** Returns the number of thumbnails stored inside this document.

	  It may be #ZERO#, which means, that there are no thumbnails at all.

	  It may be equal to the number of pages, which is what should
	  normally be.

	  Finally, it may be greater than #ZERO# and less than the number
	  of pages, in which case thumbnails should be regenerated before
	  the document can be saved. */
   int		get_thumbnails_num(void) const;

      /** Returns the size of the first encountered thumbnail image. Since
	  thumbnails can currently be generated by \Ref{generate_thumbnails}()
	  only, all thumbnail images should be of the same size. Thus,
	  the number returned is actually the size of {\em all}
	  document thumbnails.

	  The function will return #-1# if there are no thumbnails. */
   int		get_thumbnails_size(void) const;

      /** Removes all thumbnails from the document */
   void		remove_thumbnails(void);

      /** Generates thumbnails for the specified page, if and only if
          it does not have a thumbnail yet.  If you want to regenerate
          thumbnails for all pages, call \Ref{remove_thumbnails}() prior
          to calling this function.

	  @param thumb_size The size of the thumbnails in pixels. DjVu viewer
	         is able to rescale the thumbnail images if necessary, so this
		 parameter affects thumbnails quality only. 128 is a good number.
	  @param page_num The page number to genate the thumbnail for.  */
   int		generate_thumbnails(int thumb_size, int page_num);

      /** Generates thumbnails for those pages, which do not have them yet.
	  If you want to regenerate thumbnails for all pages, call
	  \Ref{remove_thumbnails}() prior to calling this function.

	  @param thumb_size The size of the thumbnails in pixels. DjVu viewer
	         is able to rescale the thumbnail images if necessary, so this
		 parameter affects thumbnails quality only. 128 is a good number.
	  @param cb The callback, which will be called after thumbnail image
	         for the next page has been generated. Regardless of if
		 the document already has thumbnail images for some of its
		 pages, the callback will be called #pages_num# times, where
		 #pages_num# is the total number of pages in the document.
		 The callback should return #FALSE# if thumbnails generating
		 should proceed. #TRUE# will stop it. */
   void		generate_thumbnails(int thumb_size,
				    bool (* cb)(int page_num, void *)=0,
				    void * cl_data=0);
      //@}
      /** Use this function to simplify annotations in the document.
        The "simplified" format is when annotations are only allowed
        either in top-level page files or in a special file with
        #SHARED_ANNO# flag on. This file is supposed to be included into
        every page. */
   void               simplify_anno(void (* progress_cb)(float progress, void *)=0,
                            void * cl_data=0);

      /** Will create a file that will be included into every page and
        marked with the #SHARED_ANNO# flag. This file can be used
        to store global annotations (annotations applicable to every page).

        {\bf Note:} There may be only one #SHARED_ANNO# file in any
        DjVu multipage document. */
   void               create_shared_anno_file(void (* progress_cb)(float progress, void *)=0,
                                      void * cl_data=0);

      /** Returns a pointer to the file with #SHARED_ANNO# flag on.
        This file should be used for storing document-wide annotations.

        {\bf Note:} There may be only one #SHARED_ANNO# file in any
        DjVu multipage document. */
   GP<DjVuFile>       get_shared_anno_file(void);

   GURL               get_doc_url(void) const;
                                                                              
      /** Returns TRUE if #class_name# is #"DjVuDocEditor"#,
	  #"DjVuDocument"# or #"DjVuPort"# */
   virtual bool		inherits(const char * class_name) const;
   virtual GP<DataPool>	request_data(const DjVuPort * source, const GURL & url);
protected:
   virtual GP<DjVuFile>	url_to_file(const GURL & url, bool dont_create);
   virtual GP<DataPool> get_thumbnail(int page_num, bool dont_decode);
   friend class CThumbNails;
public:
   class File;
private:
   bool		initialized;
   GURL		doc_url;
   GP<DataPool>	doc_pool;
   GString	tmp_doc_name;
   int		orig_doc_type;
   int		orig_doc_pages;

   GPMap<GString, File>	files_map; 	// files_map[id]=GP<File>
   GCriticalSection	files_lock;

   GMap<GString, void *>thumb_map;
   GCriticalSection	thumb_lock;

   void		(* refresh_cb)(void *);
   void		* refresh_cl_data;

   void		check(void);
   GString	find_unique_id(const char * id);
   GP<DataPool>	strip_incl_chunks(GP<DataPool> & pool);
   void		clean_files_map(void);
   bool		insert_file_type(const char * file_name,
                            DjVmDir::File::FILE_TYPE page_type,
		            int & file_pos, GMap<GString, GString> & name2id);
   bool		insert_file(const char * file_name, bool is_page,
			    int & file_pos, GMap<GString, GString> & name2id);
   void		remove_file(const char * id, bool remove_unref,
			    GMap<GString, void *> & ref_map);
   void		generate_ref_map(const GP<DjVuFile> & file,
				 GMap<GString, void *> & ref_map,
				 GMap<GURL, void *> & visit_map);
   void		move_file(const char * id, int & file_pos,
			  GMap<GString, void *> & map);
   void		unfile_thumbnails(void);
   void		file_thumbnails(void);
   void		save_file(const char * id, const char * dir,
			  bool only_modified, GMap<GString, void *> & map);
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
   return !(needs_rename()||needs_compression()||orig_doc_type==UNKNOWN_TYPE ||
	    orig_doc_type==OLD_INDEXED);
}

inline int
DjVuDocEditor::get_save_doc_type(void) const
{
   if (orig_doc_type==SINGLE_PAGE)
      if (djvm_dir->get_files_num()==1) return SINGLE_PAGE;
      else return BUNDLED;
   else if (orig_doc_type==INDIRECT) return INDIRECT;
   else if (orig_doc_type==OLD_BUNDLED || orig_doc_type==BUNDLED) return BUNDLED;
   else return UNKNOWN_TYPE;
}

inline GURL
DjVuDocEditor::get_doc_url(void) const
{
   return doc_url.is_empty() ? init_url : doc_url;
}

class GDjVuDocEditor
{
  GP<DjVuDocEditor> doc;
public:
  GDjVuDocEditor(DjVuDocEditor *_doc) : doc(_doc) {}
  bool operator!(void) const {return !(const DjVuDocEditor *)doc;}
  DjVuDocEditor* operator-> (void) {return doc;}
  const DjVuDocEditor* operator-> (void) const {return doc;}
  operator GP<DjVuDocument> (void) { return (DjVuDocEditor *)doc;}
  operator DjVuDocEditor *(void) { return doc;}
  operator const DjVuDocEditor *(void) const { return doc; }
};


//@}

#endif

