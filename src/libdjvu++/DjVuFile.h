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
//C- $Id: DjVuFile.h,v 1.10 1999-08-17 21:30:05 eaf Exp $
 
#ifndef _DJVUFILE_H
#define _DJVUFILE_H

#ifdef __GNUC__
#pragma interface
#endif

#include "GSmartPointer.h"
#include "DataPool.h"
#include "DjVuInfo.h"
#include "DjVuAnno.h"
#include "JB2Image.h"
#include "IWImage.h"
#include "GPixmap.h"
#include "DjVuPort.h"
#include "GContainer.h"
#include "DjVuNavDir.h"
#include "GCache.h"
#include "DjVmFile.h"

/** @name DjVuFile.h
    Files #"DjVuFile.h"# and #"DjVuFile.cpp"# contain implementation of the
    \Ref{DjVuFile} class, which takes the leading role in decoding of
    \Ref{DjVuImage}s.

    In the previous releases of the library the work of decoding has been
    entirely done in \Ref{DjVuImage}. Now, due to the introduction of multipage
    documents, the decoding procedure became significantly more complex and
    has been moved out from \Ref{DjVuImage} into \Ref{DjVuFile}.

    There is not much point though in creating just \Ref{DjVuFile} alone.
    The maximum power of the decoder is achieved when you create the
    \Ref{DjVuDocument} and work with {\bf it} when decoding the image.

    @memo Classes representing DjVu files.
    @author Andrei Erofeev <eaf@geocities.com>, L\'eon Bottou <leonb@research.att.com>
    @version #$Id: DjVuFile.h,v 1.10 1999-08-17 21:30:05 eaf Exp $#
*/

//@{

/** #DjVuFile# plays the central role in decoding \Ref{DjVuImage}s.
    First of all, it represents a DjVu file whether it's part of a
    multipage all-in-one-file DjVu document, or part of a multipage
    DjVu document where every page is in a separate file, or the whole
    single page document. #DjVuFile# can read its contents from a file
    and store it back when necessary.

    Second, #DjVuFile# does the greatest part of decoding work. In the
    past this was the responsibility of \Ref{DjVuImage}. Now, with the
    introduction of the multipage DjVu formats, the decoding routines
    have been extracted from the \Ref{DjVuImage} and put into this separate
    class #DjVuFile#.

    As \Ref{DjVuImage} before, #DjVuFile# now contains public class
    variables corresponding to every component, that can ever be decoded
    from a DjVu file (such as #INFO# chunk, #BG44# chunk, #SJBZ# chunk, etc.).

    As before, the decoding is initiated by a single function
    (\Ref{start_decode}() in this case, and \Ref{DjVuImage::decode}() before).
    The difference is that #DjVuFile# now handles threads creation itself.
    When you call the \Ref{start_decode}() function, it creates the decoding
    thread, which starts decoding, and which can create additional threads:
    one per each file included into this one.

    {\bf Inclusion} is also a new feature specifically designed for a
    multipage document. Indeed, inside a given document there can be a lot
    of things shared between its pages. Examples can be the document
    annotation (\Ref{DjVuAnno}) and other things like shared shapes and
    dictionary (to be implemented). To avoid putting these chunks into
    every page, we have invented new chunk called #INCL# which purpose is
    to make the decoder open the specified file and decode it.
    
    {\bf Source of data.} The #DjVuFile# can be initialized in two ways:
    \begin{itemize}
       \item With #URL# and \Ref{DjVuPort}. In this case #DjVuFile# will
             request its data thru the communication mechanism provided by
	     \Ref{DjVuPort} in the constructor. If this file references
	     (includes) any other file, data for them will also be requested
	     in the same way.
       \item With \Ref{ByteStream}. In this case the #DjVuFile# will read
             its data directly from the passed stream. This constructor
	     has been added to simplify creation of #DjVuFile#s, which do
	     no include anything else. In this case the \Ref{ByteStream}
	     is enough for the #DjVuFile# to initialize.
    \end{itemize}
	     
    {\bf Progress information.} #DjVuFile# does not do decoding silently.
    Instead, it sends a whole set of notifications through the mechanism
    provided by \Ref{DjVuPort} and \Ref{DjVuPortcaster}. It tells the user
    of the class about the progress of the decoding, about possible errors,
    chunk being decoded, etc. The data is requested using this mechanism too.

    {\bf Creating.} Depending on where you have data of the DjVu file, the
    #DjVuFile# can be initialized in two ways:
    \begin{itemize}
       \item By providing #URL# and pointer to \Ref{DjVuPort}. In this case
             #DjVuFile# will request data using communication mechanism
	     provided by \Ref{DjVuPort}. This is useful when the data is on
	     the web or when this file includes other files.
       \item By providing a \Ref{ByteStream} with the data for the file. Use
             it only when the file doesn't include other files.
    \end{itemize}
    There is also a bunch of functions provided for composing
    the desired \Ref{DjVuDocument} and modifying #DjVuFile# structure. The
    examples are \Ref{delete_chunks}(), \Ref{insert_chunk}(),
    \Ref{include_file}() and \Ref{unlink_file}().

    {\bf Caching.} In the case of plugin it's important to do the caching
    of decoded images or files. #DjVuFile# appears to be the best candidate
    for caching, and that's why it supports this procedure. Whenever a
    #DjVuFile# is successfully decoded, it's added to the cache by
    \Ref{DjVuDocument}. Next time somebody needs it, it will be extracted
    from the cache directly by another #DjVuFile# or \Ref{DjVuDocument}
    and won't be decoded again.

    {\bf URLs.} Historically the biggest strain is put on making the decoder
    available for Netscape and IE plugins where the original files reside
    somewhere in the net. That is why #DjVuFile# uses {\bf URLs} to
    identify itself and other files. If you're working with files on the
    hard disk, you have to use the local URLs instead of file names.
    A good way to do two way convertion is the \Ref{GOS} class. Sometimes it
    happens that a given file does not reside anywhere but the memory. No
    problem in this case either. There is a special port \Ref{DjVuMemoryPort},
    which can associate any URL with the corresponding data in the memory.
    All you need to do is to invent your own URL prefix for this case.
    "#memory:#" will do. The usage of absolute URLs has many advantages among
    which is the capability to cache files with their URL being the cache key.

    Please note, that the #DjVuFile# class has been designed to work closely
    with \Ref{DjVuDocument}. So please review the documentation on this class
    too. */

