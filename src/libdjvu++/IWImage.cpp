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
//C- $Id: IWImage.cpp,v 1.1.1.1 1999-10-22 19:29:25 praveen Exp $

// File "$Id: IWImage.cpp,v 1.1.1.1 1999-10-22 19:29:25 praveen Exp $"
// - Author: Leon Bottou, 08/1998

#ifdef __GNUC__
#pragma implementation
#endif

#include <assert.h>
#include <string.h>
#include <math.h>
#include "GRect.h"
#include "GException.h"
#include "GSmartPointer.h"
#include "ZPCodec.h"
#include "IWImage.h"
#include "IWTransform.h"

#define IWALLOCSIZE    4080
#define IWCODEC_MAJOR     1
#define IWCODEC_MINOR     2
#define DECIBEL_PRUNE   5.0


//////////////////////////////////////////////////////
// WAVELET DECOMPOSITION CONSTANTS
//////////////////////////////////////////////////////

// Parameters for IW44 wavelet.
// - iw_quant: quantization for all 16 sub-bands
// - iw_norm: norm of all wavelets (for db estimation)
// - iw_border: pixel border required to run filters
// - iw_shift: scale applied before decomposition


static int iw_quant[16] = {
  0x004000, 
  0x008000, 0x008000, 0x010000,
  0x010000, 0x010000, 0x020000,
  0x020000, 0x020000, 0x040000,
  0x040000, 0x040000, 0x080000, 
  0x040000, 0x040000, 0x080000
};

static float iw_norm[16] = {
  2.627989e+03F,
  1.832893e+02F, 1.832959e+02F, 5.114690e+01F,
  4.583344e+01F, 4.583462e+01F, 1.279225e+01F,
  1.149671e+01F, 1.149712e+01F, 3.218888e+00F,
  2.999281e+00F, 2.999476e+00F, 8.733161e-01F,
  1.074451e+00F, 1.074511e+00F, 4.289318e-01F
};

static const int iw_border = 3;
static const int iw_shift  = 6;
static const int iw_round  = (1<<(iw_shift-1));




//////////////////////////////////////////////////////
// MASKING DECOMPOSITION
//////////////////////////////////////////////////////

#ifndef NEED_DECODER_ONLY

//----------------------------------------------------
// Function for applying bidimensional IW44 between 
// scale intervals begin(inclusive) and end(exclusive)
// with a MASK bitmap


static void
interpolate_mask(short *data16, int w, int h, int rowsize,
                 const signed char *mask8, int mskrowsize)
{
  int i,j;
  // count masked bits
  short *count = new short[h*w];
  short *cp = count;
  for (i=0; i<h; i++, cp+=w, mask8+=mskrowsize)
    for (j=0; j<w; j++)
      cp[j] = (mask8[j] ? 0 : 0x1000);
  // copy image
  short *sdata = new short[w*h];
  short *p = sdata;
  short *q = data16;
  for (i=0; i<h; i++, p+=w, q+=rowsize)
    for (j=0; j<w; j++)
      p[j] = q[j];
  // iterate over resolutions
  int split = 1;
  int scale = 2;
  int again = 1;
  while (again && scale<w && scale<h)
    {
      again = 0;
      p = data16;
      q = sdata;
      cp = count;
      // iterate over block
      for (i=0; i<h; i+=scale, cp+=w*scale, q+=w*scale, p+=rowsize*scale)
        for (j=0; j<w; j+=scale)
          {
            int ii, jj;
            int gotz = 0;
            int gray = 0;
            int npix = 0;
            short *cpp = cp;
            short *qq = q;
            // look around when square goes beyond border
            int istart = i;
            if (istart+split>h)
              {
                istart -= scale;
                cpp -= w*scale;
                qq -= w*scale;
              }
            int jstart = j;
            if (jstart+split>w)
              jstart -= scale;
            // compute gray level
            for (ii=istart; ii<i+scale && ii<h; ii+=split, cpp+=w*split, qq+=w*split)
              for (jj=jstart; jj<j+scale && jj<w; jj+=split)
                {
                  if (cpp[jj]>0) 
                    {
                      npix += cpp[jj];
                      gray += cpp[jj] * qq[jj];
                    } 
                  else if (ii>=i && jj>=j)
                    {
                      gotz = 1;
                    }
                }
            // process result
            if (npix == 0)
              {
                // continue to next resolution
                again = 1;
                cp[j] = 0;
              }
            else
              {
                gray = gray / npix;
                // check whether initial image require fix
                if (gotz)
                  {
                    cpp = cp;
                    qq = p;
                    for (ii=i; ii<i+scale && ii<h; ii+=1, cpp+=w, qq+=rowsize)
                      for (jj=j; jj<j+scale && jj<w; jj+=1)
                        if (cpp[jj] == 0)
                          {
                            qq[jj] = gray;
                            cpp[jj] = 1;
                          }
                  }
                // store average for next iteration
                cp[j] = npix>>2;
                q[j] = gray;
              }
          }
      // double resolution
      split = scale;
      scale = scale+scale;
    }
  // free memory
  delete [] count;
  delete [] sdata;
}


static void
forward_mask(short *data16, int w, int h, int rowsize, int begin, int end,
             const signed char *mask8, int mskrowsize )
{
  int i,j;
  signed char *m;
  short *p;
  short *d;
  // Allocate buffers
  short *sdata = new short[h*w];
  signed char *smask = new signed char[h*w];
  // Copy mask
  m = smask;
  for (i=0; i<h; i+=1, m+=w, mask8+=mskrowsize)
    memcpy((void*)m, (void*)mask8, w);
  // Loop over scale
  for (int scale=begin; scale<end; scale<<=1)
    {
      // Copy data into sdata buffer
      p = data16;
      d = sdata;
      for (i=0; i<h; i+=scale)
        {
          for (j=0; j<w; j+=scale)
            d[j] = p[j];
          p += rowsize * scale;
          d += w * scale;
        }
      // Decompose
      IWTransform::forward(sdata, w, h, w, scale, scale+scale);
      // Cancel masked coefficients
      d = sdata;
      m = smask;
      for (i=0; i<h; i+=scale+scale)
        {
          for (j=scale; j<w; j+=scale+scale)
            if (m[j])
              d[j] = 0;
          d += w * scale;
          m += w * scale;
          if (i+scale < h)
            {
              for (j=0; j<w; j+=scale)
                if (m[j])
                  d[j] = 0;
              d += w * scale;
              m += w * scale;
            }
        }
      // Reconstruct
      IWTransform::backward(sdata, w, h, w, scale+scale, scale);
      // Correct visible pixels
      p = data16;
      d = sdata;
      m = smask;
      for (i=0; i<h; i+=scale)
        {
          for (j=0; j<w; j+=scale)
            if (! m[j])
              d[j] = p[j];
          p += rowsize*scale;
          m += w*scale;
          d += w*scale;
        }
      // Decompose again (no need to iterate actually!)
      IWTransform::forward(sdata, w, h, w, scale, scale+scale);
      // Copy coefficients from sdata buffer
      p = data16;
      d = sdata;
      for (i=0; i<h; i+=scale)
        {
          for (j=0; j<w; j+=scale)
            p[j] = d[j];
          p += rowsize * scale;
          d += w * scale;
        }
      // Compute new mask for next scale
      m = smask;
      signed char *m0 = m;
      signed char *m1 = m;
      for (i=0; i<h; i+=scale+scale)
        {
          m0 = m1;
          if (i+scale < h)
            m1 = m + w*scale;
          for (j=0; j<w; j+=scale+scale)
            if (m[j] && m0[j] && m1[j] && (j<=0 || m[j-scale]) && (j+scale>=w || m[j+scale]))
              m[j] = 1;
            else
              m[j] = 0;
          m = m1 + w*scale;
        }
    }
  // Free buffers
  delete [] sdata;
  delete [] smask;
}

#endif


//////////////////////////////////////////////////////
// REPRESENTATION OF WAVELET DECOMPOSED IMAGES
//////////////////////////////////////////////////////



//---------------------------------------------------------------
// Zig zag location in a 1024 liftblock.
// These numbers have been generated with the following program:
//
// int x=0, y=0;
// for (int i=0; i<5; i++) {
//   x = (x<<1) | (n&1);  n >>= 1;
//   y = (y<<1) | (n&1);  n >>= 1;
// }


