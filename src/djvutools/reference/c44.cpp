//C-  -*- C++ -*-
//C- DjVu� Reference Library (v. 3.0)
//C- 
//C- Copyright � 1999-2000 LizardTech, Inc. All Rights Reserved.
//C- 
//C- This software (the "Original Code") is subject to, and may be
//C- distributed under, the GNU General Public License, Version 2.
//C- The license should have accompanied the Original Code or you
//C- may obtain a copy of the license from the Free Software
//C- Foundation at http://www.fsf.org .
//C- 
//C- With respect to the Original Code, and subject to any third
//C- party intellectual property claims, LizardTech grants recipient
//C- a worldwide, royalty-free, non-exclusive license under patent
//C- claims infringed by making, using, or selling Original Code
//C- which are now or hereafter owned or controlled by LizardTech,
//C- but solely to the extent that any such patent is reasonably
//C- necessary to enable you to make, have made, practice, sell, or 
//C- otherwise dispose of Original Code (or portions thereof) and
//C- not to any greater extent that may be necessary to utilize
//C- further modifications or combinations.
//C- 
//C- The Original Code is provided "AS IS" WITHOUT WARRANTY OF ANY
//C- KIND, EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO
//C- ANY WARRANTY OF NON-INFRINGEMENT, OR ANY IMPLIED WARRANTY OF 
//C- MERCHANTIBILITY OF FITNESS FOR A PARTICULAR PURPOSE.
// 
// $Id: c44.cpp,v 1.4 2000-11-03 02:08:36 bcr Exp $
// $Name:  $


