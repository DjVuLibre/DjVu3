//C-  -*- C++ -*-
//C-
//C- This software contains software code or other information proprietary to
//C- AT&T Corp. ("AT&T").  Unless you accept a license to use this software,
//C- you shall not use, execute, compile, modify, redistribute, reverse
//C- compile, disassemble, or otherwise reverse engineer the AT&T software or
//C- any derived work of the AT&T software.  The text of a license can be
//C- found in file "ATTLICENSE" or at the Internet website having the URL
//C- "http://www.djvu.att.com/open".
//C-
//C- This software is provided to you "AS IS".  YOU "AS IS".  YOU ASSUME TOTAL
//C- RESPONSIBILITY AND RISK FOR YOUR USE OF THEM INCLUDING THE RISK OF ANY
//C- DEFECTS OR INACCURACIES THEREIN.  AT&T DOES NOT MAKE, AND EXPRESSLY
//C- DISCLAIMS, ANY EXPRESS OR IMPLIED WARRANTIES OF ANY KIND WHATSOEVER,
//C- INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTIES OF MERCHANTABILITY
//C- OR FITNESS FOR A PARTICULAR PURPOSE, WARRANTIES OF TITLE OR
//C- NON-INFRINGEMENT OF ANY INTELLECTUAL PROPERTY RIGHTS OR TRADEMARK RIGHTS,
//C- ANY WARRANTIES ARISING BY USAGE OF TRADE, COURSE OF DEALING OR COURSE OF
//C- PERFORMANCE, OR ANY WARRANTY THAT THE AT&T SOURCE CODE RELEASE OR AT&T
//C- CAPSULE ARE "ERROR FREE" WILL MEET YOUR REQUIREMENTS.
//C-
//C-     (C) AT&T Corp. All rights reserved.
//C-     AT&T is a registered trademark of AT&T Corp.
//C-
//C- $Id: djvuextract.cpp,v 1.8 1999-03-16 20:21:30 leonb Exp $

/** @name djvuextract

    {\bf Synopsis}
    \begin{verbatim}
    djvuextract <djvufile> [Sjbz=<maskout>] [FG44=<fgout>] [BG44=<bgout>]
    \end{verbatim}
    
    {\bf Description}\\
    Program #djvuextract# analyzes the DjVu file
    #<djvufile># and saves the various layers into the specified files.
    The reverse operation can be achieved using program \Ref{djvumake}.
    \begin{itemize}
    \item When option #Sjbz=<maskout># is specified, the foreground mask is
      saved into file #<maskout># as JB2 data. This data file can be read
      using function \Ref{JB2Image::decode} in class \Ref{JB2Image}.
    \item When option #FG44=<fgout># is specified, the foreground color image
      is saved into file #<fgout># as IW44 data.  This data file can be processed
      using programs \Ref{d44}.
    \item When option #BG44=<bgout># is specified, the background color image
      is saved into file #<bgout># as IW44 data.  This data file can be processed
      using programs \Ref{d44}.
    \end{itemize}

    @memo
    Extract components from DjVu files.
    @version
    #$Id: djvuextract.cpp,v 1.8 1999-03-16 20:21:30 leonb Exp $#
    @author
    L\'eon Bottou <leonb@research.att.com> */
//@{
//@}

#include "GException.h"
#include "ByteStream.h"
#include "IFFByteStream.h"
#include "ATTLicense.h"



struct DejaVuInfo
{
  unsigned char width_hi, width_lo;
  unsigned char height_hi, height_lo;
  char version;
} djvuinfo;

struct PrimaryHeader {
  unsigned char serial;
  unsigned char slices;
} primary;

struct SecondaryHeader {
  unsigned char major;
  unsigned char minor;
  unsigned char xhi, xlo;
  unsigned char yhi, ylo;
} secondary;


void
djvuextract(const char *filename,
          MemoryByteStream *pSjbz,
          MemoryByteStream *pBG44,
          MemoryByteStream *pFG44)
  