static int zigzagloc[1024] = {
   0,  16, 512, 528,   8,  24, 520, 536, 256, 272, 768, 784, 264, 280, 776, 792,
   4,  20, 516, 532,  12,  28, 524, 540, 260, 276, 772, 788, 268, 284, 780, 796,
 128, 144, 640, 656, 136, 152, 648, 664, 384, 400, 896, 912, 392, 408, 904, 920,
 132, 148, 644, 660, 140, 156, 652, 668, 388, 404, 900, 916, 396, 412, 908, 924,
   2,  18, 514, 530,  10,  26, 522, 538, 258, 274, 770, 786, 266, 282, 778, 794,
   6,  22, 518, 534,  14,  30, 526, 542, 262, 278, 774, 790, 270, 286, 782, 798,
 130, 146, 642, 658, 138, 154, 650, 666, 386, 402, 898, 914, 394, 410, 906, 922,
 134, 150, 646, 662, 142, 158, 654, 670, 390, 406, 902, 918, 398, 414, 910, 926,
  64,  80, 576, 592,  72,  88, 584, 600, 320, 336, 832, 848, 328, 344, 840, 856,
  68,  84, 580, 596,  76,  92, 588, 604, 324, 340, 836, 852, 332, 348, 844, 860,
 192, 208, 704, 720, 200, 216, 712, 728, 448, 464, 960, 976, 456, 472, 968, 984,
 196, 212, 708, 724, 204, 220, 716, 732, 452, 468, 964, 980, 460, 476, 972, 988,
  66,  82, 578, 594,  74,  90, 586, 602, 322, 338, 834, 850, 330, 346, 842, 858,
  70,  86, 582, 598,  78,  94, 590, 606, 326, 342, 838, 854, 334, 350, 846, 862,
 194, 210, 706, 722, 202, 218, 714, 730, 450, 466, 962, 978, 458, 474, 970, 986,
 198, 214, 710, 726, 206, 222, 718, 734, 454, 470, 966, 982, 462, 478, 974, 990, // 255
   1,  17, 513, 529,   9,  25, 521, 537, 257, 273, 769, 785, 265, 281, 777, 793,
   5,  21, 517, 533,  13,  29, 525, 541, 261, 277, 773, 789, 269, 285, 781, 797,
 129, 145, 641, 657, 137, 153, 649, 665, 385, 401, 897, 913, 393, 409, 905, 921,
 133, 149, 645, 661, 141, 157, 653, 669, 389, 405, 901, 917, 397, 413, 909, 925,
   3,  19, 515, 531,  11,  27, 523, 539, 259, 275, 771, 787, 267, 283, 779, 795,
   7,  23, 519, 535,  15,  31, 527, 543, 263, 279, 775, 791, 271, 287, 783, 799,
 131, 147, 643, 659, 139, 155, 651, 667, 387, 403, 899, 915, 395, 411, 907, 923,
 135, 151, 647, 663, 143, 159, 655, 671, 391, 407, 903, 919, 399, 415, 911, 927,
  65,  81, 577, 593,  73,  89, 585, 601, 321, 337, 833, 849, 329, 345, 841, 857,
  69,  85, 581, 597,  77,  93, 589, 605, 325, 341, 837, 853, 333, 349, 845, 861,
 193, 209, 705, 721, 201, 217, 713, 729, 449, 465, 961, 977, 457, 473, 969, 985,
 197, 213, 709, 725, 205, 221, 717, 733, 453, 469, 965, 981, 461, 477, 973, 989,
  67,  83, 579, 595,  75,  91, 587, 603, 323, 339, 835, 851, 331, 347, 843, 859,
  71,  87, 583, 599,  79,  95, 591, 607, 327, 343, 839, 855, 335, 351, 847, 863,
 195, 211, 707, 723, 203, 219, 715, 731, 451, 467, 963, 979, 459, 475, 971, 987,
 199, 215, 711, 727, 207, 223, 719, 735, 455, 471, 967, 983, 463, 479, 975, 991, // 511
  32,  48, 544, 560,  40,  56, 552, 568, 288, 304, 800, 816, 296, 312, 808, 824,
  36,  52, 548, 564,  44,  60, 556, 572, 292, 308, 804, 820, 300, 316, 812, 828,
 160, 176, 672, 688, 168, 184, 680, 696, 416, 432, 928, 944, 424, 440, 936, 952,
 164, 180, 676, 692, 172, 188, 684, 700, 420, 436, 932, 948, 428, 444, 940, 956,
  34,  50, 546, 562,  42,  58, 554, 570, 290, 306, 802, 818, 298, 314, 810, 826,
  38,  54, 550, 566,  46,  62, 558, 574, 294, 310, 806, 822, 302, 318, 814, 830,
 162, 178, 674, 690, 170, 186, 682, 698, 418, 434, 930, 946, 426, 442, 938, 954,
 166, 182, 678, 694, 174, 190, 686, 702, 422, 438, 934, 950, 430, 446, 942, 958,
  96, 112, 608, 624, 104, 120, 616, 632, 352, 368, 864, 880, 360, 376, 872, 888,
 100, 116, 612, 628, 108, 124, 620, 636, 356, 372, 868, 884, 364, 380, 876, 892,
 224, 240, 736, 752, 232, 248, 744, 760, 480, 496, 992,1008, 488, 504,1000,1016,
 228, 244, 740, 756, 236, 252, 748, 764, 484, 500, 996,1012, 492, 508,1004,1020,
  98, 114, 610, 626, 106, 122, 618, 634, 354, 370, 866, 882, 362, 378, 874, 890,
 102, 118, 614, 630, 110, 126, 622, 638, 358, 374, 870, 886, 366, 382, 878, 894,
 226, 242, 738, 754, 234, 250, 746, 762, 482, 498, 994,1010, 490, 506,1002,1018,
 230, 246, 742, 758, 238, 254, 750, 766, 486, 502, 998,1014, 494, 510,1006,1022, // 767
  33,  49, 545, 561,  41,  57, 553, 569, 289, 305, 801, 817, 297, 313, 809, 825,
  37,  53, 549, 565,  45,  61, 557, 573, 293, 309, 805, 821, 301, 317, 813, 829,
 161, 177, 673, 689, 169, 185, 681, 697, 417, 433, 929, 945, 425, 441, 937, 953,
 165, 181, 677, 693, 173, 189, 685, 701, 421, 437, 933, 949, 429, 445, 941, 957,
  35,  51, 547, 563,  43,  59, 555, 571, 291, 307, 803, 819, 299, 315, 811, 827,
  39,  55, 551, 567,  47,  63, 559, 575, 295, 311, 807, 823, 303, 319, 815, 831,
 163, 179, 675, 691, 171, 187, 683, 699, 419, 435, 931, 947, 427, 443, 939, 955,
 167, 183, 679, 695, 175, 191, 687, 703, 423, 439, 935, 951, 431, 447, 943, 959,
  97, 113, 609, 625, 105, 121, 617, 633, 353, 369, 865, 881, 361, 377, 873, 889,
 101, 117, 613, 629, 109, 125, 621, 637, 357, 373, 869, 885, 365, 381, 877, 893,
 225, 241, 737, 753, 233, 249, 745, 761, 481, 497, 993,1009, 489, 505,1001,1017,
 229, 245, 741, 757, 237, 253, 749, 765, 485, 501, 997,1013, 493, 509,1005,1021,
  99, 115, 611, 627, 107, 123, 619, 635, 355, 371, 867, 883, 363, 379, 875, 891,
 103, 119, 615, 631, 111, 127, 623, 639, 359, 375, 871, 887, 367, 383, 879, 895,
 227, 243, 739, 755, 235, 251, 747, 763, 483, 499, 995,1011, 491, 507,1003,1019,
 231, 247, 743, 759, 239, 255, 751, 767, 487, 503, 999,1015, 495, 511,1007,1023, // 1023
};




//---------------------------------------------------------------
// *** Class _IWBlock [declaration]
// Represents a block of 32x32 coefficients after zigzagging and scaling


class _IWBlock // DJVU_CLASS
{
public:
  // creating
  _IWBlock();
  // accessing scaled coefficients
  short get(int n) const;
  void  set(int n, int val, _IWMap *map);
  // converting from liftblock
  void  read_liftblock(const short *coeff, _IWMap *map);
  void  write_liftblock(short *coeff, int bmin=0, int bmax=64) const;
  // sparse array access
  const short* data(int n) const;
  short* data(int n, _IWMap *map);
  void   zero(int n);
  // sparse representation
private:
  short **(pdata[4]);
};



//---------------------------------------------------------------
// *** Class _IWMap [declaration]
// Represents all the blocks of an image



struct _IWAlloc // DJVU_CLASS
{
  struct _IWAlloc *next;
  short data[IWALLOCSIZE];
};


class _IWMap // DJVU_CLASS
{
  // construction
public:
  _IWMap(int w, int h);
  ~_IWMap();
  // creation (from image)
  void create(const signed char *img8, int imgrowsize, 
              const signed char *msk8=0, int mskrowsize=0);
  // image access
  void image(signed char *img8, int rowsize, 
             int pixsep=1, int fast=0);
  void image(int subsample, const GRect &rect, 
             signed char *img8, int rowsize, 
             int pixsep=1, int fast=0);
  // slash resolution
  void slashres(int res);
  // array of blocks
  _IWBlock *blocks;
  // geometry
  int iw, ih;
  int bw, bh;
  int nb;
  // coefficient allocation stuff
  short *alloc(int n);
  short **allocp(int n);
  _IWAlloc *chain;
  int top;
  // statistics
  int get_bucket_count() const;
  unsigned int get_memory_usage() const;
};



//---------------------------------------------------------------
// *** Class _IWBlock [implementation]


_IWBlock::_IWBlock()
{
  pdata[0] = pdata[1] = pdata[2] = pdata[3] = 0;
}

inline const short* 
_IWBlock::data(int n) const
{
  if (! pdata[n>>4])
    return 0;
  return pdata[n>>4][n&15];
}

inline short* 
_IWBlock::data(int n, _IWMap *map)
{
  if (! pdata[n>>4])
    pdata[n>>4] = map->allocp(16);
  if (! pdata[n>>4][n &15])
    pdata[n>>4][n &15] = map->alloc(16);
  return pdata[n>>4][n&15];
}

void 
_IWBlock::zero(int n)
{
  if (pdata[n>>4])
    pdata[n>>4][n&15] = 0;
}

inline short 
_IWBlock::get(int n) const
{
  int n1 = (n>>4);
  const short *d = data(n1);
  if (! d)
    return 0;
  return d[n&15];
}

inline void  
_IWBlock::set(int n, int val, _IWMap *map)
{
  int n1 = (n>>4);
  short* d = data(n1, map);
  d[n&15] = val;
}


inline void  
_IWBlock::read_liftblock(const short *coeff, _IWMap *map)
{
  int n=0;
  for (int n1=0; n1<64; n1++)
    {
      short *d = data(n1,map);
      for (int n2=0; n2<16; n2++,n++)
        d[n2] = coeff[zigzagloc[n]];
    }
}

inline void  
_IWBlock::write_liftblock(short *coeff, int bmin, int bmax) const
{
  int n = bmin<<4;
  memset(coeff, 0, 1024*sizeof(short));
  for (int n1=bmin; n1<bmax; n1++)
    {
      const short *d = data(n1);
      if (d == 0)
        n += 16;
      else
        for (int n2=0; n2<16; n2++,n++)
          coeff[zigzagloc[n]] = d[n2];
    }
}



//---------------------------------------------------------------
// *** Class _IWMap [implementation]


_IWMap::_IWMap(int w, int h)
  :  blocks(0), iw(w), ih(h), chain(0)
{
  bw = (w+0x20-1) & ~0x1f;
  bh = (h+0x20-1) & ~0x1f;
  nb = (bw * bh) / (32 * 32);
  blocks = new _IWBlock[nb];
  top = IWALLOCSIZE;
}

_IWMap::~_IWMap()
{
  while (chain)
    {
      _IWAlloc *next = chain->next;
      delete chain;
      chain = next;
    }
  delete [] blocks;
}

short *
_IWMap::alloc(int n)
{
  if (top+n > IWALLOCSIZE)
    {
      _IWAlloc *n = new _IWAlloc;
      n->next = chain;
      chain = n;
      top = 0;
    }
  short *ans = chain->data + top;
  top += n;
  memset((void*)ans, 0, sizeof(short)*n);
  return ans;
}

short **
_IWMap::allocp(int n)
{
  // Allocate enough room for pointers plus alignment
  short *p = alloc( (n+1) * sizeof(short*) / sizeof(short) );
  // Align on pointer size
  while ( ((long)p) & (sizeof(short*)-1) )
    p += 1;
  // Cast and return
  return (short**)p;
}

int 
_IWMap::get_bucket_count() const
{
  int buckets = 0;
  for (int blockno=0; blockno<nb; blockno++)
    for (int buckno=0; buckno<64; buckno++)
      if (blocks[blockno].data(buckno))
        buckets += 1;
  return buckets;
}

