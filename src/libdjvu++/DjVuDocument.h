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
//C- $Id: DjVuDocument.h,v 1.35 1999-10-05 16:00:06 leonb Exp $
 
#ifndef _DJVUDOCUMENT_H
#define _DJVUDOCUMENT_H

#ifdef __GNUC__
#pragma interface
#endif

#include "DjVuGlobal.h"
#include "GSmartPointer.h"
#include "DjVuFileCache.h"
#include "DjVuFile.h"
#include "DjVuImage.h"
#include "DjVmDir0.h"
#include "DjVmDoc.h"

/** @name DjVuDocument.h
    Files #"DjVuDocument.h"# and #"DjVuDocument.cpp"# contain implementation
    of the \Ref{DjVuDocument} class - the ideal tool for opening, decoding
    and saving DjVu single page and multi page documents.

    @memo DjVu document class.
    @author Andrei Erofeev <eaf@research.att.com>, L\'eon Bottou <leonb@research.att.com>
    @version #$Id: DjVuDocument.h,v 1.35 1999-10-05 16:00:06 leonb Exp $#
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
    \Ref{DjVuFileCache} to do caching thus avoiding unnecessary multiple decoding of
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
    GP<DjVuDocument> doc=new DjVuDocument;
    doc->init(GOS::filename_to_url(file_name));
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
	     #ZERO# pointer to the \Ref{init}() function. #DjVuDocument#
	     can access local data itself. In the case of a plugin though,
	     one would have to implement his own \Ref{DjVuPort}, which
	     would handle requests for data arising when the document
	     is being decoded.
       \item The fact that the program is threadless simplifies things a lot.
	     First of all, in the \Ref{init}() function the document
	     finishes analysis of data before the function returns. Thus
	     the information about the document type, the number of pages,
	     their names, etc. becomes immediately available. In a threaded
	     program the \Ref{init}() would start a separate thread, which
	     will read data trying to analyze it. This has been done
	     because data analysis actually requires some data. In the case
	     of the DjVu plugin this data is not immediately available.
	     Thus, to prevent the main thread from blocking, we perform
	     initialization in a separate thread. To check if the class
	     is completely initialized, use \Ref{is_init_complete}(). To wait
	     for this to happen use \Ref{wait_for_complete_init}(). Once again,
	     all these things are not required for single-threaded program.

	     Secondly, since the program is single-threaded, the image is
	     fully decoded before it's returned. In a multithreaded
	     application decoding starts in a separate thread, and the pointer
	     to the \Ref{DjVuImage} being decoded is returned
	     immediately. This has been done to enable progressive redisplay
	     in the DjVu plugin. Use communication mechanism provided by
	     \Ref{DjVuPort} and \Ref{DjVuPortcaster} to learn about progress
	     of decoding.  Or try #dimg->wait_for_complete_decode()# to wait
	     until the decoding ends.
       \item See Also: \Ref{DjVuFile}, \Ref{DjVuImage}, \Ref{GOS}.
    \end{enumerate}

    {\bf Initialization}
    As mentioned above, the #DjVuDocument# can go through several stages
    of initialization. The functionality is gradually added while it passes
    one stage after another:
    \begin{enumerate}
       \item First of all, immediately after the object is created \Ref{init}()
             function must be called. {\bf Nothing} will work until this
	     is done.
       \item If the program is single-threaded, initialization started by
	     \Ref{init}() completes before the function returns. Otherwise\
	     \Ref{init}() starts a new thread, which continues initialization
	     in parallel with the main one. This has been done to allow the
	     DjVu plugin to use as much of #DjVuDocument# functionality as
	     possible and to do it as soon as possible (not waiting for extra
	     data).
       \item The first thing the initializing thread learns about the document
	     is its type (#BUNDLED#, #INDIRECT#, #OLD_BUNDLED# or #OLD_INDEXED#).
	     As soon as it happens, document flags are changed and
	     #notify_doc_flags_changed()# request is sent through the
	     communication mechanism provided by \Ref{DjVuPortcaster}.
       \item After the document type becomes known, the initializing thread
             proceeds with learning the document structure. Gradually the
	     flags are updated with values:
	     \begin{itemize}
	        \item #DOC_DIR_KNOWN#: Contents of the document became known.
		      This is meaningful for #BUNDLED#, #OLD_BUNDLED# and
		      #INDIRECT# documents only.
		\item #DOC_NDIR_KNOWN#: Contents of the document navigation
		      directory became known. This is meaningful for old-style
		      documents (#OLD_BUNDLED# and #OLD_INDEXED#) only
		\item #DOC_INIT_COMPLETE#: The initializating thread finished.
	     \end{itemize}
    \end{enumerate} */
    