/** @name c44

    {\bf Synopsis}\\
    \begin{verbatim}
        c44 [options] pnmfile [djvufile]
    \end{verbatim}

    {\bf Description} ---
    File #"c44.cpp"# illustrates the use of classes \Ref{IWBitmap} and
    \Ref{IWPixmap} for compressing and encoding a color image or a gray level
    image using the DjVu IW44 wavelets.  This is the preferred mode for
    creating a DjVu image which does not require separate layers for encoding
    the text and the background images.  The files created by #c44# are
    recognized by both the IW44 decoder (see program \Ref{d44}) and by the
    DjVu decoder (see program \Ref{ddjvu}).

    {\bf Arguments} ---
    Argument #pnmfile# is the name of the input file.  PGM files are
    recognized for gray level images; PPM files are recognized for color
    images.  Popular file formats can be converted to PGM or PPM using the
    NetPBM package (\URL{http://www.arc.umn.edu/GVL/Software/netpbm.html})
    or the ImageMagick package (\URL{http://www.wizards.dupont.com/cristy/}).

    Optional argument #djvufile# is the name of the output file.  It is
    customary to use either suffix #".djvu"#, #".djv"#, #".iw44"# or #".iw4"#.
    Suffix #".djvu"# emphasizes the fact that IW44 files is seamlessly
    recognized by the current DjVu decoder.  Suffix #".iw4"# however was
    required by older versions of the DjVu plugin.  If this argument is
    omitted, a filename is generated by replacing the suffix of #pnmfile# with
    suffix #".iw4"#.

    {\bf Quality Specification}---
    Files produced by the DjVu IW44 Wavelet Encoder are IFF files composed of
    an arbitrary number of chunks (see \Ref{showiff} and \Ref{IWImage.h})
    containing successive refinements of the encoded image.  Each chunk is
    composed of several slices.  A typical file contains a total of 100 slices
    split between three or four chunks.  Various options provide quality
    targets (#-decibel#), slicing targets (#-slice#) or file size targets
    (#-bpp# or #-size#) for each of these refinements.  Chunks are generated
    until meeting either the decibel target, the file size target, or the
    slicing target for each chunk.
    \begin{description}
    \item[-bpp n,..,n] 
    Selects a increasing sequence of bitrates for building 
    progressive IW44 file (in bits per pixel).
    \item[-size n,..,n] 
    Selects a increasing sequence of minimal sizes for building 
    progressive IW44 file (in bytes).
    \item[-decibel n,..,n]
    Selects an increasing sequence of luminance error expressed as decibels
    ranging from 16 (very low quality) to 48 (very high quality).  This
    criterion should not be used when recoding an image which was already
    compressed with a lossy compression scheme (such as Wavelets or JPEG)
    because successive losses of quality accumulate.    \item[-slice n+...+n]
    Selects an increasing sequence of data slices expressed as integers 
    ranging from 1 to 140. 
    \end{description}
    These options take a target specification list expressed either as a comma
    separated list of increasing numbers or as a list of numbers separated by
    character #'+'#.  Both commands below for instance are equivalent:
    \begin{verbatim}
    c44 -bpp 0.1,0.2,0.5 inputfile.ppm  outputfile.djvu
    c44 -bpp 0.1+0.1+0.3 inputfile.ppm  outputfile.djvu
    \end{verbatim}
    Both these commands generate a file whose first chunk encodes the image
    with 0.1 bits per pixel, whose second chunk refines the image to 0.2 bits
    per pixel and whose third chunk refines the image to 0.5 bits per pixel.
    In other words, the second chunk provides an extra 0.1 bits per pixel and
    the third chunk provides an extra 0.3 bits per pixels.

    When no quality specification is provided, program #c44# usually generates
    a file composed of three progressive refinement chunks whose quality
    should be acceptable.  The best results however are achieved by precisely
    tuning the image quality.  As a rule of thumb, #c44# generates an
    acceptable quality when you specify a size equal to 50% to 75% of the size
    of a comparable JPEG image.

    {\bf Color Processing Specification} ---
    Five options control the encoding of the chrominance information of color
    images.  These options are of course meaningless for processing a gray
    level image.
    \begin{description}
    \item[-crcbnormal]
    Selects normal chrominance encoding (default).  Chrominance information is
    encoded at the same resolution as the luminance. 
    \item[-crcbhalf]
    Selects half resolution chrominance encoding.  Chrominance information is
    encoded at half the luminance resolution. 
    \item[-crcbdelay n]
    This option can be used with #-crcbnormal# and #-crcbhalf# for specifying
    an encoding delay which reduces the bitrate associated with the chrominance. The
    default chrominance encoding delay is 10 slices.
    \item[-crcbfull]
    Selects the highest possible quality for encoding the chrominance information. This
    is equivalent to specifying #-crcbnormal# and #-crcbdelay 0#.
    \item[-crcbnone]
    Disables encoding of the chrominance.  Only the luminance information will
    be encoded. The resulting image will show in shades of gray.
    \end{description}

    {\bf Advanced Options} ---
    Program #c44# also recognizes the following options:
    \begin{description}
    \item[-dbfrac f]
    This option alters the meaning of the -decibel option.  The decibel target then
    addresses only the average error of the specified fraction of the most
    misrepresented 32x32 pixel blocks.
    \item[-mask pbmfile] 
    This option can be used when we know that certain pixels of a background
    image are going to be covered by foreground objects like text or drawings.
    File #pbmfile# must be a PBM file whose size matches the size of the input
    file.  Each black pixel in #pbmfile# means that the value of the corresponding
    pixel in the input file is irrelevant.  The DjVu IW44 Encoder will replace
    the masked pixels by a color value whose coding cost is minimal (see
    \URL{http://www.research.att.com/~leonb/DJVU/mask}).
    \end{description}

    {\bf Generating Photo DjVu instead of IW44} ---
    Photo DjVu images have thje additional capability to store the resolution
    and gamma correction information.  Using any of the following options will
    generate a Photo DjVu Image instead of a IW44 file.  Program \Ref{d44}
    does not work on these files.  Program \Ref{ddjvu} handles both IW44 and
    DjVu files.
    \begin{description}
    \item[-dpi n]  Sets the resolution information for a Photo DjVu image.
    \item[-gamma n] Sets the gamma correction information for a Photo DjVu image.
    \end{description}

    {\bf Performance} ---
    The main design objective for the DjVu wavelets consisted of allowing
    progressive rendering and smooth scrolling of large images with limited
    memory requirements.  Decoding functions process the compressed data and
    update a memory efficient representation of the wavelet coefficients.
    Imaging function then can quickly render an arbitrary segment of the image
    using the available data.  Both process can be carried out in two threads
    of execution.  This design plays an important role in the DjVu system.

    We have investigated various state-of-the-art wavelet compression schemes:
    although these schemes may achieve slightly smaller file sizes, the
    decoding functions did not even approach our requirements.  The IW44
    wavelets reach these requirements today and may in the future implement
    more modern refinements (such as trellis quantization, bitrate
    allocation, etc.) if (and only if) these refinements can be implemented
    within our constraints.

    @memo
    DjVu IW44 wavelet encoder.
    @author
    L\'eon Bottou <leonb@research.att.com>
    @version
    #$Id: c44.cpp,v 1.4 2000-11-03 02:08:36 bcr Exp $# */
