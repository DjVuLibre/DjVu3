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
//C- $Id: DjVuFile.h,v 1.1.2.4 1999-05-03 19:21:23 eaf Exp $
 
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
#include "GPContainer.h"
#include "DjVuNavDir.h"
#include "GCache.h"
#include "DjVmFile.h"

class DjVuFile : public GPEnabled, public DjVuPort
{
public:
   enum { DECODING=1, DECODE_OK=2, DECODE_FAILED=4, DECODE_STOPPED=8,
	  DATA_PRESENT=16, ALL_DATA_PRESENT=32, INCL_FILES_CREATED=64 };

      // Decoded file contents
   GP<DjVuInfo>		info;
   GP<DjVuAnno>		anno;
   GP<IWPixmap>		bg44;
   GP<JB2Image>		fgjb;
   GP<GPixmap>		fgpm;
   GP<DjVuNavDir>	dir;
   GString		description;
   GString		mimetype;
   int			file_size;

      // Construction/destruction
   DjVuFile(const GURL & url, DjVuPort * port=0,
	    GCache<GURL, DjVuFile> * cache=0);
   virtual ~DjVuFile(void);

      // Port override
   void		disable_standard_port(void);

      // Status query
   int		get_status(void) const;
   bool		is_decoding(void) const;
   bool		is_decode_ok(void) const;
   bool		is_decode_failed(void) const;
   bool		is_decode_stopped(void) const;
   bool		is_data_present(void) const;
   bool		is_all_data_present(void) const;

   GURL		get_url(void) const;
   void		set_name(const char * name);

      // Decoding control
   void		stop_decode(bool sync);
   void		start_decode(void);
   void		wait_for_finish(void);
   void		reset(void);
   
      // Function needed by the cache
   unsigned int	get_memory_usage(void) const;

      // Way to know what is included
   GPList<DjVuFile>	get_included_files(void);

      // Way to control included files
   void		include_file(const GP<DjVuFile> & file, int chunk_pos=-1);
   void		unlink_file(const char * name);

      // Operations with chunks
   int		get_chunks_number(void);
   GString	get_chunk_name(int chunk_num);
   bool		contains_chunk(const char * chunk_name);
   void		delete_chunks(const char * chunk_name);
   void		insert_chunk(int pos, const char * chunk_name,
			     const TArray<char> & data);

      // All data (included files too) in one array
   TArray<char>		get_djvu_data(bool included_too, bool no_ndir);
   void			add_to_djvm(DjVmFile & djvm_file);

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
   
   GThread		* decode_thread;
   GP<DjVuFile>		decode_life_saver;
   GP<DataRange>	decode_data_range;

   DjVuPort		* simple_port;

   GMonitor		chunk_mon;

   GMonitor		finish_mon;
   
      // Functions called when the decoding thread starts
   static void	static_decode_func(void *);
   void		decode_func(void);
   void		decode(ByteStream & str);

      // Functions used to wait for smth
   void		wait_for_chunk(void);
   bool		wait_for_finish(bool self);

      // INCL chunk processor
   GP<DjVuFile>	process_incl_chunk(ByteStream & str);
      // Scans file for INCL chunks and creates associated files
   void		process_incl_chunks(void);

      // Trigger: called when DataRange has all data
   static void	static_trigger_cb(void *);
   void		trigger_cb(void);
      // Progress callback: called from time to time
   static void	progress_cb(int pos, void *);

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
   url=url.baseURL()+name;
}

inline void
DjVuFile::reset(void)
{
   info=0; anno=0; bg44=0; fgjb=0; fgpm=0;
   dir=0; description=""; mimetype="";
}

#endif
