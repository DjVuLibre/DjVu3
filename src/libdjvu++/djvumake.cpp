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
//C- $Id: djvumake.cpp,v 1.16 2000-02-28 23:23:51 haffner Exp $

/** @name djvumake

    {\bf Synopsis}
    \begin{verbatim}
       % djvumake <djvufile> [Sjbz=<maskfile>] [FG44=<fgfile>] [BG44=<bgfile>]
    \end{verbatim}
    This commands also supports #"Smmr"# chunks for G4/MMR encoded masks,
    #"FGjp"# and #"BGjp"# for JPEG encoded color layers, and finally #"FG2k"#
    and #"BG2k"# for JPEG-2000 encoded color layers.

    {\bf Description}

    This program assembles the DjVu file #djvufile# using the JB2 data
    contained in file #maskfile# and the IW44 data contained in files #bgfile#
    and #fgfile#.  This is useful for creating a DjVu image file from its 
    individual components.

    {\bf Recipe for creating a Photo DjVu File}

    You can simply use program \Ref{c44} and specify either option #-dpi# or
    #-gamma#.  Since these options are not supported by IW44 files, program
    \Ref{c44} will produce a Photo DjVu File instead of an IW44 file.
    Program #djvumake# however may be useful to convert an IW44 file into
    a Photo DjVu File. Assuming that you have an IW44 file with a 640x480 image,
    you can also generate Photo DjVu File using the following command:
    \begin{verbatim}
       % djvumake my.djvu INFO=640,480  BG44=my.iw4
    \end{verbatim}
    

    {\bf Recipe for creating a Bilevel DjVu File}

    You can simply use program \Ref{cjb2} which generates Bilevel DjVu Image
    files.  Program #djvumake# however can be useful to extract the mask from
    a Compound DjVu File (using #djvuextract#) and create a Bilevel DjVu Image
    (using #djvumake#).
    \begin{verbatim}
       % djvuextract mycompound.djvu Sjbz=myjb2.q
       % djvumake mybilevel.djvu Sjbz=myjb2.q
    \end{verbatim}

    
    {\bf Recipe for creating a Compound DjVu File}

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
    visible.  That means that we do not really care about the color of the
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
    Assemble DjVu files.
    @version
    #$Id: djvumake.cpp,v 1.16 2000-02-28 23:23:51 haffner Exp $#
    @author
    L\'eon Bottou <leonb@research.att.com> */
//@{
//@}

#include <stdio.h>
#include <stdlib.h>
#include "GString.h"
#include "GException.h"
#include "DjVuImage.h"
#include "MMRDecoder.h"

#include "GPixmap.h"
#include "GBitmap.h"
  static void 
  processForeground(const GPixmap* image, const JB2Image *mask,
                    GPixmap& subsampled_image, GBitmap& subsampled_mask);
  
  static void 
  processBackground(const GPixmap* image, const JB2Image *mask,
                    GPixmap& subsampled_image, GBitmap& subsampled_mask, int bg_sub);


  static void 
  maskedSubsample(const GPixmap *p_img,
                  const GBitmap *p_mask,
                  GPixmap& subsampled_image,
                  GBitmap& subsampled_mask,
                  int gridwidth, 
                  int inverted_mask = 0, 
                  int incr_flag = 0, 
                  int minpixels = 1 );
  

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
GP<MemoryByteStream> jb2stencil = 0;
GP<MemoryByteStream> mmrstencil = 0;
GP<JB2Image> stencil=0;

int w = -1;
int h = -1;