//@{
//@}

#include <stdio.h>
#include <string.h>
#include "GString.h"
#include "GException.h"
#include "IWImage.h"
#include "DjVuInfo.h"
#include "IFFByteStream.h"
#include "GOS.h"

// command line data

GString pnmfile;
GString iw4file;
GString mskfile;

int flag_mask = 0;
int flag_bpp = 0;
int flag_size = 0;
int flag_slice = 0;
int flag_decibel = 0;
int flag_crcbdelay = -1;
int flag_crcbmode = -1;  
double flag_dbfrac = -1;
int flag_dpi = -1;
double flag_gamma = -1;
int argc_bpp = 0;
int argc_size = 0;
int argc_slice = 0;
int argc_decibel = 0;
IWPixmap::CRCBMode arg_crcbmode = IWPixmap::CRCBnormal;

#define MAXCHUNKS 64
float argv_bpp[MAXCHUNKS];
int   argv_size[MAXCHUNKS];
int   argv_slice[MAXCHUNKS];
float argv_decibel[MAXCHUNKS];
IWEncoderParms parms[MAXCHUNKS];


// parse arguments

void 
usage()
{
  printf("C44 -- Image compression utility using Interpolating Wavelets (4,4)\n"
         "  Copyright � 1999-2000 LizardTech, Inc. All Rights Reserved.\n"
         "Usage: c44 [options] pnmfile [djvufile]\n\n"
         "Options:\n"
         "    -bpp n,..,n      -- select a increasing sequence of bitrates\n"
         "                        for building progressive file (in bits per pixel).\n"
         "    -size n,..,n     -- select an increasing sequence of minimal sizes\n"
         "                        for building progressive files (expressed in bytes).\n"
         "    -decibel n,..,n  -- select an increasing sequence of luminance error\n"
         "                        expressed as decibels (ranging from 16 to 50).\n"
         "    -slice n+...+n   -- select an increasing sequence of data slices\n"
         "                        expressed as integers ranging from 1 to 140.\n"
         "    -dbfrac frac     -- restrict decibel estimation on the specified fraction\n"
         "                        of the most misrepresented 32x32 blocks\n"
         "    -mask pbmfile    -- select bitmask specifying image zone to encode\n"
         "                        with minimal bitrate. (default none)\n"
         "\n"
         "Quality selection:\n"
         "    Chunks are generated until meeting either the decibel target (-decibel)\n"
         "    the file size target (-bpp or -size) or the slice target (-slice)\n"
         "    All quality specifications can be absolute (e.g. -bpp 0.1,0.2,0.5)\n"
         "    or relative (e.g. -bpp 0.1+0.1+0.3).\n"
         "\n"
         "    When no quality selection arguments are provided, c44 creates three\n"
         "    chunks at 25, 30 and 34 (estimated) decibels of luminance error.\n"
         "    The error estimation algorithm however is unacceptably optimistic when\n"
         "    recoding an image generated from wavelet or jpeg encoded file.\n"
         "\n"
         "Advanced chrominance options:\n"
         "    -crcbfull     -- encode chrominance with highest quality\n"
         "    -crcbnormal   -- encode chrominance with normal resolution (default)\n"
         "    -crcbhalf     -- encode chrominance with half resolution\n"
         "    -crcbnone     -- do not encode chrominance at all\n"
         "    -crcbdelay n  -- select chrominance coding delay (default 10)\n"
         "                     for -crcbnormal and -crcbhalf modes\n"
         "\n"
         "You can generating a Photo DjVu image instead of a IW44 image\n"
         "by using any of the following options.  These files must be decoded\n"
         "using program DDJVU rather than D44\n"
         "    -dpi n        -- sets the image resolution\n"
         "    -gamma n      -- sets the image gamma correction\n"
         "\n");
  exit(1);
}



