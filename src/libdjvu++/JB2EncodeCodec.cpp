//C-  -*- C++ -*-
//C- DjVu� Reference Library (v. 3.0)
//C- 
//C- Copyright � 1999-2000 LizardTech, Inc. All Rights Reserved.
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
//C- 
// 
// $Id: JB2EncodeCodec.cpp,v 1.3 2000-12-22 01:58:34 bcr Exp $
// $Name:  $

#ifndef NEED_DECODER_ONLY

#include "JB2Image.h"
#include "GBitmap.h"
#include <string.h>

////////////////////////////////////////
//// CLASS JB2ENCODECODEC:  DECLARATION
////////////////////////////////////////

// This class is accessed via the encode
// functions of class JB2Image


//**** Class JB2Codec
// This class implements the JB2 coder.
// Contains all contextual information for encoding a JB2Image.

class JB2Dict::JB2EncodeCodec : public JB2Dict::JB2Codec
{
public:
  JB2EncodeCodec(ByteStream &bs);
//virtual
  void code(JB2Image *jim);
  void code(JB2Dict *jim);

protected:
  void CodeNum(const int num, const int lo, const int hi, NumContext &ctx);
  void encode_libonly_shape(JB2Image *jim, int shapeno);
// virtual
  bool CodeBit(const bool bit, BitContext &ctx);
  void code_comment(GString &comment);
  void code_record_type(int &rectype);
  int code_match_index(int &index, JB2Dict *jim);
  void code_inherited_shape_count(JB2Dict *jim);
  void code_image_size(JB2Dict *jim);
  void code_image_size(JB2Image *jim);
  void code_absolute_location(JB2Blit *jblt,  int rows, int columns);
  void code_absolute_mark_size(GBitmap *bm, int border=0);
  void code_relative_mark_size(GBitmap *bm, int cw, int ch, int border=0);
  void code_bitmap_directly(GBitmap &bm,const int dw, int dy,
    unsigned char *up2, unsigned char *up1, unsigned char *up0 );
  int get_diff(const int x_diff,NumContext &rel_loc);
  void code_bitmap_by_cross_coding (GBitmap &bm, GBitmap &cbm,
    const int xd2c, const int dw, int dy, int cy,
    unsigned char *up1, unsigned char *up0, unsigned char *xup1, 
    unsigned char *xup0, unsigned char *xdn1 );

private:
  ZPCodec zp;
};


////////////////////////////////////////
//// CLASS JB2DICT: IMPLEMENTATION
////////////////////////////////////////

void 
JB2Dict::encode(ByteStream &bs) const
{
  JB2EncodeCodec codec(bs);
  codec.code((JB2Dict*)this);
}

////////////////////////////////////////
//// CLASS JB2IMAGE: IMPLEMENTATION
////////////////////////////////////////

void 
JB2Image::encode(ByteStream &bs) const
{
  JB2EncodeCodec codec(bs);
  codec.code((JB2Image*)this);
}

////////////////////////////////////////
//// CLASS JB2CODEC : IMPLEMENTATION
////////////////////////////////////////

#define START_OF_DATA                   (0)
#define NEW_MARK                        (1)
#define NEW_MARK_LIBRARY_ONLY           (2)
#define NEW_MARK_IMAGE_ONLY             (3)
#define MATCHED_REFINE                  (4)
#define MATCHED_REFINE_LIBRARY_ONLY     (5)
#define MATCHED_REFINE_IMAGE_ONLY       (6)
#define MATCHED_COPY                    (7)
#define NON_MARK_DATA                   (8)
#define REQUIRED_DICT_OR_RESET          (9)
#define PRESERVED_COMMENT               (10)
#define END_OF_DATA                     (11)

// STATIC DATA MEMBERS

static const int BIGPOSITIVE = 262142;
static const int BIGNEGATIVE = -262143;
static const int CELLCHUNK = 20000;
static const int CELLEXTRA =   500;

// CONSTRUCTOR

JB2Dict::JB2EncodeCodec::JB2EncodeCodec(ByteStream &bs)
: JB2Dict::JB2Codec(bs,1), zp(bs, true, true) {}