class DjVuFile : public GPEnabled, public DjVuPort
{
public:
   enum { DECODING=1, DECODE_OK=2, DECODE_FAILED=4, DECODE_STOPPED=8,
	  DATA_PRESENT=16, ALL_DATA_PRESENT=32, INCL_FILES_CREATED=64 };

      /** @name Decoded file contents */
      //@{
      /// Pointer to the DjVu file information component.
   GP<DjVuInfo>		info;
      /// Pointer to DjVu annotation.
   GP<DjVuAnno>		anno;
      /// Pointer to the background component of DjVu image.
   GP<IWPixmap>		bg44;
      /// Pointer to the mask of foreground component of DjVu image.
   GP<JB2Image>		fgjb;
      /// Pointer to the optional shape dictionary for the mask.
   GP<JB2Dict>		fgjd;
      /// Pointer to the colors of foreground component of DjVu image.
   GP<GPixmap>		fgpm;
      /// Pointer to the navigation directory contained in this file
   GP<DjVuNavDir>	dir;
      /// Description of the file formed during decoding
   GString		description;
      /// MIME type string describing the DjVu data.
   GString		mimetype;
      /// Size of the file.
   int			file_size;
      //@}

      /** Constructs #DjVuFile# object. This is a simplified constructor,
	  which is not supposed to be used for decoding or creating
	  #DjVuFile#s, which include other files.

	  If the file is stored on the hard drive, you may also use the
	  other constructor and pass it the file's URL, #ZERO# #port# and
	  #ZERO# #cache#. The #DjVuFile# will read the data itself.

	  If you want to receive error messages and notifications, you
	  may connect the #DjVuFile# to your own \Ref{DjVuPort} after
	  it has been constructed.

	  @param str The stream containing data for the file. */
   DjVuFile(ByteStream & str);
   
