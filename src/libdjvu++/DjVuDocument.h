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
//C- $Id: DjVuDocument.h,v 1.15 1999-09-03 23:35:40 leonb Exp $
 
#ifndef _DJVUDOCUMENT_H
#define _DJVUDOCUMENT_H

#ifdef __GNUC__
#pragma interface
#endif

#include "DjVuGlobal.h"
#include "GSmartPointer.h"
#include "GCache.h"
#include "DjVuFile.h"
#include "DjVuImage.h"
#include "DjVmDir0.h"
#include "DjVmDoc.h"

/** @name DjVuDocument.h
    Files #"DjVuDocument.h"# and #"DjVuDocument.cpp"# contain implementation
    of the \Ref{DjVuDocument} class - the ideal tool for opening, decoding
    and saving DjVu single page and multi page documents.

    @memo DjVu document class.
    @author Andrei Erofeev <eaf@geocities.com>, L\'eon Bottou <leonb@research.att.com>
    @version #$Id: DjVuDocument.h,v 1.15 1999-09-03 23:35:40 leonb Exp $#
*/

//@{

/** #DjVuDocument# provides convenient interface for opening, decoding
    and saving back DjVu documents in single page and multi page formats.

    {\bf Input formats}
    It can read multi page DjVu documents in either of the 4 formats: 2
    obsolete ({\em old bundled} and {\em old indexed}) and two new
    ({\em new bundled} and {\em new indirect}).

    {\bf Output formats}
    To encourage users to switch to the new formats, the #DjVuDocument# can
    save documents back only in the new formats: {\em bundled} and
    {\em indirect}.

    {\bf Conversion.} Since #DjVuDocument# can open DjVu documents in
    an obsolete format and save it in any of the two new formats
    ({\em new bundled} and {\em new indirect}), this class can be used for
    conversion from obsolete formats to the new ones. Although it can also
    do conversion between the new two formats, it's not the best way to
    do it. Please refer to \Ref{DjVmDoc} for details.

    {\bf Decoding.} #DjVuDocument# provides convenient interface for obtaining
    \Ref{DjVuImage} corresponding to any page of the document. It uses
    \Ref{GCache} to do caching thus avoiding unnecessary multiple decoding of
    the same page. The real decoding though is accomplished by \Ref{DjVuFile}.

    {\bf Messenging.} Being derived from \Ref{DjVuPort}, #DjVuDocument#
    takes an active part in exchanging messages (requests and notifications)
    between different parties involved in decoding. It reports (relays)
    errors, progress information and even handles some requests for data (when
    these requests deal with local files).

    Typical usage of #DjVuDocument# class in a threadless command line
    program would be the following:
    \begin{verbatim}
    GString file_name="/tmp/document.djvu";
    GP<DjVuDocument> doc=new DjVuDocument(GOS::filename_to_url(file_name));
    int pages=doc->get_pages_num();
    for(int page=0;page<pages;page++)
    {
       GP<DjVuImage> dimg=doc->get_page(page);
       // Do something
    };
    \end{verbatim}
    {\bf Comments for the code above}
    \begin{enumerate}
       \item Since the document is assumed to be stored on the hard drive,
             we don't have to cope with \Ref{DjVuPort}s and can pass
	     #ZERO# pointer to the document constructor. #DjVuDocument#
	     can access local data itself. In the case of a plugin though,
	     one would have to implement his own \Ref{DjVuPort}, which
	     would handle requests for data arising when the document
	     is being decoded.
       \item Since the program is threadless, the image will be created and
             decoded in full before #doc->get_page(page)# returns. In a
	     a multithreaded application the image will be returned right
	     after creation. The decoding will proceed in another thread.
	     This allows to do progressive redisplay in the plugin. Should
	     a necessity to synchronize occur, one can use the following:
	     #dimg->get_djvu_file()->wait_for_finish()#.
       \item See Also: \Ref{DjVuFile}, \Ref{DjVuImage}, \Ref{GOS}.
    \end{enumerate}
*/
    
class DjVuDocument : public DjVuPort
{
public:
      /** There are 4 DjVu multipage formats, which are currently recognized
	  by the plugin and other decoding programs. These constants are
	  returned by \Ref{get_doc_type}() and should help to identify format
	  of a given document.
	  \begin{enumerate}
	     \item #OLD_BUNDLED# - Obsolete bundled format
	     \item #INDEXED# - Obsolete multipage format where every page
	           is stored in a separate file and "includes" (by means
		   of an #INCL# chunk) the file with the document directory.
	     \item #BUNDLED# - Currently supported bundled format
	     \item #INDIRECT# - Currently supported "expanded" format, where
	           every page and component is stored in a separate file.
          \end{enumerate} */
   enum DOC_TYPE { OLD_BUNDLED=1, INDEXED, BUNDLED, INDIRECT };
   
   DjVuDocument(void);
   virtual ~DjVuDocument(void);

