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
//C- $Id: DjVuDocument.h,v 1.1.2.2 1999-04-26 19:20:46 eaf Exp $
 
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

class DjVuDocument : public GPEnabled, public DjVuPort
{
public:
      // Construction/destruction
   DjVuDocument(const GURL & url, bool readonly,
		DjVuPort * port=0, GCache<GURL, DjVuFile> * cache=0);
   virtual ~DjVuDocument(void);

      // Requesting specific page or even file
   GP<DjVuFile>	get_djvu_file(const GURL & url);
   GP<DjVuFile>	get_djvu_file(int page_num);

      // Requesting image corresponding to given page
   GP<DjVuImage>get_page(int page_num);

      // Cache in use
   GCache<GURL, DjVuFile> * get_cache(void) const;

      // Navigation directory
   GP<DjVuNavDir>	get_dir(void) const;

      // Way to customize existing document
   void		insert_page(const GP<DjVuFile> & file, int page_num=-1);
   void		delete_page(int page_num);

      // DjVm format related stuff
   bool		is_djvm(void) const;
   bool		djvm_contains(const GURL & url) const;
   GURL		djvm_get_first_page_url(void) const;

      // Routines for saving
   TArray<char>	get_djvm_data(void);
   void		save_as_djvm(const char * file_name);
   void		save_as_djvu(const char * dir_name);

      // Functions inherited from DjVuPort
   virtual bool		inherits(const char * class_name) const;
   virtual GP<DataRange>request_data(const DjVuPort * source, const GURL & url);
   virtual void		notify_chunk_done(const DjVuPort * source, const char * name);
private:
   GURL			dir_url;
   GCache<GURL, DjVuFile> * cache;
   DjVuSimplePort	* simple_port;
   bool			readonly;

   GP<DjVuNavDir>	dir;
   GP<DjVuFile>		dir_file;

   GPList<DjVuFile>	added_files_list;
   GCriticalSection	added_files_lock;

      // DjVm format related data
   bool			djvm;
   GURL			djvm_doc_url;
   GP<DataPool>		djvm_pool;
   DjVmDir0		djvm_dir;
   GString		djvm_first_page_name;

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

#endif