unsigned int 
_IWMap::get_memory_usage() const
{
  unsigned int usage = sizeof(_IWMap);
  usage += sizeof(_IWBlock) * nb;
  for (_IWAlloc *n = chain; n; n=n->next)
    usage += sizeof(_IWAlloc);
  return usage;
}


#ifndef NEED_DECODER_ONLY
void 
_IWMap::create(const signed char *img8, int imgrowsize, 
               const signed char *msk8, int mskrowsize )
{
  int i, j;
  // Progress
  DJVU_PROGRESS_TASK(transf,3);
  // Allocate decomposition buffer
  short *data16 = new short[bw*bh];
  // Copy pixels
  short *p = data16;
  const signed char *row = img8;
  for (i=0; i<ih; i++)
    {
      for (j=0; j<iw; j++)
        *p++ = (int)(row[j]) << iw_shift;
      row += imgrowsize;
      for (j=iw; j<bw; j++)
        *p++ = 0;
    }
  for (i=ih; i<bh; i++)
    for (j=0; j<bw; j++)
      *p++ = 0;
  // Handle bitmask
  if (msk8)
    {
      // Interpolate pixels below mask
      DJVU_PROGRESS_RUN(transf, 1);
      interpolate_mask(data16, iw, ih, bw, msk8, mskrowsize);
      // Multiscale iterative masked decomposition
      DJVU_PROGRESS_RUN(transf, 3);
      forward_mask(data16, iw, ih, bw, 1, 32, msk8, mskrowsize);
    }
  else
    {
      // Perform traditional decomposition
      DJVU_PROGRESS_RUN(transf, 3);
      IWTransform::forward(data16, iw, ih, bw, 1, 32);
    }
  // Copy coefficient into blocks
  p = data16;
  _IWBlock *block = blocks;
  for (i=0; i<bh; i+=32)
    {
      for (j=0; j<bw; j+=32)
        {
          short liftblock[1024];
          // transfer coefficients at (p+j) into aligned block
          short *pp = p + j;
          short *pl = liftblock;
          for (int ii=0; ii<32; ii++, pp+=bw)
            for (int jj=0; jj<32; jj++) 
              *pl++ = pp[jj];
          // transfer into _IWBlock (apply zigzag and scaling)
          block->read_liftblock(liftblock, this);
          block++;
        }
      // next row of blocks
      p += 32*bw;
    }
  // Free decomposition buffer
  delete [] data16;
}
#endif // NEED_DECODER_ONLY

#ifndef NEED_DECODER_ONLY
void 
_IWMap::slashres(int res)
{
  int minbucket = 1;
  if (res < 2)
    return;
  else if (res < 4)
    minbucket=16;
  else if (res < 8)
    minbucket=4;
  for (int blockno=0; blockno<nb; blockno++)
    for (int buckno=minbucket; buckno<64; buckno++)
      blocks[blockno].zero(buckno);
}
#endif // NEED_DECODER_ONLY


void 
_IWMap::image(signed char *img8, int rowsize, int pixsep, int fast)
{
  // Allocate reconstruction buffer
  short *data16 = new short[bw*bh];
  // Copy coefficients
  int i;
  short *p = data16;
  const _IWBlock *block = blocks;
  for (i=0; i<bh; i+=32)
    {
      for (int j=0; j<bw; j+=32)
        {
          short liftblock[1024];
          // transfer into _IWBlock (apply zigzag and scaling)
          block->write_liftblock(liftblock);
          block++;
          // transfer into coefficient matrix at (p+j)
          short *pp = p + j;
          short *pl = liftblock;
          for (int ii=0; ii<32; ii++, pp+=bw,pl+=32)
            memcpy((void*)pp, (void*)pl, 32*sizeof(short));
        }
      // next row of blocks
      p += 32*bw;
    }
  // Reconstruction
  if (fast)
    {
      IWTransform::backward(data16, iw, ih, bw, 32, 2);  
      p = data16;
      for (i=0; i<bh; i+=2,p+=bw)
        for (int jj=0; jj<bw; jj+=2,p+=2)
          p[bw] = p[bw+1] = p[1] = p[0];
    }
  else
    {
      IWTransform::backward(data16, iw, ih, bw, 32, 1);  
    }
  // Copy result into image
  p = data16;
  signed char *row = img8;  
  for (i=0; i<ih; i++)
    {
      signed char *pix = row;
      for (int j=0; j<iw; j+=1,pix+=pixsep)
        {
          int x = (p[j] + iw_round) >> iw_shift;
          if (x < -128)
            x = -128;
          else if (x > 127)
            x = 127;
          *pix = x;
        }
      row += rowsize;
      p += bw;
    }
  // Delete buffer
  delete [] data16;
}

void 
_IWMap::image(int subsample, const GRect &rect, 
              signed char *img8, int rowsize, int pixsep, int fast)
{
  int i;
  // Compute number of decomposition levels
  int nlevel = 0;
  while (nlevel<5 && (32>>nlevel)>subsample)
    nlevel += 1;
  int boxsize = 1<<nlevel;
  // Parameter check
  if (subsample!=(32>>nlevel))
    THROW("(IWMap::image) Unsupported subsampling factor");
  if (rect.isempty())
    THROW("(IWMap::image) Rectangle is empty");    
  GRect irect(0,0,(iw+subsample-1)/subsample,(ih+subsample-1)/subsample);
  if (rect.xmin<0 || rect.ymin<0 || rect.xmax>irect.xmax || rect.ymax>irect.ymax)
    THROW("(IWMap::image) Rectangle is out of bounds");
  // Multiresolution rectangles 
  // -- needed[i] tells which coeffs are required for the next step
  // -- recomp[i] tells which coeffs need to be computed at this level
  GRect needed[8];
  GRect recomp[8];
  int r = 1;
  needed[nlevel] = rect;
  recomp[nlevel] = rect;
  for (i=nlevel-1; i>=0; i--)
    {
      needed[i] = recomp[i+1];
      needed[i].inflate(iw_border*r, iw_border*r);
      needed[i].intersect(needed[i], irect);
      r += r;
      recomp[i].xmin = (needed[i].xmin + r-1) & ~(r-1);
      recomp[i].xmax = (needed[i].xmax) & ~(r-1);
      recomp[i].ymin = (needed[i].ymin + r-1) & ~(r-1);
      recomp[i].ymax = (needed[i].ymax) & ~(r-1);
    }
  // Working rectangle
  // -- a rectangle large enough to hold all the data
  GRect work;
  work.xmin = (needed[0].xmin) & ~(boxsize-1);
  work.ymin = (needed[0].ymin) & ~(boxsize-1);
  work.xmax = ((needed[0].xmax-1) & ~(boxsize-1) ) + boxsize;
  work.ymax = ((needed[0].ymax-1) & ~(boxsize-1) ) + boxsize;
  // -- allocate work buffer
  int dataw = work.xmax - work.xmin;     // Note: cannot use inline width() or height()
  int datah = work.ymax - work.ymin;     // because Intel C++ compiler optimizes it wrong !
  short *data = new short[dataw * datah];
  // Fill working rectangle
  // -- loop over liftblocks rows
  short *ldata = data;
  int blkw = (bw>>5);
  const _IWBlock *lblock = blocks + (work.ymin>>nlevel)*blkw + (work.xmin>>nlevel);
  for (int by=work.ymin; by<work.ymax; by+=boxsize)
    {
      // -- loop over liftblocks in row
      const _IWBlock *block = lblock;
      short *rdata = ldata;
      for (int bx=work.xmin; bx<work.xmax; bx+=boxsize)        
        {
          // -- decide how much to load
          int mlevel = nlevel;
          if (nlevel>2)
            if (bx+31<needed[2].xmin || bx>needed[2].xmax ||
                by+31<needed[2].ymin || by>needed[2].ymax )
              mlevel = 2;
          int bmax   = ((1<<(mlevel+mlevel))+15)>>4;
          int ppinc  = (1<<(nlevel-mlevel));
          int ppmod1 = (dataw<<(nlevel-mlevel));
          int ttmod0 = (32 >> mlevel);
          int ttmod1 = (ttmod0 << 5);
          // -- get current block
          short liftblock[1024];
          block->write_liftblock(liftblock, 0, bmax );
          // -- copy liftblock into image
          short *tt = liftblock;
          short *pp = rdata;
          for (int ii=0; ii<boxsize; ii+=ppinc,pp+=ppmod1,tt+=ttmod1-32)
            for (int jj=0; jj<boxsize; jj+=ppinc,tt+=ttmod0)
              pp[jj] = *tt;
          // -- next block in row
          rdata += boxsize;
          block += 1;
        }
      // -- next row of blocks
      ldata += dataw << nlevel;
      lblock += blkw;
    }
  // Perform reconstruction
  r = boxsize;
  for (i=0; i<nlevel; i++)
    {
      GRect comp = needed[i];
      comp.xmin = comp.xmin & ~(r-1);
      comp.ymin = comp.ymin & ~(r-1);
      comp.translate(-work.xmin, -work.ymin);
      // Fast mode shortcuts finer resolution
      if (fast && i>=4) 
        {
          short *pp = data + comp.ymin*dataw;
          for (int ii=comp.ymin; ii<comp.ymax; ii+=2, pp+=dataw+dataw)
            for (int jj=comp.xmin; jj<comp.xmax; jj+=2)
              pp[jj+dataw] = pp[jj+dataw+1] = pp[jj+1] = pp[jj];
          break;
        }
      else
        {
          short *pp = data + comp.ymin*dataw + comp.xmin;
          IWTransform::backward(pp, comp.width(), comp.height(), dataw, r, r>>1);
        }
      r = r>>1;
    }
  // Copy result into image
  GRect nrect = rect;
  nrect.translate(-work.xmin, -work.ymin);
  short *p = data + nrect.ymin*dataw;
  signed char *row = img8;  
  for (i=nrect.ymin; i<nrect.ymax; i++)
    {
      int j;
      signed char *pix = row;
      for (j=nrect.xmin; j<nrect.xmax; j+=1,pix+=pixsep)
        {
          int x = (p[j] + iw_round) >> iw_shift;
          if (x < -128)
            x = -128;
          else if (x > 127)
            x = 127;
          *pix = x;
        }
      row += rowsize;
      p += dataw;
    }
  // Free reconstruction area
  delete [] data;
}




//////////////////////////////////////////////////////
// ENCODING/DECODING WAVELET COEFFICIENTS 
//    USING HIERARCHICAL SET DIFFERENCE
//////////////////////////////////////////////////////


//-----------------------------------------------
// This subclass reproduces a bug in the ZPCodec passthru functions.  The bug
// was discovered long after the initial release of DjVu.  After comparing the
// performances with and without the bug, we renamed it a feature.