      /** Initializes the #DjVuDocument# object using an existing document.
          This function should be called once after creating the object.
	  The #url# should point to the real data, and the creator of the
	  document should be ready to return this data to the document
	  if it's not stored locally (in which case #DjVuDocument# can
	  access it itself).

	  Before the constructor terminates, it will query for data using
	  the communication mechanism provided by \Ref{DjVuPort} and
	  \Ref{DjVuPortcaster}. If #port# is not #ZERO#, then the request will
	  be forwarded to it. If it {\bf is} #ZERO# then #DjVuDocument# will
	  create an internal instance of \Ref{DjVuSimplePort} and will use
	  it to access local files and report errors to #stderr#. The net
	  consequence of this is that you may pass the pointer to \Ref{DjVuPort}
	  as #ZERO# if the file is stored locally because #DjVuDocument#
	  is able to access local data itself.

	  Depending on the document type the #url# should point to:
	  \begin{itemize}
	     \item {\bf Old bundled} and {\bf New bundled} formats: to the
	           document itself.
	     \item {\bf Old indexed} format: to any page of the document.
	     \item {\bf New indirect} format: to the top-level file of the
	           document. If (like in the {\em old indexed} format) you
		   point the #url# to a page, the page {\em will} be decoded,
		   but it will {\em not} be recognized to be part of the
		   document.
	  \end{itemize}

	  @param url The URL pointing to the document. If the document is
	         in a {\em bundled} format then the URL should point to it.
		 If the document is in the {\em old indexed} format then
		 URL may point to any page of this document. For {\em new
		 indirect} format the URL should point to the top-level
		 file of the document.
	  @param port If not #ZERO#, all requests and notifications will
	         be sent to it. Otherwise #DjVuDocument# will create an internal
		 instance of \Ref{DjVuSimplePort} for these purposes.
		 It's OK to make it #ZERO# if you're writing a command line
		 tool, which should work with files on the hard disk only
		 because #DjVuDocument# can access such files itself.
	  @param cache It's used to cache decoded \Ref{DjVuFile}s and
	         is actually useful in the plugin only. */
   void         init(const GURL & url, GP<DjVuPort> port=0, 
                     GCache<GURL, DjVuFile> * cache=0);

      /** Returns type of the document: #DjVuDocument::OLD_BUNDLED# or
	  #DjVuDocument::INDEXED# or #DjVuDocument:BUNDLED# or
	  #DjVuDocument::INDIRECT#. The first two formats are obsolete. */
   int		get_doc_type(void) const;

      /** Returns #TRUE# if the document is in bundled format (either in
	  #DjVuDocument::OLD_BUNDLED# or #DjVuDocument::BUNDLED# formats). */
   bool		is_bundled(void) const;

      /// Returns the URL passed to the constructor
   GURL		get_init_url(void) const;

      /// Returns data corresponding to the URL passed to the constructor.
   GP<DataPool>	get_init_data_pool(void) const;

      /** @name Accessing pages */
      //@{
      /// Returns the number of pages in the document
   int		get_pages_num(void);
      /** Translates the page number to an internal ID. (Refer to
	  \Ref{DjVmDir} for details). */
   GString	page_to_id(int page_num);
      /** Translates the page number to file name. (Refer to \Ref{DjVmDir}
	  for details). */
   GString	page_to_name(int page_num);
      /** Translates the page number to some short name, which may be used
	  to access this page directly, f.e. from the browser's "Enter URL"
	  field. Refer to \Ref{DjVmDir} for details. */
   GString	page_to_title(int page_num);
      /** Translates the page number to the full URL of the page. This URL
	  is "artificial" for the {\em bundled} formats and is obtained
	  by appending the page name to the document's URL honoring possible
	  #;# and #?# in it */
   GURL		page_to_url(int page_num);
      /** Translates the page URL back to page number. Returns #-1# if the
	  page is not in the document. */
   int		url_to_page(const GURL & url);

      /** Decodes document structure.
	  This function is meaningful only for {\em obsolete} DjVu formats,
	  which were using the so-called {\em navigation directory}. Since
	  this directory is normally placed into a separate file, its contents
	  are not known after the document is constructed. Thus functions
	  like \Ref{get_pages_num}(), etc. initially return some dummy
	  information. We do not call #decode_doc_structure()# from the
	  constructor because it will block until enough data is available.
	  This is undesirable for the plugin. */
   void		decode_doc_structure(void);
   
