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
//C- $Id: DjVuFile.h,v 1.1.2.1 1999-04-12 16:48:21 eaf Exp $
 
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
   DjVuFile(const GURL & url, DjVuPort * port,
	    GCache<GURL, DjVuFile> * cache=0);
   virtual ~DjVuFile(void);

      // Status query
   int		get_status(void) const;
   bool		is_decoding(void) const;
   bool		is_decode_ok(void) const;
   bool		is_decode_failed(void) const;
   bool		is_decode_stopped(void) const;
   bool		is_data_present(void) const;
   bool		is_all_data_present(void) const;

   GURL		get_url(void) const;

      // Decoding control
   void		stop_decode(bool sync);
   void		start_decode(void);
   void		wait_for_finish(void);
   
      // Function needed by the cache
   unsigned int	get_memory_usage(void) const;

      // Way to know what is included
   GPList<DjVuFile>	get_included_files(void);

      // All data (included files too) in one array
   TArray<char>		get_djvu_data(bool included_too, bool no_ndir);
   void			add_to_djvm(DjVmFile & djvm_file);

      // Functions inherited from DjVuPort
   virtual bool		inherits(const char * class_name) const;
   virtual void		notify_chunk_done(const DjVuPort * source, const char * name);
   virtual void		notify_file_done(const DjVuPort * source);
   virtual void		notify_file_stopped(const DjVuPort * source);
   virtual void		notify_file_failed(const DjVuPort * source);
   virtual void		notify_all_data_received(const DjVuPort * source);
private:
   struct FinishEvent
   {
      GEvent	event;
      const DjVuFile	* file;
   };
   
   GURL			url;
   GP<DataRange>	data_range;
   GCache<GURL, DjVuFile> * cache;

   GCriticalSection	status_lock;
   int			status;

   GPList<DjVuFile>	inc_files_list;
   GCriticalSection	inc_files_lock;
   
   GThread		* decode_thread;
   GP<DjVuFile>		decode_life_saver;

   GCriticalSection	finish_event_lock;
   GList<void *>	finish_event_list;

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
   GP<DjVuFile>	include_file(ByteStream & str);
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
};

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

#endif
