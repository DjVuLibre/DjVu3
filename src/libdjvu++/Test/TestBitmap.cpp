//C-  -*- C++ -*-
//C-
//C- This software contains software code or other information proprietary to
//C- AT&T Corp. ("AT&T").  Unless you accept a license to use this software,
//C- you shall not use, execute, compile, modify, redistribute, reverse
//C- compile, disassemble, or otherwise reverse engineer the AT&T software or
//C- any derived work of the AT&T software.  The text of a license can be
//C- found in file "ATTLICENSE" or at the Internet website having the URL
//C- "http://www.djvu.att.com/open".
//C-
//C- This software is provided to you "AS IS".  YOU "AS IS".  YOU ASSUME TOTAL
//C- RESPONSIBILITY AND RISK FOR YOUR USE OF THEM INCLUDING THE RISK OF ANY
//C- DEFECTS OR INACCURACIES THEREIN.  AT&T DOES NOT MAKE, AND EXPRESSLY
//C- DISCLAIMS, ANY EXPRESS OR IMPLIED WARRANTIES OF ANY KIND WHATSOEVER,
//C- INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTIES OF MERCHANTABILITY
//C- OR FITNESS FOR A PARTICULAR PURPOSE, WARRANTIES OF TITLE OR
//C- NON-INFRINGEMENT OF ANY INTELLECTUAL PROPERTY RIGHTS OR TRADEMARK RIGHTS,
//C- ANY WARRANTIES ARISING BY USAGE OF TRADE, COURSE OF DEALING OR COURSE OF
//C- PERFORMANCE, OR ANY WARRANTY THAT THE AT&T SOURCE CODE RELEASE OR AT&T
//C- CAPSULE ARE "ERROR FREE" WILL MEET YOUR REQUIREMENTS.
//C-
//C-     (C) AT&T Corp. All rights reserved.
//C-     AT&T is a registered trademark of AT&T Corp.
//C-
//C- $Id: TestBitmap.cpp,v 1.6 1999-03-16 20:21:31 leonb Exp $


#include <stdlib.h>
#include <stdio.h>
#include "GBitmap.h"
#include "ByteStream.h"


StaticByteStream bs1 ( 
"P1  16 16\n"
"0000000111000000\n"
"0000001101100000\n"
"0000011000110000\n"
"0000110000011000\n"
"0001100000001100\n"
"0011000000000110\n"
"0011000000000110\n"
"0011000000000110\n"
"0011000000000110\n"
"0011111111111110\n"
"0011111111111110\n"
"0011000000000110\n"
"0011000000000110\n"
"0111100000001111\n"
"0000000000000000\n"
"0000000000000000\n" );

StaticByteStream bs2 ( 
"P1  16 16\n"
"0000000000000000\n"
"0000000000000000\n"
"0000000000000000\n"
"0000011111110000\n"
"0001111000011100\n"
"0011110000000110\n"
"0011100000000110\n"
"0000000000001110\n"
"0000000000111110\n"
"0000001111110110\n"
"0001111000000110\n"
"0011100000000110\n"
"0111000000000110\n"
"0011100000001111\n"
"0001111111110110\n"
"0000000000000000\n" );



int
compare_GBitmap(const GBitmap  &bm1, const GBitmap &bm2)
{
  unsigned int i, j;
#ifdef DEBUG
  bm1.check_border();
  bm2.check_border();
#endif
  if (bm1.rows() == bm2.rows())
    if (bm1.columns() == bm2.columns())
      {
        for (i=0; i<bm1.rows(); i++)
          for (j=0; j<bm2.columns(); j++)
            if (bm1[i][j] != bm2[i][j]) 
              return 0;
        return 1;
      }
  return 0;
}


#define COMPBM(bm1, bm2) \
 printf("%s==%s --> %d\n", #bm1, #bm2, compare_GBitmap(bm1, bm2))

#define PRI(i) \
 printf("%s --> %d\n", #i, i)

int 
main(void)
{

  GBitmap b (bs1);
  GBitmap c (bs2);

  //// Test File I/O
  {
    MemoryByteStream mb;
    b.save_pbm( mb,0 );  mb.seek(0);
    GBitmap p1 ( mb );
    COMPBM(p1, b);
  }
  {
    MemoryByteStream mb;
    b.save_pbm( mb );  mb.seek(0);
    GBitmap p4 ( mb );
    COMPBM(p4, b);
  }
  {
    MemoryByteStream mb;
    b.save_pgm( mb,0 );  mb.seek(0);
    GBitmap p2 ( mb );
    COMPBM(p2, b);
  }
  {
    MemoryByteStream mb;
    b.save_pgm( mb,0 );  mb.seek(0);
    GBitmap p5 ( mb );
    COMPBM(p5, b);
  }
  {
    MemoryByteStream mb;
    b.save_rle( mb );  mb.seek(0);
    GBitmap r4 ( mb );
    COMPBM(r4, b);
  }

  //// Test Copy
  {
    GBitmap b2 = b;
    COMPBM(b2,b);
    COMPBM(b2,c);
    b2 = c;
    COMPBM(b2,c);    
  }

  //// Test Border
  {
    GBitmap b2 (b, 10);
    PRI(b2[-1][-9]);
    PRI(b2[23][23]);
  }

  //// Test Compress
  {
    GBitmap bc (c);
    bc.compress();
    COMPBM(bc,c);
    bc.compress();
    COMPBM(bc,b);
  }

  //// Test Blit from Bytes
  GBitmap d1(24,32);
  GBitmap d2(24,32);
  d1.set_grays(4);
  d2.set_grays(9);
  {
    d1.blit(&b,4,4);
    d1.blit(&c,10,10);
    d1.blit(&b,20,0);
    d2.blit(&c,16,8,1);
    d2.blit(&c,16,8,2);
    d2.blit(&c,0,0,3);
    StdioByteStream cout(stdout,"w");
    d1.save_pgm(cout,0);
    d2.save_pgm(cout,0);
  }
  
  //// Test blit from RLE
  b.compress();
  c.compress();
  GBitmap d1c(24,32);
  GBitmap d2c(24,32);
  d1c.set_grays(4);
  d2c.set_grays(9);
  {
    d1c.blit(&b,4,4);
    d1c.blit(&c,10,10);
    d1c.blit(&b,20,0);
    COMPBM(d1,d1c);
    d2c.blit(&c,16,8,1);
    d2c.blit(&c,16,8,2);
    d2c.blit(&c,0,0,3);
    COMPBM(d2,d2c);
  }
  // end
}
  
  



