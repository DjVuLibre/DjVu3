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
//C- $Id: MMRDecoder.cpp,v 1.6 1999-12-20 01:16:38 bcr Exp $


#ifdef __GNUC__
#pragma implementation
#endif

#include <stdio.h>
#include "MMRDecoder.h"
#include "GException.h"
#include "GContainer.h"


// ----------------------------------------
// MMR CODEBOOKS

static const char invalid_mmr_data[]="Invalid MMR Data";

struct VLCode 
{
  unsigned short code;
  short codelen;
  short value;
};

enum MMRMode
{ 
  P=0, H=1, V0=2, VR1=3, VR2=4, VR3=5, VL1=6, VL2=7, VL3=8 
};

static const VLCode mrcodes[] =
{   // Codes on 7 bits
  // 7 bit codes
  { 0x08,   4,    P }, // 0001
  { 0x10,   3,    H }, // 001
  { 0x40,   1,   V0 }, // 1
  { 0x30,   3,  VR1 }, // 011
  { 0x06,   6,  VR2 }, // 000011
  { 0x03,   7,  VR3 }, // 0000011
  { 0x20,   3,  VL1 }, // 010
  { 0x04,   6,  VL2 }, // 000010
  { 0x02,   7,  VL3 }, // 0000010
  { 0x00,   0,   -1 }  // Illegal entry
};