void 
usage()
{
  printf("DJVUMAKE -- Create a DjVu file\n"
          "  Copyright (c) AT&T 1999.  All rights reserved\n"
         "Usage: djvumake djvufile ...arguments...\n"
         "\n"
         "The arguments describe the successive chunks of the DJVU file.\n"
         "Possible arguments are:\n"
         "   INFO=w,h                    --  Create the initial information chunk\n"
         "   Sjbz=jb2file                --  Create a JB2 stencil chunk\n"
         "   FG44=iw4file                --  Create an IW44 foreground chunk\n"
         "   BG44=[iw4file][:nchunks]    --  Create one or more IW44 background chunks\n"
         "   ppm=ppmfile[:bg-subsample]  --  Using  jb2file, extracts the foreground and the background from raw ppmfile"
         "\n"
         "You may omit the specification of the information chunk. An information\n"
         "chunk will be created using the image size of the first stencil chunk\n"
         "This program tries to issue a warning when you are building an\n"
         "incorrect djvu file.\n"
         "\n");
  exit(1);
}


void
analyze_mmr_chunk(char *filename)
{
  if (!mmrstencil)
    {
      StdioByteStream bs(filename,"rb");
      mmrstencil = new MemoryByteStream();
      mmrstencil->copy(bs);
      mmrstencil->seek(0);
      int jw, jh, invert, strip;
      MMRDecoder::decode_header(*mmrstencil, jw, jh, invert,strip);
      if (w < 0) w = jw;
      if (h < 0) h = jh;
      if (jw!=w || jh!=h)
        fprintf(stderr,"djvumake: stencil size (%s) does not match info size\n", filename);
    }
}

void 
analyze_jb2_chunk(char *filename)
{
  if (!jb2stencil)
    {
      StdioByteStream bs(filename,"rb");
      stencil=new(JB2Image);
      jb2stencil = new MemoryByteStream();
      jb2stencil->copy(bs);
      jb2stencil->seek(0);
      stencil->decode(*jb2stencil);
      int jw = stencil->get_width();
      int jh = stencil->get_height();
      if (w < 0) w = jw;
      if (h < 0) h = jh;
      if (jw!=w || jh!=h)
        fprintf(stderr,"djvumake: stencil size (%s) does not match info size\n", filename);
    }
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
            analyze_jb2_chunk(argv[i]+5);
            break;
          }
      else if (!strncmp(argv[i],"Smmr=",5))
          {
            analyze_mmr_chunk(argv[i]+5);
            break;
          }
    }
  // warn
  if (w<0 || h<0)
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
create_raw_chunk(IFFByteStream &iff, char *chkid, char *filename)
{
  iff.put_chunk(chkid);
  StdioByteStream ibs(filename,"rb");
  iff.copy(ibs);
  iff.close_chunk();
}


void 
create_mmr_chunk(IFFByteStream &iff, char *chkid, char *filename)
{
  analyze_mmr_chunk(filename);
  mmrstencil->seek(0);
  iff.put_chunk(chkid);
  iff.copy(*mmrstencil);
  iff.close_chunk();
}

