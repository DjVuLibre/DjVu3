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
//C- $Id: DjVuDocument.h,v 1.5 1999-06-04 15:55:17 leonb Exp $
 
#ifndef _DJVUDOCUMENT_H
#define _DJVUDOCUMENT_H

#ifdef __GNUC__
#pragma interface
#endif

#include "GSmartPointer.h"
#include "DjVuFile.h"
#include "DjVuImage.h"
#include "DjVmDir0.h"
#include "DjVmFile.h"

/** @name DjVuDocument.h
    Files #"DjVuDocument.h"# and #"DjVuDocument.cpp"# contain implementation
    of the \Ref{DjVuDocument} class - the ideal tool for opening, decoding
    and saving DjVu single page and multi page documents.

    @memo DjVu document class.
    @author Andrei Erofeev <eaf@geocities.com>, L\'eon Bottou <leonb@research.att.com>
    @version #$Id: DjVuDocument.h,v 1.5 1999-06-04 15:55:17 leonb Exp $#
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
    
class DjVuDocument : public GPEnabled, public DjVuPort
{
public:
      /** Default constructor. Constructs empty #DjVuDocument#.

	  @param port If not #ZERO#, all requests and notifications will
	         be sent to it. Otherwise #DjVuDocument# will create an internal
		 instance of \Ref{DjVuSimplePort} for these purposes.
		 It's OK to make it #ZERO# if you're writing a command line
		 tool, which normally reports errors to #stderr#. */
   DjVuDocument(DjVuPort * port=0);
      /** Constructs the #DjVuDocument# object using an existing document.
	  The #url# should point to the real data, and the creator of the
	  document should be ready to return this data to the document
	  if it's not stored locally.

	  Before the constructor terminates, it will query for data using
	  the communication mechanism provided by \Ref{DjVuPort} and
	  \Ref{DjVuPortcaster}. If #port# is not #ZERO#, then the request will
	  be forwarded to it. If it {\bf is} #ZERO# then #DjVuDocument# will
	  create an internal instance of \Ref{DjVuSimplePort} and will use
	  it to access local files and report errors to #stderr#.

	  The #url# passed may point to DjVm (all-in-one-file multipage)
	  document or to any page of DjVu document.

	  @param url The URL pointing to the document. If it's a all-in-one-file
	         multipage DjVu document (also called DjVm) then it should
		 point to it. It can also be a URL of any page of the document
		 with every page in a separate file.
	  @param readonly If FALSE the document will be opened for editing.
	         This means, f.i. that all pages will be created in the
		 \Ref{DjVuDocument::DjVuDocument}() constructor.
	  @param port If not #ZERO#, all requests and notifications will
	         be sent to it. Otherwise #DjVuDocument# will create an internal
		 instance of \Ref{DjVuSimplePort} for these purposes.
		 It's OK to make it #ZERO# if you're writing a command line
		 tool, which should work with files on the hard disk only.
	  @param cache Is meaningful only in readonly mode when it's used
	         to cache \Ref{DjVuFile}s. */
   DjVuDocument(const GURL & url, bool readonly,
		DjVuPort * port=0, GCache<GURL, DjVuFile> * cache=0);
   virtual ~DjVuDocument(void);

      /** @name Accessing pages */
      //@{
      /** Returns \Ref{GP} pointer to \Ref{DjVuImage} corresponding to page
          #page_num#. The image doesn't have to be decoded in full.  If
          multithreaded behaviour is allowed, the decoding will be started in
          a separate thread, which enables to do progressive display.
          Negative #page_num# has a special meaning for the multipage
          documents with each page in separate files: the #DjVuDocument# will
          start decoding of the URL with which it has been initialized. */
   GP<DjVuImage>get_page(int page_num);
      /** Creates and returns \Ref{DjVuFile} corresponding to the given #url#.
	  For DjVm document (all-in-one-file multipage DjVu documents) the
	  #url# is faked by appending file name to the document's URL.
	  If there is a \Ref{DjVuFile} corresponding to this URL in the
	  cache already, it will be returned. */
   GP<DjVuFile>	get_djvu_file(const GURL & url);
      /** Creates and returns \Ref{DjVuFile} corresponding to the given
	  page. Cache will be used to take advantage of files created and
	  decoded before. Negative #page_num# has a special meaning for the
	  multipage documents with each page in separate files: the
	  #DjVuDocument# will return the file corresponding to the URL with
	  which it has been initialized. */
   GP<DjVuFile>	get_djvu_file(int page_num);
      /** Returns navigation directory of the document. This is the only
	  way to figure out the number of pages in the document and URLs
	  of files corresponding to every page. */
   GP<DjVuNavDir>	get_dir(void) const;
      //@}

      /// Returns cache being used.
   GCache<GURL, DjVuFile> * get_cache(void) const;

      /** @name Editing tools */
      //@{
      /** Inserts the given \Ref{DjVuFile} as page #page_num#. If #page_num#
	  is negative, the page will be appended. This will change the
	  file's URL, will update the document's navigation directory,
	  will insert a link to this directory into the file, and finally
	  will insert the file into the internal list. If there was {\em no}
          navigation directory before, it will be created automatically. */
   void		insert_page(const GP<DjVuFile> & file, int page_num=-1);
      /** Removes page #page_num# from the document. This will update the
	  navigation directory. If there will be only one page left, the
          navigation directory will be removed from the document automatically. */
   void		delete_page(int page_num);
      //@}

      /** @name DjVm related stuff */
      //@{
      /** Returns TRUE if the open document was in DjVm (all-in-one-file)
	  format. */
   bool		is_djvm(void) const;
      /** Returns TRUE if DjVm document contains a file with given URL.
	  File URLs in the case of DjVm documents are basically fake URLs
	  composed by appending the file name to the document's URL. */
   bool		djvm_contains(const GURL & url) const;
      /// Returns the document's URL
   GURL		djvm_get_url(void) const;
      /** Returns URL corresponding to the first page. It's a valuable
	  information used internally to get a pointer to the first page
	  in the document before the navigation directory is decoded. */
   GURL		djvm_get_first_page_url(void) const;
      /** Returns the \Ref{DataPool} that the document has been initialized
	  with. It contains all the document's data. */
   GP<DataPool>	djvm_get_init_data_pool(void) const;
      //@}

      /** @name Saving routines */
      //@{
      /** Stores all files in the document into binary array in DjVm format
	  and returns it. Please beware, that if not all files have already
	  been loaded, this will generate a bunch of requests for data. */
   TArray<char>	get_djvm_data(void);
      /// Will save the document in DjVm format in a file with name #file_name#
   void		save_as_djvm(const char * file_name);
      /** Will save the document in DjVu format (every page in a separate file)
	  into directory with name #dir_name#. */
   void		save_as_djvu(const char * dir_name);
      //@}

      /// Returns TRUE if #class_name# is #"DjVuDocument"# or #"DjVuPort"#
   virtual bool		inherits(const char * class_name) const;
   
   virtual GP<DataRange>request_data(const DjVuPort * source, const GURL & url);
   virtual void		notify_chunk_done(const DjVuPort * source, const char * name);
