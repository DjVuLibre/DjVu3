//C-  -*- C++ -*-
//C-
//C-  Copyright © 1999-2001, LizardTech, Inc. All Rights Reserved.
//C-              Unauthorized use prohibited.
//C-
// 
// $Id: bin2cpp.cpp,v 1.1 2001-05-29 22:05:28 bcr Exp $
// $Name:  $


#include <stdio.h>
#include <stdlib.h>

#include "GURL.h"
#include "GString.h"
#include "BSByteStream.h"

int
main(int argc, char ** argv)
{
   if (argc!=2)
   {
      fprintf(stderr, "Usage: %s <var_name>\n", argv[0]);
      exit(1);
   }

   const char * name=argv[1];

      // First read all data into memory
   GP<ByteStream> gstr_in=ByteStream::create(GURL::Filename::UTF8("-"),"rb");
   ByteStream &str_in=*gstr_in;
   GP<ByteStream> gmem_str1=ByteStream::create();
   ByteStream &mem_str1=*gmem_str1;
   mem_str1.copy(str_in);

      // Now compute block size for BSByteStream
   int block_size=1+(mem_str1.size() >> 10);
   if (block_size<10) block_size=10;
   if (block_size>1024) block_size=1024;

      // Compress data
   GP<ByteStream> mem_str2=ByteStream::create();
   {
      GP<ByteStream> bs_str=BSByteStream::create(mem_str2, block_size);
      mem_str1.seek(0);
      bs_str->copy(mem_str1);
   }

      // Finally: generate file
   printf("\
/* This is an AUTOMATICALLY generated file (by bin2cpp). STAY AWAY! */\n\
\n\
#include \"cin_data.h\"\n\
\n\
static int %s_len=%d;\n\
static const char %s_data[]=\"\\\n", name, mem_str2->size(), name);

   mem_str2->seek(0);
   int chars=0;
   int size=0;
   while(1)
   {
      unsigned char ch;
      if (!mem_str2->read(&ch, 1)) break;
      printf("\\%03o", ch);
      chars+=4;
      if (chars>70)
      {
	 chars=0;
	 printf("\\\n");
      }
      size++;
   }

   printf("\";\n\
\n\
class %s_class\n\
{\n\
public:\n\
   %s_class(void)\n\
   {\n\
      CINData::add(\"%s\", %s_data, %s_len);\n\
   }\n\
};\n\
\n\
static %s_class %s_var;\n\n", name, name, name, name, name, name, name);
}