void 
create_jb2_chunk(IFFByteStream &iff, char *chkid, char *filename)
{
  analyze_jb2_chunk(filename);
  jb2stencil->seek(0);
  iff.put_chunk(chkid);
  iff.copy(*jb2stencil);
  iff.close_chunk();
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
      if (flag_contains_bg)
        fprintf(stderr,"djvumake: Duplicate BGxx chunk\n");
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

void masksub(IFFByteStream &iff, char *filespec)
{
  char *s = strchr(filespec, ':');
  if (s == filespec)
    THROW("djvumake: no filename specified in first ppm specification");
  if (!s)
    s = filespec + strlen(filespec);
  GString filename(filespec, s-filespec);
  int bg_sub=3;
  if (*s == ':')
    bg_sub = atol(s+1);

  if (!stencil)
    THROW("The use of a raw ppm image requires a stencil");
  StdioByteStream ibs(filename, "rb");
  GPixmap raw_pm;
  raw_pm.init(ibs);
  
  if (bg_sub<1)
    THROW("background subsampling must be >=1");

  if ((int) stencil->get_width() != (int) raw_pm.columns())
    THROW("Stencil and raw image have different widths!");
  if ((int) stencil->get_height() != (int) raw_pm.rows())
    THROW("Stencil and raw image have different heights!");
  // foreground
  {
    GPixmap fg_img;
    GBitmap fg_mask;
    processForeground(&raw_pm, stencil, fg_img, fg_mask);
    GP<IWPixmap> fg_pm = new IWPixmap(&fg_img, &fg_mask, IWPixmap::CRCBfull);
    IWEncoderParms parms[8];
    iff.put_chunk("FG44");
    parms[0].slices = 100;
    fg_pm->encode_chunk(iff, parms[0]);
    iff.close_chunk();
  }
  
  // backgound
  {
    GPixmap bg_img;
    GBitmap bg_mask;
    processBackground(&raw_pm, stencil, bg_img, bg_mask, bg_sub);
    GP<IWPixmap> bg_pm = new IWPixmap(&bg_img, &bg_mask, IWPixmap::CRCBnormal);
    IWEncoderParms parms[8];
    // could make  quality=75accessible
    int nchunks=4; int quality=75; 
    //  int base=72+ (int) (log(bg_subsampling/3.0)/log(3.0)*10);
    //
    // The following is the precalculated values of the above formula for 
    // the meaningfull ranges of bg_subsampling.
    static const int base_offset[]={72,61,68,72,74,76,78,79,80,82};

    int base= base_offset[bg_sub];
    int linear_quality;
    if (quality >= 75)
      linear_quality = (quality - 75) * 2 + 50;
    else
      linear_quality = (quality * 2) / 3;
    
    int n=0;
    switch (nchunks) {
    case 4:
      parms[0].bytes = 10000;
      parms[n++].slices = base + linear_quality / 25;
    case 3:
      parms[n++].slices = base + linear_quality / 4;
    case 2:
      parms[n++].slices = base + linear_quality / 3;
    case 1:
      parms[n++].slices = base + linear_quality / 2;
    }
    for (int i=0; i<nchunks; i++)
      {
        iff.put_chunk("BG44");
        bg_pm->encode_chunk(iff, parms[i]);
        iff.close_chunk();
      }
  }
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
                fprintf(stderr,"djvumake: duplicate stencil chunk\n");
              flag_contains_stencil = 1;
            }
          else if (! strncmp(argv[i],"Smmr=",5))
            {
              create_mmr_chunk(iff, "Smmr", argv[i]+5);
              if (flag_contains_stencil)
                fprintf(stderr,"djvumake: duplicate stencil chunk\n");
              flag_contains_stencil = 1;
            }
          else if (! strncmp(argv[i],"FG44=",5))
            {
              if (flag_contains_fg)
                fprintf(stderr,"djvumake: duplicate 'FGxx' chunk\n");
              create_fg44_chunk(iff, "FG44", argv[i]+5);
            }
          else if (! strncmp(argv[i],"BG44=",5))
            {
              create_bg44_chunk(iff, "BG44", argv[i]+5);
            }
          else if (! strncmp(argv[i],"BGjp=",5) ||
                   ! strncmp(argv[i],"BG2k=",5)  )
            {
              if (flag_contains_bg)
                fprintf(stderr,"djvumake: Duplicate BGxx chunk\n");
              argv[i][4] = 0;
              create_raw_chunk(iff, argv[i], argv[i]+5);
              flag_contains_bg = 1;
            }
          else if (! strncmp(argv[i],"FGjp=",5) ||
                   ! strncmp(argv[i],"FG2k=",5)  )
            {
              if (flag_contains_fg)
                fprintf(stderr,"djvumake: duplicate 'FGxx' chunk\n");
              argv[i][4] = 0;
              create_raw_chunk(iff, argv[i], argv[i]+5);
              flag_contains_fg = 1;
            }
          else if (! strncmp(argv[i],"ppm=",4))
            {
              
              if (flag_contains_bg)
                fprintf(stderr,"djvumake: Duplicate BGxx chunk\n");
              if (flag_contains_bg)
                fprintf(stderr,"djvumake: Duplicate BGxx chunk\n");
              masksub(iff, argv[i]+4 );

              flag_contains_bg = 1;
              flag_contains_fg = 1;
            }
          else 
            {
              fprintf(stderr,"djvumake: illegal argument : ``%s'' (ignored)\n", argv[i]);
            }
        }
      // Close
      iff.close_chunk();
      // Sanity checks
      if (flag_contains_stencil)
        {
          // Compound or Bilevel
          if (flag_contains_bg && ! flag_contains_fg)
            fprintf(stderr,"djvumake: djvu file contains a BGxx chunk but no FGxx chunk\n");
          if (flag_contains_fg && ! flag_contains_bg)
            fprintf(stderr,"djvumake: djvu file contains a FGxx chunk but no BGxx chunk\n");
        }
      else if (flag_contains_bg)
        {
          // Photo DjVu Image
          if (flag_contains_bg!=1)
            fprintf(stderr,"djvumake: photo djvu image has subsampled BGxx chunk\n"); 
          if (flag_contains_fg)
            fprintf(stderr,"djvumake: photo djvu file contains FGxx chunk\n");            
        }
      else
        fprintf(stderr,"djvumake: djvu file contains neither Sxxx nor BGxx chunks\n");
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