class _ZPCodecBias : public ZPCodec // DJVU_CLASS
{
public:
  _ZPCodecBias(ByteStream &bs, int encoding=0) : ZPCodec(bs,encoding) {}
  void encoder(int bit, BitContext &ctx) { ZPCodec::encoder(bit,ctx); }
  int decoder(BitContext &ctx) { return ZPCodec::decoder(ctx); }
  void encoder(int bit);
  int decoder();
};

inline void 
_ZPCodecBias::encoder(int bit)
{
  int z = 0x8000 + ((a+a+a) >> 3);
  if (bit)
    encode_lps_simple(z);
  else
    encode_mps_simple(z);
}

inline int 
_ZPCodecBias::decoder()
{
  int z = 0x8000 + ((a+a+a) >> 3);
  return decode_sub_simple(0, z);
}


//-----------------------------------------------
// Class _IWCodec [declaration+implementation]
// Maintains information shared while encoding or decoding


class _IWCodec 
{
public:
  // Construction
  _IWCodec(_IWMap &map, int encoding=0);
  ~_IWCodec();
  // Coding
  int code_slice(_ZPCodecBias &zp);
  float estimate_decibel(float frac);
  // Data
  _IWMap &map;                  // working map
  _IWMap *emap;                 // encoder state
  int encoding;
  // status
  int curband;                  // current band
  int curbit;                   // current bitplane
  // quantization tables
  int quant_hi[10];             // quantization for bands 1 to 9
  int quant_lo[16];             // quantization for band 0.
  // bucket state
  char coeffstate[256];
  char bucketstate[16];
  enum { ZERO   = 1,            // this coeff never hits this bit
         ACTIVE = 2,            // this coeff is already active
         NEW    = 4,            // this coeff is becoming active
         UNK    = 8 };          // this coeff may become active
  // coding context
  BitContext ctxStart [32];
  BitContext ctxBucket[10][8];
  BitContext ctxMant;
  BitContext ctxRoot;
  // helper
  int is_null_slice(int bit, int band);
  int encode_prepare(int band, int fbucket, int nbucket, _IWBlock &blk, _IWBlock &eblk);
  int decode_prepare(int fbucket, int nbucket, _IWBlock &blk);
  void encode_buckets(_ZPCodecBias &zp, int bit, int band,
                      _IWBlock &blk, _IWBlock &eblk, int fbucket, int nbucket);
  void decode_buckets(_ZPCodecBias &zp, int bit, int band,
                      _IWBlock &blk, int fbucket, int nbucket);
};


// Constant

static struct { int start; int size; }  
bandbuckets[] = 
{
  // Code first bucket and number of buckets in each band
  { 0, 1 }, // -- band zero contains all lores info
  { 1, 1 }, { 2, 1 }, { 3, 1 }, 
  { 4, 4 }, { 8, 4 }, { 12,4 }, 
  { 16,16 }, { 32,16 }, { 48,16 }, 
};


// _IWCodec constructor

_IWCodec::_IWCodec(_IWMap &map, int encoding)
  : map(map), 
    emap(0),
    encoding(encoding),
    curband(0),
    curbit(1)
{
  // Initialize quantification
  int  j;
  int  i = 0;
  int *q = iw_quant;
  // -- lo coefficients
  for (j=0; i<4; j++)
    quant_lo[i++] = *q++;
  for (j=0; j<4; j++)
    quant_lo[i++] = *q;
  q += 1;
  for (j=0; j<4; j++)
    quant_lo[i++] = *q;
  q += 1;
  for (j=0; j<4; j++)
    quant_lo[i++] = *q;
  q += 1;
  // -- hi coefficients
  quant_hi[0] = 0;
  for (j=1; j<10; j++)
    quant_hi[j] = *q++;
  // Initialize coding contexts
  memset((void*)ctxStart, 0, sizeof(ctxStart));
  memset((void*)ctxBucket, 0, sizeof(ctxBucket));
  ctxMant = 0;
  ctxRoot = 0;
  // The encoder uses emap to track the decoder state
  if (encoding)
  {
#ifdef NEED_DECODER_ONLY
    THROW("Compiled with NEED_DECODER_ONLY");
#else
    emap = new _IWMap(map.iw, map.ih);
#endif
  }
}


// _IWCodec destructor

_IWCodec::~_IWCodec()
{
  if (emap)
    delete emap;
}

// is_null_slice
// -- check if data can be produced for this band/mask
// -- also fills the sure_zero array

inline int 
_IWCodec::is_null_slice(int bit, int band)
{
  if (band == 0)
    {
      int is_null = 1;
      for (int i=0; i<16; i++) 
        {
          int threshold = quant_lo[i];
          coeffstate[i] = ZERO;
          if (threshold>0 && threshold<0x8000)
            {
              coeffstate[i] = UNK;
              is_null = 0;
            }
        }
      return is_null;
    }
  else
    {
      int threshold = quant_hi[band];
      return (! (threshold>0 && threshold<0x8000));
    }
}


// code_slice
// -- read/write a slice of datafile

int
_IWCodec::code_slice(_ZPCodecBias &zp)
{
  // Check that code_slice can still run
  if (curbit < 0)
    return 0;
  // Perform coding
  if (! is_null_slice(curbit, curband))
    {
      for (int blockno=0; blockno<map.nb; blockno++)
        {
          int fbucket = bandbuckets[curband].start;
          int nbucket = bandbuckets[curband].size;
#ifndef NEED_DECODER_ONLY
          if (encoding)
            encode_buckets(zp, curbit, curband, 
                           map.blocks[blockno], emap->blocks[blockno], 
                           fbucket, nbucket);
          else
#endif
            decode_buckets(zp, curbit, curband, 
                           map.blocks[blockno], 
                           fbucket, nbucket);
        }
    }
  // Reduce quantization threshold
  quant_hi[curband] = quant_hi[curband] >> 1;
  if (curband == 0)
    for (int i=0; i<16; i++) 
      quant_lo[i] = quant_lo[i] >> 1;
  // Proceed to the next slice
  if (++curband >= (int)(sizeof(bandbuckets)/sizeof(bandbuckets[0])))
    {
      curband = 0;
      curbit += 1;
      if (quant_hi[(sizeof(bandbuckets)/sizeof(bandbuckets[0]))-1] == 0)
        {
          // All quantization thresholds are null
          curbit = -1;
          return 0;
        }
    }
  return 1;
}



// encode_prepare
// -- compute the states prior to encoding the buckets
#ifndef NEED_DECODER_ONLY
int
_IWCodec::encode_prepare(int band, int fbucket, int nbucket, _IWBlock &blk, _IWBlock &eblk)
{
  int bbstate = 0;
  // compute state of all coefficients in all buckets
  if (band) 
    {
      // Band other than zero
      int thres = quant_hi[band];
      char *cstate = coeffstate;
      for (int buckno=0; buckno<nbucket; buckno++, cstate+=16)
        {
          const short *pcoeff = blk.data(fbucket+buckno);
          const short *epcoeff = eblk.data(fbucket+buckno);
          int bstatetmp = 0;
          if (! pcoeff)
            {
              bstatetmp = UNK;
              // cstate[i] is not used and does not need initialization
            }
          else if (! epcoeff)
            {
              for (int i=0; i<16; i++)
                {
                  int cstatetmp = UNK;
                  if  ((int)(pcoeff[i])>=thres || (int)(pcoeff[i])<=-thres)
                    cstatetmp = NEW|UNK;
                  cstate[i] = cstatetmp;
                  bstatetmp |= cstatetmp;
                }
            }
          else
            {
              for (int i=0; i<16; i++)
                {
                  int cstatetmp = UNK;
                  if (epcoeff[i])
                    cstatetmp = ACTIVE;
                  else if  ((int)(pcoeff[i])>=thres || (int)(pcoeff[i])<=-thres)
                    cstatetmp = NEW|UNK;
                  cstate[i] = cstatetmp;
                  bstatetmp |= cstatetmp;
                }
            }
          bucketstate[buckno] = bstatetmp;
          bbstate |= bstatetmp;
        }
    }
  else
    {
      // Band zero ( fbucket==0 implies band==zero and nbucket==1 )
      const short *pcoeff = blk.data(0, &map);
      const short *epcoeff = eblk.data(0, emap);
      char *cstate = coeffstate;
      for (int i=0; i<16; i++)
        {
          int thres = quant_lo[i];
          int cstatetmp = cstate[i];
          if (cstatetmp != ZERO)
            {
              cstatetmp = UNK;
              if (epcoeff[i])
                cstatetmp = ACTIVE;
              else if ((int)(pcoeff[i])>=thres || (int)(pcoeff[i])<=-thres)
                cstatetmp = NEW|UNK;
            }
          cstate[i] = cstatetmp;
          bbstate |= cstatetmp;
        }
      bucketstate[0] = bbstate;
    }
  return bbstate;
}
#endif