class DjVuDocument : public DjVuPort
{
public:
      /** Flags describing the document initialization state.
	  \begin{itemize}
	     \item #DOC_TYPE_KNOWN#: The type of the document has been learnt.
	     \item #DOC_DIR_KNOWN#: Contents of the document became known.
		   This is meaningful for #BUNDLED#, #OLD_BUNDLED# and
		   #INDIRECT# documents only.
	     \item #DOC_NDIR_KNOWN#: Contents of the document navigation
		   directory became known. This is meaningful for old-style
		   documents (#OLD_BUNDLED# and #OLD_INDEXED#) only
	     \item #DOC_INIT_COMPLETE#: The initializating thread finished.
	  \end{itemize} */
   enum DOC_FLAGS { DOC_TYPE_KNOWN=1, DOC_DIR_KNOWN=2,
		    DOC_NDIR_KNOWN=4, DOC_INIT_COMPLETE=8,
		    DOC_INIT_FAILED=16 };
      /** Specifies the format of #DjVuDocument#. There are currently 4 DjVu
	  multipage formats recognized by the library. Two of them are obsolete
	  and should not be used.
	  \begin{enumerate}
	     \item #OLD_BUNDLED# - Obsolete bundled format
	     \item #OLD_INDEXED# - Obsolete multipage format where every page
	           is stored in a separate file and "includes" (by means
		   of an #INCL# chunk) the file with the document directory.
	     \item #BUNDLED# - Currently supported bundled format
	     \item #INDIRECT# - Currently supported "expanded" format, where
	           every page and component is stored in a separate file. There
		   is also a {\em top-level} file with the document directory.
          \end{enumerate} */
   enum DOC_TYPE { OLD_BUNDLED=1, OLD_INDEXED, BUNDLED, INDIRECT,
		   UNKNOWN_TYPE };
   enum THREAD_FLAGS { STARTED=1, FINISHED=2 };

      /** Default constructor. Please call function \Ref{init}() before
	  you start working with the #DjVuDocument#. */
   DjVuDocument(void);
   virtual ~DjVuDocument(void);

      /** Initializes the #DjVuDocument# object using an existing document.
          This function should be called once after creating the object.
	  The #url# should point to the real data, and the creator of the
	  document should be ready to return this data to the document
	  if it's not stored locally (in which case #DjVuDocument# can
	  access it itself).

	  {\bf Initializing thread}
	  In a single-threaded application, the #init()# function performs
	  the complete initialization of the #DjVuDocument# before it returns.
	  In a multi-threaded application, though, it initializes some internal
	  variables, requests data for the document and starts a new
	  {\em initializing} thread, which is responsible for determining the
	  document type and structure and completing the initialization
	  process. This additional complication is justified in the case of
	  the DjVu plugin because performing initialization requires data and
	  in the plugin the data can be supplied by the main thread only.
	  Thus, if the initialization was completed by the main thread, the
	  plugin would run out of data and block.

	  {\bf Stages of initialization}
	  Immediately after the #init()# function terminates, the
	  #DjVuDocument# object is ready for use. Its functionality will
	  not be complete (until the initializing thread finishes), but
	  the object is still very useful. Such functions as \Ref{get_page}()
	  or \Ref{get_djvu_file}() or \Ref{id_to_url}() may be called
	  before the initializing thread completes. This allows the DjVu
	  plugin start decoding as soon as possible without waiting for
	  all data to arrive.

	  To query the current stage of initialization you can use
	  \Ref{get_doc_flags}() function or listen to the
	  #notify_doc_flags_changed()# notifications distributed with the help
	  of \Ref{DjVuPortcaster}. To wait for the initialization to
	  complete use \Ref{wait_for_complete_init}().

	  {\bf Querying data}
	  The query for data is done using the communication mechanism
	  provided by \Ref{DjVuPort} and \Ref{DjVuPortcaster}. If #port#
	  is not #ZERO#, then the request for data will be forwarded to it.
	  If it {\bf is} #ZERO# then #DjVuDocument# will create an internal
	  instance of \Ref{DjVuSimplePort} and will use it to access local
	  files and report errors to #stderr#. In short, if the document
	  file is stored on the local hard disk, and you're OK about reporting
	  errors to #stderr#, you may pass #ZERO# pointer to \Ref{DjVuPort}
	  as #DjVuDocument# can take care of this situation by itself.

	  {\bf The URL}
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
                     DjVuFileCache * cache=0);

      /** Returns #TRUE# if the initialization thread finished successfully.
	  As soon as it happens, the document becomes completely initialized
	  and its every function should work properly. Please refer to
	  the description of \Ref{init}() function and of the #DjVuDocument#
	  class to learn about the initializing stages.

	  To wait for the initialization to complete use
	  \Ref{wait_for_complete_init}() function. To query the initialization
	  stage use \Ref{get_flags}() function.

	  {\bf Note:} In a single threaded application the initialization
	  completes before the \Ref{init}() function returns. */
   bool		is_init_complete(void) const;