      /** Returns \Ref{GP} pointer to \Ref{DjVuImage} corresponding to page
          #page_num#. The image will not necessarily bedecoded in full.  If
          multithreaded behaviour is allowed, the decoding will be started in
          a separate thread, which enables to do progressive display.
          Negative #page_num# has a special meaning for the {\em old indexed}
	  multipage documents: the #DjVuDocument# will start decoding of the
	  URL with which it has been initialized. */
   GP<DjVuImage>get_page(int page_num);
      /** Creates and returns \Ref{DjVuFile} corresponding to the given #url#.
	  For {\em bundled} documents the #url# is faked by appending file
	  name to the document's URL honoring possible #;# and #?# in the
	  document's URL. If there is a fully decoded \Ref{DjVuFile}
	  corresponding to this URL in the cache already, it will be returned. */
   GP<DjVuFile>	get_djvu_file(const GURL & url);
      /** Creates and returns \Ref{DjVuFile} corresponding to the given
	  page. Cache will be used to take advantage of files created and
	  decoded before. Negative #page_num# has a special meaning for the
	  {\em old indexed} multipage documents: the #DjVuDocument# will
	  return the file corresponding to the URL with which it has been
	  initialized. */
   GP<DjVuFile>	get_djvu_file(int page_num);
      //@}

      /// Returns cache being used.
   GCache<GURL, DjVuFile> * get_cache(void) const;

      /** @name Saving document to disk */
      //@{
      /** Returns pointer to the \Ref{DjVmDoc} class, which can save the
	  document contents on the hard disk in one of the two new formats:
	  {\em bundled} and {\em indirect}. You may also want to look
	  at \Ref{write}() and \Ref{expand}() if you are interested in
	  how to save the document. */
   GP<DjVmDoc>		get_djvm_doc(void);
      /** Saves the document in the {\em new bundled} format. All the data
	  is "bundled" into one file and this file is written into the
	  passed stream. */
   void			write(ByteStream & str);
      /** Saves the document in the {\em new indirect} format when every
	  page and component are stored in separate files. This format
	  is ideal for web publishing because it allows direct access to
	  any page and component. In addition to it, a top-level file
	  containing the list of all components will be created. To view
	  the document later in the plugin or in the viewer one should
	  load the top-level file.

	  @param dir_name - Name of the directory which the document should
	         be expanded into.
	  @param idx_name - Name of the top-level file containing the document
	         directory (basically, list of all files composing the document).
      */
   void			expand(const char * dir_name,
			       const char * idx_name);
      //@}
      /** Returns pointer to the internal directory of the document, if it
	  is in one of the new formats: {\em bundled} or {\em indirect}.
	  Otherwise (if the format of the input document is obsolete),
	  #ZERO# is returned. */
   GP<DjVmDir>		get_djvm_dir(void) const;

      /// Returns TRUE if #class_name# is #"DjVuDocument"# or #"DjVuPort"#
   virtual bool		inherits(const char * class_name) const;

   virtual GURL		id_to_url(const DjVuPort * source, const char * id);
   virtual GPBase	get_cached_file(const DjVuPort * source, const GURL & url);
   virtual void		cache_djvu_file(const DjVuPort * source, DjVuFile * file);
   virtual GP<DataPool>	request_data(const DjVuPort * source, const GURL & url);
   virtual void		notify_chunk_done(const DjVuPort * source, const char * name);
   virtual void		notify_all_data_received(const DjVuPort * source);
protected:
   GMap<GURL, void *>	active_files;
   GCriticalSection	active_files_lock;
   virtual void		file_destroyed(const DjVuFile *);
private:
   bool                 initialized;
   GCache<GURL, DjVuFile> *cache;
   GP<DjVuSimplePort>	simple_port;
   int			doc_type;

   GP<DjVmDir>		djvm_dir;	// New-style DjVm directory
   GP<DjVmDir0>		djvm_dir0;	// Old-style DjVm directory
   bool			dummy_ndir;
   GP<DjVuNavDir>	ndir;

   GURL			init_url;
   GP<DataPool>		init_data_pool;

   void			detect_doc_type(const GURL & doc_url);
   void			decode_ndir(void);

   GURL			id_to_url(const char * id);

   void			add_to_cache(const GP<DjVuFile> & f);

   static void		static_destroy_cb(const DjVuFile *, void *);
};

inline int
DjVuDocument::get_doc_type(void) const { return doc_type; }

inline bool
DjVuDocument::is_bundled(void) const
{
   return doc_type==BUNDLED || doc_type==OLD_BUNDLED;
}

inline GURL
DjVuDocument::get_init_url(void) const { return init_url; }

inline GP<DataPool>
DjVuDocument::get_init_data_pool(void) const { return init_data_pool; }

inline bool
DjVuDocument::inherits(const char * class_name) const
{
   return
      !strcmp("DjVuDocument", class_name) ||
      DjVuPort::inherits(class_name);
}

inline GCache<GURL, DjVuFile> *
DjVuDocument::get_cache(void) const
{
   return cache;
}

inline GP<DjVmDir>
DjVuDocument::get_djvm_dir(void) const
{
   if (doc_type!=BUNDLED && doc_type!=INDIRECT)
      THROW("The document is in obsolete format => no DjVm directory.");
   return djvm_dir;
}

//@}

#endif
