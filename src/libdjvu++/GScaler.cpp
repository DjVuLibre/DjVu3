//C-  -*- C++ -*-
//C-
//C-  Copyright (c) 1988 AT&T	
//C-  All Rights Reserved 
//C-
//C-  THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
//C-  The copyright notice above does not evidence any
//C-  actual or intended publication of such source code.
//C-
//C-  $Id: GScaler.cpp,v 1.1 1999-01-22 00:40:19 leonb Exp $

// Rescale images with fast bilinear interpolation
// $Id: GScaler.cpp,v 1.1 1999-01-22 00:40:19 leonb Exp $

#include "GScaler.h"


////////////////////////////////////////
// CONSTANTS


#define FRACBITS  4
#define FRACSIZE  (1<<FRACBITS)
#define FRACSIZE2 (FRACSIZE>>1)
#define FRACMASK  (FRACSIZE-1)






////////////////////////////////////////
// UTILITIES


static int interp_ok = 0;
static short interp[FRACSIZE][512];

static void
prepare_interp()
{
  if (! interp_ok)
    {
      interp_ok = 1;
      for (int i=0; i<FRACSIZE; i++)
        {
          short *deltas = & interp[i][256];
          for (int j = -255; j <= 255; j++)
            deltas[j] = ( j*i + FRACSIZE2 ) >> FRACBITS;
        }
    }
}


static inline int
mini(int x, int y) 
{ 
  return (x < y ? x : y);
}


static inline int
maxi(int x, int y) 
{ 
  return (x > y ? x : y);
}






////////////////////////////////////////
// GSCALER


GScaler::GScaler()
  : inw(0), inh(0), 
    xshift(0), yshift(0), redw(0), redh(0), 
    outw(0), outh(0),
    vcoord(0), hcoord(0)
{
}


GScaler::~GScaler()
{
  delete [] vcoord;
  delete [] hcoord;
  vcoord = hcoord = 0;
}


void
GScaler::set_input_size(int w, int h)
{ 
  inw = w;
  inh = h;
  if (vcoord) delete [] vcoord;
  if (hcoord) delete [] hcoord;
  vcoord = hcoord = 0;
}


void
GScaler::set_output_size(int w, int h)
{ 
  outw = w;
  outh = h;
  if (vcoord) delete [] vcoord;
  if (hcoord) delete [] hcoord;
  vcoord = hcoord = 0;
}


static void
prepare_coord(int *coord, int inmax, int outmax, int in, int out)
{
  int len = (in*FRACSIZE);
  int beg = (len+out)/(2*out) - FRACSIZE2;
  // Bresenham algorithm
  int y = beg;
  int z = out/2;
  int inmaxlim = (inmax-1)*FRACSIZE;
  for  (int x=0; x<outmax; x++)
    {
      coord[x] = mini(y,inmaxlim);
      z = z + len;
      y = y + z / out;  
      z = z % out;
    }
  // Result must fit exactly
  if (out==outmax && y!=beg+len)
    THROW("Assertion failed");
}


void 
GScaler::set_horz_ratio(int numer, int denom)
{
  if (! (inw>0 && inh>0 && outw>0 && outh>0))
    THROW("GScaler input_size and output_size are still undefined.");
  // Implicit ratio (determined by the input/output sizes)
  if (numer==0 && denom==0) {
    numer = outw;
    denom = inw;
  } else if (numer<=0 || denom<=0)
    THROW("GScaler ratios must be both null or both positive");
  // Compute horz reduction
  xshift = 0;
  redw = inw;
  while (numer+numer < denom) {
    xshift += 1;
    redw = (redw + 1) >> 1;
   numer = numer << 1;
  }
  // Compute coordinate table
  if (! hcoord) hcoord = new int[outw];
  prepare_coord(hcoord, redw, outw, denom, numer);
}


void 
GScaler::set_vert_ratio(int numer, int denom)
{
  if (! (inw>0 && inh>0 && outw>0 && outh>0))
    THROW("GScaler input_size and output_size are still undefined.");
  // Implicit ratio (determined by the input/output sizes)
  if (numer==0 && denom==0) {
    numer = outh;
    denom = inh;
  } else if (numer<=0 || denom<=0)
    THROW("GScaler ratios must be both null or both positive");
  // Compute horz reduction
  yshift = 0;
  redh = inh;
  while (numer+numer < denom) {
    yshift += 1;
    redh = (redh + 1) >> 1;
    numer = numer << 1;
  }
  // Compute coordinate table
  if (! vcoord) vcoord = new int[outh];
  prepare_coord(vcoord, redh, outh, denom, numer);
}