static const VLCode wcodes[] = {    
  // 13 bit codes
  { 0x06a0,  8,    0 }, // 00110101
  { 0x0380,  6,    1 }, // 000111
  { 0x0e00,  4,    2 }, // 0111
  { 0x1000,  4,    3 }, // 1000
  { 0x1600,  4,    4 }, // 1011
  { 0x1800,  4,    5 }, // 1100
  { 0x1c00,  4,    6 }, // 1110
  { 0x1e00,  4,    7 }, // 1111
  { 0x1300,  5,    8 }, // 10011
  { 0x1400,  5,    9 }, // 10100
  { 0x0700,  5,   10 }, // 00111
  { 0x0800,  5,   11 }, // 01000
  { 0x0400,  6,   12 }, // 001000
  { 0x0180,  6,   13 }, // 000011
  { 0x1a00,  6,   14 }, // 110100
  { 0x1a80,  6,   15 }, // 110101
  { 0x1500,  6,   16 }, // 101010
  { 0x1580,  6,   17 }, // 101011
  { 0x09c0,  7,   18 }, // 0100111
  { 0x0300,  7,   19 }, // 0001100
  { 0x0200,  7,   20 }, // 0001000
  { 0x05c0,  7,   21 }, // 0010111
  { 0x00c0,  7,   22 }, // 0000011
  { 0x0100,  7,   23 }, // 0000100
  { 0x0a00,  7,   24 }, // 0101000
  { 0x0ac0,  7,   25 }, // 0101011
  { 0x04c0,  7,   26 }, // 0010011
  { 0x0900,  7,   27 }, // 0100100
  { 0x0600,  7,   28 }, // 0011000
  { 0x0040,  8,   29 }, // 00000010
  { 0x0060,  8,   30 }, // 00000011
  { 0x0340,  8,   31 }, // 00011010
  { 0x0360,  8,   32 }, // 00011011
  { 0x0240,  8,   33 }, // 00010010
  { 0x0260,  8,   34 }, // 00010011
  { 0x0280,  8,   35 }, // 00010100
  { 0x02a0,  8,   36 }, // 00010101
  { 0x02c0,  8,   37 }, // 00010110
  { 0x02e0,  8,   38 }, // 00010111
  { 0x0500,  8,   39 }, // 00101000
  { 0x0520,  8,   40 }, // 00101001
  { 0x0540,  8,   41 }, // 00101010
  { 0x0560,  8,   42 }, // 00101011
  { 0x0580,  8,   43 }, // 00101100
  { 0x05a0,  8,   44 }, // 00101101
  { 0x0080,  8,   45 }, // 00000100
  { 0x00a0,  8,   46 }, // 00000101
  { 0x0140,  8,   47 }, // 00001010
  { 0x0160,  8,   48 }, // 00001011
  { 0x0a40,  8,   49 }, // 01010010
  { 0x0a60,  8,   50 }, // 01010011
  { 0x0a80,  8,   51 }, // 01010100
  { 0x0aa0,  8,   52 }, // 01010101
  { 0x0480,  8,   53 }, // 00100100
  { 0x04a0,  8,   54 }, // 00100101
  { 0x0b00,  8,   55 }, // 01011000
  { 0x0b20,  8,   56 }, // 01011001
  { 0x0b40,  8,   57 }, // 01011010
  { 0x0b60,  8,   58 }, // 01011011
  { 0x0940,  8,   59 }, // 01001010
  { 0x0960,  8,   60 }, // 01001011
  { 0x0640,  8,   61 }, // 00110010
  { 0x0660,  8,   62 }, // 00110011
  { 0x0680,  8,   63 }, // 00110100
  { 0x1b00,  5,   64 }, // 11011
  { 0x1200,  5,  128 }, // 10010
  { 0x0b80,  6,  192 }, // 010111
  { 0x0dc0,  7,  256 }, // 0110111
  { 0x06c0,  8,  320 }, // 00110110
  { 0x06e0,  8,  384 }, // 00110111
  { 0x0c80,  8,  448 }, // 01100100
  { 0x0ca0,  8,  512 }, // 01100101
  { 0x0d00,  8,  576 }, // 01101000
  { 0x0ce0,  8,  640 }, // 01100111
  { 0x0cc0,  9,  704 }, // 011001100
  { 0x0cd0,  9,  768 }, // 011001101
  { 0x0d20,  9,  832 }, // 011010010
  { 0x0d30,  9,  896 }, // 011010011
  { 0x0d40,  9,  960 }, // 011010100
  { 0x0d50,  9, 1024 }, // 011010101
  { 0x0d60,  9, 1088 }, // 011010110
  { 0x0d70,  9, 1152 }, // 011010111
  { 0x0d80,  9, 1216 }, // 011011000
  { 0x0d90,  9, 1280 }, // 011011001
  { 0x0da0,  9, 1344 }, // 011011010
  { 0x0db0,  9, 1408 }, // 011011011
  { 0x0980,  9, 1472 }, // 010011000
  { 0x0990,  9, 1536 }, // 010011001
  { 0x09a0,  9, 1600 }, // 010011010
  { 0x0c00,  6, 1664 }, // 011000  (what did they think?)
  { 0x09b0,  9, 1728 }, // 010011011
  { 0x0020, 11, 1792 }, // 00000001000
  { 0x0030, 11, 1856 }, // 00000001100
  { 0x0034, 11, 1920 }, // 00000001101
  { 0x0024, 12, 1984 }, // 000000010010
  { 0x0026, 12, 2048 }, // 000000010011
  { 0x0028, 12, 2112 }, // 000000010100
  { 0x002a, 12, 2176 }, // 000000010101
  { 0x002c, 12, 2240 }, // 000000010110
  { 0x002e, 12, 2304 }, // 000000010111
  { 0x0038, 12, 2368 }, // 000000011100
  { 0x003a, 12, 2432 }, // 000000011101
  { 0x003c, 12, 2496 }, // 000000011110
  { 0x003e, 12, 2560 }, // 000000011111
  { 0x0000,  0,   -1 }  // Illegal entry
};