void 
parse_bpp(const char *q)
{
  flag_bpp = 1;
  argc_bpp = 0;
  double lastx = 0;
  while (*q)
    {
      char *ptr; 
      double x = strtod(q, &ptr);
      if (ptr == q)
        G_THROW("c44: illegal bitrate specification (number expected)");
      if (lastx>0 && q[-1]=='+')
        x += lastx;
      if (x<=0 || x>24 || x<lastx)
        G_THROW("c44: illegal bitrate specification (number out of range)");
      lastx = x;
      if (*ptr && *ptr!='+' && *ptr!=',')
        G_THROW("c44: illegal bitrate specification (comma expected)");        
      q = (*ptr ? ptr+1 : ptr);
      argv_bpp[argc_bpp++] = (float)x;
      if (argc_bpp>MAXCHUNKS)
        G_THROW("c44: illegal bitrate specification (too many chunks)");                
    }
  if (argc_bpp < 1)
    G_THROW("c44: illegal bitrate specification (no chunks)");                    
}


void 
parse_size(const char *q)
{
  flag_size = 1;
  argc_size = 0;
  int lastx = 0;
  while (*q)
    {
      char *ptr; 
      int x = strtol(q, &ptr, 10);
      if (ptr == q)
        G_THROW("c44: illegal size specification (number expected)");
      if (lastx>0 && q[-1]=='+')
        x += lastx;
      if (x<lastx)
        G_THROW("c44: illegal size specification (number out of range)");
      lastx = x;
      if (*ptr && *ptr!='+' && *ptr!=',')
        G_THROW("c44: illegal size specification (comma expected)");        
      q = (*ptr ? ptr+1 : ptr);
      argv_size[argc_size++] = x;
      if (argc_size>=MAXCHUNKS)
        G_THROW("c44: illegal size specification (too many chunks)");                
    }
  if (argc_size < 1)
    G_THROW("c44: illegal size specification (no chunks)");                    
}

void 
parse_slice(const char *q)
{
  flag_slice = 1;
  argc_slice = 0;
  int lastx = 0;
  while (*q)
    {
      char *ptr; 
      int x = strtol(q, &ptr, 10);
      if (ptr == q)
        G_THROW("c44: illegal slice specification (number expected)");
      if (lastx>0 && q[-1]=='+')
        x += lastx;
      if (x<1 || x>1000 || x<lastx)
        G_THROW("c44: illegal slice specification (number out of range)");
      lastx = x;
      if (*ptr && *ptr!='+' && *ptr!=',')
        G_THROW("c44: illegal slice specification (comma expected)");        
      q = (*ptr ? ptr+1 : ptr);
      argv_slice[argc_slice++] = x;
      if (argc_slice>=MAXCHUNKS)
        G_THROW("c44: illegal slice specification (too many chunks)");                
    }
  if (argc_slice < 1)
    G_THROW("c44: illegal slice specification (no chunks)");                    
}


void 
parse_decibel(const char *q)
{
  flag_decibel = 1;
  argc_decibel = 0;
  double lastx = 0;
  while (*q)
    {
      char *ptr; 
      double x = strtod(q, &ptr);
      if (ptr == q)
        G_THROW("c44: illegal decibel specification (number expected)");
      if (lastx>0 && q[-1]=='+')
        x += lastx;
      if (x<16 || x>50 || x<lastx)
        G_THROW("c44: illegal decibel specification (number out of range)");
      lastx = x;
      if (*ptr && *ptr!='+' && *ptr!=',')
        G_THROW("c44: illegal decibel specification (comma or plus expected)");        
      q = (*ptr ? ptr+1 : ptr);
      argv_decibel[argc_decibel++] = (float)x;
      if (argc_decibel>=MAXCHUNKS)
        G_THROW("c44: illegal decibel specification (too many chunks)");                
    }
  if (argc_decibel < 1)
    G_THROW("c44: illegal decibel specification (no chunks)");                    
}