void
GScaler::make_rectangles(const GRect &desired, GRect &red, GRect &inp)
{
  // Parameter validation
  if (desired.xmin<0 || desired.ymin<0 ||
      desired.xmax>outw || desired.ymax>outh )
    THROW("Desired output exceeds full image output rect");
  // Compute ratio (if not done yet)
  if (!vcoord) 
    set_vert_ratio(0,0);
  if (!hcoord) 
    set_horz_ratio(0,0);
  // Compute reduced bounds
  red.xmin = (hcoord[desired.xmin]) >> FRACBITS;
  red.ymin = (vcoord[desired.ymin]) >> FRACBITS;
  red.xmax = (hcoord[desired.xmax-1]+FRACSIZE-1) >> FRACBITS;
  red.ymax = (vcoord[desired.ymax-1]+FRACSIZE-1) >> FRACBITS;
  // Borders
  red.xmin = maxi(red.xmin, 0);
  red.xmax = mini(red.xmax+1, redw);
  red.ymin = maxi(red.ymin, 0);
  red.ymax = mini(red.ymax+1, redh);
  // Input
  inp.xmin = maxi(red.xmin<<xshift, 0); 
  inp.xmax = mini(red.xmax<<xshift, inw); 
  inp.ymin = maxi(red.ymin<<yshift, 0); 
  inp.ymax = mini(red.ymax<<yshift, inh); 
}


void 
GScaler::get_input_rect( const GRect &desired_output, GRect &required_input )
{
  GRect red;
  make_rectangles(desired_output, red, required_input);
}






////////////////////////////////////////
// GBITMAPSCALER


GBitmapScaler::GBitmapScaler()
  : lbuffer(0), p1(0), p2(0)
{
}


GBitmapScaler::GBitmapScaler(int inw, int inh, int outw, int outh)
  : lbuffer(0), conv(0), p1(0), p2(0)
{
  set_input_size(inw, inh);
  set_output_size(outw, outh);
}


GBitmapScaler::~GBitmapScaler()
{
  delete [] p1;
  delete [] p2;
  delete [] lbuffer;
  delete [] conv;
  p1 = p2 = lbuffer = 0;
}


unsigned char *
GBitmapScaler::get_line(int fy, 
                        const GRect &required_red, 
                        const GRect &provided_input,
                        const GBitmap &input )
{
  if (fy < required_red.ymin)
    fy = required_red.ymin; 
  else if (fy >= required_red.ymax)
    fy = required_red.ymax - 1;
  // Cached line
  if (fy == l2)
    return p2;
  if (fy == l1)
    return p1;
  // Shift
  unsigned char *p = p1;
  p1 = p2;
  l1 = l2;
  p2 = p;
  l2 = fy;
  if (xshift==0 && yshift==0)
    {
      // Fast mode
      int dx = required_red.xmin-provided_input.xmin;
      int dx1 = required_red.xmax-provided_input.xmin;
      const unsigned char *inp1 = input[fy-provided_input.ymin] + dx;
      while (dx++ < dx1)
        *p++ = conv[*inp1++];
      return p2;
    }
  else
    {
      // Compute location of line
      GRect line;
      line.xmin = required_red.xmin << xshift;
      line.xmax = required_red.xmax << xshift;
      line.ymin = fy << yshift;
      line.ymax = (fy+1) << yshift;
      line.intersect(line, provided_input);
      line.translate(-provided_input.xmin, -provided_input.ymin);
      // Prepare variables
      const unsigned char *botline = input[line.ymin];
      int rowsize = input.rowsize();
      int sw = 1<<xshift;
      int div = xshift+yshift;
      int rnd = 1<<(div-1);
      // Compute averages
      for (int x=line.xmin; x<line.xmax; x+=sw,p++)
        {
          int g=0, s=0;
          const unsigned char *inp0 = botline + x;
          int sy1 = mini(line.height(), (1<<yshift));
          for (int sy=0; sy<sy1; sy++,inp0+=rowsize)
        {
          const unsigned char *inp1 = inp0;
          int sx1 = mini(x+sw, line.xmax);
          for (int sx=x; sx<sx1; s++,sx++,inp1++)
            g += conv[*inp1];
        }
          if (s == rnd+rnd)
            *p = (g+rnd)>>div;
          else
            *p = (g+s/2)/s;
        }
      // Return
      return p2;
    }
}