// encode_buckets
// -- code a sequence of buckets in a given block
#ifndef NEED_DECODER_ONLY
void
_IWCodec::encode_buckets(_ZPCodecBias &zp, int bit, int band, 
                         _IWBlock &blk, _IWBlock &eblk,
                         int fbucket, int nbucket)
{
  // compute state of all coefficients in all buckets
  int bbstate = encode_prepare(band, fbucket, nbucket, blk, eblk);

  // code root bit
  if ((nbucket<16) || (bbstate&ACTIVE))
    {
      bbstate |= NEW;
    }
  else if (bbstate & UNK)
    {
      zp.encoder( (bbstate&NEW) ? 1 : 0 , ctxRoot);
#ifdef TRACE
      printf("bbstate[bit=%d,band=%d] = %d\n", bit, band, bbstate);
#endif
    }
  
  // code bucket bits
  if (bbstate & NEW)
    for (int buckno=0; buckno<nbucket; buckno++)
      {
        // Code bucket bit
        if (bucketstate[buckno] & UNK)
          {
            // Context
            int ctx = 0;
#ifndef NOCTX_BUCKET_UPPER
            if (band>0)
              {
                int k = (fbucket+buckno)<<2;
                const short *b = eblk.data(k>>4);
                if (b)
                  {
                    k = k & 0xf;
                    if (b[k])
                      ctx += 1;
                    if (b[k+1])
                      ctx += 1;
                    if (b[k+2])
                      ctx += 1;
                    if (ctx<3 && b[k+3])
                      ctx += 1;
                  }
              }
#endif
#ifndef NOCTX_BUCKET_ACTIVE
            if (bbstate & ACTIVE)
              ctx |= 4; 
#endif
            // Code
            zp.encoder( (bucketstate[buckno]&NEW) ? 1 : 0, ctxBucket[band][ctx] );
#ifdef TRACE
            printf("  bucketstate[bit=%d,band=%d,buck=%d] = %d\n", 
                   bit, band, buckno, bucketstate[buckno] & ~ZERO);
#endif
          }
      }
  
  // code new active coefficient (with their sign)
  if (bbstate & NEW)
    {
      int thres = quant_hi[band];
      char *cstate = coeffstate;
      for (int buckno=0; buckno<nbucket; buckno++, cstate+=16)
        if (bucketstate[buckno] & NEW)
          {
            int i;
#ifndef NOCTX_EXPECT
            int gotcha = 0;
            const int maxgotcha = 7;
            for (i=0; i<16; i++)
              if (cstate[i] & UNK)
                gotcha += 1;
#endif
            const short *pcoeff = blk.data(fbucket+buckno);
            short *epcoeff = eblk.data(fbucket+buckno, emap);
            // iterate within bucket
            for (i=0; i<16; i++)
              {
                if (cstate[i] & UNK)
                  {
                    // Prepare context
                    int ctx = 0;
#ifndef NOCTX_EXPECT
                    if (gotcha>=maxgotcha)
                      ctx = maxgotcha;
                    else
                      ctx = gotcha;
#endif
#ifndef NOCTX_ACTIVE
                    if (bucketstate[buckno] & ACTIVE)
                      ctx |= 8;
#endif
                    // Code
                    zp.encoder( (cstate[i]&NEW) ? 1 : 0, ctxStart[ctx] );
                    if (cstate[i] & NEW)
                      {
                        // Code sign
                        zp.encoder( (pcoeff[i]<0) ? 1 : 0 );
                        // Set encoder state
                        if (band==0)
                          thres = quant_lo[i];
                        epcoeff[i] = thres + (thres>>1);
                      }
#ifndef NOCTX_EXPECT
                    if (cstate[i] & NEW)
                      gotcha = 0;
                    else if (gotcha > 0)
                      gotcha -= 1;
#endif
#ifdef TRACE
                    printf("    coeffstate[bit=%d,band=%d,buck=%d,c=%d] = %d\n", 
                           bit, band, buckno, i, cstate[i]);
#endif
                  }
              }
          }
    }

  // code mantissa bits
  if (bbstate & ACTIVE)
    {
      int thres = quant_hi[band];
      char *cstate = coeffstate;
      for (int buckno=0; buckno<nbucket; buckno++, cstate+=16)
        if (bucketstate[buckno] & ACTIVE)
          {
            const short *pcoeff = blk.data(fbucket+buckno);
            short *epcoeff = eblk.data(fbucket+buckno, emap);
            for (int i=0; i<16; i++)
              if (cstate[i] & ACTIVE)
                {
                  // get coefficient
                  int coeff = pcoeff[i];
                  int ecoeff = epcoeff[i];
                  if (coeff < 0)
                    coeff = -coeff;
                  // get band zero thresholds
                  if (band == 0)
                    thres = quant_lo[i];
                  // compute mantissa bit
                  int pix = 0;
                  if (coeff >= ecoeff)
                    pix = 1;
                  // encode second or lesser mantissa bit
                  if (ecoeff <= 3*thres)
                    zp.encoder(pix, ctxMant);                      
                  else
                    zp.encoder(pix);
                  // adjust epcoeff
                  epcoeff[i] = ecoeff - (pix ? 0 : thres) + (thres>>1);
                }
          }
    }
}
#endif // NEED_DECODER_ONLY




// decode_prepare
// -- prepare the states before decoding buckets

int
_IWCodec::decode_prepare(int fbucket, int nbucket, _IWBlock &blk)
{  
  int bbstate = 0;
  char *cstate = coeffstate;
  if (fbucket)
    {
      // Band other than zero
      for (int buckno=0; buckno<nbucket; buckno++, cstate+=16)
        {
          int bstatetmp = 0;
          const short *pcoeff = blk.data(fbucket+buckno);
          if (! pcoeff)
            {
              // cstate[0..15] will be filled later
              bstatetmp = UNK;
            }
          else
            {
              for (int i=0; i<16; i++)
                {
                  int cstatetmp = UNK;
                  if (pcoeff[i])
                    cstatetmp = ACTIVE;
                  cstate[i] = cstatetmp;
                  bstatetmp |= cstatetmp;
                }
            }
          bucketstate[buckno] = bstatetmp;
          bbstate |= bstatetmp;
        }
    }
  else
    {
      // Band zero ( fbucket==0 implies band==zero and nbucket==1 )
      const short *pcoeff = blk.data(0);
      if (! pcoeff)
        {
          // cstate[0..15] will be filled later
          bbstate = UNK;      
        }
      else
        {
          for (int i=0; i<16; i++)
            {
              int cstatetmp = cstate[i];
              if (cstatetmp != ZERO)
                {
                  cstatetmp = UNK;
                  if (pcoeff[i])
                    cstatetmp = ACTIVE;
                }
              cstate[i] = cstatetmp;
              bbstate |= cstatetmp;
            }
        }
      bucketstate[0] = bbstate;
    }
  return bbstate;
}


// decode_buckets
// -- code a sequence of buckets in a given block

void
_IWCodec::decode_buckets(_ZPCodecBias &zp, int bit, int band, 
                         _IWBlock &blk,
                         int fbucket, int nbucket)
{
  // compute state of all coefficients in all buckets
  int bbstate = decode_prepare(fbucket, nbucket, blk);
  // code root bit
  if ((nbucket<16) || (bbstate&ACTIVE))
    {
      bbstate |= NEW;
    }
  else if (bbstate & UNK)
    {
      if (zp.decoder(ctxRoot))
        bbstate |= NEW;
#ifdef TRACE
      printf("bbstate[bit=%d,band=%d] = %d\n", bit, band, bbstate);
#endif
    }
  
  // code bucket bits
  if (bbstate & NEW)
    for (int buckno=0; buckno<nbucket; buckno++)
      {
        // Code bucket bit
        if (bucketstate[buckno] & UNK)
          {
            // Context
            int ctx = 0;
#ifndef NOCTX_BUCKET_UPPER
            if (band>0)
              {
                int k = (fbucket+buckno)<<2;
                const short *b = blk.data(k>>4);
                if (b)
                  {
                    k = k & 0xf;
                    if (b[k])
                      ctx += 1;
                    if (b[k+1])
                      ctx += 1;
                    if (b[k+2])
                      ctx += 1;
                    if (ctx<3 && b[k+3])
                      ctx += 1;
                  }
              }
#endif
#ifndef NOCTX_BUCKET_ACTIVE
            if (bbstate & ACTIVE)
              ctx |= 4; 
#endif
            // Code
            if (zp.decoder( ctxBucket[band][ctx] ))
              bucketstate[buckno] |= NEW;
#ifdef TRACE
            printf("  bucketstate[bit=%d,band=%d,buck=%d] = %d\n", 
                   bit, band, buckno, bucketstate[buckno]);
#endif
          }
      }

  // code new active coefficient (with their sign)
  if (bbstate & NEW)
    {
      int thres = quant_hi[band];
      char *cstate = coeffstate;
      for (int buckno=0; buckno<nbucket; buckno++, cstate+=16)
        if (bucketstate[buckno] & NEW)
          {
            int i;
            short *pcoeff = (short*)blk.data(fbucket+buckno);
            if (!pcoeff)
              {
                pcoeff = blk.data(fbucket+buckno, &map);
                // time to fill cstate[0..15]
                if (fbucket == 0) // band zero
                  {
                    for (i=0; i<16; i++)
                      if (cstate[i] != ZERO)
                        cstate[i] = UNK;
                  }
                else
                  {
                    for (i=0; i<16; i++)
                      cstate[i] = UNK;
                  }
              }
#ifndef NOCTX_EXPECT
            int gotcha = 0;
            const int maxgotcha = 7;
            for (i=0; i<16; i++)
              if (cstate[i] & UNK)
                gotcha += 1;
#endif
            for (i=0; i<16; i++)
              {
                if (cstate[i] & UNK)
                  {
                    // find lores threshold
                    if (band == 0)
                      thres = quant_lo[i];
                    // prepare context
                    int ctx = 0;
#ifndef NOCTX_EXPECT
                    if (gotcha>=maxgotcha)
                      ctx = maxgotcha;
                    else
                      ctx = gotcha;
#endif
#ifndef NOCTX_ACTIVE
                    if (bucketstate[buckno] & ACTIVE)
                      ctx |= 8;
#endif
                    // code difference bit
                    if (zp.decoder( ctxStart[ctx] ))
                      {
                        cstate[i] |= NEW;
                        int halfthres = thres>>1;
                        int coeff = thres+halfthres-(halfthres>>2);
                        if (zp.decoder())
                          pcoeff[i] = -coeff;
                        else
                          pcoeff[i] = coeff;
                      }
#ifndef NOCTX_EXPECT
                    if (cstate[i] & NEW)
                      gotcha = 0;
                    else if (gotcha > 0)
                      gotcha -= 1;
#endif
#ifdef TRACE
                    printf("    coeffstate[bit=%d,band=%d,buck=%d,c=%d] = %d\n", 
                           bit, band, buckno, i, cstate[i]);
#endif
                  }
              }
          }
    }
  
  // code mantissa bits
  if (bbstate & ACTIVE)
    {
      int thres = quant_hi[band];
      char *cstate = coeffstate;
      for (int buckno=0; buckno<nbucket; buckno++, cstate+=16)
        if (bucketstate[buckno] & ACTIVE)
          {
            short *pcoeff = (short*)blk.data(fbucket+buckno);
            for (int i=0; i<16; i++)
              if (cstate[i] & ACTIVE)
                {
                  int coeff = pcoeff[i];
                  if (coeff < 0)
                    coeff = -coeff;
                  // find lores threshold
                  if (band == 0)
                    thres = quant_lo[i];
                  // adjust coefficient
                  if (coeff <= 3*thres)
                    {
                      // second mantissa bit
                      coeff = coeff + (thres>>2);
                      if (zp.decoder(ctxMant))
                        coeff = coeff + (thres>>1);
                      else
                        coeff = coeff - thres + (thres>>1);
                    }
                  else
                    {
                      if (zp.decoder())
                        coeff = coeff + (thres>>1);
                      else
                        coeff = coeff - thres + (thres>>1);
                    }
                  // store coefficient
                  if (pcoeff[i] > 0)
                    pcoeff[i] = coeff;
                  else
                    pcoeff[i] = -coeff;
                }
          }
    }
}