{
  IFFByteStream BG44(*pBG44);
  IFFByteStream FG44(*pFG44); 
  int color_bg = 1;
  int color_fg = 1;
  StdioByteStream ibs(filename,"rb");
  IFFByteStream iff(ibs);
  GString chkid;
  if (! iff.get_chunk(chkid))
    THROW("Malformed DJVU file");
  if (chkid != "FORM:DJVU")
    THROW("This IFF file is not a DJVU file");
  while (iff.get_chunk(chkid))
    {
      if (chkid=="INFO")
        {
          if (iff.readall((void*)&djvuinfo,sizeof(djvuinfo)) < sizeof(djvuinfo))
            THROW("Cannot read INFO chunk");
          fprintf(stderr, "%s: (%d x %d) version %d\n", 
                  filename, 
                  (djvuinfo.width_hi<<8)+djvuinfo.width_lo, 
                  (djvuinfo.height_hi<<8)+djvuinfo.height_lo,
                  djvuinfo.version );
        }
      else if (chkid == "Sjbz")
        {
          pSjbz->copy(iff);
        }
      else if (chkid == "FG44")
        {
          MemoryByteStream temp;
          temp.copy(iff);
          temp.seek(0);
          if (temp.readall((void*)&primary, sizeof(primary))<sizeof(primary))
            THROW("Cannot read primary header in FG44 chunk");
          if (primary.serial == 0)
            {
              if (temp.readall((void*)&secondary, sizeof(secondary))<sizeof(secondary))
                THROW("Cannot read secondary header in FG44 chunk");
              color_fg = ! (secondary.major & 0x80);
              FG44.put_chunk(color_fg ? "FORM:PM44" : "FORM:BM44");
            }
          temp.seek(0);
          FG44.put_chunk(color_fg ? "PM44" : "BM44");
          FG44.copy(temp);
          FG44.close_chunk();
        }
      else if (chkid == "BG44")
        {
          MemoryByteStream temp;
          temp.copy(iff);
          temp.seek(0);
          if (temp.readall((void*)&primary, sizeof(primary))<sizeof(primary))
            THROW("Cannot read primary header in BG44 chunk");
          if (primary.serial == 0)
            {
              if (temp.readall((void*)&secondary, sizeof(secondary))<sizeof(secondary))
                THROW("Cannot read secondary header in BG44 chunk");
              color_bg = ! (secondary.major & 0x80);
              BG44.put_chunk(color_bg ? "FORM:PM44" : "FORM:BM44");
            }
          temp.seek(0);
          BG44.put_chunk(color_bg ? "PM44" : "BM44");
          BG44.copy(temp);
          BG44.close_chunk();
        }
      else
        {
          fprintf(stderr, "  unrecognized chunk %s\n", (const char*)chkid);
        }
      iff.close_chunk();
    }
}

void 
usage()
{
  fprintf(stderr, 
          "DJVUEXTRACT -- Extracts components of a DJVU file\n"
          "%s\n"
          "Usage: djvuextract <djvufile> [Sjbz=file] [BG44=file] [FG44=file]\n",
          ATTLicense::get_usage_text());
  exit(1);
}


int
main(int argc, char **argv)
{
  TRY
    {
      ATTLicense::process_cmdline(argc,argv);
      if (argc<2)
        usage();
      MemoryByteStream Sjbz;
      MemoryByteStream BG44;
      MemoryByteStream FG44;
      djvuextract(argv[1], &Sjbz, &BG44, &FG44);
      for (int i=2; i<argc; i++)
        {
          Sjbz.seek(0);
          BG44.seek(0);
          FG44.seek(0);
          if (! strncmp(argv[i],"Sjbz=",5))
            {
              if (Sjbz.size()==0)
                THROW("No chunk Sjbz in this DJVU file");
              StdioByteStream obs(argv[i]+5,"wb");
              obs.copy(Sjbz);
            }
          else if (! strncmp(argv[i],"BG44=",5))
            {
              if (BG44.size()==0)
                THROW("No chunk BG44 in this DJVU file");
              StdioByteStream obs(argv[i]+5,"wb");
              obs.copy(BG44);
            }
          else if (! strncmp(argv[i],"FG44=",5))
            {
              if (FG44.size()==0)
                THROW("No chunk FG44 in this DJVU file");
              StdioByteStream obs(argv[i]+5,"wb");
              obs.copy(FG44);
            }
          else
            usage();
        }
    }
  CATCH(ex)
    {
      ex.perror();
      exit(1);
    }
  ENDCATCH;
  return 0;
}