// Returns a dilated version (1 pixel, 8-neighborhood) of the original GBitmap
// (width and height of resulting GBitmap are logically 2 pixels more)

static GP<GBitmap> 
dilate8(const GBitmap *p_bm)
{
  const GBitmap& bm = *p_bm;
  GP<GBitmap> p_newbm = new GBitmap(bm.rows()+2,bm.columns()+2); 
  GBitmap& newbm = *p_newbm;
  for(unsigned int y=0; y<bm.rows(); y++)
    {
      for(unsigned int x=0; x<bm.columns(); x++)
        {
          if(bm[y][x]) 
            {
              // Set all the 8-neighborhood to black
              newbm[y][x]=1;
              newbm[y][x+1]=1;
              newbm[y][x+2]=1;
              newbm[y+1][x]=1;
              newbm[y+1][x+1]=1;
              newbm[y+1][x+2]=1;
              newbm[y+2][x]=1;
              newbm[y+2][x+1]=1;
              newbm[y+2][x+2]=1;
            }
        }
    }
  return p_newbm;
}


// Returns an eroded version (1-pixel, 8 neighborhood) of the original GBitmap
// (width and height of resulting GBitmap are logically 2 pixels less)

static GP<GBitmap> 
erode8(const GBitmap *p_bm)
{
  const GBitmap& bm = *p_bm;
  int newnrows = bm.rows()-2;
  int newncolumns = bm.columns()-2;
  if(newnrows<=0 || newncolumns<=0) // then return an empty GBitmap 
    return new GBitmap;
    
  GP<GBitmap> p_newbm = new GBitmap(newnrows,newncolumns); 
  GBitmap& newbm = *p_newbm;
  for(int y=0; y<newnrows; y++)
    {
      for(int x=0; x<newncolumns; x++)
        {
          // Check if there's a white pixel in the 8-neighborhood
          if(   !( bm[y  ][x] && bm[y  ][x+1] && bm[y  ][x+2]
                && bm[y+1][x] && bm[y+1][x+1] && bm[y+1][x+2]
                && bm[y+2][x] && bm[y+2][x+1] && bm[y+2][x+2]))
            newbm[y][x] = 0; // then set current to white
          else
            newbm[y][x] = 1; // else set current to black
        }
    }
  return p_newbm;
}

// Returns a new JB2Image that is a copy of the original with every blit dilated (8 neighborhod)

