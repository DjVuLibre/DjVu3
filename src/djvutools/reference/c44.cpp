//C-  -*- C++ -*-
//C- DjVu� Reference Library (v. 3.5)
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
// $Id: c44.cpp,v 1.34 2001-08-24 22:34:33 docbill Exp $
// $Name:  $


/** @name c44

    {\bf Synopsis}\\
    \begin{verbatim}
        c44 [options] pnmfile [djvufile]
        c44 [options] jpegfile [djvufile]
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
    Argument #pnmfile# or #jpegfile# is the name of the input file.  PGM files
    are recognized for gray level images; PPM files are recognized for color
    images.  Popular file formats can be converted to PGM or PPM using the
    NetPBM package (\URL{http://www.arc.umn.edu/GVL/Software/netpbm.html})
    or the ImageMagick package (\URL{http://www.wizards.dupont.com/cristy/}).

    Optional argument #djvufile# is the name of the output file.  It is
    customary to use either suffix #".djvu"#, #".djv"#, #".iw44"# or #".iw4"#.
    Suffix #".djvu"# emphasizes the fact that IW44 files is seamlessly
    recognized by the current DjVu decoder.  Suffix #".iw4"# however was
    required by older versions of the DjVu plugin.  If this argument is
    omitted, a filename is generated by replacing the suffix of #pnmfile# 
    or #jpegfile# with suffix #".iw4"#.

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
    #$Id: c44.cpp,v 1.34 2001-08-24 22:34:33 docbill Exp $# */
//@{
//@}

#include "GString.h"
#include "GException.h"
#include "IW44Image.h"
#include "DjVuInfo.h"
#include "IFFByteStream.h"
#include "GOS.h"
#include "GBitmap.h"
#include "GPixmap.h"
#include "GURL.h"
#include "DjVuMessage.h"
#include "JPEGDecoder.h"

#include <locale.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// command line data

GURL pnmurl;
GURL iw4url;
GURL mskurl;

GUTF8String percent;

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
IW44Image::CRCBMode arg_crcbmode = IW44Image::CRCBnormal;

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
         "  Copyright © 1999-2000 LizardTech, Inc. All Rights Reserved.\n"
         "Usage: c44 [options] pnmfile [djvufile]\n\n"
         "Usage: c44 [options] jpegfile [djvufile]\n\n"
         "Options:\n"
         "    -percent n,..,n    -- selects the percentage of original file size\n"
         "                        for building progressive file.\n"
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
         "You can generate a Photo DjVu image instead of a IW44 image\n"
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
        G_THROW( ERR_MSG("c44.bitrate_not_number") );
      if (lastx>0 && q[-1]=='+')
        x += lastx;
      if (x<=0 || x>24 || x<lastx)
        G_THROW( ERR_MSG("c44.bitrate_out_of_range") );
      lastx = x;
      if (*ptr && *ptr!='+' && *ptr!=',')
        G_THROW( ERR_MSG("c44.bitrate_comma_expected") );
      q = (*ptr ? ptr+1 : ptr);
      argv_bpp[argc_bpp++] = (float)x;
      if (argc_bpp>MAXCHUNKS)
        G_THROW( ERR_MSG("c44.bitrate_too_many") );
    }
  if (argc_bpp < 1)
    G_THROW( ERR_MSG("c44.bitrate_no_chunks") );
}