int 
resolve_quality(int npix)
{
  // Convert ratio specification into size specification
  if (flag_bpp)
    {
      if (flag_size)
        G_THROW("Options -size and -bpp are exclusive");
      flag_size = flag_bpp;
      argc_size = argc_bpp;
      for (int i=0; i<argc_bpp; i++)
        argv_size[i] = (int)(npix*argv_bpp[i]/8.0+0.5);
    }
  // Compute number of chunks
  int nchunk = 0;
  if (flag_slice && nchunk<argc_slice)
    nchunk = argc_slice;
  if (flag_size && nchunk<argc_size)
    nchunk = argc_size;
  if (flag_decibel && nchunk<argc_decibel)
    nchunk = argc_decibel;
  // Force default values
  if (nchunk == 0)
    {
#ifdef DECIBELS_25_30_34
      nchunk = 3;
      flag_decibel = 1;
      argc_decibel = 3;
      argv_decibel[0]=25;
      argv_decibel[1]=30;
      argv_decibel[2]=34;
#else
      nchunk = 3;
      flag_slice = 1;
      argc_slice = 3;
      argv_slice[0]=74;
      argv_slice[1]=87;
      argv_slice[2]=97;
#endif
    }
  // Complete short specifications
  while (argc_size < nchunk)
    argv_size[argc_size++] = 0;
  while (argc_slice < nchunk)
    argv_slice[argc_slice++] = 0;
  while (argc_decibel < nchunk)
    argv_decibel[argc_decibel++] = 0.0;
  // Fill parm structure
  for(int i=0; i<nchunk; i++)
    {
      parms[i].bytes = argv_size[i];
      parms[i].slices = argv_slice[i];
      parms[i].decibels = argv_decibel[i];
    }
  // Return number of chunks
  return nchunk;
}