// _IWCodec::estimate_decibel
// -- estimate encoding error (after code_slice) in decibels.
#ifndef NEED_DECODER_ONLY
float
_IWCodec::estimate_decibel(float frac)
{
  int i,j;
  float *q;
  // Test that we are encoding
  if (!encoding || !emap)
    THROW("Cannot estimate error when decoding");
  // Fill norm arrays
  float norm_lo[16];
  float norm_hi[10];
  // -- lo coefficients
  q = iw_norm;
  for (i=j=0; i<4; j++)
    norm_lo[i++] = *q++;
  for (j=0; j<4; j++)
    norm_lo[i++] = *q;
  q += 1;
  for (j=0; j<4; j++)
    norm_lo[i++] = *q;
  q += 1;
  for (j=0; j<4; j++)
    norm_lo[i++] = *q;
  q += 1;
  // -- hi coefficients
  norm_hi[0] = 0;
  for (j=1; j<10; j++)
    norm_hi[j] = *q++;
  // Initialize mse array
  float *xmse = new float[map.nb];
  // Compute mse in each block
  for (int blockno=0; blockno<map.nb; blockno++)
    {
      float mse = 0;
      // Iterate over bands
      for (int bandno=0; bandno<10; bandno++)
        {
          int fbucket = bandbuckets[bandno].start;
          int nbucket = bandbuckets[bandno].size;
          _IWBlock &blk = map.blocks[blockno];
          _IWBlock &eblk = emap->blocks[blockno];
          float norm = norm_hi[bandno];
          for (int buckno=0; buckno<nbucket; buckno++)
            {
              const short *pcoeff = blk.data(fbucket+buckno);
              const short *epcoeff = eblk.data(fbucket+buckno);
              if (pcoeff)
                {
                  if (epcoeff)
                    {
                      for (i=0; i<16; i++)
                        {
                          if (bandno == 0)
                            norm = norm_lo[i];
                          float delta = (float)(pcoeff[i]<0 ? -pcoeff[i] : pcoeff[i]);
                          delta = delta - epcoeff[i];
                          mse = mse + norm * delta * delta;
                        }
                    }
                  else
                    {
                      for (i=0; i<16; i++)
                        {
                          if (bandno == 0)
                            norm = norm_lo[i];
                          float delta = (float)(pcoeff[i]);
                          mse = mse + norm * delta * delta;
                        }
                    }
                }
            }
        }
      xmse[blockno] = mse / 1024;
    }
  // Compute partition point
  int n = 0;
  int m = map.nb - 1;
  int p = (int)floor(m*(1.0-frac)+0.5);
  p = (p>m ? m : (p<0 ? 0 : p));
  float pivot = 0;
  // Partition array
  while (n < p)
    {
      int l = n;
      int h = m;
      if (xmse[l] > xmse[h]) { float tmp=xmse[l]; xmse[l]=xmse[h]; xmse[h]=tmp; }
      pivot = xmse[(l+h)/2];
      if (pivot < xmse[l]) { float tmp=pivot; pivot=xmse[l]; xmse[l]=tmp; }
      if (pivot > xmse[h]) { float tmp=pivot; pivot=xmse[h]; xmse[h]=tmp; }
      while (l < h)
        {
          if (xmse[l] > xmse[h]) { float tmp=xmse[l]; xmse[l]=xmse[h]; xmse[h]=tmp; }
          while (xmse[l]<pivot || (xmse[l]==pivot && l<h)) l++;
          while (xmse[h]>pivot) h--;
        }
      if (p>=l) 
        n = l;
      else 
        m = l-1;
    }
  // Compute average mse
  float mse = 0;
  for (i=p; i<map.nb; i++)
    mse = mse + xmse[i];
  mse = mse / (map.nb - p);
  // Return
  delete [] xmse;
  float factor = 255 << iw_shift;
  float decibel = (float)(10.0 * log ( factor * factor / mse ) / 2.302585125);
  return decibel;
}
#endif // NEED_DECODER_ONLY


//////////////////////////////////////////////////////
// DEFINITION OF CHUNK HEADERS
//////////////////////////////////////////////////////


struct PrimaryHeader {
  unsigned char serial;
  unsigned char slices;
};  

struct SecondaryHeader {
  unsigned char major;
  unsigned char minor;
};

struct TertiaryHeader1 {        // VER 1.1
  unsigned char xhi, xlo;
  unsigned char yhi, ylo;
};

struct TertiaryHeader2 {        // VER 1.2
  unsigned char xhi, xlo;
  unsigned char yhi, ylo;
  unsigned char crcbdelay;
};



//////////////////////////////////////////////////////
// UTILITIES
//////////////////////////////////////////////////////


template <class T> inline T
min(T x, T y)
{
  return (x <= y) ? x : y;
}

template <class T> inline T
max(T x, T y)
{
  return (y <= x) ? x : y;
}




//////////////////////////////////////////////////////
// CLASS IWBITMAP
//////////////////////////////////////////////////////


IWBitmap::IWBitmap()
  : db_frac(1.0), 
    ymap(0), ycodec(0), 
    cslice(0), cserial(0), cbytes(0)
{
}


#ifndef NEED_DECODER_ONLY
IWBitmap::IWBitmap(const GBitmap *bm, const GBitmap *mask)
  : db_frac(1.0),
    ymap(0), ycodec(0),
    cslice(0), cserial(0), cbytes(0)
{
  init(bm, mask);
}
#endif

#ifndef NEED_DECODER_ONLY
void
IWBitmap::init(const GBitmap *bm, const GBitmap *mask)
{
  // Free
  close_codec();
  delete ymap;
  ymap = 0;
  // Init
  int i, j;
  int w = bm->columns();
  int h = bm->rows();
  int g = bm->get_grays()-1;
  signed char *buffer = new signed char[w*h];
  // Prepare gray level conversion table
  signed char  bconv[256];
  for (i=0; i<256; i++)
    bconv[i] = max(0,min(255,i*255/g)) - 128;
  // Perform decomposition
  TRY
    {
      // Prepare mask information
      const signed char *msk8 = 0;
      int mskrowsize = 0;
      if (mask)
        {
          msk8 = (const signed char*)((*mask)[0]);
          mskrowsize = mask->rowsize();
        }
      // Prepare a buffer of signed bytes
      for (i=0; i<h; i++)
        {
          signed char *bufrow = buffer + i*w;
          const unsigned char *bmrow = (*bm)[i];
          for (j=0; j<w; j++)
            bufrow[j] = bconv[bmrow[j]];
        }
      // Create map
      ymap = new _IWMap( w, h );
      ymap->create(buffer, w, msk8, mskrowsize);
    }
  CATCH(ex)
    {
      delete [] buffer;
      RETHROW;
    }
  ENDCATCH;
  // Delete buffer
  delete [] buffer;
  buffer = 0;
}
#endif


IWBitmap::~IWBitmap()
{
  delete ycodec;
  delete ymap;
}


int 
IWBitmap::get_width() const
{
  if (ymap) 
    return ymap->iw;
  else
    return 0;
}

int 
IWBitmap::get_height() const
{
  if (ymap) 
    return ymap->ih;
  else
    return 0;
}


int
IWBitmap::get_percent_memory() const
{
  int buckets = 0;
  int maximum = 0;
  if (ymap) 
    {
      buckets += ymap->get_bucket_count();
      maximum += 64 * ymap->nb;
    }
  return 100*buckets/ (maximum ? maximum : 1);
}

unsigned int
IWBitmap::get_memory_usage() const
{
  unsigned int usage = sizeof(GBitmap);
  if (ymap)
    usage += ymap->get_memory_usage();
  return usage;
}


GP<GBitmap> 
IWBitmap::get_bitmap()
{
  // Check presence of data
  if (ymap == 0)
    return 0;
  // Perform wavelet reconstruction
  int w = ymap->iw;
  int h = ymap->ih;
  GP<GBitmap> pbm = new GBitmap(h, w);
  ymap->image((signed char*)(*pbm)[0],pbm->rowsize());
  // Shift image data
  for (int i=0; i<h; i++)
    {
      unsigned char *urow = (*pbm)[i];
      signed char *srow = (signed char*)urow;
      for (int j=0; j<w; j++)
        urow[j] = (int)(srow[j]) + 128;
    }
  pbm->set_grays(256);
  return pbm;
}


GP<GBitmap>
IWBitmap::get_bitmap(int subsample, const GRect &rect)
{
  if (ymap == 0)
    return 0;
  // Allocate bitmap
  int w = rect.width();
  int h = rect.height();
  GP<GBitmap> pbm = new GBitmap(h,w);
  ymap->image(subsample, rect, (signed char*)(*pbm)[0],pbm->rowsize());
  // Shift image data
  for (int i=0; i<h; i++)
    {
      unsigned char *urow = (*pbm)[i];
      signed char *srow = (signed char*)urow;
      for (int j=0; j<w; j++)
        urow[j] = (int)(srow[j]) + 128;
    }
  pbm->set_grays(256);
  return pbm;
}


int
IWBitmap::decode_chunk(ByteStream &bs)
{
  // Check
  if (ycodec && ycodec->encoding)
    THROW("(IWBitmap::decode_chunk) Codec still open for encoding");
  // Open
  if (! ycodec)
    {
      cslice = cserial = 0;
      delete ymap;
      ymap = 0;
    }
  // Read primary header
  struct PrimaryHeader primary;
  if (bs.readall((void*)&primary, sizeof(primary)) != sizeof(primary))
    THROW("(IWBitmap::decode_chunk) Cannot read primary header");
  if (primary.serial != cserial)
    THROW("(IWBitmap::decode_chunk) Chunk does not bear expected serial number");
  int nslices = cslice + primary.slices;
  // Read auxilliary headers
  if (cserial == 0)
    {
      struct SecondaryHeader secondary;
      if (bs.readall((void*)&secondary, sizeof(secondary)) != sizeof(secondary))
        THROW("(IWBitmap::decode_chunk) Cannot read secondary header");
      if ((secondary.major & 0x7f) != IWCODEC_MAJOR)
        THROW("(IWBitmap::decode_chunk) File has been compressed with an incompatible IWCodec");
      if (secondary.minor > IWCODEC_MINOR)
        THROW("(IWBitmap::decode_chunk) File has been compressed with a more recent IWCodec");
      // Read tertiary header
      struct TertiaryHeader2 tertiary;
      unsigned int header3size = sizeof(tertiary);
      if (bs.readall((void*)&tertiary, header3size) != header3size)
        THROW("(IWBitmap::decode_chunk) Cannot read tertiary header");
      if (! (secondary.major & 0x80))
        THROW("(IWBitmap::decode_chunk) File contains a color image\n");
      // Create ymap and ycodec
      int w = (tertiary.xhi << 8) | tertiary.xlo;
      int h = (tertiary.yhi << 8) | tertiary.ylo;
      assert(! ymap);
      ymap = new _IWMap(w, h);
      assert(! ycodec);
      ycodec = new _IWCodec(*ymap, 0);
    }
  // Read data
  assert(ymap);
  assert(ycodec);
  _ZPCodecBias zp(bs, 0);
  int flag = 1;
  while (flag && cslice<nslices)
    {
      flag = ycodec->code_slice(zp);
      cslice++;
    }
  // Return
  cserial += 1;
  return nslices;
}