void 
parse_size(const char *q,const int size=0)
{
  percent=GUTF8String();
  flag_size = 1;
  argc_size = 0;
  int lastx = 0;
  while (*q)
    {
      char *ptr; 
      int x = strtol(q, &ptr, 10);
      if (ptr == q)
        G_THROW( ERR_MSG("c44.size_not_number") );
      if (lastx>0 && q[-1]=='+')
        x += lastx;
      if (x<lastx)
        G_THROW( ERR_MSG("c44.size_out_of_range") );
      lastx = x;
      if (*ptr && *ptr!='+' && *ptr!=',')
        G_THROW( ERR_MSG("c44.size_comma_expected") );
      q = (*ptr ? ptr+1 : ptr);
      argv_size[argc_size++] = size?((x*size)/100):x;
      if (argc_size>=MAXCHUNKS)
        G_THROW( ERR_MSG("c44.size_too_many") );
    }
  if (argc_size < 1)
    G_THROW( ERR_MSG("c44.size_no_chunks") );
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
        G_THROW( ERR_MSG("c44.slice_not_number") );
      if (lastx>0 && q[-1]=='+')
        x += lastx;
      if (x<1 || x>1000 || x<lastx)
        G_THROW( ERR_MSG("c44.slice_out_of_range") );
      lastx = x;
      if (*ptr && *ptr!='+' && *ptr!=',')
        G_THROW( ERR_MSG("c44.slice_comma_expected") );
      q = (*ptr ? ptr+1 : ptr);
      argv_slice[argc_slice++] = x;
      if (argc_slice>=MAXCHUNKS)
        G_THROW( ERR_MSG("c44.slice_too_many") );
    }
  if (argc_slice < 1)
    G_THROW( ERR_MSG("c44.slice_no_chunks") );
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
        G_THROW( ERR_MSG("c44.decibel_not_number") );
      if (lastx>0 && q[-1]=='+')
        x += lastx;
      if (x<16 || x>50 || x<lastx)
        G_THROW( ERR_MSG("c44.decibel_out_of_range") );
      lastx = x;
      if (*ptr && *ptr!='+' && *ptr!=',')
        G_THROW( ERR_MSG("c44.decibel_comma_expected") );
      q = (*ptr ? ptr+1 : ptr);
      argv_decibel[argc_decibel++] = (float)x;
      if (argc_decibel>=MAXCHUNKS)
        G_THROW( ERR_MSG("c44.decibel_too_many") );
    }
  if (argc_decibel < 1)
    G_THROW( ERR_MSG("c44.decibel_no_chunks") );
}