      /// Call this function when you don't need the #DjVuDocument# any more.
   void		stop(void);

      /** If the document has already learnt its type, the function will
	  returns it: #DjVuDocument::OLD_BUNDLED# or
	  #DjVuDocument::OLD_INDEXED# or #DjVuDocument:BUNDLED# or
	  #DjVuDocument::INDIRECT#. The first two formats are obsolete.
	  Otherwise (if the type is unknown yet), #UNKNOWN_TYPE# will
	  be returned.

	  {\bf Note:} To check the stage of the document initialization
	  use \Ref{get_flags}() or \Ref{is_init_complete}() functions. To
	  wait for the initialization to complete use \Ref{wait_for_complete_init}().
	  For single threaded applications the initialization completes
	  before the \Ref{init}() function returns. */
   int		get_doc_type(void) const;

      /** Returns the document flags. The flags describe the degree in which
	  the #DjVuDocument# object is initialized. Every time the flags
	  are changed, a #notify_doc_flags_changed()# notification is
	  distributed using the \Ref{DjVuPortcaster} communication
	  mechanism.

	  {\bf Note:} To wait for the initialization to complete use
	  \Ref{wait_for_complete_init}(). For single threaded applications
	  the initialization completes before the \Ref{init}() function
	  returns. */
   long		get_doc_flags(void) const;

      /** Returns #TRUE# if the document is in bundled format (either in
	  #DjVuDocument::OLD_BUNDLED# or #DjVuDocument::BUNDLED# formats). */
   bool		is_bundled(void) const;

      /// Returns the URL passed to the \Ref{init}() function
   GURL		get_init_url(void) const;

      /** Returns data corresponding to the URL passed to the \Ref{init}()
	  function.

	  {\bf Note:} The pointer returned is guaranteed to be non-#ZERO#
	  only after the #DjVuDocument# learns its type (passes through
	  the first stage of initialization process). Plese refer to
	  \Ref{init}() for details. */
   GP<DataPool>	get_init_data_pool(void) const;

