//C-  -*- C++ -*-
//C-
//C-  Copyright (c) 1998 AT&T	
//C-  All Rights Reserved 
//C-
//C-  THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
//C-  The copyright notice above does not evidence any
//C-  actual or intended publication of such source code.
//C-
//C-  $Id: djvumake.cpp,v 1.2 1999-02-15 23:23:37 leonb Exp $



/** @name djvumake

    {\bf Synopsis}
    \begin{verbatim}
       djvumake <djvufile> <chunk-specification>
    \end{verbatim}
    
    {\bf Recipe for Creating a Bilevel DjVu File}\\
    The first step consists in creating a \Ref{JB2Image} object according to
    the guidelines specified in section \Ref{JB2Image.h}.  Then use function
    #JB2Image::encode# to save the JB2 data into a file named #"myjb2.q"# for
    instance.  You can then assemble file #"my.djvu"# using #djvumake#
    with the following arguments:
    \begin{verbatim}
       djvumake my.djvu Sjbz=myjb2.q
    \end{verbatim}
    
    {\bf Recipe for Creating a Color DjVu File}\\
    Let's assume that you already have decided what is going into the
    background layer and what is going into the foreground layer.  Remember
    that the size of the background image is computed by rounding up the ratio
    between the size of the mask and an integer background sub-sampling ratio
    in range 1 to 12.  Choosing a sub-sampling ratio of 3 is usually a good
    starting point.  Save this background image into a PPM file named
    #"mybg.ppm"# and encode it using \Ref{c44}:
    \begin{verbatim} 
       c44 -slice 74+10+9+4 mybg.ppm  mybg.iw4 
    \end{verbatim}
    You must also create a JB2 file containing the foreground mask as
    explained in the {\em Recipe for Creating a Bilevel DjVu File}. Each zero
    pixel in the mask means that the corresponding pixel in the raw image
    belongs to the background. Each non zero pixel means that the
    corresponding pixel in the raw image belongs to the foreground. Let us
    call this file #"myjb2.q"#.

    You must then prepare the foreground color image.  Remember that the size
    of the foreground color image is computed by rounding up the ratio between
    the size of the mask and an integer background sub-sampling ratio in range
    1 to 12.  Choosing a sub-sampling ratio of 12 is usually adequate.  First
    create a PPM image #"myfg.ppm"# and a PBM image mask #"myfgmask.pbm"#.
    Both the PPM image and the PBM image mask have the same size as the
    foreground color image.  For each pixel in the PPM image and the PBM image
    mask, locate the corresponding foreground pixels in the raw image.  If
    there are at least two such pixels, store their average into the PPM image
    and store a zero into the PBM image mask.  Otherwise, store an arbitrary
    color into the PPM image and store a one into the PBM image mask.  Then
    encode the foreground color image using \Ref{c44} as a single chunk and
    using the masking option:
    \begin{verbatim} 
       c44 -slice 100 -crcbfull -mask myfgmask.pbm myfg.ppm  myfg.iw4
    \end{verbatim}
    Finally assemble the DjVu file using the following command:
    \begin{verbatim}
       djvumake my.djvu Sjbz=myjb2.q FG44=myfg.iw4 BG44=mybg.iw4
    \end{verbatim}

    {\bf Creating an IW44 File}\\
    You do not need program #djvumake# to create an IW44 File.  
    Program \Ref{c44} already produces compliant IW44 files.

    @memo
    Create Bilevel DjVu files or Color DjVu files.
    @version
    #$Id: djvumake.cpp,v 1.2 1999-02-15 23:23:37 leonb Exp $#
    @author
    Leon Bottou <leonb@research.att.com> */
//@{
//@}

#include <stdio.h>
#include <stdlib.h>
#include "GString.h"
#include "GException.h"
#include "DjVuImage.h"

#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

int flag_contains_fg      = 0;
int flag_contains_bg      = 0;
int flag_contains_stencil = 0;
int flag_contains_bg44    = 0;

IFFByteStream *bg44iff    = 0;
MemoryByteStream *jb2stencil = 0;

int w;
int h;


void 
usage()
{
  printf("djvumake -- Create a DjVu file\n"
         "Usage: djvumake djvufile ...arguments...\n"
         "\n"
         "The arguments describe the successive chunks of the DJVU file.\n"
         "Possible arguments are:\n"
         "   INFO=w,h                    --  Create the initial information chunk\n"
         "   Sjbz=jb2file                --  Create a JB2 stencil chunk\n"
         "   FG44=iw4file                --  Create an IW44 foreground chunk\n"
         "   BG44=[iw4file][:nchunks]    --  Create one or more IW44 background chunks\n"
         "\n"
         "* You may omit the specification of the information chunk. An information\n"
         "  chunk will be created using the image size of the first stencil chunk\n"
         "* Although this program tries to issue a warning when you are building an\n"
         "  incorrect djvu file. There is no guarantee that these warnings flag\n"
         "  all conditions.\n"
         "\n");
  exit(1);
}



