//C-  -*- C++ -*-
//C- DjVu® Reference Library (v. 3.5)
//C- 
//C- Copyright © 1999-2001 LizardTech, Inc. All Rights Reserved.
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
// $Id: annotate.cpp,v 1.19 2001-09-21 20:21:30 leonb Exp $
// $Name:  $

/*****************************************************************************
 *
 *   $Revision: 1.19 $
 *   $Date: 2001-09-21 20:21:30 $
 *   @(#) $Id: annotate.cpp,v 1.19 2001-09-21 20:21:30 leonb Exp $
 *
 *****************************************************************************/

static const char RCSVersion[]="@(#) $Id: annotate.cpp,v 1.19 2001-09-21 20:21:30 leonb Exp $";

#include "GIFFManager.h"
#include "GOS.h"
#include "GURL.h"
#include "DjVuMessage.h"
#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

static const char ascii_ant[]="ANTa";
static const char binary_ant[]="ANTz";
inline static void WrongParams(void)
{
   G_THROW( ERR_MSG("annotate.usage") );
}

static inline void del_anno(GIFFManager &mng)
{
   const int chunksa=mng.get_chunks_number(ascii_ant); 
   for(int i=0;i<chunksa;i++)
     mng.del_chunk(ascii_ant);
   const int chunksz=mng.get_chunks_number(binary_ant); 
   for(int j=0;j<chunksz;j++)
     mng.del_chunk(binary_ant);
}

static void remove_djvu(GArray<GUTF8String> & argv)
   // argv[]: annotate -remove <djvu_in> [<djvu_out>]
{
   const int argc=argv.hbound()+1;
   if (argc<3) WrongParams();

   GP<ByteStream> src=ByteStream::create(GURL::Filename::UTF8(argv[2]), "rb");
   GP<GIFFManager> gmng=GIFFManager::create();
   GIFFManager &mng=*gmng;
   mng.load_file(src);
   
   del_anno(mng);
      
   GUTF8String fname;
   if(argc > 3)
   {
     fname=argv[3];
   }else
   {
     const int last_dot_pos=argv[2].rsearch('.');
     GUTF8String ext;
     if(last_dot_pos >= 0)
     {
       ext=last_dot_pos+(const char *)argv[2];
       argv[2].setat(last_dot_pos,0);
     }
     fname=argv[2]+"_ant"+ext;
   }
   GP<ByteStream> dst=ByteStream::create(GURL::Filename::UTF8(fname), "wb");
   mng.save_file(dst);
}

static void extract_djvu(GArray<GUTF8String> & argv)
   // argv[]: annotate -extract <djvu_in> <annotate_out> [<djvu_out>]
{
   const int argc=argv.hbound()+1;
   if (argc<4) WrongParams();
   GP<ByteStream> src=ByteStream::create(GURL::Filename::UTF8(argv[2]), "rb");
   GP<GIFFManager> gmng=GIFFManager::create();
   GIFFManager &mng=*gmng;
   mng.load_file(src);

   GP<GIFFChunk> chunk=mng.get_chunk(ascii_ant);
   if (!chunk)
   {
     chunk=mng.get_chunk(binary_ant);
     if(!chunk)
     {
       G_THROW(GUTF8String( ERR_MSG("annotate.failed_chunk") "\t")+ascii_ant);
     }
   }
   TArray<char> ant_contents=chunk->get_data();
   GP<ByteStream> ant=ByteStream::create(GURL::Filename::UTF8(argv[3]), "wb");
   ant->write((const char *)ant_contents, ant_contents.size());
   
   if (argc>4)
   {
      del_anno(mng);
      GP<ByteStream> dst=ByteStream::create(GURL::Filename::UTF8(argv[4]), "wb");
      mng.save_file(dst);
   }
}

static void insert_djvu(GArray<GUTF8String> & argv)
   // argv[]: annotate -insert <djvu_in> <annotate_in> [<djvu_out>]
{
   const int argc=argv.hbound()+1;
   if (argc<4) WrongParams();
   
   GP<ByteStream> src=ByteStream::create(GURL::Filename::UTF8(argv[2]), "rb");
   GP<GIFFManager> gmng=GIFFManager::create();
   GIFFManager &mng=*gmng;
   mng.load_file(src);
   
   struct stat st;
   if (stat(argv[3], &st)<0)
      G_THROW( ERR_MSG("annotate.failed_stat") );
   TArray<char> ant_contents(st.st_size-1);
   GP<ByteStream> ant=ByteStream::create(GURL::Filename::UTF8(argv[3]), "rb");
   ant->read(ant_contents, ant_contents.size());
  
   del_anno(mng);

   GUTF8String ant_name=ascii_ant;
   int info_pos;
   if (mng.get_chunk("INFO", &info_pos))
   {
      char buffer[128];
      sprintf(buffer, "[%d]", info_pos+1);
      ant_name+=buffer;
   } else ant_name+="[0]";
   mng.add_chunk(ant_name, ant_contents);

   GUTF8String fname;
   if(argc > 4)
   {
     fname=argv[4];
   }else
   {
     GUTF8String gdot=argv[2];
     gdot=gdot.getNative2UTF8();
     const int last_dot_pos=gdot.rsearch('.');
     GUTF8String ext;
     if(last_dot_pos >= 0)
     {
       ext=last_dot_pos+(const char *)gdot;
       gdot.setat(last_dot_pos,0);
     }
     fname=gdot+"_ant"+ext;
   }
   GP<ByteStream> dst=ByteStream::create(GURL::Filename::UTF8(fname),"wb");
   mng.save_file(dst);
}

int main(int argc, char ** argv)
{
  setlocale(LC_ALL,"");
  djvu_programname(argv[0]);
  GArray<GUTF8String> dargv(0,argc-1);
  for(int i=0;i<argc;++i)
    dargv[i]=GNativeString(argv[i]);

   G_TRY
   {
      if (argc<2) WrongParams();
      
      if (dargv[1] == "-extract") extract_djvu(dargv);
      else if (dargv[1] ==  "-insert") insert_djvu(dargv);
      else remove_djvu(dargv);
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

