//C-  -*- C++ -*-
//C-
//C-  Copyright (c) 1998 AT&T	
//C-  All Rights Reserved 
//C-
//C-  THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
//C-  The copyright notice above does not evidence any
//C-  actual or intended publication of such source code.
//C-
//C-  $Id: djvumake.cpp,v 1.5 1999-03-01 17:02:45 leonb Exp $

/** @name djvumake

    {\bf Synopsis}
    \begin{verbatim}
       % djvumake <djvufile> [Sjbz=<jb2file>] [FG44=<iw4file>] [BG44=<iw4file>]
    \end{verbatim}

    {\bf Recipe for creating a Color DjVu File}\\
    You should first use program \Ref{c44} and produce an IW44 file "my.iw4".
    Assuming that this image is 640 pixels wide and 480 pixels high, you can
    assemble file #"my.djvu"# using #djvumake# with the following arguments.
    \begin{verbatim}
       % djvumake my.djvu  INFO=640,480  BG44=my.iw4
    \end{verbatim}
    
    {\bf Recipe for creating a Bilevel DjVu File}\\
    The first step consists in creating a \Ref{JB2Image} object according to
    the guidelines specified in section \Ref{JB2Image.h}.  Then use function
    #JB2Image::encode# to save the JB2 data into a file named #"myjb2.q"# for
    instance.  You can then assemble file #"my.djvu"# using #djvumake#
    with the following arguments:
    \begin{verbatim}
       % djvumake my.djvu Sjbz=myjb2.q
    \end{verbatim}
    
    {\bf Recipe for creating a Compound DjVu File}\\
    Let us assume that you use a program like Gimp \URL{http://www.gimp.org}
    or Photoshop.  You have created your image using two layers.  The
    background layer contains all pictures and background details.  The
    foreground layer contains the text and the drawings. Transparency is
    controlled by a layer mask attached to the foreground layer.  The layer
    mask contains the shape of the text and drawings in black and white.  The
    actual foreground layer contains large patches of color which are only
    displayed where the layer mask is black.  You can see a Gimp example in
    file #"@Samples/layers.xcf.gz"#.

    This layered model is very close to the Compound DjVu Image model.  In the
    DjVu model however, the three images (the background layer, the foreground
    layer mask, and the actual foreground layer) can have different
    resolutions.
    \begin{itemize}
    \item
    The size of the foreground layer mask is always equal to the size of the
    DjVu image.  You must create a JB2 file containing the foreground mask as
    explained in the {\em Recipe for Creating a Bilevel DjVu File}. Each zero
    pixel in the mask means that the corresponding pixel in the raw image
    belongs to the background. Each non zero pixel means that the
    corresponding pixel in the raw image belongs to the foreground. Let us
    call this file #"myjb2.q"#.
    \item
    The size of the background image is computed by rounding up the ratio
    between the size of the mask and an integer background sub-sampling ratio
    in range 1 to 12.  Choosing a sub-sampling ratio of 3 is usually a good
    starting point.  You must then subsample the background layer image and save
    it into a PPM file named #"mybg.ppm"#.  
    \item
    The size of the foreground color image is computed by rounding up the
    ratio between the size of the mask and an integer background sub-sampling
    ratio in range 1 to 12.  Choosing a sub-sampling ratio of 12 is usually
    adequate.  You must then subsample the background layer image and save
    it into a PPM file named #"myfg.ppm"#.  
    \end{itemize}
    
    When you subsample these images, you should consider some refinements.
    The color of each pixel of the subsampled image is an average of the
    colors of a couple of pixels in the original image.  When you compute this
    average, you eliminate the original pixels which are not visible, such as
    pixels of the background layer which are masked by the foreground, or
    pixels of the foreground color layer which are not visible because of the
    mask transparency.
    
    It sometimes happens that you cannot compute the color of a pixel in the
    subsampled image because none of the pixels in the corresponding image are
    visible.  That means that we do not really care abou the color of the
    subsampled pixel because it is not visible at all.  It is not desirable of
    course to encode the color value of such pixels.  This is possible using
    the {\em masking} feature of the wavelet encoder.  You must first save two
    PBM images named #"mybg.pbm"# and #"myfg.pbm"#.  These images have the
    same size as the corresponding PPM images.  A black pixel in these images
    mean that we should not code the color of the corresponding pixel in the
    PPM image.

    We must then encode both images using the \Ref{c44} wavelet encoder.
    The following commands to the trick:
    \begin{verbatim}
      % c44 -slice 74+10+9+4 -mask mybg.msk mybg.ppm mybg.iw4 
      % c44 -slice 100 -crcbfull -mask myfg.msk myfg.ppm myfg.iw4
    \end{verbatim}
    Note that we use different options. The background wavelet file
    #"mybg.iw4"# contains four refinement chunks specified by option #-slice#.
    The foreground wavelet file #"myfg.iw4"# contains a single chunk (option
    #-slice#) and allocated more bits for encoding the colors (option
    #-crcbfull#). 

    The last step consists of assembling the DjVu file using #djvumake#.
    \begin{verbatim}
       djvumake my.djvu Sjbz=myjb2.q FG44=myfg.iw4 BG44=mybg.iw4
    \end{verbatim}

    @memo
    Create DjVu files.
    @version
    #$Id: djvumake.cpp,v 1.5 1999-03-01 17:02:45 leonb Exp $#
    @author
    Leon Bottou <leonb@research.att.com> */