      /** Constructs #DjVuFile# object. As you can notice, the data is not
	  directly passed to the constructor. The #DjVuFile# will ask for it
	  through the \Ref{DjVuPort} mechanism before the constructor
	  finishes. If the data is stored locally on the hard disk then the
	  pointer to \Ref{DjVuPort} may be set to #ZERO#, which will make
	  #DjVuFile# read all data from the hard disk and report all errors
	  to #stderr#.

	  {\bf Note}. If the file includes (by means of #INCL# chunks) other
	  files then you should be ready to
	  \begin{enumerate}
	     \item Reply to requests \Ref{DjVuPort::id_to_url}() issued to
	           translate IDs (used in #INCL# chunks) to absolute URLs.
		   Usually, when the file is created by \Ref{DjVuDocument}
		   this job is done by it. If you construct such a file
		   manually, be prepared to do the ID to URL translation
	     \item Provide data for all included files.
	  \end{enumerate}

	  @param url The URL assigned to this file. It will be used when
	         the #DjVuFile# asks for data.
	  @param port All communication between #DjVuFile#s and \Ref{DjVuDocument}s
	         is done through the \Ref{DjVuPort} mechanism. If the {\em url}
		 is not local or the data does not reside on the hard disk,
		 the {\em port} parameter must not be #ZERO#. If the {\em port}
		 is #ZERO# then #DjVuFile# will create an internal instance
		 of \Ref{DjVuSimplePort} for accessing local files and
		 reporting errors. It can later be disabled by means
		 of \Ref{disable_standard_port}() function.
	  @param cache Pointer to the cache of files. Before creating
	         included files (if any) the #DjVuFile# will check the cache
		 to see if they have already been decoded. */
   DjVuFile(const GURL & url, DjVuPort * port=0,
	    GCache<GURL, DjVuFile> * cache=0);
   virtual ~DjVuFile(void);

      /** Disables the built-in port for accessing local files, which may
	  have been created in the case when the #port# argument to
	  the \Ref{DjVuFile::DjVuFile}() constructor is #ZERO# */
   void		disable_standard_port(void);

      /** Looks for #decoded# navigation directory (\Ref{DjVuNavDir}) in this
	  or included files. Returns #ZERO# if nothing could be found.

	  {\bf Note.} This function does {\bf not} attempt to decode #NDIR#
	  chunks. It is looking for predecoded components. #NDIR# can be
	  decoded either during regular decoding (initiated by
	  \Ref{start_decode}() function) or by \Ref{decode_ndir}() function,
	  which processes this and included files recursively in search
	  of #NDIR# chunks and decodes them. */
   GP<DjVuNavDir>	find_ndir(void);

      /** @name Status query functions */
      //@{
      /** Returns the #DjVuFile# status. The value returned is the
	  result of ORing one or more of the following constants:
	  \begin{itemize}
	     \item #DECODING# The decoding is in progress
	     \item #DECODE_OK# The decoding has finished successfully
	     \item #DECODE_FAILED# The decoding has failed
	     \item #DECODE_STOPPED# The decoding has been stopped by
	           \Ref{stop_decode}() function
	     \item #DATA_PRESENT# All data for this file has been received.
	           It's especially important in the case of Netscape or IE
		   plugins when the data is being received while the
		   decoding is done.
	     \item #ALL_DATA_PRESENT# Not only data for this file, but also
	           for all included file has been received.
	     \item #INCL_FILES_CREATED# All #INCL# and #INCF# chunks have been
	           processed and the corresponding #DjVuFile#s created. This
		   is important to know to be sure that the list returned by
		   \Ref{get_included_files}() is OK.
	  \end{itemize} */
   int		get_status(void) const;
      /// Returns #TRUE# if the file is being decoded.
   bool		is_decoding(void) const;
      /// Returns #TRUE# if decoding of the file has finished successfully.
   bool		is_decode_ok(void) const;
      /// Returns #TRUE# if decoding of the file has failed.
   bool		is_decode_failed(void) const;
      /** Returns #TRUE# if decoding of the file has been stopped by
	  \Ref{stop_decode}() function. */
   bool		is_decode_stopped(void) const;
      /// Returns #TRUE# if this file has received all data.
   bool		is_data_present(void) const;
      /** Returns #TRUE# if this file {\bf and} all included files have
	  received all data. */
   bool		is_all_data_present(void) const;
      /** Returns #TRUE# if all included files have been created. Only when
	  this function returns 1, the \Ref{get_included_files}() returns
	  the correct information. */
   bool		are_incl_files_created(void) const;
      //@}