void 
GBitmapScaler::scale( const GRect &provided_input, const GBitmap &input,
                      const GRect &desired_output, GBitmap &output )
{
  // Compute rectangles
  GRect required_input; 
  GRect required_red;
  make_rectangles(desired_output, required_red, required_input);
  // Parameter validation
  if (provided_input.width() != (int)input.columns() ||
      provided_input.height() != (int)input.rows() )
    THROW("Input pixmap does not match input rectangle");
  if (provided_input.xmin > required_input.xmin ||
      provided_input.ymin > required_input.ymin ||
      provided_input.xmax < required_input.xmax ||
      provided_input.ymax < required_input.ymax  )
    THROW("Provided input pixmap is too small");
  // Adjust output pixmap
  if (desired_output.width() != (int)output.columns() ||
      desired_output.height() != (int)output.rows() )
    output.init(desired_output.height(), desired_output.width());
  output.set_grays(256);
  // Prepare temp stuff
  delete [] p1;
  delete [] p2;
  delete [] lbuffer;
  p1 = p2 = lbuffer = 0;
  prepare_interp();
  int bufw = required_red.width();
  lbuffer = new unsigned char[bufw+2];
  p1 = new unsigned char[bufw];
  p2 = new unsigned char[bufw];
  l1 = l2 = -1;
  // Prepare gray conversion array (conv)
  delete [] conv;
  conv = new unsigned char[256];
  int maxgray = input.get_grays()-1;
  for (int i=0; i<256; i++) 
    {
      if (i<= maxgray)
        conv[i] = ((i*255) + (maxgray>>1)) / maxgray;
      else
        conv[i] = 255;
    }
  // Loop on output lines
  for (int y=desired_output.ymin; y<desired_output.ymax; y++)
    {
      // Perform vertical interpolation
      {
        int fy = vcoord[y];
        int fy1 = fy>>FRACBITS;
        int fy2 = fy1+1;
        const unsigned char *lower, *upper;
        // Obtain upper and lower line in reduced image
        lower = get_line(fy1, required_red, provided_input, input);
        upper = get_line(fy2, required_red, provided_input, input);
        // Compute line
        int npix = bufw;
        unsigned char *dest = lbuffer+1;
        const short *deltas = & interp[fy&FRACMASK][256];
        while (--npix >= 0)
          {
            int l = *lower;
            int u = *upper;
            *dest = l + deltas[u-l];
            upper += 1;
            lower += 1;
            dest  += 1;
          }
      }
      // Perform horizontal interpolation
      {
        // Prepare for side effects
        lbuffer[0]   = lbuffer[1];
        lbuffer[bufw] = lbuffer[bufw];
        unsigned char *line = lbuffer+1-required_red.xmin;
        unsigned char *dest  = output[y-desired_output.ymin];
        // Loop horizontally
        for (int x=desired_output.xmin; x<desired_output.xmax; x++)
          {
            int n = hcoord[x];
            const unsigned char *lower = line + (n>>FRACBITS);
            const short *deltas = &interp[n&FRACMASK][256];
            int l = lower[0];
            int u = lower[1];
            *dest = l + deltas[u-l];
            dest++;
          }
      }
    }
  // Free temporaries
  delete [] p1;
  delete [] p2;
  delete [] lbuffer;
  delete [] conv;
  p1 = p2 = lbuffer = conv = 0;
}






////////////////////////////////////////
// GPIXMAPSCALER


GPixmapScaler::GPixmapScaler()
  : lbuffer(0), p1(0), p2(0)
{
}


GPixmapScaler::GPixmapScaler(int inw, int inh, int outw, int outh)
  : lbuffer(0), p1(0), p2(0)
{
  set_input_size(inw, inh);
  set_output_size(outw, outh);
}


GPixmapScaler::~GPixmapScaler()
{
  delete [] p1;
  delete [] p2;
  delete [] lbuffer;
  p1 = p2 = lbuffer = 0;
}


GPixel *
GPixmapScaler::get_line(int fy, 
                        const GRect &required_red, 
                        const GRect &provided_input,
                        const GPixmap &input )
{
  if (fy < required_red.ymin)
    fy = required_red.ymin; 
  else if (fy >= required_red.ymax)
    fy = required_red.ymax - 1;
  // Cached line
  if (fy == l2)
    return p2;
  if (fy == l1)
    return p1;
  // Shift
  GPixel *p = p1;
  p1 = p2;
  l1 = l2;
  p2 = p;
  l2 = fy;
  // Compute location of line
  GRect line;
  line.xmin = required_red.xmin << xshift;
  line.xmax = required_red.xmax << xshift;
  line.ymin = fy << yshift;
  line.ymax = (fy+1) << yshift;
  line.intersect(line, provided_input);
  line.translate(-provided_input.xmin, -provided_input.ymin);
  // Prepare variables
  const GPixel *botline = input[line.ymin];
  int rowsize = input.rowsize();
  int sw = 1<<xshift;
  int div = xshift+yshift;
  int rnd = 1<<(div-1);
  // Compute averages
  for (int x=line.xmin; x<line.xmax; x+=sw,p++)
    {
      int r=0, g=0, b=0, s=0;
      const GPixel *inp0 = botline + x;
      int sy1 = mini(line.height(), (1<<yshift));
      for (int sy=0; sy<sy1; sy++,inp0+=rowsize)
        {
          const GPixel *inp1 = inp0;
          int sx1 = mini(x+sw, line.xmax);
          for (int sx=x; sx<sx1; sx++,inp1++)
            {
              r += inp1->r;  
              g += inp1->g;  
              b += inp1->b; 
              s += 1;
            }
        }
      if (s == rnd+rnd)
        {
          p->r = (r+rnd) >> div;
          p->g = (g+rnd) >> div;
          p->b = (b+rnd) >> div;
        }
      else
        {
          p->r = (r+s/2)/s;
          p->g = (g+s/2)/s;
          p->b = (b+s/2)/s;
        }
    }
  // Return
  return p2;
}