int 
resolve_quality(int npix)
{
  // Convert ratio specification into size specification
  if (flag_bpp)
    {
      if (flag_size)
        G_THROW( ERR_MSG("c44.exclusive") );
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
parse(DArray<GUTF8String> &argv)
{
  const int argc=argv.hbound()+1;
  for (int i=1; i<argc; i++)
    {
      if (argv[i][0] == '-')
        {
          if (argv[i] == "-percent")
            {
              if (++i >= argc)
                G_THROW( ERR_MSG("c44.no_bpp_arg") );
              if (flag_bpp || flag_size)
                G_THROW( ERR_MSG("c44.multiple_bitrate") );
              percent=argv[i];
            }
          else if (argv[i] == "-bpp")
            {
              if (++i >= argc)
                G_THROW( ERR_MSG("c44.no_bpp_arg") );
              if (flag_bpp || flag_size)
                G_THROW( ERR_MSG("c44.multiple_bitrate") );
              parse_bpp(argv[i]);
            }
          else if (argv[i] == "-size")
            {
              if (++i >= argc)
                G_THROW( ERR_MSG("c44.no_size_arg") );
              if (flag_bpp || flag_size)
                G_THROW( ERR_MSG("c44.multiple_size") );
              parse_size(argv[i]);
            }
          else if (argv[i] == "-decibel")
            {
              if (++i >= argc)
                G_THROW( ERR_MSG("c44.no_decibel_arg") );
              if (flag_decibel)
                G_THROW( ERR_MSG("c44.multiple_decibel") );
              parse_decibel(argv[i]);
            }
          else if (argv[i] == "-slice")
            {
              if (++i >= argc)
                G_THROW( ERR_MSG("c44.no_slice_arg") );
              if (flag_slice)
                G_THROW( ERR_MSG("c44.multiple_slice") );
              parse_slice(argv[i]);
            }
          else if (argv[i] == "-mask")
            {
              if (++i >= argc)
                G_THROW( ERR_MSG("c44.no_mask_arg") );
              if (! mskurl.is_empty())
                G_THROW( ERR_MSG("c44.multiple_mask") );
              mskurl = GURL::Filename::UTF8(argv[i]);
            }
          else if (argv[i] == "-dbfrac")
            {
              if (++i >= argc)
                G_THROW( ERR_MSG("c44.no_dbfrac_arg") );
              if (flag_dbfrac>0)
                G_THROW( ERR_MSG("c44.multiple_dbfrac") );
              char *ptr;
              flag_dbfrac = strtod(argv[i], &ptr);
              if (flag_dbfrac<=0 || flag_dbfrac>1 || *ptr)
                G_THROW( ERR_MSG("c44.illegal_dbfrac") );
            }
          else if (argv[i] == "-crcbnone")
            {
              if (flag_crcbmode>=0 || flag_crcbdelay>=0)
                G_THROW( ERR_MSG("c44.incompatable_chrominance") );
              flag_crcbdelay = flag_crcbmode = 0;
              arg_crcbmode = IW44Image::CRCBnone;
            }
          else if (argv[i] == "-crcbhalf")
            {
              if (flag_crcbmode>=0)
                G_THROW( ERR_MSG("c44.incompatable_chrominance") );
              flag_crcbmode = 0;
              arg_crcbmode = IW44Image::CRCBhalf;
            }
          else if (argv[i] == "-crcbnormal")
            {
              if (flag_crcbmode>=0)
                G_THROW( ERR_MSG("c44.incompatable_chrominance") );
              flag_crcbmode = 0;
              arg_crcbmode = IW44Image::CRCBnormal;
            }
          else if (argv[i] == "-crcbfull")
            {
              if (flag_crcbmode>=0 || flag_crcbdelay>=0)
                G_THROW( ERR_MSG("c44.incompatable_chrominance") );
              flag_crcbdelay = flag_crcbmode = 0;
              arg_crcbmode = IW44Image::CRCBfull;
            }
          else if (argv[i] == "-crcbdelay")
            {
              if (++i >= argc)
                G_THROW( ERR_MSG("c44.no_crcbdelay_arg") );
              if (flag_crcbdelay>=0)
                G_THROW( ERR_MSG("c44.incompatable_chrominance") );
              char *ptr; 
              flag_crcbdelay = strtol(argv[i], &ptr, 10);
              if (*ptr || flag_crcbdelay<0 || flag_crcbdelay>=100)
                G_THROW( ERR_MSG("c44.illegal_crcbdelay") );
            }
          else if (argv[i] == "-dpi")
            {
              if (++i >= argc)
                G_THROW( ERR_MSG("c44.no_dpi_arg") );
              if (flag_dpi>0)
                G_THROW( ERR_MSG("c44.duplicate_dpi") );
              char *ptr; 
              flag_dpi = strtol(argv[i], &ptr, 10);
              if (*ptr || flag_dpi<25 || flag_dpi>4800)
                G_THROW( ERR_MSG("c44.illegal_dpi") );
            }
          else if (argv[i] == "-gamma")
            {
              if (++i >= argc)
                G_THROW( ERR_MSG("c44.no_gamma_arg") );
              if (flag_gamma > 0)
                G_THROW( ERR_MSG("c44.duplicate_gamma") );
              char *ptr; 
              flag_gamma = strtod(argv[i], &ptr);
              if (*ptr || flag_gamma<=0.25 || flag_gamma>=5)
                G_THROW( ERR_MSG("c44.illegal_gamma") );
            }
          else
            usage();
        }
      else if (pnmurl.is_empty())
        pnmurl = GURL::Filename::UTF8(argv[i]);
      else if (iw4url.is_empty())
        iw4url = GURL::Filename::UTF8(argv[i]);
      else
        usage();
    }
  if (pnmurl.is_empty())
    usage();
  if (iw4url.is_empty())
    {
      GURL codebase=pnmurl.base();
      GUTF8String base = pnmurl.fname();
      int dot = base.rsearch('.');
      if (dot >= 1)
        base = base.substr(0,dot);
      const char *ext=(flag_dpi>0 || flag_gamma>0)?".djvu":".iw4";
      iw4url = GURL::UTF8(base+ext,codebase);
    }
}



GP<GBitmap>
getmask(int w, int h)
{
  GP<GBitmap> msk8;
  if (! mskurl.is_empty())
    {
      GP<ByteStream> mbs=ByteStream::create(mskurl,"rb");
      msk8 = GBitmap::create(*mbs);
      if (msk8->columns() != (unsigned int)w || 
          msk8->rows()    != (unsigned int)h  )
        G_THROW( ERR_MSG("c44.different_size") );
    }
  return msk8;
}


static void 
create_photo_djvu_file(IW44Image &iw, int w, int h,
                       IFFByteStream &iff, int nchunks, IWEncoderParms parms[])
{
  // Prepare info chunk
  GP<DjVuInfo> ginfo=DjVuInfo::create();
  DjVuInfo &info=*ginfo;
  info.width = w;
  info.height = h;
  info.dpi = (flag_dpi>0 ? flag_dpi : 100);
  info.gamma = (flag_gamma>0 ? flag_gamma : 2.2);
  // Write djvu header and info chunk
  iff.put_chunk("FORM:DJVU", 1);
  iff.put_chunk("INFO");
  info.encode(*iff.get_bytestream());
  iff.close_chunk();
  // Write all chunks
  int flag = 1;
  for (int i=0; flag && i<nchunks; i++)
    {
      iff.put_chunk("BG44");
      flag = iw.encode_chunk(iff.get_bytestream(), parms[i]);
      iff.close_chunk();
    }
  // Close djvu chunk
  iff.close_chunk();
}


int
main(int argc, char **argv)
{
  setlocale(LC_ALL,"");
  djvu_programname(argv[0]);
  DArray<GUTF8String> dargv(0,argc-1);
  for(int i=0;i<argc;++i)
    dargv[i]=GNativeString(argv[i]);
  G_TRY
    {
      // Parse arguments
      parse(dargv);
      // Check input file
      GP<ByteStream> gibs=ByteStream::create(pnmurl,"rb");
      ByteStream &ibs=*gibs;
      char prefix[16];
      if (ibs.readall((void*)prefix, sizeof(prefix)) != sizeof(prefix))
        G_THROW( ERR_MSG("c44.failed_pnm_header") );
      if(percent.length())
        {
          parse_size(percent,gibs->size());
        }
      else if(prefix[0]!='P' &&prefix[0]!='A' && prefix[0]!='F' && !flag_mask && !flag_bpp && !flag_size && !flag_slice && !flag_decibel)
        {
          parse_size("10,20,30,50",gibs->size());
        }
      // Load images
      int w = 0;
      int h = 0;
      ibs.seek(0);
      GP<IW44Image> iw;
      // Check color vs gray
      if (prefix[0]=='P' && (prefix[1]=='2' || prefix[1]=='5'))
        {
          // gray file
          GP<GBitmap> gibm=GBitmap::create(ibs);
          GBitmap &ibm=*gibm;
          w = ibm.columns();
          h = ibm.rows();
          iw = IW44Image::create_encode(ibm, getmask(w,h));
        }
      else if (!GStringRep::cmp(prefix,"AT&TFORM",8) || !GStringRep::cmp(prefix,"FORM",4))
        {
          char *s = (prefix[0]=='F' ? prefix+8 : prefix+12);
          GP<IFFByteStream> giff=IFFByteStream::create(gibs);
          IFFByteStream &iff=*giff;
          const bool color=!GStringRep::cmp(s,"PM44",4);
          if (color || !GStringRep::cmp(s,"BM44",4))
            {
              iw = IW44Image::create_encode(IW44Image::COLOR);
              iw->decode_iff(iff);
              w = iw->get_width();
              h = iw->get_height();
            }
          else
            G_THROW( ERR_MSG("c44.unrecognized") );
          // Check that no mask has been specified.
          if (! mskurl.is_empty())
            G_THROW( ERR_MSG("c44.failed_mask") );
        }
      else  // just for kicks, try jpeg.
        {
          // color file
          GP<GPixmap> gipm=JPEGDecoder::decode(ibs);
          GPixmap &ipm=*gipm;
          w = ipm.columns();
          h = ipm.rows();
          iw = IW44Image::create_encode(ipm, getmask(w,h), arg_crcbmode);
        }
      // Call destructor on input file
      gibs=0;
              
      // Perform compression PM44 or BM44 as required
      if (iw)
        {
          iw4url.deletefile();
          GP<IFFByteStream> iff=IFFByteStream::create(ByteStream::create(iw4url,"wb"));
          if (flag_crcbdelay >= 0)
            iw->parm_crcbdelay(flag_crcbdelay);
          if (flag_dbfrac > 0)
            iw->parm_dbfrac((float)flag_dbfrac);
          int nchunk = resolve_quality(w*h);
          if (flag_gamma>0 || flag_dpi>0)
            // Create djvu file
            create_photo_djvu_file(*iw, w, h, *iff, nchunk, parms);
          else
            // Create iw44 file
            iw->encode_iff(*iff, nchunk, parms);
        }
    }
  G_CATCH(ex)
    {
      ex.perror( ERR_MSG("Error") );
      exit(1);
    }
  G_ENDCATCH;
  return 0;
}