      /** @name File name */
      //@{
      /// Returns the URL assigned to this file
   GURL		get_url(void) const;
      /** Changes the {\bf name} of the file (last component of the URL).
	  This doesn't change the file's location. */
   void		set_name(const char * name);
      //@}

      /** @name Decode control routines */
      //@{
      /** Starts decode. If threads are enabled, the decoding will be
	  done in another thread. Be sure to use \Ref{wait_for_finish}()
	  or listen for notifications sent through the \Ref{DjVuPortcaster}
	  to remain in sync. */
   void		start_decode(void);
      /** Stops decode. If #sync# is 1 then the function will not return
	  until the decoding thread actually dies. Otherwise it will
	  just signal the thread to stop and will return immediately.
	  Decoding of all included files will be stopped too. */
   void		stop_decode(bool sync);
      /** Wait for the decoding to finish. This will wait for the
	  termination of included files too. */
   void		wait_for_finish(void);
      /** Looks for #NDIR# chunk (navigation directory), and decodes its
	  contents. If the #NDIR# chunk has not been found in {\em this} file,
	  but this file includes others, the procedure will continue
	  recursively. This function is useful to obtain the document
	  navigation directory before any page has been decoded. After it
	  returns the directory can be obtained by calling \Ref{find_ndir}()
	  function.

	  {\bf Warning.} Contrary to \Ref{start_decode}(), this function
	  does not return before it completely decodes the directory.
	  Make sure, that this file and all included files have enough data. */
   GP<DjVuNavDir>	decode_ndir(void);
      /// Clears all decoded components.
   void		reset(void);
      /** Processes #INCL# chunks and creates included files.
	  Normally you won't need to call this function because included
	  files are created automatically when the file is being decoded.
	  But if due to some reason you'd like to obtain the list of included
	  files without decoding this file, this is an ideal function to call.

	  {\bf Warning.} This function does not return before it reads the
	  whole file, which may block your application under some circumstances
	  if not all data is available. */
   void		process_incl_chunks(void);
      //@}
   
      // Function needed by the cache
   unsigned int	get_memory_usage(void) const;

      /** @name Operations with included files */
      //@{
      /** Returns the list of included DjVuFiles.
	  
	  {\bf Warning.} Included files are normally created during decoding.
	  Before that they do not exist. So, if you call this function at
	  that time it will have to read all the data from this file
	  in order to find #INCL# chunks, which may block your application,
	  if not all data is available. */
   GPList<DjVuFile>	get_included_files(void);
      /** Includes the given #file# into this one. Since the procedure
	  of inclusion also implies inserting the #INCL# chunk somewhere
	  (you want to save the results after all, don't you), it's
	  necessary to specify chunk position #chunk_pos#.

	  @param file The file to be included
	  @param chunk_pos Position of the #INCL# chunk, which has to
	         be inserted into {\bf this} file. #-1# means append. */
   void		include_file(const GP<DjVuFile> & file, int chunk_pos=-1);
      /** Removes included file with given #name#. This will remove
	  the #INCL# chunk too. */
   void		unlink_file(const char * name);
      //@}
   

      /** @name Operations with chunks (underlying IFF data) */
      //@{
      /// Returns the number of chunks in the IFF file data
   int		get_chunks_number(void);
      /// Returns the name of chunk number #chunk_num#
   GString	get_chunk_name(int chunk_num);
      /// Returns 1 if this file contains chunk with name #chunk_name#
   bool		contains_chunk(const char * chunk_name);
      /** Removes all chunks with name #chunk_name# from the underlying
	  IFF file structure. You can't use this function to delete #INCL#
	  chunks. Use \Ref{unlink_file}() instead. */
   void		delete_chunks(const char * chunk_name);
      /** Inserts any chunk into the underlying IFF file data. Please beware,
	  that the insertion will not decode the chunk contents.

	  @param pos Position at which the chunk should be inserted.
	         #-1# means to append.
	  @param chunk_name Name of the chunk to be inserted
	  @param data The actual data that will be inserted. */
   void		insert_chunk(int pos, const char * chunk_name,
			     const TArray<char> & data);
      //@}

