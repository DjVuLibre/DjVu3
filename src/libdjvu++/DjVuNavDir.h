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
//C- $Id: DjVuNavDir.h,v 1.2 1999-05-25 19:42:28 eaf Exp $

#ifndef _DJVUNAVDIR_H
#define _DJVUNAVDIR_H

#ifdef __GNUC__
#pragma interface
#endif

#include "ByteStream.h"
#include "GContainer.h"
#include "GSmartPointer.h"
#include "GString.h"
#include "GThreads.h"
#include "GURL.h"
#include "Arrays.h"

/** @name DjVuNavDir.h
    Files #"DjVuNavDir.h"# and #"DjVuNavDir.cpp"# contain implementation of the
    multipage DjVu navigation directory. This directory lists all the pages,
    that a given document is composed of. The navigation (switching from page
    to page in the plugin) is not possible before this directory is decoded.

    Refer to the \Ref{DjVuNavDir} class description for greater details.

    @memo DjVu Navigation Directory
    @author Andrei Erofeev <eaf@geocities.com>
    @version #$Id: DjVuNavDir.h,v 1.2 1999-05-25 19:42:28 eaf Exp $#
*/

//@{

//*****************************************************************************
//********************* Note: this class is thread-safe ***********************
//*****************************************************************************

/** DjVu Navigation Directory.

    This class implements the {\em navigation directory} of a multipage
    DjVu document - basically a list of pages that this document is composed
    of. We would like to emphasize, that this is the list of namely
    {\bf pages}, not {\bf files}. Any page may include any
    number of additional files. When you've got an all-in-one-file multipage
    DjVu document (DjVm archive) you may get the files list from \Ref{DjVmDir0}
    class.

    The \Ref{DjVuNavDir} class can decode and encode the navigation directory
    from {\bf NDIR} IFF chunk. It's normally created by the library during
    decoding procedure and can be accessed like any other component of
    the \Ref{DjVuImage} being decoded.
    
    In a typical multipage DjVu document the navigation directory is stored
    in a separate IFF file containing only one chunk: {\bf NDIR} chunk.
    This file should be included (by means of the {\bf INCL} chunk) into
    every page of the document to enable the navigation. */
class DjVuNavDir : public GPEnabled
{
private:
   GCriticalSection		lock;
   GURL				baseURL;
   DArray<GString>		page2name;
   GMap<GString, int>		name2page;
   GMap<GURL, int>		url2page;
public:
   int		get_memory_usage(void) const { return 1024; };

      /** Constructs #DjVuNavDir# object. #dir_url# is the URL of the file
	  containing the directory source data. It will be used later
	  in translation by functions like \Ref{url_to_page}() and
	  \Ref{page_to_url}() */
   DjVuNavDir(const char * dir_url=0);
      /** Constructs #DjVuNavDir# object by decoding its contents from
	  the stream. #dir_url# is the URL of the file containing the
	  directory source data. */
   DjVuNavDir(ByteStream & str, const char * dir_url=0);
   virtual ~DjVuNavDir(void) {};

      /// Decodes the directory contents from the given \Ref{ByteStream}
   void		decode(ByteStream & str);

      /// Encodes the directory contents into the given \Ref{ByteStream}
   void		encode(ByteStream & str);

      /** Inserts a new page at position #where# pointing to a file
	  with name #name#.

	  @param where The position where the page should be inserted.
	  	 #-1# means to append.
	  @param name The name of the file corresponding to this page.
	  	 The name may not contain slashes. The file may include
		 other files. */
   void		insert_page(int where, const char * name);

      /// Deletes page with number #page_num# from the directory.
   void		delete_page(int page_num);

      /// Returns the number of pages in the directory.
   int		get_pages_num(void) const;
      /** Converts the #url# to page number. Returns #-1# if the #url#
	  does not correspond to anything in the directory. */
   int		url_to_page(const GURL & url) const;
      /** Converts file name #name# to page number. Returns #-1# if file
	  with given name cannot be found. */
   int		name_to_page(const char * name) const;
      /** Converts given #page# to URL. Throws an exception if page number
	  is invalid. */
   GURL		page_to_url(int page) const;
      /** Converts given #page# to URL. Throws an exception if page number
	  is invalid. */
   GString	page_to_name(int page) const;
};

//@}

#endif
