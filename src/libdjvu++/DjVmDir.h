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
//C- $Id: DjVmDir.h,v 1.1.1.1 1999-10-22 19:29:23 praveen Exp $

#ifndef _DJVMDIR_H
#define _DJVMDIR_H


/** @name DjVmDir.h
    Files #"DjVmDir.h"# and #"DjVmDir.cpp"# contain implementation of
    DjVu multipage document directory represented by class \Ref{DjVmDir}.

    @memo Implements DjVu multipage document directory
    @author Andrei Erofeev <eaf@research.att.com>
    @version
    #$Id: DjVmDir.h,v 1.1.1.1 1999-10-22 19:29:23 praveen Exp $# */
//@{


#ifdef __GNUC__
#pragma interface
#endif

#include "DjVuGlobal.h"
#include "GString.h"
#include "GContainer.h"
#include "GContainer.h"
#include "ByteStream.h"
#include "GThreads.h"

/** Implements DjVu multipage document directory.  There are currently
    two multipage DjVu formats supported: {\em bundled} and {\em
    indirect}.  In the first format all files composing a given
    document are packaged (or bundled) into one file, in the second
    one every page and component is stored in a separate file and
    there is one more file, which contains the list of all others.

    In both cases there is somewhere a #DIRM# chunk with the {\em
    multipage DjVu document directory}. This directory lists all files
    composing the given document, helps to access every file, identify
    pages and maintain user-specified shortcuts.

    Every directory record describes a file composing the document.
    Each file is identified by a small string named the identifier
    (ID).  Each file may also contain a file name and a title.  Most
    of the files represent pages of a document.  Some files however
    represent data shared by several pages.  The pages refer to these
    supporting files by means of an indirection chunk (#"INCL"#
    chunks) identifying the supporting file by its identifier.

    The #DjVmDir# class does compression and decompression of the
    directory data when it's written or read from the #DIRM# chunk.
    Normally you don't have to create this class yourself. It's done
    automatically when \Ref{DjVmDoc} class initializes itself. It may
    be useful though to be able to access records in the directory
    because some classes (like \Ref{DjVuDocument} and \Ref{DjVmDoc})
    return a pointer to #DjVmDir# in some cases. 
*/

class DjVmDir : public GPEnabled
{
public:
  static const int version;
  /** This class represents the directory records managed by 
      class \Ref{DjVmDir}. */
  class File : public GPEnabled
  {
    friend class DjVmDir;
  public:
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
    int	offset;
    /** Size of the file data in a bundled DJVM file.  This number is
        relevant in the {\em bundled} case only when everything is
        packed into one single file. */
    int size;
    /** Tests if this file represents a page of the document. */
    bool is_page(void) const 
      { return (flags & IS_PAGE)!=0; } ;
    /** Returns the page number of this file. This function returns
        #-1# if this file does not represent a page of the document. */
    int	get_page_num(void) const 
      { return page_num; } ;
    /** Default constructor. */
    File(void);
    /** Full constructor. */
    File(const char *name, const char *id, const char *title, bool page);
  private:
    enum FLAGS { IS_PAGE=1, HAS_NAME=2, HAS_TITLE=4 };
    char flags;
    int	page_num;
  };

  DjVmDir(void) {};
  virtual ~DjVmDir() {};
  /** Decodes the directory from the specified stream. */
  void decode(ByteStream & stream);
  /** Encodes the directory into the specified stream. */
  void encode(ByteStream & stream) const;
  /** Tests if directory is from an {\em indirect} document
      (where every page is stored in a separate file.) */
  bool is_indirect(void) const;
  /** Tests if the directory is from a {\em bundled} document
      (where everything is bundled into one file.) */
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
  /** Returns the number of file records which represent pages. */
  int get_pages_num(void) const;
  /** Inserts the specified file record at the specified position.
      #pos=-1# means to append */
  void insert_file(File * file, int pos=-1);
  /** Removes a file record with the given #id# from the directory */
  void delete_file(const char * id);
private:
   GCriticalSection 	class_lock;
   GPList<File>		files_list;
   GPArray<File>	page2file;
   GPMap<GString, File>	name2file;
   GPMap<GString, File>	id2file;
   GPMap<GString, File>	title2file;
};

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

//@}

// ----- THE END
#endif