private:
   GCache<GURL, DjVuFile> * cache;
   DjVuSimplePort	* simple_port;
   bool			readonly;

   bool			dummy_dir;
   GP<DjVuNavDir>	dir;
   GP<DjVuFile>		dir_file;	// Used in editing only

   GPList<DjVuFile>	added_files_list;
   GCriticalSection	added_files_lock;

      // DjVm format related data
   bool			djvm;
   GURL			djvm_doc_url;
   GP<DataPool>		djvm_pool;
   DjVmDir0		djvm_dir;
   GString		djvm_first_page_name;

   GURL			init_url;

   void			detect_doc_type(const GURL & doc_url);
   GString		djvm_url2name(const GURL & url) const;
   GURL			djvm_name2url(const char * name) const;

   GP<DjVmFile>		get_djvm_file(void);
   GPList<DjVuFile>	get_shared_files(void);
   void			check_nav_structure(void);
   void			unlink_empty_files(void);
   void			add_to_cache(const GP<DjVuFile> & f);
};

inline bool
DjVuDocument::inherits(const char * class_name) const
{
   return
      !strcmp("DjVuDocument", class_name) ||
      DjVuPort::inherits(class_name);
}

inline bool
DjVuDocument::is_djvm(void) const
{
   if (!readonly)
      THROW("The document is being modified. Request for type is irrelevant.");
   return djvm;
}

inline GP<DataPool>
DjVuDocument::djvm_get_init_data_pool(void) const
{
   if (!djvm) THROW("The document is not in DJVM format.");
   return djvm_pool;
}

inline GURL
DjVuDocument::djvm_get_url(void) const
{
   if (!djvm) THROW("The document is not in DJVM format.");
   return djvm_doc_url;
}

inline GP<DjVuNavDir>
DjVuDocument::get_dir(void) const
{
   return dir;
}

inline GCache<GURL, DjVuFile> *
DjVuDocument::get_cache(void) const
{
   return cache;
}

//@}

#endif
