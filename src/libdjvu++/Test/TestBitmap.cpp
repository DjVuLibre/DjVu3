//C-  -*- C++ -*-
//C-
//C-  Copyright © 2000-2001, LizardTech, Inc. All Rights Reserved.
//C-              Unauthorized use prohibited.
//C-
//C-


#include <stdlib.h>
#include <stdio.h>
#include <locale.h>
#include "GBitmap.h"
#include "ByteStream.h"

const char *bmap1=
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
"0000000000000000\n";

const char *bmap2=
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
"0000000000000000\n";



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
 DjVuPrintMessage("%s==%s --> %d\n", #bm1, #bm2, compare_GBitmap(bm1, bm2))

#define PRI(i) \
 DjVuPrintMessage("%s --> %d\n", #i, i)

int 
main(void)
{
  setlocale(LC_ALL,"");

   GP<ByteStream> gbs1=ByteStream::create_static(bmap1,strlen(bmap1));
   GP<ByteStream> gbs2=ByteStream::create_static(bmap2,strlen(bmap2));

   GP<GBitmap> gb=GBitmap::create(*gbs1);
   GP<GBitmap> gc=GBitmap::create(*gbs2);
   GBitmap &b=*gb;
   GBitmap &c=*gc;

  //// Test File I/O
  {
    GP<ByteStream> gmb=ByteStream::create();
    ByteStream &mb=*gmb;
    b.save_pbm( mb,0 );  mb.seek(0);
    GP<GBitmap> gp1=GBitmap::create( mb );
    GBitmap &p1=*gp1;
    COMPBM(p1, b);
  }
  {
    GP<ByteStream> gmb=ByteStream::create();
    ByteStream &mb=*gmb;
    b.save_pbm( mb );  mb.seek(0);
    GP<GBitmap> gp4=GBitmap::create( mb );
    GBitmap &p4=*gp4;
    COMPBM(p4, b);
  }
  {
    GP<ByteStream> gmb=ByteStream::create();
    ByteStream &mb=*gmb;
    b.save_pgm( mb,0 );  mb.seek(0);
    GP<GBitmap> gp2=GBitmap::create( mb );
    GBitmap &p2=*gp2;
    COMPBM(p2, b);
  }
  {
    GP<ByteStream> gmb=ByteStream::create();
    ByteStream &mb=*gmb;
    b.save_pgm( mb,0 );  mb.seek(0);
    GP<GBitmap> gp5=GBitmap::create( mb );
    GBitmap &p5=*gp5;
    COMPBM(p5, b);
  }
  {
    GP<ByteStream> gmb=ByteStream::create();
    ByteStream &mb=*gmb;
    b.save_rle( mb );  mb.seek(0);
    GP<GBitmap> gr4=GBitmap::create( mb );
    GBitmap &r4=*gr4;
    COMPBM(r4, b);
  }

  //// Test Copy
  {
    GP<GBitmap> gb2 =GBitmap::create(b);
    GBitmap &b2=*gb2;
    COMPBM(b2,b);
    COMPBM(b2,c);
    gb2 =GBitmap::create(c);
    COMPBM(*gb2,c);    
  }

  //// Test Border
  {
    GP<GBitmap> gb2=GBitmap::create(b, 10);
    GBitmap &b2=*gb2;
    PRI(b2[-1][-9]);
    PRI(b2[23][23]);
  }

  //// Test Compress
  {
    GP<GBitmap> gbc=GBitmap::create(c);
    GBitmap &bc=*gbc;
    bc.compress();
    COMPBM(bc,c);
    bc.compress();
    COMPBM(bc,b);
  }

  //// Test Blit from Bytes
  GP<GBitmap> gd1=GBitmap::create(24,32);
  GP<GBitmap> gd2=GBitmap::create(24,32);
  GBitmap &d1=*gd1;
  GBitmap &d2=*gd2;
  
  d1.set_grays(4);
  d2.set_grays(9);
  {
    d1.blit(&b,4,4);
    d1.blit(&c,10,10);
    d1.blit(&b,20,0);
    d2.blit(&c,16,8,1);
    d2.blit(&c,16,8,2);
    d2.blit(&c,0,0,3);
    GP<ByteStream> gcout=ByteStream::create(stdout,"w", false); 
    ByteStream &cout=*gcout;
    d1.save_pgm(cout,0);
    d2.save_pgm(cout,0);
  }
  
  //// Test blit from RLE
  b.compress();
  c.compress();
  GP<GBitmap> gd1c=GBitmap::create(24,32);
  GP<GBitmap> gd2c=GBitmap::create(24,32);
  GBitmap &d1c=*gd1c;
  GBitmap &d2c=*gd2c;
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
  
  