void
parse(int argc, char **argv)
{
  for (int i=1; i<argc; i++)
    {
      if (argv[i][0] == '-')
        {
          if (!strcmp(argv[i],"-bpp"))
            {
              if (++i >= argc)
                G_THROW("c44: no argument for option '-bpp'");
              if (flag_bpp || flag_size)
                G_THROW("c44: multiple bitrate specification");
              parse_bpp(argv[i]);
            }
          else if (!strcmp(argv[i],"-size"))
            {
              if (++i >= argc)
                G_THROW("c44: no argument for option '-size'");
              if (flag_bpp || flag_size)
                G_THROW("c44: multiple bitrate specification");
              parse_size(argv[i]);
            }
          else if (!strcmp(argv[i],"-decibel"))
            {
              if (++i >= argc)
                G_THROW("c44: no argument for option '-decibel'");
              if (flag_decibel)
                G_THROW("c44: multiple decibel specification");
              parse_decibel(argv[i]);
            }
          else if (!strcmp(argv[i],"-slice"))
            {
              if (++i >= argc)
                G_THROW("c44: no argument for option '-slice'");
              if (flag_slice)
                G_THROW("c44: multiple slice specification");
              parse_slice(argv[i]);
            }
          else if (!strcmp(argv[i],"-mask"))
            {
              if (++i >= argc)
                G_THROW("c44: no argument for option '-mask'");
              if (!! mskfile)
                G_THROW("c44: multiple mask specification");
              mskfile = argv[i];
            }
          else if (!strcmp(argv[i],"-dbfrac"))
            {
              if (++i >= argc)
                G_THROW("c44: no argument for option '-dbfrac'");
              if (flag_dbfrac>0)
                G_THROW("c44: multiple dbfrac specification");                
              char *ptr;
              flag_dbfrac = strtod(argv[i], &ptr);
              if (flag_dbfrac<=0 || flag_dbfrac>1 || *ptr)
                G_THROW("c44: illegal dbfrac specification");
            }
          else if (!strcmp(argv[i],"-crcbnone"))
            {
              if (flag_crcbmode>=0 || flag_crcbdelay>=0)
                G_THROW("c44: incompatible chrominance options");
              flag_crcbdelay = flag_crcbmode = 0;
              arg_crcbmode = IWPixmap::CRCBnone;
            }
          else if (!strcmp(argv[i],"-crcbhalf"))
            {
              if (flag_crcbmode>=0)
                G_THROW("c44: incompatible chrominance options");
              flag_crcbmode = 0;
              arg_crcbmode = IWPixmap::CRCBhalf;
            }
          else if (!strcmp(argv[i],"-crcbnormal"))
            {
              if (flag_crcbmode>=0)
                G_THROW("c44: incompatible chrominance options");
              flag_crcbmode = 0;
              arg_crcbmode = IWPixmap::CRCBnormal;
            }
          else if (!strcmp(argv[i],"-crcbfull"))
            {
              if (flag_crcbmode>=0 || flag_crcbdelay>=0)
                G_THROW("c44: incompatible chrominance options");
              flag_crcbdelay = flag_crcbmode = 0;
              arg_crcbmode = IWPixmap::CRCBfull;
            }
          else if (!strcmp(argv[i],"-crcbdelay"))
            {
              if (++i >= argc)
                G_THROW("c44: no argument for option '-crcbdelay'");
              if (flag_crcbdelay>=0)
                G_THROW("c44: incompatible chrominance options");
              char *ptr; 
              flag_crcbdelay = strtol(argv[i], &ptr, 10);
              if (*ptr || flag_crcbdelay<0 || flag_crcbdelay>=100)
                G_THROW("c44: illegal argument for option '-crcbdelay'");
            }
          else if (!strcmp(argv[i],"-dpi"))
            {
              if (++i >= argc)
                G_THROW("c44: no argument for option '-dpi'");
              if (flag_dpi>0)
                G_THROW("c44: duplicate dpi option");
              char *ptr; 
              flag_dpi = strtol(argv[i], &ptr, 10);
              if (*ptr || flag_dpi<25 || flag_dpi>4800)
                G_THROW("c44: illegal argument for option '-dpi'");
            }
          else if (!strcmp(argv[i],"-gamma"))
            {
              if (++i >= argc)
                G_THROW("c44: no argument for option '-gamma'");
              if (flag_gamma > 0)
                G_THROW("c44: duplicate gamma option");
              char *ptr; 
              flag_gamma = strtod(argv[i], &ptr);
              if (*ptr || flag_gamma<=0.25 || flag_gamma>=5)
                G_THROW("c44: illegal gamma specification");
            }
          else
            usage();
        }
      else if (!pnmfile)
        pnmfile = argv[i];
      else if (!iw4file)
        iw4file = argv[i];
      else
        usage();
    }
  if (!pnmfile)
    usage();
  if (!iw4file)
    {
      GString dir = GOS::dirname(pnmfile);
      GString base = GOS::basename(pnmfile);
      int dot = base.rsearch('.');
      if (dot >= 1)
        base = base.substr(0,dot);
      if (flag_dpi>0 || flag_gamma>0)
        iw4file = GOS::expand_name(base,dir) + ".djvu";
      else
        iw4file = GOS::expand_name(base,dir) + ".iw4";
    }
}



GP<GBitmap>
getmask(int w, int h)
{
  static GP<GBitmap> msk8;
  if (!! mskfile)
    {
      StdioByteStream mbs(mskfile,"rb");
      msk8 = new GBitmap(mbs);
      if (msk8->columns() != (unsigned int)w || 
          msk8->rows()    != (unsigned int)h  )
        G_THROW("c44: mask and image have different size");
    }
  return msk8;
}


static void 
create_photo_djvu_file(IWPixmap *iwp, IWBitmap *iwb, int w, int h,
                       IFFByteStream &iff, int nchunks, IWEncoderParms *parms)
{
  // Prepare info chunk
  DjVuInfo info;
  info.width = w;
  info.height = h;
  info.dpi = (flag_dpi>0 ? flag_dpi : 100);
  info.gamma = (flag_gamma>0 ? flag_gamma : 2.2);
  // Write djvu header and info chunk
  iff.put_chunk("FORM:DJVU", 1);
  iff.put_chunk("INFO");
  info.encode(iff);
  iff.close_chunk();
  // Write all chunks
  int flag = 1;
  for (int i=0; flag && i<nchunks; i++)
    {
      iff.put_chunk("BG44");
      if (iwp)
        flag = iwp->encode_chunk(iff, parms[i]);
      else
        flag = iwb->encode_chunk(iff, parms[i]);
      iff.close_chunk();
    }
  // Close djvu chunk
  iff.close_chunk();
}