#ifndef NEED_DECODER_ONLY
int  
IWBitmap::encode_chunk(ByteStream &bs, const IWEncoderParms &parm)
{
  // Check
  if (parm.slices==0 && parm.bytes==0 && parm.decibels==0)
    THROW("(IWBitmap::encode_chunk) Must specify a stopping criterion");
  if (ycodec && !ycodec->encoding)
    THROW("(IWBitmap::encode_chunk) Codec still open for decoding");
  if (! ymap)
    THROW("(IWBitmap::encode_chunk) IWBitmap object is empty");
  // Open codec
  if (!ycodec)
    {
      cslice = cserial = cbytes = 0;
      ycodec = new _IWCodec(*ymap, 1);
    }
  // Adjust cbytes
  cbytes += sizeof(struct PrimaryHeader);
  if (cserial == 0)
    cbytes += sizeof(struct SecondaryHeader) + sizeof(struct TertiaryHeader2);
  // Prepare zcoded slices
  int flag = 1;
  int nslices = 0;
  MemoryByteStream mbs;
  DJVU_PROGRESS_TASK(chunk, parm.slices-cslice);
  {
    float estdb = -1.0;
    _ZPCodecBias zp(mbs,1);
    while (flag)
      {
        if (parm.decibels>0  && estdb>=parm.decibels)
          break;
        if (parm.bytes>0  && mbs.tell()+cbytes>=parm.bytes)
          break;
        if (parm.slices>0 && nslices+cslice>=parm.slices)
          break;
        DJVU_PROGRESS_RUN(chunk, (1+nslices-cslice)|0xf);
        flag = ycodec->code_slice(zp);
        if (flag && parm.decibels>0.0)
          if (ycodec->curband==0 || estdb>=parm.decibels-DECIBEL_PRUNE)
            estdb = ycodec->estimate_decibel(db_frac);
        nslices++;
      }
  }
  // Write primary header
  struct PrimaryHeader primary;
  primary.serial = cserial;
  primary.slices = nslices;
  bs.writall((void*)&primary, sizeof(primary));
  // Write auxilliary headers
  if (cserial == 0)
    {
      struct SecondaryHeader secondary;
      secondary.major = IWCODEC_MAJOR + 0x80;
      secondary.minor = IWCODEC_MINOR;
      bs.writall((void*)&secondary, sizeof(secondary));
      struct TertiaryHeader2 tertiary;
      tertiary.xhi = (ymap->iw >> 8) & 0xff;
      tertiary.xlo = (ymap->iw >> 0) & 0xff;
      tertiary.yhi = (ymap->ih >> 8) & 0xff;
      tertiary.ylo = (ymap->ih >> 0) & 0xff;
      tertiary.crcbdelay = 0;
      bs.writall((void*)&tertiary, sizeof(tertiary));
    }
  // Write slices
  mbs.seek(0);
  bs.copy(mbs);
  // Return
  cbytes  += mbs.tell();
  cslice  += nslices;
  cserial += 1;
  return flag;
}
#endif // NEED_DECODER_ONLY

void 
IWBitmap::close_codec()
{
  delete ycodec;
  ycodec = 0;
  cslice = cbytes = cserial = 0;
}

void 
IWBitmap::parm_dbfrac(float frac)
{
  if (frac>0 && frac<=1)
    db_frac = frac;
  else
    THROW("(IWBitmap::parm_dbfrac) parameter out of range");
}


int 
IWBitmap::get_serial()
{
  return cserial;
}


#ifndef NEED_DECODER_ONLY
void 
IWBitmap::encode_iff(IFFByteStream &iff, int nchunks, const IWEncoderParms *parms)
{
  if (ycodec)
    THROW("(IWBitmap::encode_iff) Codec has been left open");
  int flag = 1;
  iff.put_chunk("FORM:BM44", 1);
  DJVU_PROGRESS_TASK(iff, nchunks);
  for (int i=0; flag && i<nchunks; i++)
    {
      DJVU_PROGRESS_RUN(iff, i+1);
      iff.put_chunk("BM44");
      flag = encode_chunk(iff, parms[i]);
      iff.close_chunk();
    }
  iff.close_chunk();
  close_codec();
}
#endif

void 
IWBitmap::decode_iff(IFFByteStream &iff, int maxchunks)
{
  if (ycodec)
    THROW("(IWBitmap::decode_iff)  Codec has been left open");
  GString chkid;
  iff.get_chunk(chkid);
  if (chkid != "FORM:BM44")
    THROW("(IWBitmap::decode_iff) File is corrupted (cannot read FORM.BM44)");
  while (--maxchunks>=0 && iff.get_chunk(chkid))
    {
      if (chkid == "BM44")
        decode_chunk(iff);
      iff.close_chunk();
    }
  iff.close_chunk();
  close_codec();
}




//////////////////////////////////////////////////////
// CLASS IWENCODERPARMS
//////////////////////////////////////////////////////


IWEncoderParms::IWEncoderParms()
{
  // Zero represent default values
  memset((void*)this, 0, sizeof(IWEncoderParms));
}





//////////////////////////////////////////////////////
// CLASS IWPIXMAP
//////////////////////////////////////////////////////


IWPixmap::IWPixmap()
  : crcb_delay(10), crcb_half(0), db_frac(1.0),
    ymap(0), cbmap(0), crmap(0),
    ycodec(0), cbcodec(0), crcodec(0),
    cslice(0), cserial(0), cbytes(0)
{
}


#ifndef NEED_DECODER_ONLY
IWPixmap::IWPixmap(const GPixmap *pm, const GBitmap *mask, CRCBMode crcbmode)
  : crcb_delay(10), crcb_half(0), db_frac(1.0),
    ymap(0), cbmap(0), crmap(0),
    ycodec(0), cbcodec(0), crcodec(0),
    cslice(0), cserial(0), cbytes(0)
{
  init(pm, mask, crcbmode);
}
#endif

#ifndef NEED_DECODER_ONLY
void
IWPixmap::init(const GPixmap *pm, const GBitmap *mask, CRCBMode crcbmode)
{
  /* Free */
  close_codec();
  delete ymap;
  delete cbmap;
  delete crmap;
  ymap = cbmap = crmap = 0;
  /* Create */
  int w = pm->columns();
  int h = pm->rows();
  signed char *buffer = 0;
  TRY
    {
      buffer = new signed char[w*h];
      // Create maps
      ymap = new _IWMap(w,h);
      // Handle CRCB mode
      switch (crcbmode) 
        {
        case CRCBnone:   crcb_half=1; crcb_delay=-1; break;
        case CRCBhalf:   crcb_half=1; crcb_delay=10; break;        
        case CRCBnormal: crcb_half=0; crcb_delay=10; break;
        case CRCBfull:   crcb_half=0; crcb_delay= 0; break;
        }
      // Prepare mask information
      const signed char *msk8 = 0;
      int mskrowsize = 0;
      if (mask)
        {
          msk8 = (const signed char*)((*mask)[0]);
          mskrowsize = mask->rowsize();
        }
      // Fill buffer with luminance information
      DJVU_PROGRESS_TASK(create,3);
      DJVU_PROGRESS_RUN(create, (crcb_delay>=0 ? 1 : 3));
      IWTransform::RGB_to_Y((*pm)[0], w, h, pm->rowsize(), buffer, w);
      if (crcb_delay < 0)
        {
          // Stupid inversion for gray images
          signed char *e = buffer + w*h;
          for (signed char *b=buffer; b<e; b++)
            *b = 255 - *b;
        }
      // Create YMAP
      ymap->create(buffer, w, msk8, mskrowsize);
      // Create chrominance maps
      if (crcb_delay >= 0)
        {
          cbmap = new _IWMap(w,h);
          crmap = new _IWMap(w,h);
          // Process CB information
          DJVU_PROGRESS_RUN(create,2);
          IWTransform::RGB_to_Cb((*pm)[0], w, h, pm->rowsize(), buffer, w);
          cbmap->create(buffer, w, msk8, mskrowsize);
          // Process CR information
          DJVU_PROGRESS_RUN(create,3);
          IWTransform::RGB_to_Cr((*pm)[0], w, h, pm->rowsize(), buffer, w); 
          crmap->create(buffer, w, msk8, mskrowsize);
          // Perform chrominance reduction (CRCBhalf)
          if (crcb_half)
            {
              cbmap->slashres(2);
              crmap->slashres(2);
            }
        }
    }
  CATCH(ex)
    {
      delete [] buffer;
      RETHROW;
    }
  ENDCATCH;
  // Delete buffer
  delete [] buffer;
  buffer = 0;
}
#endif // NEED_DECODER_ONLY


IWPixmap::~IWPixmap()
{
  close_codec();
  delete ymap;
  delete crmap;
  delete cbmap;
  ymap = crmap = cbmap = 0;
}


int 
IWPixmap::get_width() const
{
  if (ymap) 
    return ymap->iw;
  else
    return 0;
}

int 
IWPixmap::get_height() const
{
  if (ymap) 
    return ymap->ih;
  else
    return 0;
}


int
IWPixmap::get_percent_memory() const
{
  int buckets = 0;
  int maximum = 0;
  if (ymap)
    {
      buckets += ymap->get_bucket_count();
      maximum += 64*ymap->nb;
    }
  if (cbmap)
    {
      buckets += cbmap->get_bucket_count();
      maximum += 64*cbmap->nb;
    }
  if (crmap)
    {
      buckets += crmap->get_bucket_count();
      maximum += 64*crmap->nb;
    }
  return 100*buckets/ (maximum ? maximum : 1);
}

unsigned int
IWPixmap::get_memory_usage() const
{
  unsigned int usage = sizeof(GPixmap);
  if (ymap)
    usage += ymap->get_memory_usage();
  if (cbmap)
    usage += cbmap->get_memory_usage();
  if (crmap)
    usage += crmap->get_memory_usage();
  return usage;
}


