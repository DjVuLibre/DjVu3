//C-  -*- C++ -*-
//C-
//C-  Copyright (c) 1988 AT&T	
//C-  All Rights Reserved 
//C-
//C-  THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
//C-  The copyright notice above does not evidence any
//C-  actual or intended publication of such source code.
//C-
//C-  $Id: annotate.cpp,v 1.1.1.1 1999-01-22 00:40:19 leonb Exp $

/*****************************************************************************
 *
 *   $Revision: 1.1.1.1 $
 *   $Date: 1999-01-22 00:40:19 $
 *   @(#) $Id: annotate.cpp,v 1.1.1.1 1999-01-22 00:40:19 leonb Exp $
 *
 *****************************************************************************/

static char RCSVersion[]="@(#) $Id: annotate.cpp,v 1.1.1.1 1999-01-22 00:40:19 leonb Exp $";

#include "GIFFManager.h"
#include <stdio.h>
#include <sys/stat.h>

inline static void WrongParams(void)
{
   THROW("USAGE:\n\tannotate -extract|-insert <djvu_in> <ant_file> [<djvu_out>]\n"
	 "or\tannotate -remove <djvu_in> [<djvu_out>].\n");
}

static void remove_djvu(int argc, char ** argv)
   // argv[]: annotate -remove <djvu_in> [<djvu_out>]
{
   if (argc<3) WrongParams();
   
   StdioByteStream src(argv[2], "r");
   GIFFManager mng;
   mng.loadFile(src);
   
   int chunks=mng.getChunksNumber("ANTa"); 
   for(int i=0;i<chunks;i++) mng.delChunk("ANTa");
      
   char * dot, * last_dot=0;
   for(dot=argv[2];*dot;dot++)
      if (*dot=='.') last_dot=dot;
   if (!last_dot) last_dot=argv[2]+strlen(argv[2]);
   else *last_dot++=0;
   StdioByteStream dst(argc>3 ? GString(argv[3]) :
		       GString(argv[2])+"_ant."+last_dot, "w");
   mng.saveFile(dst);
}

static void extract_djvu(int argc, char ** argv)
   // argv[]: annotate -extract <djvu_in> <annotate_out> [<djvu_out>]
{
   if (argc<4) WrongParams();
   
   StdioByteStream src(argv[2], "r");
   GIFFManager mng;
   mng.loadFile(src);

   GP<GIFFChunk> chunk=mng.getChunk("ANTa");
   if (!chunk) THROW("Failed to find chunk 'ANTa' in the source file.");
   GArray<char> ant_contents=chunk->getData();
   StdioByteStream ant(argv[3], "w");
   ant.write(ant_contents, ant_contents.size());
   
   if (argc>4)
   {
      int chunks=mng.getChunksNumber("ANTa");
      for(int i=0;i<chunks;i++) mng.delChunk("ANTa");
      StdioByteStream dst(argv[4], "w");
      mng.saveFile(dst);
   };
}

static void insert_djvu(int argc, char ** argv)
   // argv[]: annotate -insert <djvu_in> <annotate_in> [<djvu_out>]
{
   if (argc<4) WrongParams();
   
   StdioByteStream src(argv[2], "r");
   GIFFManager mng;
   mng.loadFile(src);
   
   struct stat st;
   if (stat(argv[3], &st)<0)
      THROW("Failed to stat annotation file.");
   GArray<char> ant_contents(st.st_size-1);
   StdioByteStream ant(argv[3], "r");
   ant.read(ant_contents, ant_contents.size());
  
   int chunks=mng.getChunksNumber("ANTa"); 
   for(int i=0;i<chunks;i++) mng.delChunk("ANTa");

   GString ant_name="ANTa";
   int info_pos;
   if (mng.getChunk("INFO", &info_pos))
   {
      char buffer[128];
      sprintf(buffer, "[%d]", info_pos+1);
      ant_name+=buffer;
   } else ant_name+="[0]";
   mng.addChunk(ant_name, ant_contents, ant_contents.size());
      
   char * dot, * last_dot=0;
   for(dot=argv[2];*dot;dot++)
      if (*dot=='.') last_dot=dot;
   if (!last_dot) last_dot=argv[2]+strlen(argv[2]);
   else *last_dot++=0;
   StdioByteStream dst(argc>4 ? GString(argv[4]) :
		       GString(argv[2])+"_ant."+last_dot, "w");
   mng.saveFile(dst);
}

int main(int argc, char ** argv)
{
   TRY
   {
      if (argc<2) WrongParams();
      
      if (!strcmp(argv[1], "-extract")) extract_djvu(argc, argv);
      else if (!strcmp(argv[1], "-insert")) insert_djvu(argc, argv);
      else remove_djvu(argc, argv);
   } 
   CATCH(exc)
   {
      exc.perror();
   } 
   ENDCATCH;
   return 0;
}