      /** @name Accessing pages */
      //@{
      /** Returns the number of pages in the document. If there is still
	  insufficient information about the document structure (initialization
	  has not finished yet), #1# will be returned. Plese refer to
          \Ref{init}() for details. */
   int		get_pages_num(void) const;
      /** Translates the page number to the full URL of the page. This URL
	  is "artificial" for the {\em bundled} formats and is obtained
	  by appending the page name to the document's URL honoring possible
	  #;# and #?# in it. Negative page number has a special meaning for
	  #OLD_INDEXED# documents: it points to the URL, which the
	  #DjVuDocument# has been initialized with. For other formats this
	  is the same as page #0#.

	  The function tries it best to map the page number to the URL.
	  Although, if the document structure has not been fully discovered
	  yet, an empty URL will be returned. Use \Ref{wait_for_complete_init}()
	  to wait until the document initialization completes. Refer to
	  \Ref{init}() for details.

	  Depending on the document format, the function assumes, that there
	  is enough information to complete the request when:
	  \begin{itemize}
	     \item #OLD_INDEXED#: If #page_num<0#, #DOC_TYPE_KNOWN# flag must
		   be set. Otherwise #DOC_NDIR_KNOWN# must be set.
	     \item #OLD_BUNDLED#: If #page_num=0#, #DOC_DIR_KNOWN# flag must
		   be set. Otherwise #DOC_NDIR_KNOWN# flag must be set.
	     \item #INDIRECT# and #BUNDLED#: #DOC_DIR_KNOWN# flag must be set.
	  \end{itemize} */
   GURL		page_to_url(int page_num) const;
      /** Translates the page URL back to page number. Returns #-1# if the
	  page is not in the document. If the document's structure has
          not been learnt yet #0# will be returned.

	  Depending on the document format, the function starts working
	  properly as soon as:
	  \begin{itemize}
	     \item #OLD_INDEXED# and #OLD_BUNDLED#: #DOC_NDIR_KNOWN# is set
	     \item #INDIRECT# and #BUNDLED#: #DOC_DIR_KNOWN# is set.
	  \end{itemize} */
   int		url_to_page(const GURL & url) const;
      /** Translates the textual ID to the complete URL if possible.
	  
	  Depending on the document format the translation is done in the
	  following way:
	  \begin{itemize}
	     \item For #BUNDLED# and #INDIRECT# documents the function
		   scans the \Ref{DjVmDir} (the document directory) and
		   matches the ID against:
		   \begin{enumerate}
		      \item File ID from the \Ref{DjVmDir}
		      \item File name from the \Ref{DjVmDir}
		      \item File title from the \Ref{DjVmDir}
		   \end{enumerate}
		   Then for #BUNDLED# document the URL is obtained by
		   appending the #name# of the found file to the document's
		   URL.

		   For #INDIRECT# documents the URL is obtained by
		   appending the #name# of the found file to the URL of
		   the directory containingn the document.
	     \item For #OLD_BUNDLED# documents the function compares the ID
		   with internal name of every file inside the bundle and
		   composes an artificial URL by appending the file name to
		   the document's URL.
	     \item For #OLD_INDEXED# documents the function composes the
	           URL by appending the ID to the URL of the directory
		   containing the document.
	  \end{itemize}

	  If information obtained by the initialization thread is not
	  sufficient yet, the #id_to_url()# may return an empty URL.
	  Depending on the document type, the information is sufficient when
	  \begin{itemize}
	     \item #BUNDLED# and #INDIRECT#: #DOC_DIR_KNOWN# flag is set.
	     \item #OLD_BUNDLED# and #OLD_INDEXED#: #DOC_TYPE_KNOWN# flag is set.
	  \end{itemize} */
   GURL		id_to_url(const char * id) const;

      /** Returns \Ref{GP} pointer to \Ref{DjVuImage} corresponding to page
          #page_num#. If caching is enabled and there is a {\em fully decoded}
	  \Ref{DjVuFile} in the cache, the image will be reused and will
	  be returned fully decoded. Otherwise, if multithreaded behaviour
	  is allowed, the decoding will be started in a separate thread,
	  which enables to do progressive redisplay. Thus, in this case
	  the image returned may be partially decoded.

	  Negative #page_num# has a special meaning for the {\em old indexed}
	  multipage documents: the #DjVuDocument# will start decoding of the
	  URL with which it has been initialized. For other formats page
	  #-1# is the same as page #0#.

	  #DjVuDocument# can also connect the created page to the specified
	  #port# {\em before starting decoding}. This option will allow
	  the future owner of \Ref{DjVuImage} to receive all messages and
	  requests generated during its decoding.

	  If this function is called before the document's structure becomes
	  known (the initialization process completes), the \Ref{DjVuFile},
	  which the returned image will be attached to, will be assigned a
	  temporary artifical URL, which will be corrected as soon as enough
	  information becomes available. The trick prevents the main thread
	  from blocking and in some cases helps to start decoding earlier.
	  The URL is corrected and decoding will start as soon as
	  #DjVuDocument# passes some given stages of initialization and
	  \Ref{page_to_url}(), \Ref{id_to_url}() functions start working
	  properly. Please look through their description for details.

	  {\bf Note:} To wait for the initialization to complete use
	  \Ref{wait_for_complete_init}(). For single threaded applications
	  the initialization completes before the \Ref{init}() function
	  returns. */
   GP<DjVuImage>get_page(int page_num, DjVuPort * port=0);
      /** Returns \Ref{GP} pointer to \Ref{DjVuImage} corresponding to the
	  specified ID. This function behaves exactly as the #get_page()#
	  function above. The only thing worth mentioning here is how the #ID#
	  parameter is treated.

	  First of all the function checks, if the ID contains a number.
	  If so, it just calls the #get_page()# function above. If ID is
	  #ZERO# or just empty, page number #-1# is assumed. Otherwise
	  the ID is translated to the URL using \Ref{id_to_url}(). */
   GP<DjVuImage>get_page(const char * id, DjVuPort * port=0);
   