int
main(int argc, char **argv)
{
  G_TRY
    {
      // Parse arguments
      parse(argc, argv);
      // Check input file
      StdioByteStream ibs(pnmfile,"rb");
      char prefix[16];
      if (ibs.readall((void*)prefix, sizeof(prefix)) != sizeof(prefix))
        G_THROW("c44: cannot read pnm file header");
      // Load images
      int w = 0;
      int h = 0;
      IWPixmap *iwp = 0;
      IWBitmap *iwb = 0;
      // Check color vs gray
      if (prefix[0]=='P' && (prefix[1]=='3' || prefix[1]=='6'))
        {
          // color file
          ibs.seek(0);
          GPixmap ipm(ibs);
          w = ipm.columns();
          h = ipm.rows();
          iwp = new IWPixmap(&ipm, getmask(w,h), arg_crcbmode);
        }
      else if (prefix[0]=='P' && (prefix[1]=='2' || prefix[1]=='5'))
        {
          // gray file
          ibs.seek(0);
          GBitmap ibm(ibs);
          w = ibm.columns();
          h = ibm.rows();
          iwb = new IWBitmap(&ibm, getmask(w,h));
        }
      else if (!strncmp(prefix,"AT&TFORM",8) || !strncmp(prefix,"FORM",4))
        {
          char *s = (prefix[0]=='F' ? prefix+8 : prefix+12);
          ibs.seek(0);
          IFFByteStream iff(ibs);
          if (!strncmp(s,"PM44",4))
            {
              iwp = new IWPixmap();
              iwp->decode_iff(iff);
              w = iwp->get_width();
              h = iwp->get_height();
            }
          else if (!strncmp(s,"BM44",4))
            {
              iwb = new IWBitmap();
              iwb->decode_iff(iff);
              w = iwb->get_width();
              h = iwb->get_height();
            }
          else
            G_THROW("Unrecognized file");
          // Check that no mask has been specified.
          if (!! mskfile)
            G_THROW("Cannot apply mask on an already compressed image");
        }
      else
        {
          G_THROW("Unrecognized file");
        }
      // Call destructor on input file
      ibs.ByteStream::~ByteStream();
              
      // Perform compression PM44 or BM44 as required
      if (iwp)
        {
          remove(iw4file);
          StdioByteStream obs(iw4file,"wb");
          IFFByteStream iff(obs);
          if (flag_crcbdelay >= 0)
            iwp->parm_crcbdelay(flag_crcbdelay);
          if (flag_dbfrac > 0)
            iwp->parm_dbfrac((float)flag_dbfrac);
          int nchunk = resolve_quality(w*h);
          if (flag_gamma>0 || flag_dpi>0)
            // Create djvu file
            create_photo_djvu_file(iwp, 0, w, h, iff, nchunk, parms);
          else
            // Create iw44 file
            iwp->encode_iff(iff, nchunk, parms);
        }
      else if (iwb)
        {
          remove(iw4file);
          StdioByteStream obs(iw4file,"wb");
          IFFByteStream iff(obs);
          if (flag_dbfrac > 0)
            iwb->parm_dbfrac((float)flag_dbfrac);
          int nchunk = resolve_quality(w*h);
          if (flag_gamma>0 || flag_dpi>0)
            // Create djvu file
            create_photo_djvu_file(0, iwb, w, h, iff, nchunk, parms);
          else
            // Create iw44 file
            iwb->encode_iff(iff, nchunk, parms);
        }
      // Cleanup memory
      delete iwp;
      delete iwb;
    }
  G_CATCH(ex)
    {
      ex.perror("Exception while executing C44");
      exit(1);
    }
  G_ENDCATCH;
  return 0;
}