GP<JB2Image> 
dilate8(const JB2Image *im)
{
  int i;
  GP<JB2Image> newim = new JB2Image;
  newim->set_dimension(im->get_width(),im->get_height());
  for(i=0; i<im->get_shape_count(); i++)
    {
      const JB2Shape* shape = im->get_shape(i);
      JB2Shape newshape;
      newshape.parent = shape->parent;
      if (shape->bits) 
        newshape.bits = dilate8(shape->bits);
      else
        newshape.bits = 0;
      newim->add_shape(newshape);
    }
  for(i=0; i<im->get_blit_count(); i++)
    {
      const JB2Blit* blit = im->get_blit(i);
      JB2Blit newblit;
      newblit.bottom = blit->bottom - 1;
      newblit.left = blit->left - 1;
      newblit.shapeno = blit->shapeno;
      newim->add_blit(newblit);
    }
  return newim;
}

// Returns a new JB2Image that is a copy of the original with every blit eroded (8-neighborhood)
GP<JB2Image> 
erode8(const JB2Image *im)
{
  int i;
  GP<JB2Image> newim = new JB2Image;
  newim->set_dimension(im->get_width(),im->get_height());
  for(i=0; i<im->get_shape_count(); i++)
    {
      const JB2Shape* shape = im->get_shape(i);
      JB2Shape newshape;
      newshape.parent = shape->parent;
      if (shape->bits) 
        newshape.bits = erode8(shape->bits);
      else
        newshape.bits = 0;
      newim->add_shape(newshape);
    }
  for(i=0; i<im->get_blit_count(); i++)
    {
      const JB2Blit* blit = im->get_blit(i);
      JB2Blit newblit;
      newblit.bottom = blit->bottom + 1;
      newblit.left = blit->left + 1;
      newblit.shapeno = blit->shapeno;
      newim->add_blit(newblit);
    }
  return newim;
}





// ************************************
// * public static methods of MaskSub *
// ************************************

// maskedSubsample method:
// Subsamples only the pixels of <image> that are not masked (<mask>).
// This call resizes and fills the resulting <subsampled_image> and <subsampled_mask>.
// Their dimension is the dimension of the original <image> divided by <gridwidth>
// and rounded to the superior integer.
// For each square grid (gridwidth times gridwidth) of the subsampling mesh
// that contains at least <minpixels> non-masked pixels, their value is averaged 
// to give the value of the corresponding <subsampled_image> pixel, and the
// <subsampled_mask> is cleared at this position.
// If <inverted_mask> is true, then pixels are considered to be masked when mask==0
//
// The <incr_flag> can be set to specify incremental mode.
// This mode is used when calling maskedSubsample a second time 
// with a different mask. This will only set the pixels that have not previously been 
// set (i.e. for which the subsampled_mask hasn't been cleared). In this mode, the 
// subsampled_image and subsampled_mask are expected to already have the correct size.
// (See explanation in MaskSub::processBackground code to know how and why this is used)
 