static const VLCode bcodes[] = {
  // 13 bit codes
  { 0x01b8, 10,    0 }, // 0000110111
  { 0x0800,  3,    1 }, // 010
  { 0x1800,  2,    2 }, // 11
  { 0x1000,  2,    3 }, // 10
  { 0x0c00,  3,    4 }, // 011
  { 0x0600,  4,    5 }, // 0011
  { 0x0400,  4,    6 }, // 0010
  { 0x0300,  5,    7 }, // 00011
  { 0x0280,  6,    8 }, // 000101
  { 0x0200,  6,    9 }, // 000100
  { 0x0100,  7,   10 }, // 0000100
  { 0x0140,  7,   11 }, // 0000101
  { 0x01c0,  7,   12 }, // 0000111
  { 0x0080,  8,   13 }, // 00000100
  { 0x00e0,  8,   14 }, // 00000111
  { 0x0180,  9,   15 }, // 000011000
  { 0x00b8, 10,   16 }, // 0000010111
  { 0x00c0, 10,   17 }, // 0000011000
  { 0x0040, 10,   18 }, // 0000001000
  { 0x019c, 11,   19 }, // 00001100111
  { 0x01a0, 11,   20 }, // 00001101000
  { 0x01b0, 11,   21 }, // 00001101100
  { 0x00dc, 11,   22 }, // 00000110111
  { 0x00a0, 11,   23 }, // 00000101000
  { 0x005c, 11,   24 }, // 00000010111
  { 0x0060, 11,   25 }, // 00000011000
  { 0x0194, 12,   26 }, // 000011001010
  { 0x0196, 12,   27 }, // 000011001011
  { 0x0198, 12,   28 }, // 000011001100
  { 0x019a, 12,   29 }, // 000011001101
  { 0x00d0, 12,   30 }, // 000001101000
  { 0x00d2, 12,   31 }, // 000001101001
  { 0x00d4, 12,   32 }, // 000001101010
  { 0x00d6, 12,   33 }, // 000001101011
  { 0x01a4, 12,   34 }, // 000011010010
  { 0x01a6, 12,   35 }, // 000011010011
  { 0x01a8, 12,   36 }, // 000011010100
  { 0x01aa, 12,   37 }, // 000011010101
  { 0x01ac, 12,   38 }, // 000011010110
  { 0x01ae, 12,   39 }, // 000011010111
  { 0x00d8, 12,   40 }, // 000001101100
  { 0x00da, 12,   41 }, // 000001101101
  { 0x01b4, 12,   42 }, // 000011011010
  { 0x01b6, 12,   43 }, // 000011011011
  { 0x00a8, 12,   44 }, // 000001010100
  { 0x00aa, 12,   45 }, // 000001010101
  { 0x00ac, 12,   46 }, // 000001010110
  { 0x00ae, 12,   47 }, // 000001010111
  { 0x00c8, 12,   48 }, // 000001100100
  { 0x00ca, 12,   49 }, // 000001100101
  { 0x00a4, 12,   50 }, // 000001010010
  { 0x00a6, 12,   51 }, // 000001010011
  { 0x0048, 12,   52 }, // 000000100100
  { 0x006e, 12,   53 }, // 000000110111
  { 0x0070, 12,   54 }, // 000000111000
  { 0x004e, 12,   55 }, // 000000100111
  { 0x0050, 12,   56 }, // 000000101000
  { 0x00b0, 12,   57 }, // 000001011000
  { 0x00b2, 12,   58 }, // 000001011001
  { 0x0056, 12,   59 }, // 000000101011
  { 0x0058, 12,   60 }, // 000000101100
  { 0x00b4, 12,   61 }, // 000001011010
  { 0x00cc, 12,   62 }, // 000001100110
  { 0x00ce, 12,   63 }, // 000001100111
  { 0x0078, 10,   64 }, // 0000001111
  { 0x0190, 12,  128 }, // 000011001000
  { 0x0192, 12,  192 }, // 000011001001
  { 0x00b6, 12,  256 }, // 000001011011
  { 0x0066, 12,  320 }, // 000000110011
  { 0x0068, 12,  384 }, // 000000110100
  { 0x006a, 12,  448 }, // 000000110101
  { 0x006c, 13,  512 }, // 0000001101100
  { 0x006d, 13,  576 }, // 0000001101101
  { 0x004a, 13,  640 }, // 0000001001010
  { 0x004b, 13,  704 }, // 0000001001011
  { 0x004c, 13,  768 }, // 0000001001100
  { 0x004d, 13,  832 }, // 0000001001101
  { 0x0072, 13,  896 }, // 0000001110010
  { 0x0073, 13,  960 }, // 0000001110011
  { 0x0074, 13, 1024 }, // 0000001110100
  { 0x0075, 13, 1088 }, // 0000001110101
  { 0x0076, 13, 1152 }, // 0000001110110
  { 0x0077, 13, 1216 }, // 0000001110111
  { 0x0052, 13, 1280 }, // 0000001010010
  { 0x0053, 13, 1344 }, // 0000001010011
  { 0x0054, 13, 1408 }, // 0000001010100
  { 0x0055, 13, 1472 }, // 0000001010101
  { 0x005a, 13, 1536 }, // 0000001011010
  { 0x005b, 13, 1600 }, // 0000001011011
  { 0x0064, 13, 1664 }, // 0000001100100
  { 0x0065, 13, 1728 }, // 0000001100101
  { 0x0020, 11, 1792 }, // 00000001000
  { 0x0030, 11, 1856 }, // 00000001100
  { 0x0034, 11, 1920 }, // 00000001101
  { 0x0024, 12, 1984 }, // 000000010010
  { 0x0026, 12, 2048 }, // 000000010011
  { 0x0028, 12, 2112 }, // 000000010100
  { 0x002a, 12, 2176 }, // 000000010101
  { 0x002c, 12, 2240 }, // 000000010110
  { 0x002e, 12, 2304 }, // 000000010111
  { 0x0038, 12, 2368 }, // 000000011100
  { 0x003a, 12, 2432 }, // 000000011101
  { 0x003c, 12, 2496 }, // 000000011110
  { 0x003e, 12, 2560 }, // 000000011111
  { 0x0000,  0,   -1 }  // Illegal entry
};




