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
//C- $Id: DjVuDocument.h,v 1.1.2.1 1999-04-12 16:48:21 eaf Exp $
 
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
   DjVuDocument(const GURL & url, DjVuPort * port=0,
		GCache<GURL, DjVuFile> * cache=0);
   virtual ~DjVuDocument(void);

   GP<DjVuFile>	get_djvu_file(const GURL & url);
   GP<DjVuFile>	get_djvu_file(int page_num);

   GP<DjVuImage>get_page(int page_num);
   
   GP<DjVuNavDir>	get_dir(void) const;

   bool		is_djvm(void) const;
   bool		djvm_contains(const GURL & url) const;
   GURL		djvm_get_first_page_url(void) const;
   GURL		get_init_url(void) const;

   TArray<char>	get_djvm_data(void);
   void		save_as_djvm(const char * file_name);
   void		save_as_djvu(const char * dir_name);

      // Functions inherited from DjVuPort
   virtual bool		inherits(const char * class_name) const;
   virtual GP<DataRange>request_data(const DjVuPort * source, const GURL & url);
   virtual void		notify_chunk_done(const DjVuPort * source, const char * name);
private:
   GURL			init_url;
   GCache<GURL, DjVuFile> * cache;
   DjVuSimplePort	* simple_port;

   GP<DjVuNavDir>	dir;

      // DjVm format related data
   bool			djvm;
   GP<DataPool>		djvm_pool;
   DjVmDir0		djvm_dir;
   GString		djvm_first_page_name;

      // DjVu format related data
   GP<DataPool>		djvu_pool;

   void			detect_doc_type(void);
   GString		djvm_url2name(const GURL & url) const;
   GURL			djvm_name2url(const char * name) const;

   GP<DjVmFile>		get_djvm_file(void);
};

inline bool
DjVuDocument::inherits(const char * class_name) const
{
   return
      !strcmp("DjVuDocument", class_name) ||
      DjVuPort::inherits(class_name);
}

inline GURL
DjVuDocument::get_init_url(void) const
{
   return init_url;
}

inline bool
DjVuDocument::is_djvm(void) const
{
   return djvm;
}

inline GP<DjVuNavDir>
DjVuDocument::get_dir(void) const
{
   return dir;
}

#endif