      /** Returns \Ref{DjVuFile} corresponding to the specified page.
	  Normally it translates the page number to the URL using
	  \Ref{page_to_url}() and then creates \Ref{DjVuFile} initializing
	  it with data from the URL.

	  The behaviour becomes different, though in the case when the
	  document structure is unknown at the moment this function is called.
	  In this situations it invents a temporary URL, creates a
	  \Ref{DjVuFile}, initializes it with this URL and returns
	  immediately. The caller may start decoding the file right away
	  (if necessary). The decoding will block but will automatically
	  continue as soon as enough information is collected about the
	  document. This trick should be quite transparent to the user and
	  helps to prevent the main thread from blocking. The decoding will
	  unblock and this function will stop using this "trick" as soon
	  as #DjVuDocument# passes some given stages of initialization and
	  \Ref{page_to_url}(), \Ref{id_to_url}() functions start working
	  properly.

	  {\bf Note:} To wait for the initialization to complete use
	  \Ref{wait_for_complete_init}(). For single threaded applications
	  the initialization completes before the \Ref{init}() function
	  returns. */
   GP<DjVuFile>	get_djvu_file(int page_num);
      /** Returns \Ref{DjVuFile} corresponding to the specified ID.
          This function behaves exactly as the #get_djvu_file()# function
	  above. The only thing worth mentioning here is how the #ID#
	  parameter is treated.

	  First of all the function checks, if the ID contains a number.
	  If so, it just calls the #get_djvu_file()# function above. If ID is
	  #ZERO# or just empty, page number #-1# is assumed. Otherwise
	  the ID is translated to the URL using \Ref{id_to_url}(). */
   GP<DjVuFile>	get_djvu_file(const char * id);
      //@}

      /** Waits until the document becomes completely initialized. As
	  described in \Ref{init}(), for multithreaded applications the
	  initialization is carried out in parallel with the main thread.
	  This function blocks the calling thread until the initializing
	  thread reads enough data, receives information about the document
	  format and exits.  This function returns #true# if the
	  initialization is successful. You can use \Ref{get_flags}() or
	  \Ref{is_init_complete}() to check more precisely the degree of
	  initialization. */
   bool    	   wait_for_complete_init(void);
   
      /// Returns cache being used.
   DjVuFileCache * get_cache(void) const;

      /** @name Saving document to disk */
      //@{
      /** Returns pointer to the \Ref{DjVmDoc} class, which can save the
	  document contents on the hard disk in one of the two new formats:
	  {\em bundled} and {\em indirect}. You may also want to look
	  at \Ref{write}() and \Ref{expand}() if you are interested in
	  how to save the document.

	  {\bf Plugin Warning}. This function will read contents of the whole
	  document. Thus, if you call it from the main thread (the thread,
	  which transfers data from Netscape), the plugin will block. */
   GP<DjVmDoc>		get_djvm_doc(void);
      /** Saves the document in the {\em new bundled} format. All the data
	  is "bundled" into one file and this file is written into the
	  passed stream.

	  {\bf Plugin Warning}. This function will read contents of the whole
	  document. Thus, if you call it from the main thread (the thread,
	  which transfers data from Netscape), the plugin will block. */
   void			write(ByteStream & str);
      /** Saves the document in the {\em new indirect} format when every
	  page and component are stored in separate files. This format
	  is ideal for web publishing because it allows direct access to
	  any page and component. In addition to it, a top-level file
	  containing the list of all components will be created. To view
	  the document later in the plugin or in the viewer one should
	  load the top-level file.

	  {\bf Plugin Warning}. This function will read contents of the whole
	  document. Thus, if you call it from the main thread (the thread,
	  which transfers data from Netscape), the plugin will block.
	  
	  @param dir_name - Name of the directory which the document should
	         be expanded into.
	  @param idx_name - Name of the top-level file containing the document
	         directory (basically, list of all files composing the document).
      */
   void			expand(const char * dir_name,
			       const char * idx_name);
      /** This function can be used instead of \Ref{write}() and \Ref{expand}().
	  It allows to save the document either in the new #BUNDLED# format
	  or in the new #INDIRECT# format depending on the value of parameter
	  #bundled#.

	  Depending on the document's type, the meaning of #where# is:
	  \begin{itemize}
	     \item For #BUNDLED# documents this is the name of the file
	     \item For #INDIRECT# documents this is the name of top-level
	           index file. All document files will be saved into the
		   save directory where the index file will resize. */
   virtual void		save_as(const char * where, bool bundled);
      //@}
      /** Returns pointer to the internal directory of the document, if it
	  is in one of the new formats: #BUNDLED# or #INDIRECT#.
	  Otherwise (if the format of the input document is obsolete),
	  #ZERO# is returned.

	  #ZERO# will also be returned if the initializing thread has not
	  learnt enough information about the document (#DOC_DIR_KNOWN# has
	  not been set yet). Check \Ref{is_init_complete}() and \Ref{init}()
          for details. */
   GP<DjVmDir>		get_djvm_dir(void) const;