// ----------------------------------------
// SOURCE OF BITS


class MMRDecoder::VLSource
{
private:
  ByteStream &inp;
  unsigned char buffer[64];
  unsigned int codeword;
  int lowbits;
  int bufpos;
  int bufmax;
public:
  // Initializes a bit source on a bytestream
  VLSource(ByteStream &inp);
  // Returns a 32 bits integer with at least the 
  // next sixteen code bits in the high order bits.
  unsigned int peek() 
    { return codeword; }
  // Ensures that next #peek()# contains at least
  // the next 24 code bits.
  void preload();
  // Consumes #n# bits.
  void shift(int n)
    { codeword<<=n; lowbits+=n; if (lowbits>=16) preload(); }
};

MMRDecoder::VLSource::VLSource(ByteStream &inp)
  : inp(inp), codeword(0), 
    lowbits(0), bufpos(0), bufmax(0)
{
  lowbits = 32;
  preload();
}

void
MMRDecoder::VLSource::preload()
{
  while (lowbits>=8) 
    {
      if (bufpos >= bufmax) 
	{
	  bufpos = 0; // Refill buffer
	  bufmax = inp.read((void*)buffer, sizeof(buffer));
	  if (bufmax <= 0)
            return;   // Fill with zeroes (invalid code)
	}
      lowbits -= 8;
      codeword |= buffer[bufpos++] << lowbits;
    }
}



// ----------------------------------------
// VARIABLE LENGTH CODES



class MMRDecoder::VLTable
{
public:
  const VLCode *code;
  int codewordshift;
  unsigned char *index;
  ~VLTable();
  // Construct a VLTable given a codebook with #nbits# long codes.
  VLTable(const VLCode *codes, int nbits);
  // Reads one symbol from a VLSource
  int decode(MMRDecoder::VLSource *src);
};

inline int
MMRDecoder::VLTable::decode(MMRDecoder::VLSource *src)    
{ 
  const VLCode &c = code[ index[ src->peek() >> codewordshift ] ];
  src->shift(c.codelen); 
  return c.value; 
}

MMRDecoder::VLTable::VLTable(const VLCode *codes, int nbits)
  : code(codes)
{
  int i;
  // count entries
  int ncodes = 0;
  while (codes[ncodes].codelen)
    ncodes++;
  // check arguments
  if (nbits<=1 || nbits>16)
    THROW(invalid_mmr_data);
  if (ncodes>=256)
    THROW(invalid_mmr_data);
  codewordshift = 32 - nbits;
  // allocate table
  int size = (1<<nbits);
  index = new unsigned char[size];
  // fill table with pointer to illegal entry
  for (i=0; i<size; i++)
    index[i] = ncodes;
  // process codes
  for (i=0; i<ncodes; i++) {
    int c = codes[i].code;
    int b = codes[i].codelen;
    if(b<=0 || b>nbits)
    {
      THROW(invalid_mmr_data);
    }
    // fill table entries whose index high bits are code.
    int n = c + (1<<(nbits-b));
    while ( --n >= c ) {
      if(index[n] != ncodes)
       THROW("ambiguous MMR codebook");
      index[n] = i;
    }
  }
}

MMRDecoder::VLTable::~VLTable()
{
  delete index;
}





// ----------------------------------------
// MMR DECODER



MMRDecoder::~MMRDecoder()
{
  delete wtable;
  delete btable;
  delete mrtable;
  delete src;
  delete [] refline;
}


