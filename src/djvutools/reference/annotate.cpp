//C-  -*- C++ -*-
//C-
//C-  Copyright (c) 1988 AT&T	
//C-  All Rights Reserved 
//C-
//C-  THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
//C-  The copyright notice above does not evidence any
//C-  actual or intended publication of such source code.
//C-
//C-  $Id: annotate.cpp,v 1.7.2.1 2001-03-22 02:04:15 bcr Exp $

/*****************************************************************************
 *
 *   $Revision: 1.7.2.1 $
 *   $Date: 2001-03-22 02:04:15 $
 *   @(#) $Id: annotate.cpp,v 1.7.2.1 2001-03-22 02:04:15 bcr Exp $
 *
 *****************************************************************************/

static const char RCSVersion[]="@(#) $Id: annotate.cpp,v 1.7.2.1 2001-03-22 02:04:15 bcr Exp $";

#include "GIFFManager.h"
#include "GOS.h"
#include "GURL.h"
#include <stdio.h>
#include <sys/stat.h>

static const char ascii_ant[]="ANTa";
static const char binary_ant[]="ANTz";
inline static void WrongParams(void)
{
   G_THROW("annotate.usage");
}

static inline void del_anno(GIFFManager &mng)
{
   const int chunksa=mng.get_chunks_number(ascii_ant); 
   for(int i=0;i<chunksa;i++)
     mng.del_chunk(ascii_ant);
   const int chunksz=mng.get_chunks_number(binary_ant); 
   for(int i=0;i<chunksz;i++)
     mng.del_chunk(binary_ant);
}

static void remove_djvu(int argc, char ** argv)
   // argv[]: annotate -remove <djvu_in> [<djvu_out>]
{
   if (argc<3) WrongParams();
   
   GP<ByteStream> src=ByteStream::create(GOS::filename_to_url(argv[2]), "rb");
   GP<GIFFManager> gmng=GIFFManager::create();
   GIFFManager &mng=*gmng;
   mng.load_file(src);
   
   del_anno(mng);
      
   char * dot, * last_dot=0;
   for(dot=argv[2];*dot;dot++)
      if (*dot=='.') last_dot=dot;
   if (!last_dot) last_dot=argv[2]+strlen(argv[2]);
   else *last_dot++=0;
   GP<ByteStream> dst=ByteStream::create(GOS::filename_to_url(argc>3 ? GString(argv[3]) :
		       GString(argv[2])+"_ant."+last_dot), "wb");
   mng.save_file(dst);
}

static void extract_djvu(int argc, char ** argv)
   // argv[]: annotate -extract <djvu_in> <annotate_out> [<djvu_out>]
{
   if (argc<4) WrongParams();
   
   GP<ByteStream> src=ByteStream::create(GOS::filename_to_url(argv[2]), "rb");
   GP<GIFFManager> gmng=GIFFManager::create();
   GIFFManager &mng=*gmng;
   mng.load_file(src);

   GP<GIFFChunk> chunk=mng.get_chunk(ascii_ant);
   if (!chunk)
   {
     chunk=mng.get_chunk(binary_ant);
     if(!chunk)
     {
       G_THROW(GString("annotate.failed_chunk\t")+ascii_ant);
     }
   }
   TArray<char> ant_contents=chunk->get_data();
   GP<ByteStream> ant=ByteStream::create(GOS::filename_to_url(argv[3]), "wb");
   ant->write((const char *)ant_contents, ant_contents.size());
   
   if (argc>4)
   {
      del_anno(mng);
      GP<ByteStream> dst=ByteStream::create(GOS::filename_to_url(argv[4]), "wb");
      mng.save_file(dst);
   }
}

static void insert_djvu(int argc, char ** argv)
   // argv[]: annotate -insert <djvu_in> <annotate_in> [<djvu_out>]
{
   if (argc<4) WrongParams();
   
   GP<ByteStream> src=ByteStream::create(GOS::filename_to_url(argv[2]), "rb");
   GP<GIFFManager> gmng=GIFFManager::create();
   GIFFManager &mng=*gmng;
   mng.load_file(src);
   
   struct stat st;
   if (stat(argv[3], &st)<0)
      G_THROW("annotate.failed_stat");
   TArray<char> ant_contents(st.st_size-1);
   GP<ByteStream> ant=ByteStream::create(GOS::filename_to_url(argv[3]), "rb");
   ant->read(ant_contents, ant_contents.size());
  
   del_anno(mng);

   GString ant_name=ascii_ant;
   int info_pos;
   if (mng.get_chunk("INFO", &info_pos))
   {
      char buffer[128];
      sprintf(buffer, "[%d]", info_pos+1);
      ant_name+=buffer;
   } else ant_name+="[0]";
   mng.add_chunk(ant_name, ant_contents);
      
   char * dot, * last_dot=0;
   for(dot=argv[2];*dot;dot++)
      if (*dot=='.') last_dot=dot;
   if (!last_dot) last_dot=argv[2]+strlen(argv[2]);
   else *last_dot++=0;
   GP<ByteStream> dst=ByteStream::create(GOS::filename_to_url(argc>4 ? GString(argv[4]) :
		       GString(argv[2])+"_ant."+last_dot), "wb");
   mng.save_file(dst);
}

int main(int argc, char ** argv)
{
   G_TRY
   {
      if (argc<2) WrongParams();
      
      if (!strcmp(argv[1], "-extract")) extract_djvu(argc, argv);
      else if (!strcmp(argv[1], "-insert")) insert_djvu(argc, argv);
      else remove_djvu(argc, argv);
   } 
   G_CATCH(exc)
   {
      exc.perror();
      exit(1);
   } 
   G_ENDCATCH;
   exit(0);
#ifdef WIN32
   return 0;
#endif
}