void
create_info_chunk(IFFByteStream &iff, int argc, char **argv)
{
  if (argc>2 && !strncmp(argv[2],"INFO=",5))
    {
      // process info specification
      char *ptr = argv[2]+5;
      // size
      w = strtol(ptr, &ptr, 10);
      if (w<=0 || w>=16384)
        THROW("djvumake: incorrect width in 'INFO' chunk specification\n");
      if (*ptr++ != ',')
        THROW("djvumake: comma expected in 'INFO' chunk specification (before height)\n");
      h = strtol(ptr, &ptr, 10);      
      if (h<=0 || h>=16384)
        THROW("djvumake: incorrect height in 'INFO' chunk specification\n");
      // rest
      if (*ptr)
        THROW("djvumake: syntax error in 'INFO' chunk specification\n");
    }
  else
    {
      // search stencil chunk
      for (int i=2; i<argc; i++)
        if (!strncmp(argv[i],"Sjbz=",5))
          {
            JB2Image image;
            StdioByteStream bs(argv[i]+5,"rb");
            jb2stencil = new MemoryByteStream();
            jb2stencil->copy(bs);
            jb2stencil->seek(0);
            image.decode(*jb2stencil);
            w = image.get_width();
            h = image.get_height();
            jb2stencil->seek(0);
            break;
          }
    }
  // warn
  if (w==0 || h==0)
    fprintf(stderr,"djvumake: cannot determine image size\n");
  // write info chunk
  DjVuInfo info;
  info.width = w;
  info.height = h;
  iff.put_chunk("INFO");
  info.encode(iff);
  iff.close_chunk();
}


void 
create_jb2_chunk(IFFByteStream &iff, char *chkid, char *filename)
{
  if (!jb2stencil)
    {
      StdioByteStream bs(filename,"rb");
      JB2Image image;
      jb2stencil = new MemoryByteStream();
      jb2stencil->copy(bs);
      jb2stencil->seek(0);
      image.decode(*jb2stencil);
      int jw = image.get_width();
      int jh = image.get_height();
      jb2stencil->seek(0);
      if (jw!=w || jh!=h)
        fprintf(stderr,"djvumake: stencil size (%s) does not match info size\n", filename);
    }
  jb2stencil->seek(0);
  iff.put_chunk(chkid);
  iff.copy(*jb2stencil);
  iff.close_chunk();
  delete jb2stencil;
  jb2stencil = 0;
}




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
create_fg44_chunk(IFFByteStream &iff, char *ckid, char *filename)
{
  StdioByteStream bs(filename,"rb");
  IFFByteStream bsi(bs);
  GString chkid;
  bsi.get_chunk(chkid);
  if (chkid != "FORM:PM44" && chkid != "FORM:BM44")
    THROW("djvumake: FG44 file has incorrect format (wrong IFF header)");
  bsi.get_chunk(chkid);
  if (chkid!="PM44" && chkid!="BM44")
    THROW("djvumake: FG44 file has incorrect format (wring IFF header)");
  MemoryByteStream mbs;
  mbs.copy(bsi);
  bsi.close_chunk();  
  if (bsi.get_chunk(chkid))
    fprintf(stderr,"djvumake: FG44 file contains more than one chunk\n");
  bsi.close_chunk();  
  mbs.seek(0);
  if (mbs.readall((void*)&primary, sizeof(primary)) != sizeof(primary))
    THROW("djvumake: FG44 file is corrupted (cannot read primary header)");    
  if (primary.serial != 0)
    THROW("djvumake: FG44 file is corrupted (wrong serial number)");
  if (mbs.readall((void*)&secondary, sizeof(secondary)) != sizeof(secondary))
    THROW("djvumake: FG44 file is corrupted (cannot read secondary header)");    
  int iw = (secondary.xhi<<8) + secondary.xlo;
  int ih = (secondary.yhi<<8) + secondary.ylo;
  int red;
  for (red=1; red<=12; red++)
    if (iw==(w+red-1)/red && ih==(h+red-1)/red)
      break;
  if (red>12)
    fprintf(stderr, "djvumake: FG44 reduction is not in [1..12] range\n");
  mbs.seek(0);
  iff.put_chunk(ckid);
  iff.copy(mbs);
  iff.close_chunk();
}