MMRDecoder::MMRDecoder(ByteStream &bs, int width, int height)
  : width(width), height(height), lineno(0)
{
  refline = new unsigned char [width+5];
  memset(refline, 0, width);
  src = new VLSource(bs);
  mrtable = new VLTable(mrcodes, 7);
  btable = new VLTable(bcodes, 13);
  wtable = new VLTable(wcodes, 13);
}

const unsigned char *
MMRDecoder::scanline()
{
  // Check if all lines have been returned
  if (lineno >= height)
    return 0;
  // Loop until scanline is complete
  unsigned char CurColor=0;
  unsigned char *ptr=refline,*StartRun=refline,*EndRun;
  unsigned char *endptr=refline+width;
  for (EndRun=ptr;(EndRun<endptr)&&(EndRun[0]==CurColor); EndRun++);
  while (ptr < endptr)
    {
      // (Optimization idea: Make #refline# an array of run lengths.)
      // Process MMR codes
      switch ( mrtable->decode(src) )
        {
          /* Pass Mode */
        case P: 
          { 
            while(ptr<StartRun) // extend left side of run to ptr
              (ptr++)[0] = CurColor;
            for(ptr=EndRun;(ptr<endptr)&&(ptr[0]!=CurColor);(ptr++)[0]=CurColor);
            break;
          }
          /* Horizontal Mode */
        case H: 
          { 
            int len;
            VLTable *table;
            // First run
            unsigned char *NewEndRun= ptr;
            table = (CurColor ? btable : wtable);
            do
            {
              NewEndRun += (len = table->decode(src));
            } while (len >= 64);
            if (len<0 || NewEndRun>endptr) 
              THROW(invalid_mmr_data);
            if(NewEndRun > StartRun)
            {
              while (ptr < StartRun)  // extend left side of run to ptr
                *(ptr++) = CurColor;
              if(NewEndRun>EndRun)
              {
                for(ptr=EndRun;ptr<NewEndRun;*(ptr++)=CurColor);
              }else
              {
                ptr=NewEndRun;
              }
            }else
            {
              while (ptr < NewEndRun) 
                *(ptr++) = CurColor;
            }
            // Second run
            table = (!CurColor ? btable : wtable);
            do
            { 
              NewEndRun += (len = table->decode(src));
            } while (len >= 64);
            if (len<0 || NewEndRun>endptr) 
              THROW(invalid_mmr_data);
            while (ptr < NewEndRun)
              *(ptr++) = !CurColor;
            break;
          }
          /* Vertical Modes */
        case V0:
          while (ptr<StartRun)  // extend left side of run to ptr
            (ptr++)[0] = CurColor;
          ptr=EndRun;
          CurColor = !CurColor;
          break;
        case VR3:
          (EndRun++)[0]=CurColor;
        case VR2:
          (EndRun++)[0]=CurColor;
        case VR1:
          (EndRun++)[0]=CurColor;
          if (EndRun > endptr)
            THROW(invalid_mmr_data);
          while (ptr < StartRun)  // extend left side of run to ptr
            (ptr++)[0] = CurColor;
          ptr=EndRun;
          CurColor = !CurColor;
          break;
          /* Uncommon modes */
        case VL3:
          --EndRun;
        case VL2:
          --EndRun;
        case VL1:
          --EndRun;
          if(EndRun<=StartRun) // extend left side of run to ptr
          {
            while (ptr<EndRun) // extend right side of run to new EndRun
              (ptr++)[0] = CurColor;
          }else
          {
            while (ptr<StartRun)  // extend left side of run to ptr
              (ptr++)[0] = CurColor;
            ptr=EndRun;
          }
          CurColor = !CurColor;
          break;
          /* Uncommon modes */
        default: 
          {
            src->preload();
            unsigned int m = src->peek();
            // -- Could be EOFB ``000000000001000000000001''
            //    TIFF6 says remaining lines are white
            //    What about the current line ?
            if ((m & 0xffffff00) == 0x00100100)
              {
                lineno = height;
                return 0;
              }
            // -- Could be UNCOMPRESSED ``0000001111''
            //    TIFF6 says people should not do this.
            //    RFC1314 says people should do this.
            else if ((m & 0xffc00000) == 0x03c00000)
              {
                src->shift(10);
                while ((m = (src->peek() & 0xfc000000)))
                  {
                    if (m == 0x04000000)       // 000001
                      {
                        src->shift(6);
                        if (ptr+4 >= endptr)
                          THROW(invalid_mmr_data);
                        (ptr++)[0] = 0;
                        (ptr++)[0] = 0;
                        (ptr++)[0] = 0;
                        (ptr++)[0] = 0;
                        (ptr++)[0] = 0;
                      } 
                    else                       // 000010 to 111111 
                      {
                        src->shift(1);
                        if (ptr >= endptr)
                          THROW(invalid_mmr_data);
                        (ptr++)[0] = ((m & 0x80000000) ? 1 : 0);
                      }
                  }
                // Analyze uncompressed termination code.
                m = src->peek() & 0xff000000;  
                src->shift(8);
                if((CurColor=!(m==0x02000000))  // 00000010
                  &&(m!=0x03000000))           // 00000011
                {
                  THROW(invalid_mmr_data);
                }
                // Cross fingers and proceed ...
                break;
              }
            // -- Unknown MMR code.
            THROW(invalid_mmr_data);
          }
        }
      // Compute EndRun
      for(StartRun=ptr;(StartRun<endptr)&&(StartRun[0]!=CurColor);StartRun++);
      for(EndRun=StartRun;(EndRun<endptr)&&(EndRun[0]==CurColor);++EndRun);
    }
  /* Increment and return */
  lineno += 1;
  return refline;
}