      /// Returns TRUE if #class_name# is #"DjVuDocument"# or #"DjVuPort"#
   virtual bool		inherits(const char * class_name) const;

   virtual GURL		id_to_url(const DjVuPort * source, const char * id);
   virtual GPBase	id_to_file(const DjVuPort * source, const char * id);
   virtual GP<DataPool>	request_data(const DjVuPort * source, const GURL & url);
   virtual void		notify_file_flags_changed(const DjVuFile * source,
						  long set_mask, long clr_mask);
protected:
   GURL			init_url;
   GP<DataPool>		init_data_pool;
   GP<DjVmDir>		djvm_dir;	// New-style DjVm directory
   int			doc_type;
   
   virtual GP<DjVuFile>	url_to_file(const GURL & url);
private:
   class UnnamedFile : public GPEnabled
   {
   public:
      enum { ID, PAGE_NUM };
      int		id_type;
      GString		id;
      int		page_num;
      GURL		url;
      GP<DjVuFile>	file;
      GP<DataPool>	data_pool;

      UnnamedFile(int id_type, const char * id, int page_num, const GURL & url,
		  const GP<DjVuFile> & file);
   };
   bool                 init_called;
   GSafeFlags		flags;
   GSafeFlags		init_thread_flags;
   DjVuFileCache	* cache;
   GP<DjVuSimplePort>	simple_port;

   GP<DjVmDir0>		djvm_dir0;	// Old-style DjVm directory
   GP<DjVuNavDir>	ndir;		// Old-style navigation directory
   GString		first_page_name;// For OLD_BUNDLED docs only

   GP<DjVuFile>		ndir_file;	// Used to query NDIR.
   
   GPList<UnnamedFile>	ufiles_list;
   GCriticalSection	ufiles_lock;

   GP<DjVuDocument>	init_life_saver;

      // Reads document contents in another thread trying to determine
      // its type and structure
   GThread		init_thr;
   static void		static_init_thread(void *);
   void			init_thread(void);

   void                 check() const;

   void			add_to_cache(const GP<DjVuFile> & f);
   void			check_unnamed_files(void);
   void			set_file_name(const DjVuFile * file);
};

inline
DjVuDocument::UnnamedFile::UnnamedFile(int xid_type, const char * xid,
				       int xpage_num, const GURL & xurl,
				       const GP<DjVuFile> & xfile) :
      id_type(xid_type), id(xid), page_num(xpage_num), url(xurl), file(xfile) {}

inline bool
DjVuDocument::is_init_complete(void) const
{
   return (flags & DOC_INIT_COMPLETE)!=0;
}

inline bool
DjVuDocument::wait_for_complete_init(void)
{
   flags.enter();
   while(!(flags & DOC_INIT_FAILED) &&
	 !(flags & DOC_INIT_COMPLETE)) flags.wait();
   flags.leave();
   return (flags & DOC_INIT_COMPLETE)!=0;
}

inline int
DjVuDocument::get_doc_type(void) const { return doc_type; }

inline long
DjVuDocument::get_doc_flags(void) const { return flags; }

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

inline DjVuFileCache *
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
