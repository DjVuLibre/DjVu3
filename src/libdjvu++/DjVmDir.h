//C-  -*- C++ -*-
//C- DjVu� Reference Library (v. 3.0)
//C- 
//C- Copyright � 1999-2001 LizardTech, Inc. All Rights Reserved.
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
// 
// $Id: DjVmDir.h,v 1.31 2001-02-15 01:12:22 bcr Exp $
// $Name:  $

#ifndef _DJVMDIR_H
#define _DJVMDIR_H


/** @name DjVmDir.h
    Files #"DjVmDir.h"# and #"DjVmDir.cpp"# implement class \Ref{DjVmDir} for
    representing the directory of a DjVu multipage document.

    {\bf Bundled vs. Indirect format} --- There are currently two multipage
    DjVu formats supported: {\em bundled} and {\em indirect}.  In the first
    format all component files composing a given document are packaged (or
    bundled) into one file, in the second one every page and component is
    stored in a separate file and there is one more file, which contains the
    list of all others.

    {\bf Multipage DjVu format} --- Multipage DjVu documents follow the EA
    IFF85 format (cf. \Ref{IFFByteStream.h}.)  A document is composed of a
    #"FORM:DJVM"# whose first chunk is a #"DIRM"# chunk containing the {\em
    document directory}.  This directory lists all component files composing
    the given document, helps to access every component file and identify the
    pages of the document.
    \begin{itemize} 
    \item In a {\em bundled} multipage file, the component files 
         are stored immediately after the #"DIRM"# chunk,
         within the #"FORM:DJVU"# composite chunk.  
    \item In an {\em indirect} multipage file, the component files are 
          stored in different files whose URLs are composed using information 
          stored in the #"DIRM"# chunk.
    \end{itemize} 
    Most of the component files represent pages of a document.  Some files
    however represent data shared by several pages.  The pages refer to these
    supporting files by means of an inclusion chunk (#"INCL"# chunks)
    identifying the supporting file.

    {\bf Document Directory} --- Every directory record describes a component
    file.  Each component file is identified by a small string named the
    identifier (ID).  Each component file also contains a file name and a
    title.  The format of the #"DIRM"# chunk is described in section
    \Ref{Format of the DIRM chunk.}.

    Theoretically, IDs are used to uniquely identify each component file in
    #"INCL"# chunks, names are used to compose the the URLs of the component
    files in an indirect multipage DjVu file, and titles are cosmetic names
    possibly displayed when viewing a page of a document.  There are however
    many problems with this scheme, and we {\em strongly suggest}, with the
    current implementation to always make the file ID, the file name and the
    file title identical.

    @memo Implements DjVu multipage document directory
    @author Andrei Erofeev <eaf@geocities.com>
    @version
    #$Id: DjVmDir.h,v 1.31 2001-02-15 01:12:22 bcr Exp $# */
//@{


#ifdef __GNUC__
#pragma interface
#endif

#include "GString.h"
#include "GThreads.h"

class ByteStream;

/** Implements DjVu multipage document directory.  There are currently two
    multipage DjVu formats supported: {\em bundled} and {\em indirect}.  In
    the first format all component files composing a given document are
    packaged (or bundled) into one file, in the second one every page and
    component is stored in a separate file and there is one more file, which
    contains the list of all others.

    The multipage document directory lists all component files composing the
    given document, helps to access every file, identify pages and maintain
    user-specified shortcuts.  Every directory record describes a file
    composing the document.  Each file is identified by a small string named
    the identifier (ID).  Each file may also contain a file name and a title.

    The #DjVmDir# class represents a multipage document directory.  Its main
    purpose is to encode and decode the document directory when writing or
    reading the #DIRM# chunk.  Normally you don't have to create this class
    yourself. It's done automatically when \Ref{DjVmDoc} class initializes
    itself. It may be useful though to be able to access records in the
    directory because some classes (like \Ref{DjVuDocument} and \Ref{DjVmDoc})
    return a pointer to #DjVmDir# in some cases. */

class DjVmDir : public GPEnabled
{
public:
   class File;