      /** @name Encoding routines */
      //@{
      /** The main function that encodes data back into binary stream.
	  The data returned will reflect possible changes made into the
	  chunk structure, annotation chunk #ANTa# and navigation directory
	  chunk #NDIR#.

	  @param included_too Process included files too
	  @param no_ndir Get rid of #NDIR# chunks. */
   TArray<char>		get_djvu_data(bool included_too, bool no_ndir);
      /** Will add the contents of this and all included files to
	  the given \Ref{DjVmFile}. It's normally used by
	  \Ref{DjVuDocument::get_djvm_data}(). */
      //void			add_to_djvm(DjVmFile & djvm_file);
      //@}

      // Internal. Used by DjVuDocument
   void			move(const GURL & dir_url);
   void			change_cache(GCache<GURL, DjVuFile> * cache);

      // Functions inherited from DjVuPort
   virtual bool		inherits(const char * class_name) const;
   virtual void		notify_chunk_done(const DjVuPort * source, const char * name);
   virtual void		notify_file_done(const DjVuPort * source);
   virtual void		notify_file_stopped(const DjVuPort * source);
   virtual void		notify_file_failed(const DjVuPort * source);
   virtual void		notify_all_data_received(const DjVuPort * source);
private:
   GURL			url;
   GP<DataRange>	data_range;
   GCache<GURL, DjVuFile> * cache;

   GMonitor		status_mon;
   int			status;

   GPList<DjVuFile>	inc_files_list;
   GCriticalSection	inc_files_lock;

   GCriticalSection	trigger_lock;
   
   GThread		* decode_thread;
   GEvent		decode_thread_started_ev;
   GP<DjVuFile>		decode_life_saver;
   GP<DataRange>	decode_data_range;

   DjVuPort		* simple_port;

   GMonitor		chunk_mon;

   GMonitor		finish_mon;
   
      // Functions called when the decoding thread starts
   static void	static_decode_func(void *);
   void		decode_func(void);
   void		decode(ByteStream & str);
      // Functions dealing with the shape directory (fgjd)
   static GP<JB2Dict> static_get_fgjd(void *);
   GP<JB2Dict> get_fgjd(int block=0);

      // Functions used to wait for smth
   void		wait_for_chunk(void);
   bool		wait_for_finish(bool self);

      // INCL chunk processor
   GP<DjVuFile>	process_incl_chunk(ByteStream & str);

      // Trigger: called when DataRange has all data
   static void	static_trigger_cb(void *);
   void		trigger_cb(void);
      // Progress callback: called from time to time
   static void	progress_cb(int pos, void *);

   GP<DjVuNavDir>find_ndir(GMap<GURL, void *> & map);
   GP<DjVuNavDir>decode_ndir(GMap<GURL, void *> & map);
   void		add_djvu_data(IFFByteStream & str,
			      GMap<GURL, void *> & map,
			      bool included_too, bool no_ndir);
   void		add_to_djvm(DjVmFile & djvm_file,
			    GMap<GURL, void *> & map);
   void		move(GMap<GURL, void *> & map, const GURL & dir_url);
   void		change_cache(GMap<GURL, void *> & map,
			     GCache<GURL, DjVuFile> * cache);
};

inline void
DjVuFile::disable_standard_port(void)
{
   delete simple_port; simple_port=0;
}

inline bool
DjVuFile::inherits(const char * class_name) const
{
   return
      !strcmp("DjVuFile", class_name) ||
      DjVuPort::inherits(class_name);
}

inline void
DjVuFile::wait_for_finish(void)
{
   while(wait_for_finish(1));
}

inline GURL
DjVuFile::get_url(void) const
{
   return url;
}

inline void
DjVuFile::set_name(const char * name)
{
   url=url.base()+name;
}

inline void
DjVuFile::reset(void)
{
   info=0; anno=0; bg44=0; fgjb=0; fgpm=0;
   dir=0; description=""; mimetype="";
}

//@}

#endif