inline bool
JB2Dict::JB2EncodeCodec::CodeBit(const bool bit, BitContext &ctx)
{
    zp.encoder(bit?1:0, ctx);
    return bit;
}

void
JB2Dict::JB2EncodeCodec::CodeNum(int num, int low, int high, NumContext &ctx)
{
  if (num < low || num > high)
    G_THROW("JB2Image.bad_number");
  JB2Codec::CodeNum(low,high,&ctx,num);
}

// CODE COMMENTS

void 
JB2Dict::JB2EncodeCodec::code_comment(GString &comment)
{
  // Encode size
      int size=comment.length();
      CodeNum(size, 0, BIGPOSITIVE, dist_comment_length);
      for (int i=0; i<size; i++) 
        {
          CodeNum(comment[i], 0, 255, dist_comment_byte);
        }
}

// CODE SIMPLE VALUES

inline void 
JB2Dict::JB2EncodeCodec::code_record_type(int &rectype)
{
  CodeNum(rectype, START_OF_DATA, END_OF_DATA, dist_record_type);
}

int 
JB2Dict::JB2EncodeCodec::code_match_index(int &index, JB2Dict *jim)
{
    int match=shape2lib[index];
    CodeNum(match, 0, lib2shape.hbound(), dist_match_index);
    return match;
}

// CODE PAIRS

void
JB2Dict::JB2EncodeCodec::code_inherited_shape_count(JB2Dict *jim)
{
  CodeNum(jim->get_inherited_shape_count(),
    0, BIGPOSITIVE, inherited_shape_count_dist);
}

void 
JB2Dict::JB2EncodeCodec::code_image_size(JB2Dict *jim)
{
  CodeNum(0, 0, BIGPOSITIVE, image_size_dist);
  CodeNum(0, 0, BIGPOSITIVE, image_size_dist);
  JB2Codec::code_image_size(jim);
}

void 
JB2Dict::JB2EncodeCodec::code_image_size(JB2Image *jim)
{
  image_columns = jim->get_width();
  CodeNum(image_columns, 0, BIGPOSITIVE, image_size_dist);
  image_rows = jim->get_height();
  CodeNum(image_rows, 0, BIGPOSITIVE, image_size_dist);
  JB2Codec::code_image_size(jim);
}

inline int
JB2Dict::JB2EncodeCodec::get_diff(int x_diff,NumContext &rel_loc)
{
   CodeNum(x_diff, BIGNEGATIVE, BIGPOSITIVE, rel_loc);
   return x_diff;
}

void 
JB2Dict::JB2EncodeCodec::code_absolute_location(JB2Blit *jblt, int rows, int columns)
{
  // Check start record
  if (!gotstartrecordp)
    G_THROW("JB2Image.no_start");
  // Code TOP and LEFT
  CodeNum(jblt->left+1, 1, image_columns, abs_loc_x);
  CodeNum(jblt->bottom+rows-1+1, 1, image_rows, abs_loc_y);
}

void 
JB2Dict::JB2EncodeCodec::code_absolute_mark_size(GBitmap *bm, int border)
{
  CodeNum(bm->columns(), 0, BIGPOSITIVE, abs_size_x);
  CodeNum(bm->rows(), 0, BIGPOSITIVE, abs_size_y);
}

void 
JB2Dict::JB2EncodeCodec::code_relative_mark_size(GBitmap *bm, int cw, int ch, int border)
{
  CodeNum(bm->columns()-cw, BIGNEGATIVE, BIGPOSITIVE, rel_size_x);
  CodeNum(bm->rows()-ch, BIGNEGATIVE, BIGPOSITIVE, rel_size_y);
}

// CODE BITMAP DIRECTLY

void 
JB2Dict::JB2EncodeCodec::code_bitmap_directly(
  GBitmap &bm,const int dw, int dy,
  unsigned char *up2, unsigned char *up1, unsigned char *up0 )
{
      // iterate on rows (encoding)
      while (dy >= 0)
        {
          int context=get_direct_context(up2, up1, up0, 0);
          for (int dx=0;dx < dw;)
            {
              int n = up0[dx++];
              zp.encoder(n, bitdist[context]);
              context=shift_direct_context(context, n, up2, up1, up0, dx);
            }
          // next row
          dy -= 1;
          up2 = up1;
          up1 = up0;
          up0 = bm[dy];
        }
}