   static const int version;
      /** Class \Ref{DjVmDir::File} represents the directory records
          managed by class \Ref{DjVmDir}. */
   DjVmDir(void) { } ;
      /** Decodes the directory from the specified stream. */
   void decode(GP<ByteStream> stream);
      /** Encodes the directory into the specified stream. */
   void encode(GP<ByteStream> stream) const;
      /** Tests if directory defines an {\em indirect} document. */
   bool is_indirect(void) const;
      /** Tests if the directory defines a {\em bundled} document. */
   bool is_bundled(void) const;
      /** Translates page numbers to file records. */
   GP<File> page_to_file(int page_num) const;
      /** Translates file names to file records. */
   GP<File> name_to_file(const GString & name) const;
      /** Translates file IDs to file records. */
   GP<File> id_to_file(const char * id) const;
      /** Translates file shortcuts to file records. */
   GP<File> title_to_file(const char * title) const;
      /** Returns position of the file in the directory. */
   int get_file_pos(const File * f) const;
      /** Returns position of the given page in the directory. */
   int get_page_pos(int page_num) const;
      /** Returns a copy of the list of file records. */
   GPList<File> get_files_list(void) const;
      /** Returns the number of file records. */
   int get_files_num(void) const;
      /** Returns the number of file records representing pages. */
   int get_pages_num(void) const;
      /** Returns back pointer to the file with #SHARED_ANNO# flag.
	  Note that there may be only one file with shared annotations
	  in any multipage DjVu document. */
   GP<File> get_shared_anno_file(void) const;
      /** Changes the title of the file with ID #id#. */
   void set_file_title(const char * id, const char * title);
      /** Changes the name of the file with ID #id#. */
   void set_file_name(const char * id, const char * name);
      /** Inserts the specified file record at the specified position.
	  Specifying #pos# equal to #-1# means to append. */
   void insert_file(const GP<File> & file, int pos=-1);
      /** Removes a file record with ID #id#. */
   void delete_file(const char * id);
private:
   GCriticalSection 	class_lock;
   GPList<File>		files_list;
   GPArray<File>	page2file;
   GPMap<GString, File>	name2file;
   GPMap<GString, File>	id2file;
   GPMap<GString, File>	title2file;
};

class DjVmDir::File : public GPEnabled
{
public:
	 // Out of the record: INCLUDE below must be zero and PAGE must be one.
	 // This is to avoid problems with the File constructor, which now takes
	 // 'int file_type' as the last argument instead of 'bool is_page'
   
	 /** File type. Possible file types are:
	     \begin{description}
	     \item[PAGE] This is a top level page file. It may include other
	     #INCLUDE#d files, which may in turn be shared between
	     different pages.
	     \item[INCLUDE] This file is included into some other file inside
	     this document.
	     \item[THUMBNAILS] This file contains thumbnails for the document
	     pages.
	     \item[SHARED_ANNO] This file contains annotations shared by
	     all the pages. It's supposed to be included into every page
	     for the annotations to take effect. There may be only one
	     file with shared annotations in a document.
	     \end{description} */
      enum FILE_TYPE { INCLUDE=0, PAGE=1, THUMBNAILS=2, SHARED_ANNO=3 };
	 /** File name.  The optional file name must be unique and is assigned
	     either by encoder or by user when the document is composed.  In the
	     case of an {\em indirect} document, this is the relative URL of the
	     file.  By keeping the name in {\em bundled} document we guarantee,
	     that it can be expanded later into {\em indirect} document and files
	     will still have the same names. */
      GString name;
	 /** File identifier.  The encoder assigns a unique identifier to each file
	     in a multipage document. Indirection chunks in other files (#"INCL"#
	     chunks) may refer to another file using its identifier. */
      GString id;
	 /** File title.  The file title is assigned by the user and may be used as
	     a shortcut for viewing a particular page.  Names like #"chapter1"# or
	     #"appendix"# are appropriate. */
      GString title;
	 /** Offset of the file data in a bundled DJVM file.  This number is
	     relevant in the {\em bundled} case only when everything is packed into
	     one single file. */
      int offset;
	 /** Size of the file data in a bundled DJVM file.  This number is
	     relevant in the {\em bundled} case only when everything is
	     packed into one single file. */
      int size;
         /** Tests whether a file ID is legal.
             This function only checks that #id# is syntactically legal.
             It does not check for duplicate file IDs in a directory. */  
      static bool is_legal_id(const char *id);
      GString get_str_type(void) const;
	 /** Tests if this file represents a page of the document. */
      bool is_page(void) const 
	 { return (flags & TYPE_MASK)==PAGE; }
	 /** Returns #TRUE# if this file is included into some other files of
	     this document */
      bool is_include(void) const
	 { return (flags & TYPE_MASK)==INCLUDE; }
	 /** Returns #TRUE# if this file contains thumbnails for the document pages. */
      bool is_thumbnails(void) const
	 { return (flags & TYPE_MASK)==THUMBNAILS; }
	 /** Returns the page number of this file. This function returns
	     #-1# if this file does not represent a page of the document. */
      bool is_shared_anno(void) const
	 { return (flags & TYPE_MASK)==SHARED_ANNO; }
      int get_page_num(void) const 
	 { return page_num; } 
	 /** Default constructor. */
      File(void);
	 /** Full constructor. */
      File(const char *name, const char *id, const char *title, FILE_TYPE file_type);
	 // Obsolete
      File(const char *name, const char *id, const char *title, bool page);
private:
      friend class DjVmDir;
      enum FLAGS_0 { IS_PAGE_0=1, HAS_NAME_0=2, HAS_TITLE_0=4 };
      enum FLAGS_1 { HAS_NAME=0x80, HAS_TITLE=0x40, TYPE_MASK=0x3f };
      unsigned char flags;
      int page_num;
};

