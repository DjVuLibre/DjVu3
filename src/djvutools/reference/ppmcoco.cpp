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
//C- $Id: ppmcoco.cpp,v 1.1 2000-09-21 21:14:00 mrosen Exp $

/** @name ppmcoco

    {\bf Synopsis}
    \begin{verbatim}
        ppmcoco [-from <gamma>] [-to <gamma>] [<ppmin>] [<ppmout>]
    \end{verbatim}

    {\bf Arguments}\\
    \begin{description}
    \item[-from <gamma>]  
    Gamma coefficient of the display for which the image was designed.
    The default value 2.2 is assumed when this argument is omitted.
    \item[-to <gamma>]  
    Gamma coefficient of the display on the image will be rendered.
    The default value 2.2 is assumed when this argument is omitted.
    \item[<ppmin>]
    Name of the PPM file to read. A single dash (#"-"#) means that the PPM
    file is to be read on the standard input.  See 
    \Ref{PNM and RLE file formats} for more information about PPM files.
    \item[<ppmout>]
    Name of the PPM file into which the color corrected image will be written.
    Omitting this argument or providing a single dash (#"-"#) means that the
    PPM data will be written to the standard output.
    \end{description}
    

    {\bf Description} ---
    File #"ppmcoco.cpp"# illustrates how the DjVu Reference Library performs
    color correction on \Ref{GPixmap} objects.  Each color image is in theory
    designed to be rendered on a particular device.  In order to display this
    image on a different device, we need to correct the image in order to
    compensate the differences between the color profile of the {\em design
    device} (the device for which the image was designed) and the color
    profile of the {\em rendering device} (the device on which the image will
    be displayed.)

    The current release of the DjVu Reference Library only implements gamma
    correction.  Cathodic displays are reasonably well characterized by a
    single floating point number called ``gamma''.  Typical gamma values 
    are given in the table below:
    \begin{center}\begin{tabular}{ll}
    {\bf computer} & {\bf gamma}\\
        PC                &  2.5 \\
        Apple Macintosh   &  1.9 \\
        Silicon Graphics  &  1.4
    \end{tabular}\end{center}

    Gamma correction is performed by function \Ref{GPixmap::color_correct} in
    class \Ref{GPixmap}.  The argument of function #color_correct# must be the
    ratio between the gamma coefficient of the rendering device and the gamma
    coefficient of the design device.  This function works best when both
    gamma coefficients are close, partly because the color values have a
    limited dynamical range, and partly because we do not exactly perform a
    regular gamma correction: regular gamma correction tends to reveal ugly
    data compression artifacts in the dark areas of the image.  We suggest
    therefore to design images with a intermediate gamma coefficient.  Value
    2.2 gives decent results on all computers.  This value is suggested by
    several photo editing programs (such as Photoshop), and is
    supported by virtually all scanner drivers.

    Program #ppmcoco# reads a PPM file #ppmin# containing an image designed
    for a display whose gamma coefficient is specified with option #-from#.
    Color correction is then performed for a display whose gamma coefficient
    is specified with option #-to#.  The corrected file is then written into
    PPM file #ppmout#.

    @memo
    Perform color correction on PPM files.
    @author
    L\'eon Bottou <leonb@research.att.com>
    @version
    #$Id: ppmcoco.cpp,v 1.1 2000-09-21 21:14:00 mrosen Exp $# */
//@{
//@}




#include <stdlib.h>
#include <stdio.h>
#include "GException.h"
#include "ByteStream.h"
#include "GPixmap.h"
#include "GString.h"

double fromGamma = 2.2;
double toGamma = 2.2;

int 
usage(void)
{
  fprintf(stderr,
          "PPMCOCO -- Color correction program\n"
          "  Copyright (c) AT&T 1999.  All rights reserved\n"
          "usage: ppmcoco [-from gamma] [-to gamma] [<ppmin>] [<ppmout>]\n" );
  exit(1);
}

double
str_to_gamma(const char *str)
{
  char *strend;
  double gamma = strtod(str, &strend);
  if (strend==str || *strend)
    G_THROW("incorrect gamma specification");
  if (gamma<0.1 || gamma>10)
    G_THROW("gamma out of range (0.25 , 5)");
  return gamma;
}


int 
main (int argc, char **argv)
{
  GString infile("-");
  GString outfile("-");
  G_TRY
    {
      // parse
      if (argc==1)
        usage();
      int flag = 0;
      for (int i=1; i<argc; i++)
        {
          if (!strcmp(argv[i],"-from") && i+1<argc)
            {
          fromGamma = str_to_gamma(argv[++i]);
            }
          else if (!strcmp(argv[i],"-to") && i+1<argc)
            {
              toGamma = str_to_gamma(argv[++i]);
            }
          else if (flag==0)
            {
              flag = 1;
              infile = argv[i];
            }
          else if (flag == 1)
            {
              flag = 2;
              outfile = argv[i];
            }
          else
            usage();
        }
      // compute
      double gamma_correction = toGamma / fromGamma;
      if (gamma_correction<0.1)
        gamma_correction = 0.1;
      else if (gamma_correction>10)
        gamma_correction = 10;
      if (gamma_correction<0.2 || gamma_correction>5)
        fprintf(stderr,"warning: strong correction reduces image quality\n");
      // perform
      GPixmap pm;
      StdioByteStream ibs(infile,"rb"); 
      pm.init(ibs); 
      pm.color_correct(gamma_correction);
      StdioByteStream obs(outfile,"wb"); 
      pm.save_ppm(obs); 
    }
  G_CATCH(ex)
    {
      ex.perror();
      exit(1);
    }
  G_ENDCATCH;
  return 0;
}