// CODE BITMAP BY CROSS CODING

void 
JB2Dict::JB2EncodeCodec::code_bitmap_by_cross_coding (GBitmap &bm, GBitmap &cbm,
  const int xd2c, const int dw, int dy, int cy,
  unsigned char *up1, unsigned char *up0, unsigned char *xup1, 
  unsigned char *xup0, unsigned char *xdn1 )
{
      // iterate on rows (encoding)
      while (dy >= 0)
        {
          int context=get_cross_context(up1, up0, xup1, xup0, xdn1, 0);
          for(int dx=0;dx < dw;)
            {
              const int n = up0[dx++];
              zp.encoder(n, cbitdist[context]);
              context=shift_cross_context(context, n,  
                                  up1, up0, xup1, xup0, xdn1, dx);
            }
          // next row
          up1 = up0;
          up0 = bm[--dy];
          xup1 = xup0;
          xup0 = xdn1;
          xdn1 = cbm[(--cy)-1] + xd2c;
        }
}

// CODE JB2DICT

void 
JB2Dict::JB2EncodeCodec::code(JB2Dict *jim)
{
      // -------------------------
      // THIS IS THE ENCODING PART
      // -------------------------
      int firstshape = jim->get_inherited_shape_count();
      int nshape = jim->get_shape_count();
      init_library(jim);
      // Code headers.
      int rectype = REQUIRED_DICT_OR_RESET;
      if (jim->get_inherited_shape_count() > 0)
        code_record(rectype, jim, NULL);
      rectype = START_OF_DATA;
      code_record(rectype, jim, NULL);
      // Code Comment.
      rectype = PRESERVED_COMMENT;
      if (!! jim->comment)
        code_record(rectype, jim, NULL);
      // Encode every shape
      int shapeno;
      DJVU_PROGRESS_TASK(jb2code,"jb2 encode", nshape-firstshape);
      for (shapeno=firstshape; shapeno<nshape; shapeno++)
        {
          DJVU_PROGRESS_RUN(jb2code, (shapeno-firstshape)|0xff);
          // Code shape
          JB2Shape *jshp = jim->get_shape(shapeno);
          rectype = NEW_MARK_LIBRARY_ONLY;
          if (jshp->parent >= 0)
            rectype = MATCHED_REFINE_LIBRARY_ONLY;
          code_record(rectype, jim, jshp);
          add_library(shapeno, jshp);
	  // Check numcoder status
	  if (cur_ncell > CELLCHUNK) 
	    {
	      rectype = REQUIRED_DICT_OR_RESET;
	      code_record(rectype, 0, 0);	      
	    }
        }
      // Code end of data record
      rectype = END_OF_DATA;
      code_record(rectype, jim, NULL); 
      zp.ZPCodec::~ZPCodec();
}

// CODE JB2IMAGE