void 
create_bg44_chunk(IFFByteStream &iff, char *ckid, char *filespec)
{
  if (! bg44iff)
    {
      char *s = strchr(filespec, ':');
      if (s == filespec)
        THROW("djvumake: no filename specified in first BG44 specification");
      if (!s)
        s = filespec + strlen(filespec);
      GString filename(filespec, s-filespec);
      ByteStream *pbs = new StdioByteStream(filename,"rb");
      bg44iff = new IFFByteStream(*pbs);
      GString chkid;
      bg44iff->get_chunk(chkid);
      if (chkid != "FORM:PM44" && chkid != "FORM:BM44")
        THROW("djvumake: BG44 file has incorrect format (wrong IFF header)");        
      if (*s == ':')
        filespec = s+1;
      else 
        filespec = "99";
    }
  else
    {
      if (*filespec!=':')
        THROW("djvumake: filename specified in BG44 refinement");
      filespec += 1;
    }
  int nchunks = strtol(filespec, &filespec, 10);
  if (nchunks<1 || nchunks>99)
    THROW("djvumake: invalid number of chunks in BG44 specification");    
  if (*filespec)
    THROW("djvumake: invalid BG44 specification (syntax error)");
  
  int flag = (nchunks>=99);
  GString chkid;
  while (nchunks-->0 && bg44iff->get_chunk(chkid))
    {
      if (chkid!="PM44" && chkid!="BM44")
        {
          fprintf(stderr,"djvumake: BG44 file contains unrecognized chunks (ignored)\n");
          nchunks += 1;
          bg44iff->close_chunk();
          continue;
        }
      MemoryByteStream mbs;
      mbs.copy(*bg44iff);
      bg44iff->close_chunk();  
      mbs.seek(0);
      if (mbs.readall((void*)&primary, sizeof(primary)) != sizeof(primary))
        THROW("djvumake: BG44 file is corrupted (cannot read primary header)\n");    
      if (primary.serial == 0)
        {
          if (mbs.readall((void*)&secondary, sizeof(secondary)) != sizeof(secondary))
            THROW("djvumake: BG44 file is corrupted (cannot read secondary header)\n");    
          int iw = (secondary.xhi<<8) + secondary.xlo;
          int ih = (secondary.yhi<<8) + secondary.ylo;
          int red;
          for (red=1; red<=12; red++)
            if (iw==(w+red-1)/red && ih==(h+red-1)/red)
              break;
          if (red>12)
            fprintf(stderr, "djvumake: BG44 reduction is not in [1..12] range\n");
        }
      mbs.seek(0);
      iff.put_chunk(ckid);
      iff.copy(mbs);
      iff.close_chunk();
      flag = 1;
    }
  if (!flag)
    fprintf(stderr,"djvumake: no more chunks in BG44 file\n");
}



int
main(int argc, char **argv)
{
  TRY
    {
      // Print usage when called without enough arguments
      if (argc <= 2)
        usage();
      // Open djvu file
      remove(argv[1]);
      StdioByteStream obs(argv[1],"wb");
      IFFByteStream iff(obs);
      // Create header
      iff.put_chunk("FORM:DJVU", 1);
      // Create information chunk
      create_info_chunk(iff, argc, argv);
      // Parse all arguments
      for (int i=2; i<argc; i++)
        {
          if (! strncmp(argv[i],"INFO=",5))
            {
              if (i>2)
                fprintf(stderr,"djvumake: 'INFO' chunk should appear first (ignored)\n");
            }
          else if (! strncmp(argv[i],"Sjbz=",5))
            {
              create_jb2_chunk(iff, "Sjbz", argv[i]+5);
              if (flag_contains_stencil)
                fprintf(stderr,"djvumake: duplicate 'Sjbz' chunk\n");
              flag_contains_stencil = 1;
            }
          else if (! strncmp(argv[i],"FG44=",5))
            {
              create_fg44_chunk(iff, "FG44", argv[i]+5);
              if (flag_contains_fg)
                fprintf(stderr,"djvumake: duplicate 'FG44' chunk\n");
              flag_contains_fg = 1;
            }
          else if (! strncmp(argv[i],"BG44=",5))
            {
              create_bg44_chunk(iff, "BG44", argv[i]+5);
              flag_contains_bg = 1;
            }
          else
            {
              fprintf(stderr,"djvumake: illegal argument %d (ignored) : %s\n", i, argv[i]);
            }
        }
      // Close
      iff.close_chunk();
      // Sanity checks
      if (! flag_contains_stencil)
        fprintf(stderr,"djvumake: djvu file contains no 'Sjbz' chunk\n");
      if (flag_contains_bg && !flag_contains_fg)
        fprintf(stderr,"djvumake: djvu file contains a 'BG44' chunk but no 'FG44' chunk\n");
    }
  CATCH(ex)
    {
      remove(argv[1]);
      ex.perror("Type 'djvumake' without arguments for more help");
      exit(1);
    }
  ENDCATCH;
  return 0;
}