// ----------------------------------------
// MAIN DECODING ROUTINE

void 
MMRDecoder::decode_header(ByteStream &inp, int &width, int &height, int &invert)
{
  unsigned long int magic = inp.read32();
  if (magic == 0x4d4d5200) // "MMR\00"
    invert = 0;
  else if (magic == 0x4d4d5201) // "MMR\01"
    invert = 1;
  else 
    THROW("Cannot recognize G4/MMR header");
  width = inp.read16();
  height = inp.read16();
  if (width<=0 || height<=0)
    THROW("Corrupted G4/MMR header");
}

static inline int MAX(int a, int b) { return a>b ? a : b; }
static inline int MIN(int a, int b) { return a<b ? a : b; }

GP<JB2Image>
MMRDecoder::decode(ByteStream &inp)
{
  // Read header
  int width, height, invert;
  decode_header(inp, width, height, invert);
  // Prepare image
  GP<JB2Image> jimg = new JB2Image();
  jimg->set_dimension(width, height);
  // Choose pertinent blocksize
  int blocksize = MIN(500,MAX(64,MAX(width/17,height/22)));
  int blocksperline = (width+blocksize-1)/blocksize;
  // Prepare decoder
  MMRDecoder dcd(inp, width, height);
  // Loop on stripes
  int line = height-1;
  while (line >= 0)
    {
      int stripeline = MIN(blocksize-1,line);
      GPArray<GBitmap> blocks(0,blocksperline-1);
      // Loop on scanlines
      for(; stripeline >= 0; stripeline--,line--)
	{
	  // Decode one scanline
	  const unsigned char *s = dcd.scanline();
          if (s == 0)
            continue;
	  // Loop on blocks
          int x = 0;
          for (int b=0; b<blocksperline; b++)
            {
              unsigned char *bptr = 0;
              int firstx = x;
              int lastx = MIN(x+blocksize,width);
              for(; x<lastx; x++)
                {
                  if (s[x] ^ invert)
                    {
                      // Get pointer to line if needed
                      if (! bptr)
                        {
                          // Create block if needed
                          if (! blocks[b])
                            {
                              int w = lastx - firstx;
                              int h = stripeline + 1;
                              blocks[b] = new GBitmap(h,w);
                            }
                          bptr = (*blocks[b])[stripeline] - firstx;
                        }
                      bptr[x] = 1;
                    }
                }
            }
	}
      // Insert blocks into JB2Image
      for (int b=0; b<blocksperline; b++)
	{
	  JB2Shape shape;
	  shape.bits = blocks[b];
	  if (shape.bits) 
	    {
	      shape.parent = -1;
	      shape.bits->compress();
	      JB2Blit blit;
	      blit.left = b*blocksize;
	      blit.bottom = line+1;
	      blit.shapeno = jimg->add_shape(shape);
	      jimg->add_blit(blit);
	    }
	}
    }
  // Return
  return jimg;
}