void 
JB2Dict::JB2EncodeCodec::code(JB2Image *jim)
{
      // -------------------------
      // THIS IS THE ENCODING PART
      // -------------------------
      int i;
      init_library(jim);
      int firstshape = jim->get_inherited_shape_count();
      int nshape = jim->get_shape_count();
      int nblit = jim->get_blit_count();
      // Initialize shape2lib 
      shape2lib.resize(0,nshape-1);
      for (i=firstshape; i<nshape; i++)
        shape2lib[i] = -1;
      // Determine shapes that go into library (shapeno>=firstshape)
      //  shape2lib is -2 if used by one blit
      //  shape2lib is -3 if used by more than one blit
      //  shape2lib is -4 if used as a parent
      for (i=0; i<nblit; i++)
        {
          JB2Blit *jblt = jim->get_blit(i);
          int shapeno = jblt->shapeno;
          if (shapeno < firstshape)
            continue;
          if (shape2lib[shapeno] >= -2) 
            shape2lib[shapeno] -= 1;
          shapeno = jim->get_shape(shapeno)->parent;
          while (shapeno>=firstshape && shape2lib[shapeno]>=-3)
            {
              shape2lib[shapeno] = -4;
              shapeno = jim->get_shape(shapeno)->parent;
            }
        }
      // Code headers.
      int rectype = REQUIRED_DICT_OR_RESET;
      if (jim->get_inherited_shape_count() > 0)
        code_record(rectype, jim, NULL, NULL);
      rectype = START_OF_DATA;
      code_record(rectype, jim, NULL, NULL);
      // Code Comment.
      rectype = PRESERVED_COMMENT;
      if (!! jim->comment)
        code_record(rectype, jim, NULL, NULL);
      // Encode every blit
      int blitno;
      DJVU_PROGRESS_TASK(jb2code,"jb2 encode", nblit);
      for (blitno=0; blitno<nblit; blitno++)
        {
          DJVU_PROGRESS_RUN(jb2code, blitno|0xff);
          JB2Blit *jblt = jim->get_blit(blitno);
          int shapeno = jblt->shapeno;
          JB2Shape *jshp = jim->get_shape(shapeno);
          // Tests if shape exists in library
          if (shape2lib[shapeno] >= 0)
            {
              int rectype = MATCHED_COPY;
              code_record(rectype, jim, NULL, jblt);
            }
          // Avoid coding null shapes/blits
          else if (jshp->bits) 
            {
              // Make sure all parents have been coded
              if (jshp->parent>=0 && shape2lib[jshp->parent]<0)
                encode_libonly_shape(jim, jshp->parent);
              // Allocate library entry when needed
#define LIBRARY_CONTAINS_ALL
              int libraryp = 0;
#ifdef LIBRARY_CONTAINS_MARKS // baseline
              if (jshp->parent >= -1)
                libraryp = 1;
#endif
#ifdef LIBRARY_CONTAINS_SHARED // worse             
              if (shape2lib[shapeno] <= -3)
                libraryp = 1;
#endif
#ifdef LIBRARY_CONTAINS_ALL // better
              libraryp = 1;
#endif
              // Test all blit cases
              if (jshp->parent<-1 && !libraryp)
                {
                  int rectype = NON_MARK_DATA;
                  code_record(rectype, jim, jshp, jblt);
                }
              else if (jshp->parent < 0)
                {
                  int rectype = (libraryp ? NEW_MARK : NEW_MARK_IMAGE_ONLY);
                  code_record(rectype, jim, jshp, jblt);
                }
              else 
                {
                  int rectype = (libraryp ? MATCHED_REFINE : MATCHED_REFINE_IMAGE_ONLY);
                  code_record(rectype, jim, jshp, jblt);
                }
              // Add shape to library
              if (libraryp) 
                add_library(shapeno, jshp);
            }
	  // Check numcoder status
	  if (cur_ncell > CELLCHUNK) 
	    {
	      rectype = REQUIRED_DICT_OR_RESET;
	      code_record(rectype, 0, 0, 0);
	    }
        }
      // Code end of data record
      rectype = END_OF_DATA;
      code_record(rectype, jim, NULL, NULL); 
      zp.ZPCodec::~ZPCodec();
}

////////////////////////////////////////
//// HELPERS
////////////////////////////////////////

void 
JB2Dict::JB2EncodeCodec::encode_libonly_shape(JB2Image *jim, int shapeno )
{
  // Recursively encode parent shape
  JB2Shape *jshp = jim->get_shape(shapeno);
  if (jshp->parent>=0 && shape2lib[jshp->parent]<0)
    encode_libonly_shape(jim, jshp->parent);
  // Test that library shape must be encoded
  if (shape2lib[shapeno] < 0)
    {
      // Code library entry
      int rectype = NEW_MARK_LIBRARY_ONLY;
      if (jshp->parent >= 0)
        rectype = MATCHED_REFINE_LIBRARY_ONLY;
      code_record(rectype, jim, jshp, NULL);      
      // Add shape to library
      add_library(shapeno, jshp);
      // Check numcoder status
      if (cur_ncell > CELLCHUNK) 
	{
	  rectype = REQUIRED_DICT_OR_RESET;
	  code_record(rectype, 0, 0, 0);
	}
    }
}

#endif /* NEED_DECODER_ONLY */