static void
maskedSubsample(const GPixmap* img,
                         const GBitmap *p_mask,
                         GPixmap& subsampled_image,
                         GBitmap& subsampled_mask,
                         int gridwidth, int inverted_mask, 
                         int incr_flag, int minpixels
                         )
{
  const GPixmap& image= *img;
  const GBitmap& mask = *p_mask;
  int imageheight = image.rows();
  int imagewidth = image.columns();
  
  // compute the size of the resulting subsampled image
  int subheight = imageheight/gridwidth;
  if(imageheight%gridwidth)
    subheight++;
  int subwidth = imagewidth/gridwidth;
  if(imagewidth%gridwidth)
    subwidth++;

  if(!incr_flag) // set the sizes unless in incremental mode
    {
      subsampled_image.init(subheight, subwidth);
      subsampled_mask.init(subheight, subwidth);
    }

  int row, col;  // row and col in the subsampled image
  int posx, posxend, posy, posyend; // corresponding square in the original image

  for(row=0, posy=0; row<subheight; row++, posy+=gridwidth)
    {
      GPixel* subsampled_image_row = subsampled_image[row]; // row row of subsampled image
      unsigned char* subsampled_mask_row = subsampled_mask[row]; // row row of subsampled mask

      posyend = posy+gridwidth;
      if(posyend>imageheight)
        posyend = imageheight;

      for(col=0, posx=0; col<subwidth; col++, posx+=gridwidth) 
        {
          // Do not overwrite if in incremental mode
          if(!(incr_flag && subsampled_mask_row[col]==0)) 
            {
              posxend = posx+gridwidth;
              if(posxend>imagewidth)
                posxend = imagewidth;

              int count = 0;
              int r = 0;
              int g = 0;
              int b = 0;
              for(int y=posy; y<posyend; y++)
                {
                  const unsigned char* mask_y = mask[y]; // Row y of the mask
                  for(int x=posx; x<posxend; x++)
                    {
                      unsigned char masked = (inverted_mask ? !mask_y[x] :mask_y[x]);
                      if(!masked)
                        {
                          GPixel p = image[y][x];
                          r += p.r;
                          g += p.g;
                          b += p.b;
                          count ++;
                        }
                    }
                }

              /* minpixels pixels are enough to give the color */
              /* so set it, and do not mask this point */
              if(count >= minpixels)   
                {
                  GPixel p;
                  p.r = r/count;
                  p.g = g/count;
                  p.b = b/count;
                  subsampled_image_row[col] = p;
                  subsampled_mask_row[col] = 0;
                } 
              else /* make it bright red and masked */ 
                {
                  subsampled_image_row[col] = GPixel::RED;
                  subsampled_mask_row[col] = 1;
                }
            }
        }
    }
}





static void processForeground(const GPixmap* image, const JB2Image *mask,
                                GPixmap& subsampled_image, GBitmap& subsampled_mask)
{
  GP<JB2Image> eroded_mask = erode8(mask);
  maskedSubsample(image, eroded_mask->get_bitmap(), 
                  subsampled_image, subsampled_mask, 12, TRUE);
#ifdef DEBUG_SAVE
  StdioByteStream mask_pbm("mask.pbm","wb");
  mask->get_GBitmap()->save_pbm(mask_pbm);
  StdioByteStream sub_fg_ppm("sub_fg.ppm","wb");
  subsampled_image.save_ppm(sub_fg_ppm);
  StdioByteStream sub_fg_mask_pbm("sub_fg_mask.pbm","wb");
  subsampled_mask.save_pbm(sub_fg_mask_pbm);
#endif
}

static void processBackground(const GPixmap* image, const JB2Image *mask,
                           GPixmap& subsampled_image, GBitmap& subsampled_mask,
                              int bg_sub)
{
  GP<JB2Image> dilated_mask1 = dilate8(mask);
  GP<JB2Image> dilated_mask2 = dilate8(dilated_mask1);
  maskedSubsample(image, dilated_mask2->get_bitmap(), subsampled_image, 
                  subsampled_mask, bg_sub, FALSE);
#ifdef TEMPORARY_DISABLED
  // Now we call maskedSubsample in incremental mode to solve the following problem: 
  // The problem may occur with N=2: if dilation removes all the pixels in a 3x3 box, 
  // nothing is left to sample the background color
  // One deletion will remove less backgound pixels.
  maskedSubsample(image, dilated_mask1->get_GBitmap(), subsampled_image, 
                  subsampled_mask, 3, FALSE, TRUE);
#endif
#ifdef DEBUG_SAVE
  StdioByteStream sub_bg_ppm("sub_bg.ppm","wb");
  subsampled_image.save_ppm(sub_bg_ppm);
  StdioByteStream sub_bg_mask_pbm("sub_bg_mask.pbm","wb");
  subsampled_mask.save_pbm(sub_bg_mask_pbm);
#endif
}



