//C-  -*- C++ -*-
//C-
//C- Copyright (c) 1999 AT&T Corp.
//C- Copyright (c) 2000 LizardTech Inc.
//C- All rights reserved.
//C-
//C- This software may only be used by you under license from LizardTech
//C- Inc. A copy of LizardTech's Source Code Agreement is available at
//C- LizardTech's Internet website having the URL <http://www.djvu.com/open>.
//C- If you received this software without first entering into a license with
//C- LizardTech, you have an infringing copy of this software and cannot use it
//C- without violating LizardTech's intellectual property rights.
//C-
//C- $Id: djvuextract.cpp,v 1.1 2000-09-21 21:14:00 mrosen Exp $

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
      start from #1#.
    \end{itemize}
    This commands also supports #"Smmr"# chunks for G4/MMR encoded masks,
    #"FGjp"# and #"BGjp"# for JPEG encoded color layers, and finally #"FG2k"#
    and #"BG2k"# for JPEG-2000 encoded color layers.

    @memo
    Extract components from DjVu files.
    @version
    #$Id: djvuextract.cpp,v 1.1 2000-09-21 21:14:00 mrosen Exp $#
    @author
    L\'eon Bottou <leonb@research.att.com> - Initial implementation\\
    Andrei Erofeev <eaf@geocities.com> - Multipage support */
//@{
//@}

#include "GException.h"
#include "ByteStream.h"
#include "IFFByteStream.h"
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
display_info_chunk(ByteStream& ibs, const char *filename)
{
  ibs.seek(0);
  IFFByteStream iff(ibs);
  GString chkid;
  if (! iff.get_chunk(chkid))
    G_THROW("Malformed DJVU file");
  if (chkid != "FORM:DJVU")
    G_THROW("This is not a layered DJVU file");
  // Search info chunk
  while (iff.get_chunk(chkid))
    {
      if (chkid=="INFO")
        {
          if (iff.readall((void*)&djvuinfo,sizeof(djvuinfo)) < sizeof(djvuinfo))
            G_THROW("Cannot read INFO chunk");
          fprintf(stderr, "%s: (%d x %d) version %d\n", 
                  filename, 
                  (djvuinfo.width_hi<<8)+djvuinfo.width_lo, 
                  (djvuinfo.height_hi<<8)+djvuinfo.height_lo,
                  djvuinfo.version );
        }
      iff.close_chunk();
    }
}


void
extract_chunk(ByteStream& ibs, const char *id, ByteStream &out)
{
  ibs.seek(0);
  IFFByteStream iff(ibs);
  GString chkid;
  if (! iff.get_chunk(chkid))
    G_THROW("Malformed DJVU file");
  if (chkid != "FORM:DJVU")
    G_THROW("This is not a layered DJVU file");
  

  // Special case for FG44 and BG44
  if (!strcmp(id,"BG44") || !strcmp(id,"FG44"))
    {
      // Rebuild IW44 file
      IFFByteStream iffout(out);
      int color_bg = -1;
      while (iff.get_chunk(chkid))
        {
          if (chkid == id)
            {
              MemoryByteStream temp;
              temp.copy(iff);
              temp.seek(0);
              if (temp.readall((void*)&primary, sizeof(primary))<sizeof(primary))
                G_THROW("Cannot read primary header in BG44 chunk");
              if (primary.serial == 0)
                {
                  if (temp.readall((void*)&secondary, sizeof(secondary))<sizeof(secondary))
                    G_THROW("Cannot read secondary header in BG44 chunk");
                  color_bg = ! (secondary.major & 0x80);
                  iffout.put_chunk(color_bg ? "FORM:PM44" : "FORM:BM44");
                }
              if (color_bg < 0)
                G_THROW("IW44 chunks are not in proper order");
              temp.seek(0);
              iffout.put_chunk(color_bg ? "PM44" : "BM44");
              iffout.copy(temp);
              iffout.close_chunk();
            }
          iff.close_chunk();
        }
    }
  else
    {
      // Just concatenates all matching chunks
      while (iff.get_chunk(chkid))
        {
          if (chkid == id)
            out.copy(iff);
          iff.close_chunk();
        }
    }
}


void 
usage()
{
  fprintf(stderr, 
          "DJVUEXTRACT -- Extracts components of a DJVU file\n"
          "  Copyright (c) AT&T 1999.  All rights reserved\n"
          "Usage:\n"
	  "   djvuextract <djvufile> [-page=<num>] {...<chunkid>=<file>...} \n");
  exit(1);
}


int
main(int argc, char **argv)
{
  G_TRY
    {
      int i;

      // Process page number
      int page_num=0;
      for(i=1;i<argc;i++)
	 if (!strncmp(argv[i], "-page=", 6))
           {
             page_num = atoi(argv[i]+6) - 1;
             for(int j=i;j<argc-1;j++) 
               argv[j]=argv[j+1];
             argc--;
             break;
           } 
      if (page_num<0)
        {
          fprintf(stderr, "Invalid page number\n");
          usage();
        }
      
      // Check that chunk names are legal
      if (argc<=2)
        usage();
      for (i=2; i<argc; i++)
        if (IFFByteStream::check_id(argv[i]) || argv[i][4]!='=' || argv[i][5]==0)
          usage();

      // Decode
      GP<DjVuDocument> doc=new DjVuDocument;
      doc->init(GOS::filename_to_url(argv[1]));
      if (! doc->wait_for_complete_init())
        G_THROW("Decoding failed. Nothing can be done.");        
      GP<DjVuFile> file=doc->get_djvu_file(page_num);
      GP<ByteStream> pibs = file->get_djvu_bytestream(false, false);
      // Search info chunk
      display_info_chunk(*pibs, argv[1]);
      // Extract required chunks
      for (i=2; i<argc; i++)
        {
          MemoryByteStream mbs;
          argv[i][4] = 0;
          extract_chunk(*pibs, argv[i], mbs);
          if (mbs.size() == 0)
            {
              fprintf(stderr, "  %s --> not found!\n", argv[i]);
            }
          else
            {
              StdioByteStream obs(argv[i]+5,"wb");
              mbs.seek(0);
              obs.copy(mbs);
              fprintf(stderr, "  %s --> \"%s\" (%d bytes)\n", 
                      argv[i], argv[i]+5, mbs.size());
            }
        }
    }
  G_CATCH(ex)
    {
      ex.perror();
      exit(1);
    }
  G_ENDCATCH;
  return 0;
}