GP<GPixmap> 
IWPixmap::get_pixmap()
{
  // Check presence of data
  if (ymap == 0)
    return 0;
  // Allocate pixmap
  int w = ymap->iw;
  int h = ymap->ih;
  GP<GPixmap> ppm = new GPixmap(h, w);
  // Perform wavelet reconstruction
  signed char *ptr = (signed char*) (*ppm)[0];
  int rowsep = ppm->rowsize() * sizeof(GPixel);
  int pixsep = sizeof(GPixel);
  ymap->image(ptr, rowsep, pixsep);
  if (crmap && cbmap && crcb_delay >= 0)
  {
    cbmap->image(ptr+1, rowsep, pixsep, crcb_half);
    crmap->image(ptr+2, rowsep, pixsep, crcb_half);
  }
  // Convert image data to RGB
  if (crmap && cbmap && crcb_delay >= 0)
    {
      IWTransform::YCbCr_to_RGB((*ppm)[0], w, h, ppm->rowsize());
    }
  else
    {
      for (int i=0; i<h; i++)
        {
          GPixel *pixrow = (*ppm)[i];
          for (int j=0; j<w; j++, pixrow++)
            pixrow->b = pixrow->g = pixrow->r 
              = 127 - (int)(((signed char*)pixrow)[0]);
        }
    }
  // Return
  return ppm;
}



GP<GPixmap>
IWPixmap::get_pixmap(int subsample, const GRect &rect)
{
  if (ymap == 0)
    return 0;
  // Allocate
  int w = rect.width();
  int h = rect.height();
  GP<GPixmap> ppm = new GPixmap(h,w);
  // Perform wavelet reconstruction
  signed char *ptr = (signed char*) (*ppm)[0];
  int rowsep = ppm->rowsize() * sizeof(GPixel);
  int pixsep = sizeof(GPixel);
  ymap->image(subsample, rect, ptr, rowsep, pixsep);
  if (crmap && cbmap && crcb_delay >= 0)
  {
    cbmap->image(subsample, rect, ptr+1, rowsep, pixsep, crcb_half);
    crmap->image(subsample, rect, ptr+2, rowsep, pixsep, crcb_half);
  }
  // Convert image data to RGB
  if (crmap && cbmap && crcb_delay >= 0)
    {
      IWTransform::YCbCr_to_RGB((*ppm)[0], w, h, ppm->rowsize());
    }
  else
    {
      for (int i=0; i<h; i++)
        {
          GPixel *pixrow = (*ppm)[i];
          for (int j=0; j<w; j++, pixrow++)
            pixrow->b = pixrow->g = pixrow->r 
              = 127 - (int)(((signed char*)pixrow)[0]);
        }
    }
  // Return
  return ppm;
}


int
IWPixmap::decode_chunk(ByteStream &bs)
{
  // Check
  if (ycodec && ycodec->encoding)
    THROW("(IWPixmap::decode_chunk) Codec still open for encoding");
  // Open
  if (! ycodec)
    {
      cslice = cserial = 0;
      delete ymap;
      ymap = 0;
    }
  // Read primary header
  struct PrimaryHeader primary;
  if (bs.readall((void*)&primary, sizeof(primary)) != sizeof(primary))
    THROW("(IWPixmap::decode_chunk) Cannot read primary header");
  if (primary.serial != cserial)
    THROW("(IWPixmap::decode_chunk) Chunk does not bear expected serial number");
  int nslices = cslice + primary.slices;
  // Read secondary header
  if (cserial == 0)
    {
      struct SecondaryHeader secondary;
      if (bs.readall((void*)&secondary, sizeof(secondary)) != sizeof(secondary))
        THROW("(IWPixmap::decode_chunk) Cannot read secondary header");
      if ((secondary.major & 0x7f) != IWCODEC_MAJOR)
        THROW("(IWPixmap::decode_chunk) File has been compressed with an incompatible IWCodec");
      if (secondary.minor > IWCODEC_MINOR)
        THROW("(IWPixmap::decode_chunk) File has been compressed with a more recent IWCodec");
      // Read tertiary header
      struct TertiaryHeader2 tertiary;
      unsigned int header3size = sizeof(tertiary);
      if (secondary.minor < 2)
        header3size = sizeof(TertiaryHeader1);
      if (bs.readall((void*)&tertiary, header3size) != header3size)
        THROW("(IWBitmap::decode_chunk) Cannot read tertiary header");
      // Handle header information
      int w = (tertiary.xhi << 8) | tertiary.xlo;
      int h = (tertiary.yhi << 8) | tertiary.ylo;
      crcb_delay = 0;
      crcb_half = 0;
      if (secondary.minor>=2)
        crcb_delay = tertiary.crcbdelay & 0x7f;
      if (secondary.minor>=2)
        crcb_half = (tertiary.crcbdelay & 0x80 ? 0 : 1);
      if (secondary.major & 0x80)
        crcb_delay = -1;
      // Create ymap and ycodec      
      assert(! ymap);
      assert(! ycodec);
      ymap = new _IWMap(w, h);
      ycodec = new _IWCodec(*ymap, 0);
      if (crcb_delay >= 0)
        {
          cbmap = new _IWMap(w, h);
          crmap = new _IWMap(w, h);
          cbcodec = new _IWCodec(*cbmap, 0);
          crcodec = new _IWCodec(*crmap, 0);
        }
    }
  // Read data
  assert(ymap);
  assert(ycodec);
  _ZPCodecBias zp(bs, 0);
  int flag = 1;
  while (flag && cslice<nslices)
    {
      flag = ycodec->code_slice(zp);
      if (crcodec && cbcodec && crcb_delay<=cslice)
        {
          flag |= cbcodec->code_slice(zp);
          flag |= crcodec->code_slice(zp);
        }
      cslice++;
    }
  // Return
  cserial += 1;
  return nslices;
}


#ifndef NEED_DECODER_ONLY
int  
IWPixmap::encode_chunk(ByteStream &bs, const IWEncoderParms &parm)
{
  // Check
  if (parm.slices==0 && parm.bytes==0 && parm.decibels==0)
    THROW("(IWPixmap::encode_chunk) Must specify a stopping criterion");
  if (ycodec && !ycodec->encoding)
    THROW("(IWPixmap::encode_chunk) Codec still open for decoding");
  if (!ymap)
    THROW("(IWPixmap::encode_chunk) IWPixmap object is empty");
  // Open
  if (!ycodec)
    {
      cslice = cserial = cbytes = 0;
      ycodec = new _IWCodec(*ymap, 1);
      if (crmap && cbmap)
        {
          cbcodec = new _IWCodec(*cbmap, 1);
          crcodec = new _IWCodec(*crmap, 1);
        }
    }
  // Adjust cbytes
  cbytes += sizeof(struct PrimaryHeader);
  if (cserial == 0)
    cbytes += sizeof(struct SecondaryHeader) + sizeof(struct TertiaryHeader2);
  // Prepare zcodec slices
  int flag = 1;
  int nslices = 0;
  MemoryByteStream mbs;
  DJVU_PROGRESS_TASK(chunk, parm.slices-cslice);
  {
    float estdb = -1.0;
    _ZPCodecBias zp(mbs,1);
    while (flag)
      {
        if (parm.decibels>0  && estdb>=parm.decibels)
          break;
        if (parm.bytes>0  && mbs.tell()+cbytes>=parm.bytes)
          break;
        if (parm.slices>0 && nslices+cslice>=parm.slices)
          break;
        DJVU_PROGRESS_RUN(chunk, (1+nslices-cslice)|0xf);
        flag = ycodec->code_slice(zp);
        if (flag && parm.decibels>0)
          if (ycodec->curband==0 || estdb>=parm.decibels-DECIBEL_PRUNE)
            estdb = ycodec->estimate_decibel(db_frac);
        if (crcodec && cbcodec && cslice+nslices>=crcb_delay)
          {
            flag |= cbcodec->code_slice(zp);
            flag |= crcodec->code_slice(zp);
          }
        nslices++;
      }
  }
  // Write primary header
  struct PrimaryHeader primary;
  primary.serial = cserial;
  primary.slices = nslices;
  bs.writall((void*)&primary, sizeof(primary));
  // Write secondary header
  if (cserial == 0)
    {
      struct SecondaryHeader secondary;
      secondary.major = IWCODEC_MAJOR;
      secondary.minor = IWCODEC_MINOR;
      if (! (crmap && cbmap))
        secondary.major |= 0x80;
      bs.writall((void*)&secondary, sizeof(secondary));
      struct TertiaryHeader2 tertiary;
      tertiary.xhi = (ymap->iw >> 8) & 0xff;
      tertiary.xlo = (ymap->iw >> 0) & 0xff;
      tertiary.yhi = (ymap->ih >> 8) & 0xff;
      tertiary.ylo = (ymap->ih >> 0) & 0xff;
      tertiary.crcbdelay = (crcb_half ? 0x00 : 0x80);
      tertiary.crcbdelay |= (crcb_delay>=0 ? crcb_delay : 0x00);
      bs.writall((void*)&tertiary, sizeof(tertiary));
    }
  // Write slices
  mbs.seek(0);
  bs.copy(mbs);
  // Return
  cbytes  += mbs.tell();
  cslice  += nslices;
  cserial += 1;
  return flag;
}
#endif // NEED_DECODER_ONLY

void 
IWPixmap::close_codec()
{
  delete ycodec;
  delete crcodec;
  delete cbcodec;
  ycodec = crcodec = cbcodec = 0;
  cslice = cbytes = cserial = 0;
}

int 
IWPixmap::parm_crcbdelay(int parm)
{
  if (parm >= 0)
    crcb_delay = parm;
  return crcb_delay;
}

void 
IWPixmap::parm_dbfrac(float frac)
{
  if (frac>0 && frac<=1)
    db_frac = frac;
  else
    THROW("(IWPixmap::parm_dbfrac) parameter out of range");
}

int 
IWPixmap::get_serial()
{
  return cserial;
}

#ifndef NEED_DECODER_ONLY
void 
IWPixmap::encode_iff(IFFByteStream &iff, int nchunks, const IWEncoderParms *parms)
{
  if (ycodec)
    THROW("(IWPixmap::encode_iff) Codec has been left open");
  int flag = 1;
  iff.put_chunk("FORM:PM44", 1);
  DJVU_PROGRESS_TASK(iff, nchunks);
  for (int i=0; flag && i<nchunks; i++)
    {
      DJVU_PROGRESS_RUN(iff, i+1);
      iff.put_chunk("PM44");
      flag = encode_chunk(iff, parms[i]);
      iff.close_chunk();
    }
  iff.close_chunk();
  close_codec();
}
#endif

void 
IWPixmap::decode_iff(IFFByteStream &iff, int maxchunks)
{
  if (ycodec)
    THROW("(IWPixmap::decode_iff)  Codec has been left open");
  GString chkid;
  iff.get_chunk(chkid);
  if (chkid!="FORM:PM44" && chkid!="FORM:BM44")
    THROW("(IWPixmap::decode_iff) File is corrupted (cannot read FORM.BM44)");
  while (--maxchunks>=0 && iff.get_chunk(chkid))
    {
      if (chkid=="PM44" || chkid=="BM44")
        decode_chunk(iff);
      iff.close_chunk();
    }
  iff.close_chunk();
  close_codec();
}



