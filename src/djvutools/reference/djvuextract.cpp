//C-  -*- C++ -*-
//C- DjVu� Reference Library (v. 3.0)
//C- 
//C- Copyright � 1999-2001 LizardTech, Inc. All Rights Reserved.
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
// $Id: djvuextract.cpp,v 1.17 2001-04-21 00:16:57 bcr Exp $
// $Name:  $

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
    #$Id: djvuextract.cpp,v 1.17 2001-04-21 00:16:57 bcr Exp $#
    @author
    L\'eon Bottou <leonb@research.att.com> - Initial implementation\\
    Andrei Erofeev <eaf@geocities.com> - Multipage support */
//@{
//@}

#include "GException.h"
#include "ByteStream.h"
#include "IFFByteStream.h"
#include "DjVuDocument.h"
#include "DjVuFile.h"
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
display_info_chunk(GP<ByteStream> ibs, const GURL &url)
{
  ibs->seek(0);
  GP<IFFByteStream> giff=IFFByteStream::create(ibs);
  IFFByteStream &iff=*giff;
  GUTF8String chkid;
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
          DjVuPrintError("%s: (%d x %d) version %d\n", 
                  (const char *)url, 
                  (djvuinfo.width_hi<<8)+djvuinfo.width_lo, 
                  (djvuinfo.height_hi<<8)+djvuinfo.height_lo,
                  djvuinfo.version );
        }
      iff.close_chunk();
    }
}


static void
extract_chunk(GP<ByteStream> ibs, const GUTF8String &id, GP<ByteStream> out)
{
  ibs->seek(0);
  GP<IFFByteStream> giff=IFFByteStream::create(ibs);
  IFFByteStream &iff=*giff;
  GUTF8String chkid;
  if (! iff.get_chunk(chkid))
    G_THROW("Malformed DJVU file");
  if (chkid != "FORM:DJVU")
    G_THROW("This is not a layered DJVU file");
  

  // Special case for FG44 and BG44
  if (id == "BG44" || id == "FG44")
    {
      // Rebuild IW44 file
      GP<IFFByteStream> giffout=IFFByteStream::create(out);
      IFFByteStream &iffout=*giffout;
      int color_bg = -1;
      while (iff.get_chunk(chkid))
        {
          if (chkid == id)
            {
              GP<ByteStream> gtemp=ByteStream::create();
              ByteStream &temp=*gtemp;
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
            out->copy(iff);
          iff.close_chunk();
        }
    }
}


void 
usage()
{
  DjVuPrintError("%s",
          "DJVUEXTRACT -- Extracts components of a DJVU file\n"
          "  Copyright © 1999-2000 LizardTech, Inc. All Rights Reserved.\n"
          "Usage:\n"
	  "   djvuextract <djvufile> [-page=<num>] {...<chunkid>=<file>...} \n");
  exit(1);
}


int
main(int argc, char **argv)
{
  DArray<GUTF8String> dargv(0,argc-1);
  for(int i=0;i<argc;++i)
  {
    GUTF8String g(argv[i]);
    dargv[i]=g.getNative2UTF8();
  }
  G_TRY
    {
      int i;

      // Process page number
      int page_num=0;
      for(i=1;i<argc;i++)
	 if (dargv[i].ncmp("-page=", 6))
           {
              page_num = dargv[i].substr(6,dargv[i].length()).toInt() - 1; // atoi(6+(const char *)dargv[i]) - 1;
             for(int j=i;j<argc-1;j++) 
               dargv[j]=dargv[j+1];
             argc--;
             break;
           } 
      if (page_num<0)
        {
          DjVuPrintError("%s", "Invalid page number\n");
          usage();
        }
      
      // Check that chunk names are legal
      if (argc<=2)
        usage();
      for (i=2; i<argc; i++)
        if (IFFByteStream::check_id(dargv[i]) || dargv[i][4]!='=' || dargv[i][5]==0)
          usage();

      // Decode
      const GURL::Filename::UTF8 url1(dargv[1]);
      GP<DjVuDocument> doc=DjVuDocument::create_wait(url1);
      if (! doc->wait_for_complete_init())
        G_THROW("Decoding failed. Nothing can be done.");        
      GP<DjVuFile> file=doc->get_djvu_file(page_num);
      GP<ByteStream> pibs = file->get_djvu_bytestream(false, false);
      // Search info chunk
      display_info_chunk(pibs, url1);
      // Extract required chunks
      for (i=2; i<argc; i++)
        {
          GP<ByteStream> gmbs=ByteStream::create();
          dargv[i].setat(4,0);
          extract_chunk(pibs, dargv[i], gmbs);
          ByteStream &mbs=*gmbs;
          if (mbs.size() == 0)
            {
              DjVuPrintError("  %s --> not found!\n", (const char *)dargv[i]);
            }
          else
            {
              const GURL::Filename::UTF8 url(5+(const char *)dargv[i]);
              GP<ByteStream> obs=ByteStream::create(url,"wb");
              mbs.seek(0);
              obs->copy(mbs);
              DjVuPrintError("  %s --> \"%s\" (%d bytes)\n", 
                      (const char *)dargv[i], (const char *)dargv[i]+5, mbs.size());
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