//@{
//@}

#include <stdio.h>
#include <stdlib.h>
#include "GString.h"
#include "GException.h"
#include "DjVuImage.h"
#include "ATTLicense.h"

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
  printf("DJVUMAKE -- Create a DjVu file\n"
         "%s\n"
         "Usage: djvumake djvufile ...arguments...\n"
         "\n"
         "The arguments describe the successive chunks of the DJVU file.\n"
         "Possible arguments are:\n"
         "   INFO=w,h                    --  Create the initial information chunk\n"
         "   Sjbz=jb2file                --  Create a JB2 stencil chunk\n"
         "   FG44=iw4file                --  Create an IW44 foreground chunk\n"
         "   BG44=[iw4file][:nchunks]    --  Create one or more IW44 background chunks\n"
         "\n"
         "You may omit the specification of the information chunk. An information\n"
         "chunk will be created using the image size of the first stencil chunk\n"
         "This program tries to issue a warning when you are building an\n"
         "incorrect djvu file.\n"
         "\n", 
         ATTLicense::get_usage_text());
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
  flag_contains_fg = red;
  if (red>12)
    fprintf(stderr, "djvumake: FG44 subsampling is not in [1..12] range\n");
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
          flag_contains_bg = red;
          if (red>12)
            fprintf(stderr, "djvumake: BG44 subsampling is not in [1..12] range\n");
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
      ATTLicense::process_cmdline(argc,argv);
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
              if (flag_contains_fg)
                fprintf(stderr,"djvumake: duplicate 'FG44' chunk\n");
              create_fg44_chunk(iff, "FG44", argv[i]+5);
            }
          else if (! strncmp(argv[i],"BG44=",5))
            {
              create_bg44_chunk(iff, "BG44", argv[i]+5);
            }
          else
            {
              fprintf(stderr,"djvumake: illegal argument %d (ignored) : %s\n", i, argv[i]);
            }
        }
      // Close
      iff.close_chunk();
      // Sanity checks
      if (flag_contains_stencil)
        {
          if (flag_contains_bg && ! flag_contains_fg)
            fprintf(stderr,"djvumake: djvu file contains a BG44 chunk but no FG44 chunk\n");
          if (flag_contains_fg && ! flag_contains_bg)
            fprintf(stderr,"djvumake: djvu file contains a FG44 chunk but no BG44 chunk\n");
        }
      else if (flag_contains_bg)
        {
          // Color DjVu Image
          if (flag_contains_bg!=1)
            fprintf(stderr,"djvumake: color djvu image has subsampled BG44 chunk\n"); 
          if (flag_contains_fg)
            fprintf(stderr,"djvumake: color djvu file contains FG44 chunk\n");            
        }
      else
        fprintf(stderr,"djvumake: djvu file contains no Sjbz or BG44 chunk\n");
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