/** @name Format of the DIRM chunk.

    {\bf Variants} --- There are two versions of the #"DIRM"# chunk format.
    The version number is identified by the seven low bits of the first byte
    of the chunk.  Version {\bf 0} is obsolete and should never be used.  This
    section describes version {\bf 1}.  There are two major multipage DjVu
    formats supported: {\em bundled} and {\em indirect}.  The #"DIRM"# chunk
    indicates which format is used in the most significant bit of the first
    byte of the chunk.  The document is bundled when this bit is set.
    Otherwise the document is indirect.

    {\bf Unencoded data} --- The #"DIRM"# chunk is composed some unencoded
    data followed by \Ref{bzz} encoded data.  The unencoded data starts with
    the version byte and a 16 bit integer representing the number of component
    files.  All integers are encoded with the most significant byte first.
    \begin{verbatim}
          BYTE:             Flags/Version:  0x<bundled>0000011
          INT16:            Number of component files.
    \end{verbatim}
    When the document is a bundled document (i.e. the flag #bundled# is set),
    this header is followed by the offsets of each of the component files within
    the #"FORM:DJVM"#.  These offsets allow for random component file access.
    \begin{verbatim}
          INT32:            Offset of first component file.
          INT32:            Offset of second component file.
          ...
          INT32:            Offset of last component file.
    \end{verbatim}

    {\bf BZZ encoded data} --- The rest of the chunk is entirely compressed
    with the BZZ general purpose compressor.  We describe now the data fed
    into (or retrieved from) the BZZ codec (cf. \Ref{BSByteStream}.)  First
    come the sizes and the flags associated with each component file.
    \begin{verbatim}
          INT24:             Size of the first component file.
          INT24:             Size of the second component file.
          ...
          INT24:             Size of the last component file.
          BYTE:              Flag byte for the first component file.
          BYTE:              Flag byte for the second component file.
          ...
          BYTE:              Flag byte for the last component file.
    \end{verbatim}
    The flag bytes have the following format:
    \begin{verbatim}
          0b<hasname><hastitle>000000     for a file included by other files.
          0b<hasname><hastitle>000001     for a file representing a page.
          0b<hasname><hastitle>000010     for a file containing thumbnails.
    \end{verbatim}
    Flag #hasname# is set when the name of the file is different from the file
    ID.  Flag #hastitle# is set when the title of the file is different from
    the file ID.  These flags are used to avoid encoding the same string three
    times.  Then come zero terminated strings representing the IDs of each
    file.  These strings are followed by zero terminated strings representing
    the names of only those files for which the flag #hasname# is set (if
    any).  Finally come zero-terminated strings representing the titles of
    only those files for which the flag #hastitle# is set (if any).  The
    \Ref{bzz} encoding system makes sure that all these strings will be
    encoded efficiently.
    \begin{verbatim}
          ZSTR:     ID of the first component file.
          ... 
          ZSTR:     ID of the last component file.
          ZSTR:     Name of the first file for which #hasname# is set.
          ...  
          ZSTR:     Name of the last file for which #hasname# is set..
          ZSTR:     Title of the first file for which #hastitle# is set.
          ...  
          ZSTR:     Title of the last file for which #hastitle# is set..
    \end{verbatim}

    {\bf Ideas for future evolutions} ---
    Besides the version byte contained in the #"DIRM"# chunk of a multipage
    document, there is a 16 bit version number in the #"INFO"# chunk of every
    page (see \Ref{DjVuInfo.h}.)  These version numbers have different values
    and different formats.  Although one could argue that the capability to
    parse a multipage file and the capability to view the pages are different
    in nature, tracking two version numbers is often a useless complexity.
    Here is a suggested format for the beginning of the unencoded part:
    \begin{verbatim}
          BYTE:             Flags:  0x<bundled>0000011
          INT24:            Number of component files (extended to 32 bits)
          INT16:            DjVu version number (cf "DjVuInfo.h")
    \end{verbatim}
    The concept of IDs, names and titles did not provide all the expected
    benefits.  A simpler and more effective scheme would consist of only
    providing a unique ID for each component file.  The URLs of the component
    file of indirect document could be constructed using a single template
    stored in the #"DIRM"# chunk.  This arrangement would give much more freedom 
    for placing the component files on a web server. Here are a few examples
    of templates:
    \begin{verbatim}
         "${ID}"
         "${DJVUBASE}/${DJVUNAME}.dir/${ID}"
         "/cgi-bin/onthefly.pl?DOC=${DJVUNAME}.djvu&FILE=${ID}"
    \end{verbatim}
    Finally the file titles would be advantageously replaced by an optional
    chunk #"NAVM"# following the DIRM chunk and describing how the user can
    navigate the document.  This chunk would define aliases for accessing the
    pages (such as ``Chapter 1'' and ``Part 2'').  It could also define how
    these aliases can be presented in a menu representing the structure of the
    document.  The PDF specification could be a good source of ideas.

    @memo Description of the format of the DIRM chunk.  */
//@}



// -------------- IMPLEMENTATION


inline bool
DjVmDir::is_indirect(void) const
{
  GCriticalSectionLock lock((GCriticalSection *) &class_lock);
  return files_list.size() && files_list[files_list] != 0 &&
     files_list[files_list]->offset==0;
}

inline bool
DjVmDir::is_bundled(void) const
{
  return !is_indirect();
}


// ----- THE END
#endif
