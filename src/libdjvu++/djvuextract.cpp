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
//C- $Id: djvuextract.cpp,v 1.12 1999-06-04 15:55:17 leonb Exp $

/** @name djvuextract

    {\bf Synopsis}
    \begin{verbatim}
    djvuextract <djvufile> [-page=<page>] [Sjbz=<maskout>] [FG44=<fgout>] [BG44=<bgout>]
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
    \item Optionally one can provide a #-page# option to select a given
      page from the document, if it's a multipage document. The page numbers
      start from #1#. The exact behaviour of the program depends on the document
      type:
      \begin{itemize}
         \item {\bf Multipage all-in-one-file DjVu documents.}

	 If the option is specified, the desired page will be open. Otherwise
	 the first page.

	 \item {\bf Multipage each-page-in-separate-file DjVu documents.}

	 Here, if no #-page# option is given, the page corresponding to the
	 specified file will be considered. Otherwise the program will process
	 the given file in search of navigation directory, will learn the name
	 of the file containing the desired page, will open and interpret it.
      \end{itemize}
    \end{itemize}

    @memo
    Extract components from DjVu files.
    @version
    #$Id: djvuextract.cpp,v 1.12 1999-06-04 15:55:17 leonb Exp $#
    @author
    L\'eon Bottou <leonb@research.att.com> - Initial implementation\\
    Andrei Erofeev <eaf@geocities.com> - Multipage support */
//@{
//@}

#include "GException.h"
#include "ByteStream.h"
#include "IFFByteStream.h"
#include "ATTLicense.h"
#include "DjVuDocument.h"
#include "GOS.h"


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
djvuextract(const char *filename, int page_num,
	    MemoryByteStream *pSjbz,
	    MemoryByteStream *pBG44,
	    MemoryByteStream *pFG44)
  
{
  GP<DjVuDocument> doc=new DjVuDocument(GOS::filename_to_url(filename), 1);
  GP<DjVuFile> file=doc->get_djvu_file(page_num);
  TArray<char> data=file->get_djvu_data(0, 0);
  MemoryByteStream ibs(data, data.size());
  IFFByteStream iff(ibs);
  
  IFFByteStream BG44(*pBG44);
  IFFByteStream FG44(*pFG44); 
  int color_bg = 1;
  int color_fg = 1;
  
  GString chkid;
  if (! iff.get_chunk(chkid))
    THROW("Malformed DJVU file");
  if (chkid == "FORM:DJVM")
     THROW("This is multipage DJVU file. Please break it into pieces.");
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
      else if (chkid!="ANTa" && chkid!="INCL" &&
	       chkid!="INCD" && chkid!="NDIR")
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
          "Usage:\n"
	  "   djvuextract <djvufile> [-page=<page_num>] [Sjbz=file] \\\n"
	  "               [BG44=file] [FG44=file]\n",
          ATTLicense::get_usage_text());
  exit(1);
}


int
main(int argc, char **argv)
{
  TRY
    {
      ATTLicense::process_cmdline(argc,argv);
      int page_num=0;
      for(int i=1;i<argc;i++)
	 if (!strncmp(argv[i], "-page=", 6))
	 {
	    page_num=atoi(argv[i]+6);
	    if (page_num<=0)
	    {
	       fprintf(stderr, "Invalid page number '%s' specified\n\n", argv[i]+6);
	       usage();
	    }
	    for(int j=i;j<argc-1;j++) argv[j]=argv[j+1];
	    argc--;
	    break;
	 } else if (!strcmp(argv[i], "-page"))
	 {
	    if (i+1>=argc)
	    {
	       fprintf(stderr, "Option '-page' must be followed by a number\n\n");
	       usage();
	    }
	    page_num=atoi(argv[i+1]);
	    if (page_num<=0)
	    {
	       fprintf(stderr, "Invalid page number '%s' specified\n\n", argv[i+1]);
	       usage();
	    }
	    for(int j=i;j<argc-2;j++) argv[j]=argv[j+2];
	    argc-=2;
	    break;
	 }
      
      if (argc<=2)
        usage();
      MemoryByteStream Sjbz;
      MemoryByteStream BG44;
      MemoryByteStream FG44;
      djvuextract(argv[1], page_num-1, &Sjbz, &BG44, &FG44);
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