void 
GPixmapScaler::scale( const GRect &provided_input, const GPixmap &input,
                      const GRect &desired_output, GPixmap &output )
{
  // Compute rectangles
  GRect required_input; 
  GRect required_red;
  make_rectangles(desired_output, required_red, required_input);
  // Parameter validation
  if (provided_input.width() != (int)input.columns() ||
      provided_input.height() != (int)input.rows() )
    THROW("Input pixmap does not match input rectangle");
  if (provided_input.xmin > required_input.xmin ||
      provided_input.ymin > required_input.ymin ||
      provided_input.xmax < required_input.xmax ||
      provided_input.ymax < required_input.ymax  )
    THROW("Provided input pixmap is too small");
  // Adjust output pixmap
  if (desired_output.width() != (int)output.columns() ||
      desired_output.height() != (int)output.rows() )
    output.init(desired_output.height(), desired_output.width());
  // Prepare temp stuff
  delete [] p1;
  delete [] p2;
  delete [] lbuffer;
  p1 = p2 = lbuffer = 0;
  prepare_interp();
  int bufw = required_red.width();
  lbuffer = new GPixel[bufw+2];
  if (xshift>0 || yshift>0)
    {
      p1 = new GPixel[bufw];
      p2 = new GPixel[bufw];
      l1 = l2 = -1;
    }
  // Loop on output lines
  for (int y=desired_output.ymin; y<desired_output.ymax; y++)
    {
      // Perform vertical interpolation
      {
        int fy = vcoord[y];
        int fy1 = fy>>FRACBITS;
        int fy2 = fy1+1;
        const GPixel *lower, *upper;
        // Obtain upper and lower line in reduced image
        if (xshift>0 || yshift>0)
          {
            lower = get_line(fy1, required_red, provided_input, input);
            upper = get_line(fy2, required_red, provided_input, input);
          }
        else
          {
            int dx = required_red.xmin-provided_input.xmin;
            fy1 = maxi(fy1, required_red.ymin);
            fy2 = mini(fy2, required_red.ymax-1);
            lower = input[fy1-provided_input.ymin] + dx;
            upper = input[fy2-provided_input.ymin] + dx;
          }
        // Compute line
        int npix = bufw;
        GPixel *dest = lbuffer+1;
        const short *deltas = & interp[fy&FRACMASK][256];
        while (--npix >= 0)
          {
            // Optimizer will reorder/schedule
            int lower_r = lower->r;
            int lower_g = lower->g;
            int lower_b = lower->b;
            int delta_r = deltas[(int)upper->r - lower_r];
            int delta_g = deltas[(int)upper->g - lower_g];
            int delta_b = deltas[(int)upper->b - lower_b];
            dest->r = lower_r + delta_r;
            dest->g = lower_g + delta_g;
            dest->b = lower_b + delta_b;
            // Next pixels
            upper += 1;
            lower += 1;
            dest  += 1;
          }
      }
      // Perform horizontal interpolation
      {
        // Prepare for side effects
        lbuffer[0]   = lbuffer[1];
        lbuffer[bufw] = lbuffer[bufw];
        GPixel *line = lbuffer+1-required_red.xmin;
        GPixel *dest  = output[y-desired_output.ymin];
        // Loop horizontally
        for (int x=desired_output.xmin; x<desired_output.xmax; x++)
          {
            int n = hcoord[x];
            const GPixel *lower = line + (n>>FRACBITS);
            const short *deltas = &interp[n&FRACMASK][256];
            // Optimizer will reorder/schedule
            int lower_r = lower[0].r;
            int lower_g = lower[0].g;
            int lower_b = lower[0].b;
            int delta_r = deltas[(int)lower[1].r - lower_r];
            int delta_g = deltas[(int)lower[1].g - lower_g];
            int delta_b = deltas[(int)lower[1].b - lower_b];
            dest->r = lower_r + delta_r;
            dest->g = lower_g + delta_g;
            dest->b = lower_b + delta_b;
            dest++;
          }
      }
    }
  // Free temporaries
  delete [] p1;
  delete [] p2;
  delete [] lbuffer;
  p1 = p2 = lbuffer = 0;
}


