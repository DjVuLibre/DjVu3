//C-  -*- C++ -*-
//C- DjVu® Reference Library (v. 3.0)
//C- 
//C- Copyright © 1999-2000 LizardTech, Inc. All Rights Reserved.
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
// $Id: JB2Image.cpp,v 1.45 2000-12-20 00:00:16 bcr Exp $
// $Name:  $

#ifdef __GNUC__
#pragma implementation
#endif

#include "JB2Image.h"
#include "GException.h"
#include "GThreads.h"
#include "ZPCodec.h"
#include "GRect.h"
#include "GBitmap.h"
#include <string.h>


////////////////////////////////////////
//// CLASS JB2CODEC:  DECLARATION
////////////////////////////////////////

// This class is accessed via the encode and decode
// functions of class JB2Image


//**** Class _JB2Codec
// This class implements both the JB2 coder and decoder.
// Contains all contextual information for encoding/decoding a JB2Image.


class _JB2Codec
{
public:
  // Constructor Destructor
  virtual ~_JB2Codec();
  // Functions
  void set_dict_callback(JB2DecoderCallback *cb, void *arg);
  virtual void code(JB2Image *jim) = 0;
  virtual void code(JB2Dict *jim) = 0;
protected:
  // Forbidden assignment
  _JB2Codec(ByteStream &bs, const bool xencoding=false);
  _JB2Codec(const _JB2Codec &ref);
  _JB2Codec& operator=(const _JB2Codec &ref);
//private:
  // Coder
  ZPCodec zp;
  bool encoding;
  // NumCoder
  typedef unsigned int NumContext;
  static const int BIGPOSITIVE;
  static const int BIGNEGATIVE;
  static const int CELLCHUNK;
  static const int CELLEXTRA;
  int cur_ncell;
  int max_ncell;
  BitContext *bitcells;
  NumContext *leftcell;
  NumContext *rightcell;
  virtual int CodeBit(const int bit, BitContext &ctx) = 0;
  int CodeNum(int lo, int hi, NumContext &ctx,int v);
  void reset_numcoder();
  // Info
  char refinementp;
  char gotstartrecordp;
  JB2DecoderCallback *cbfunc;
  void *cbarg;
  // Code comment
  NumContext dist_comment_byte;
  NumContext dist_comment_length;
  virtual void code_comment(GString &comment) = 0;
  // Code values
  NumContext dist_record_type;
  NumContext dist_match_index;
  BitContext dist_refinement_flag;
  inline void code_eventual_lossless_refinement();
  virtual void code_record_type(int &rectype) = 0;
  virtual int code_match_index(int &index, JB2Dict *jim)=0;
  // Library
  void init_library(JB2Dict *jim);
  int add_library(int shapeno, JB2Shape *jshp);
  GTArray<int> shape2lib;
  GTArray<int> lib2shape;
  struct LibRect { short top,left,right,bottom; };
  GTArray<LibRect> libinfo;
  // Code pairs
  NumContext abs_loc_x;
  NumContext abs_loc_y;
  NumContext abs_size_x;
  NumContext abs_size_y;
  NumContext image_size_dist;
  NumContext inherited_shape_count_dist;
  BitContext offset_type_dist;
  NumContext rel_loc_x_current;
  NumContext rel_loc_x_last;
  NumContext rel_loc_y_current;
  NumContext rel_loc_y_last;
  NumContext rel_size_x;
  NumContext rel_size_y;
  int last_bottom;
  int last_left;
  int last_right;
  int last_row_bottom;
  int last_row_left;
  int image_columns;
  int image_rows;
  virtual void code_inherited_shape_count(JB2Dict *jim)=0;
  virtual void code_image_size(JB2Dict *jim);
  virtual void code_image_size(JB2Image *jim);
  void code_relative_location(JB2Blit *jblt, int rows, int columns);
  virtual void code_absolute_location(JB2Blit *jblt,  int rows, int columns)=0;
  virtual void code_absolute_mark_size(GBitmap *bm, int border=0) = 0;
  virtual void code_relative_mark_size(GBitmap *bm, int cw, int ch, int border=0) = 0;
  int short_list[3];
  int short_list_pos;
  inline void fill_short_list(int v);
  int update_short_list(int v);
  // Code bitmaps
  BitContext bitdist[1024];
  BitContext cbitdist[2048];
  void code_bitmap_directly (GBitmap &bm);
  virtual void code_bitmap_directly(GBitmap &bm,const int dw, int dy,
    unsigned char *up2, unsigned char *up1, unsigned char *up0 )=0;
  void code_bitmap_by_cross_coding (GBitmap &bm, GBitmap *cbm, const int libno);
  virtual void code_bitmap_by_cross_coding (GBitmap &bm, GBitmap &cbm,
    const int xd2c, const int dw, int dy, int cy,
    unsigned char *up1, unsigned char *up0, unsigned char *xup1, 
    unsigned char *xup0, unsigned char *xdn1 )=0;
  // Code records
  void code_record(int &rectype, JB2Dict *jim, JB2Shape *jshp);
  void code_record(int &rectype, JB2Image *jim, JB2Shape *jshp, JB2Blit *jblt);
  // Helpers
  void encode_libonly_shape(JB2Image *jim, int shapeno);
  void compute_bounding_box(GBitmap *cbm, LibRect *lrect);
  virtual int get_diff(int x_diff,NumContext &rel_loc) = 0;
};

class _JB2DecodeCodec : public _JB2Codec
{
public:
  _JB2DecodeCodec(ByteStream &bs);
  virtual ~_JB2DecodeCodec();
  virtual void code(JB2Image *jim);
  virtual void code(JB2Dict *jim);
protected:
  int CodeNum(int lo, int hi, NumContext &ctx);
  virtual int CodeBit(const int bit, BitContext &ctx);
  virtual void code_comment(GString &comment);
  virtual void code_record_type(int &rectype);
  virtual int code_match_index(int &index, JB2Dict *jim);
  virtual void code_inherited_shape_count(JB2Dict *jim);
  virtual void code_image_size(JB2Dict *jim);
  virtual void code_image_size(JB2Image *jim);
  virtual void code_absolute_location(JB2Blit *jblt,  int rows, int columns);
  virtual void code_absolute_mark_size(GBitmap *bm, int border=0);
  virtual void code_relative_mark_size(GBitmap *bm, int cw, int ch, int border=0);
  virtual void code_bitmap_directly(GBitmap &bm,const int dw, int dy,
    unsigned char *up2, unsigned char *up1, unsigned char *up0 );
  virtual void code_bitmap_by_cross_coding (GBitmap &bm, GBitmap &cbm,
    const int xd2c, const int dw, int dy, int cy,
    unsigned char *up1, unsigned char *up0, unsigned char *xup1, 
    unsigned char *xup0, unsigned char *xdn1 );
  virtual int get_diff(int x_diff,NumContext &rel_loc);
};

#ifndef NEED_DECODER_ONLY
class _JB2EncodeCodec : public _JB2Codec
{
public:
  _JB2EncodeCodec(ByteStream &bs);
  virtual ~_JB2EncodeCodec();
  virtual void code(JB2Image *jim);
  virtual void code(JB2Dict *jim);
protected:
  void CodeNum(int num, int lo, int hi, NumContext &ctx);
  virtual int CodeBit(const int bit, BitContext &ctx);
  virtual void code_comment(GString &comment);
  virtual void code_record_type(int &rectype);
  virtual int code_match_index(int &index, JB2Dict *jim);
  virtual void code_inherited_shape_count(JB2Dict *jim);
  virtual void code_image_size(JB2Dict *jim);
  virtual void code_image_size(JB2Image *jim);
  virtual void code_absolute_location(JB2Blit *jblt,  int rows, int columns);
  virtual void code_absolute_mark_size(GBitmap *bm, int border=0);
  virtual void code_relative_mark_size(GBitmap *bm, int cw, int ch, int border=0);
  virtual void code_bitmap_directly(GBitmap &bm,const int dw, int dy,
    unsigned char *up2, unsigned char *up1, unsigned char *up0 );
  virtual int get_diff(int x_diff,NumContext &rel_loc);
  virtual void code_bitmap_by_cross_coding (GBitmap &bm, GBitmap &cbm,
    const int xd2c, const int dw, int dy, int cy,
    unsigned char *up1, unsigned char *up0, unsigned char *xup1, 
    unsigned char *xup0, unsigned char *xdn1 );
};
#endif

////////////////////////////////////////
//// CLASS JB2DICT: IMPLEMENTATION
////////////////////////////////////////


JB2Dict::JB2Dict()
  : inherited_shapes(0)
{
}

void
JB2Dict::init()
{
  inherited_shapes = 0;
  inherited_dict = 0;
  shapes.empty();
}

void 
JB2Dict::set_inherited_dict(GP<JB2Dict> dict)
{
  if (shapes.size() > 0)
    G_THROW("JB2Image.cant_set");
  if (inherited_dict)
    G_THROW("JB2Image.cant_change");
  inherited_dict = dict; 
  inherited_shapes = dict->get_shape_count();
  // Make sure that inherited bitmaps are marked as shared
  for (int i=0; i<inherited_shapes; i++)
    {
      JB2Shape *jshp = dict->get_shape(i);
      if (jshp->bits) jshp->bits->share();
    }
}

void
JB2Dict::compress()
{
  for (int i=shapes.lbound(); i<=shapes.hbound(); i++)
    shapes[i].bits->compress();
}

unsigned int
JB2Dict::get_memory_usage() const
{
  unsigned int usage = sizeof(JB2Dict);
  usage += sizeof(JB2Shape) * shapes.size();
  for (int i=shapes.lbound(); i<=shapes.hbound(); i++)
    if (shapes[i].bits)
      usage += shapes[i].bits->get_memory_usage();
  return usage;
}

int  
JB2Dict::add_shape(const JB2Shape &shape)
{
  if (shape.parent >= get_shape_count())
    G_THROW("JB2Image.bad_parent_shape");
  int index = shapes.size();
  shapes.touch(index);
  shapes[index] = shape;
  return index + inherited_shapes;
}

#ifndef NEED_DECODER_ONLY
void 
JB2Dict::encode(ByteStream &bs) const
{
  _JB2EncodeCodec codec(bs);
  codec.code((JB2Dict*)this);
}
#endif

void 
JB2Dict::decode(ByteStream &bs, JB2DecoderCallback *cb, void *arg)
{
  init();
  _JB2DecodeCodec codec(bs);
  codec.set_dict_callback(cb,arg);
  codec.code((JB2Dict*)this);
}



////////////////////////////////////////
//// CLASS JB2IMAGE: IMPLEMENTATION
////////////////////////////////////////


JB2Image::JB2Image()
  : width(0), height(0), reproduce_old_bug(false)
{
}

void
JB2Image::init()
{
  width = height = 0;
  blits.empty();
  JB2Dict::init();
}

unsigned int
JB2Image::get_memory_usage() const
{
  unsigned int usage = JB2Dict::get_memory_usage();
  usage += sizeof(JB2Image) - sizeof(JB2Dict);
  usage += sizeof(JB2Blit) * blits.size();
  return usage;
}

void 
JB2Image::set_dimension(int awidth, int aheight)
{
  width = awidth;
  height = aheight;
}

int  
JB2Image::add_blit(const JB2Blit &blit)
{
  if (blit.shapeno >= (unsigned int)get_shape_count())
    G_THROW("JB2Image.bad_shape");
  int index = blits.size();
  blits.touch(index);
  blits[index] = blit;
  return index;
}

GP<GBitmap>
JB2Image::get_bitmap(int subsample, int align) const
{
  if (width==0 || height==0)
    G_THROW("JB2Image.cant_create");
  int swidth = (width + subsample - 1) / subsample;
  int sheight = (height + subsample - 1) / subsample;
  int border = ((swidth + align - 1) & ~(align - 1)) - swidth;
  GP<GBitmap> bm = new GBitmap(sheight, swidth, border);
  bm->set_grays(1+subsample*subsample);
  for (int blitno = 0; blitno < get_blit_count(); blitno++)
    {
      const JB2Blit *pblit = get_blit(blitno);
      const JB2Shape  *pshape = get_shape(pblit->shapeno);
      if (pshape->bits)
        bm->blit(pshape->bits, pblit->left, pblit->bottom, subsample);
    }
  return bm;
}

GP<GBitmap>
JB2Image::get_bitmap(const GRect &rect, int subsample, int align, int dispy) const
{
  if (width==0 || height==0)
    G_THROW("JB2Image.cant_create");
  int rxmin = rect.xmin * subsample;
  int rymin = rect.ymin * subsample;
  int swidth = rect.width();
  int sheight = rect.height();
  int border = ((swidth + align - 1) & ~(align - 1)) - swidth;
  GP<GBitmap> bm = new GBitmap(sheight, swidth, border);
  bm->set_grays(1+subsample*subsample);
  for (int blitno = 0; blitno < get_blit_count(); blitno++)
    {
      const JB2Blit *pblit = get_blit(blitno);
      const JB2Shape  *pshape = get_shape(pblit->shapeno);
      if (pshape->bits)
        bm->blit(pshape->bits, pblit->left-rxmin, pblit->bottom-rymin+dispy, subsample);
    }
  return bm;
}


#ifndef NEED_DECODER_ONLY
void 
JB2Image::encode(ByteStream &bs) const
{
  _JB2EncodeCodec codec(bs);
  codec.code((JB2Image*)this);
}
#endif

void 
JB2Image::decode(ByteStream &bs, JB2DecoderCallback *cb, void *arg)
{
  init();
  _JB2DecodeCodec codec(bs);
  codec.set_dict_callback(cb,arg);
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

const int _JB2Codec::BIGPOSITIVE = 262142;
const int _JB2Codec::BIGNEGATIVE = -262143;
const int _JB2Codec::CELLCHUNK = 20000;
const int _JB2Codec::CELLEXTRA =   500;


// CONSTRUCTOR

_JB2DecodeCodec::_JB2DecodeCodec(ByteStream &bs) : _JB2Codec(bs,0) {}

#ifndef NEED_DECODER_ONLY
_JB2EncodeCodec::_JB2EncodeCodec(ByteStream &bs) : _JB2Codec(bs,1) {}
#endif

_JB2Codec::_JB2Codec(ByteStream &bs, const bool xencoding)
  : zp(bs, xencoding, true),
    encoding(xencoding),
    cur_ncell(0),
    bitcells(0),
    leftcell(0),
    rightcell(0),
    refinementp(0),
    gotstartrecordp(0),
    cbfunc(0),
    cbarg(0),
    dist_comment_byte(0),
    dist_comment_length(0),
    dist_record_type(0),
    dist_match_index(0),
    dist_refinement_flag(0),
    abs_loc_x(0),
    abs_loc_y(0),
    abs_size_x(0),
    abs_size_y(0),
    image_size_dist(0),
    inherited_shape_count_dist(0),
    offset_type_dist(0),
    rel_loc_x_current(0),
    rel_loc_x_last(0),
    rel_loc_y_current(0),
    rel_loc_y_last(0),
    rel_size_x(0),
    rel_size_y(0)
{
  memset(bitdist, 0, sizeof(bitdist));
  memset(cbitdist, 0, sizeof(cbitdist));
  // Initialize numcoder
  max_ncell = CELLCHUNK+CELLEXTRA;
  bitcells  = new BitContext[max_ncell];
  leftcell  = new NumContext[max_ncell];
  rightcell = new NumContext[max_ncell];
  bitcells[0] = 0; // dummy cell
  leftcell[0] = rightcell[0] = 0;
  cur_ncell = 1;
}


_JB2Codec::~_JB2Codec()
{
  delete [] bitcells;
  delete [] rightcell;
  delete [] leftcell;
}

_JB2DecodeCodec::~_JB2DecodeCodec() {}

#ifndef NEED_DECODER_ONLY
_JB2EncodeCodec::~_JB2EncodeCodec() {}
#endif

void 
_JB2Codec::reset_numcoder()
{
  dist_comment_byte = 0;
  dist_comment_length = 0;
  dist_record_type = 0;
  dist_match_index = 0;
  abs_loc_x = 0;
  abs_loc_y = 0;
  abs_size_x = 0;
  abs_size_y = 0;
  image_size_dist = 0;
  inherited_shape_count_dist = 0;
  rel_loc_x_current = 0;
  rel_loc_x_last = 0;
  rel_loc_y_current = 0;
  rel_loc_y_last = 0;
  rel_size_x = 0;
  rel_size_y = 0;
  memset(bitcells, 0, sizeof(BitContext)*max_ncell);
  memset(leftcell, 0, sizeof(NumContext)*max_ncell);
  memset(rightcell, 0, sizeof(NumContext)*max_ncell);
  cur_ncell = 1;
}


void 
_JB2Codec::set_dict_callback(JB2DecoderCallback *cb, void *arg)
{
  cbfunc = cb;
  cbarg = arg;
}


// CODE NUMBERS

#ifndef NEED_DECODER_ONLY
inline int
_JB2EncodeCodec::CodeBit(const int bit, BitContext &ctx)
{
    zp.encoder(bit, ctx);
    return bit;
}
#endif

inline int
_JB2DecodeCodec::CodeBit(const int, BitContext &ctx)
{
  return zp.decoder(ctx);
}

int
_JB2DecodeCodec::CodeNum(int low, int high, NumContext &ctx)
{
  return _JB2Codec::CodeNum(low,high,ctx,0);
}

#ifndef NEED_DECODER_ONLY
void
_JB2EncodeCodec::CodeNum(int num, int low, int high, NumContext &ctx)
{
  if (num < low || num > high)
    G_THROW("JB2Image.bad_number");
  _JB2Codec::CodeNum(low,high,ctx,num);
}
#endif

int
_JB2Codec::CodeNum(int low, int high, NumContext &ctx, int v)
{
  int cutoff, decision, negative=0, phase, range, temp;
  NumContext *pctx = &ctx;
  // Check
  if ((int)ctx >= cur_ncell)
    G_THROW("JB2Image.bad_numcontext");
  // Start all phases
  phase = 1;
  cutoff = 0;
  range = 0xffffffff;
  while (range != 1)
    {
      if (! *pctx)
        {
          if (cur_ncell >= max_ncell)
            {
              int nmax_ncell = max_ncell + CELLCHUNK;
              BitContext *nbitcells = new BitContext[nmax_ncell];
              NumContext *nleftcell = new NumContext[nmax_ncell];
              NumContext *nrightcell = new NumContext[nmax_ncell];
              memcpy(nbitcells, bitcells, max_ncell*sizeof(BitContext));
              memcpy(nleftcell, leftcell, max_ncell*sizeof(NumContext));
              memcpy(nrightcell, rightcell, max_ncell*sizeof(NumContext));
              delete bitcells; bitcells=nbitcells;
              delete leftcell; leftcell=nleftcell;
              delete rightcell; rightcell=nrightcell;
              max_ncell = nmax_ncell;
            }
          *pctx = cur_ncell ++;
          bitcells[*pctx] = 0;
          leftcell[*pctx] = rightcell[*pctx] = 0;
        }
      // encode
      if (encoding)
        {
          decision = (v >= cutoff);
          if (low < cutoff && high >= cutoff)
            zp.encoder(decision, bitcells[*pctx]);
        }
      else
        {
          if (low >= cutoff)
            decision = 1;
          else if (high < cutoff) 
            decision = 0;
          else 
            decision = zp.decoder (bitcells[*pctx]);
	}
      // context for new bit
      if (decision)
        pctx = &rightcell[*pctx];
      else
        pctx = &leftcell[*pctx];
      // phase dependent part
      switch (phase) 
        {
	case 1:
          negative = !decision;
          if (negative) 
            {
              if (encoding)
                v = - v - 1;
              temp = - low - 1; 
              low = - high - 1; 
              high = temp;
	    }
          phase = 2; cutoff =  1;
          break;
          
	case 2:
          if (!decision) 
            {
              phase = 3;
              range = (cutoff + 1) / 2;
              if (range == 1)
                cutoff = 0;
              else
                cutoff = cutoff - range / 2;
	    }
          else 
            { 
              cutoff = 2 * cutoff + 1; 
            }
          break;

	case 3:
          range /= 2;
          if (range == 1) 
            {
              if (!decision) 
                cutoff --;
	    }
          else 
            {
              if (!decision)
                cutoff -= range / 2;
              else               
                cutoff += range / 2;
	    }
          break;
	}
    }
    return (negative)?(- cutoff - 1):cutoff;
}



// CODE COMMENTS

#ifndef NEED_DECODER_ONLY
void 
_JB2EncodeCodec::code_comment(GString &comment)
{
  // Encode size
      int size=comment.length();
      CodeNum(size, 0, BIGPOSITIVE, dist_comment_length);
      for (int i=0; i<size; i++) 
        {
          CodeNum(comment[i], 0, 255, dist_comment_byte);
        }
}
#endif

void 
_JB2DecodeCodec::code_comment(GString &comment)
{
      int size=CodeNum(0, BIGPOSITIVE, dist_comment_length);
      comment.empty();
      char *combuf = comment.getbuf(size);
      for (int i=0; i<size; i++) 
        {
          combuf[i]=CodeNum(0, 255, dist_comment_byte);
        }
      comment.getbuf();
}


// LIBRARY


void
_JB2Codec::init_library(JB2Dict *jim)
{
  int nshape = jim->get_inherited_shape_count();
  shape2lib.resize(0,nshape-1);
  lib2shape.resize(0,nshape-1);
  libinfo.resize(0,nshape-1);
  for (int i=0; i<nshape; i++)
    {
      shape2lib[i] = i;
      lib2shape[i] = i;
      JB2Shape *jshp = jim->get_shape(i);
      compute_bounding_box(jshp->bits, &(libinfo[i]));
    }
}

int 
_JB2Codec::add_library(int shapeno, JB2Shape *jshp)
{
  const int libno = lib2shape.hbound() + 1;
  lib2shape.touch(libno);
  lib2shape[libno] = shapeno;
  shape2lib.touch(shapeno);
  shape2lib[shapeno] = libno;
  libinfo.touch(libno);
  compute_bounding_box(jshp->bits, &(libinfo[libno]));
  return libno;
}


// CODE SIMPLE VALUES

#ifndef NEED_DECODER_ONLY
inline void 
_JB2EncodeCodec::code_record_type(int &rectype)
{
  CodeNum(rectype, 
             START_OF_DATA, END_OF_DATA, 
             dist_record_type);
}
#endif

inline void 
_JB2DecodeCodec::code_record_type(int &rectype)
{
  rectype=CodeNum( START_OF_DATA, END_OF_DATA, dist_record_type);
}

inline void
_JB2Codec::code_eventual_lossless_refinement()
{
  refinementp=CodeBit(refinementp, dist_refinement_flag);
}

#ifndef NEED_DECODER_ONLY
int 
_JB2EncodeCodec::code_match_index(int &index, JB2Dict *jim)
{
    int match=shape2lib[index];
    CodeNum(match, 0, lib2shape.hbound(), dist_match_index);
    return match;
}
#endif

int 
_JB2DecodeCodec::code_match_index(int &index, JB2Dict *jim)
{
    int match=CodeNum(0, lib2shape.hbound(), dist_match_index);
    index = lib2shape[match];
    return match;
}


// HANDLE SHORT LIST

inline void 
_JB2Codec::fill_short_list(int v)
{
  short_list[0] = short_list[1] = short_list[2] = v;
  short_list_pos = 0;
}

int 
_JB2Codec::update_short_list(int v)
{
  if (++ short_list_pos == 3)
    short_list_pos = 0;
  int *s = short_list;
  s[short_list_pos] = v;

  if (s[0] >= s[1])
    if (s[0] > s[2])
      if (s[1] >= s[2])
        return s[1];
      else
        return s[2];
    else
      return s[0];
  else
    if (s[0] < s[2])
      if (s[1] >= s[2])
        return s[2];
      else
        return s[1];
    else
      return s[0];
}



// CODE PAIRS


#ifndef NEED_DECODER_ONLY
void
_JB2EncodeCodec::code_inherited_shape_count(JB2Dict *jim)
{
  CodeNum(jim->get_inherited_shape_count(),
    0, BIGPOSITIVE, inherited_shape_count_dist);
}
#endif

void
_JB2DecodeCodec::code_inherited_shape_count(JB2Dict *jim)
{
  int size=CodeNum(0, BIGPOSITIVE, inherited_shape_count_dist);
    {
      GP<JB2Dict> dict = jim->get_inherited_dict();
      if (!dict && size>0)
        {
          // Call callback function to obtain dictionary
          if (cbfunc)
            dict = (*cbfunc)(cbarg);
          if (dict)
            jim->set_inherited_dict(dict);
        }
      if (!dict && size>0)
        G_THROW("JB2Image.need_dict");
      if (dict && size!=dict->get_shape_count())
        G_THROW("JB2Image.bad_dict");
    }
}

#ifndef NEED_DECODER_ONLY
void 
_JB2EncodeCodec::code_image_size(JB2Dict *jim)
{
  CodeNum(0, 0, BIGPOSITIVE, image_size_dist);
  CodeNum(0, 0, BIGPOSITIVE, image_size_dist);
  _JB2Codec::code_image_size(jim);
}
#endif

void 
_JB2DecodeCodec::code_image_size(JB2Dict *jim)
{
  int w=CodeNum(0, BIGPOSITIVE, image_size_dist);
  int h=CodeNum(0, BIGPOSITIVE, image_size_dist);
  if (w || h)
    G_THROW("JB2Image.bad_dict2");
  _JB2Codec::code_image_size(jim);
}

void 
_JB2Codec::code_image_size(JB2Dict *jim)
{
  last_left = 1;
  last_row_left = 0;
  last_row_bottom = 0;
  last_right = 0;
  fill_short_list(last_row_bottom);
  gotstartrecordp = 1;
}

#ifndef NEED_DECODER_ONLY
void 
_JB2EncodeCodec::code_image_size(JB2Image *jim)
{
  image_columns = jim->get_width();
  CodeNum(image_columns, 0, BIGPOSITIVE, image_size_dist);
  image_rows = jim->get_height();
  CodeNum(image_rows, 0, BIGPOSITIVE, image_size_dist);
  _JB2Codec::code_image_size(jim);
}
#endif

void 
_JB2DecodeCodec::code_image_size(JB2Image *jim)
{
  image_columns=CodeNum(0, BIGPOSITIVE, image_size_dist);
  image_rows=CodeNum(0, BIGPOSITIVE, image_size_dist);
  if (!image_columns || !image_rows)
    G_THROW("JB2Image.zero_dim");
  jim->set_dimension(image_columns, image_rows);
  _JB2Codec::code_image_size(jim);
}

void 
_JB2Codec::code_image_size(JB2Image *jim)
{
  last_left = 1 + image_columns;
  last_row_left = 0;
  last_row_bottom = image_rows;
  last_right = 0;
  fill_short_list(last_row_bottom);
  gotstartrecordp = 1;
}

#ifndef NEED_DECODER_ONLY
inline int
_JB2EncodeCodec::get_diff(int x_diff,NumContext &rel_loc)
{
   CodeNum(x_diff, BIGNEGATIVE, BIGPOSITIVE, rel_loc);
   return x_diff;
}
#endif

inline int
_JB2DecodeCodec::get_diff(int,NumContext &rel_loc)
{
   return CodeNum(BIGNEGATIVE, BIGPOSITIVE, rel_loc);
}

void 
_JB2Codec::code_relative_location(JB2Blit *jblt, int rows, int columns)
{
  // Check start record
  if (!gotstartrecordp)
    G_THROW("JB2Image.no_start");
  // Find location
  int bottom=0, left=0, top=0, right=0;
  int x_diff, y_diff;
  if (encoding)
    {
      left = jblt->left + 1;
      bottom = jblt->bottom + 1;
      right = left + columns - 1;
      top = bottom + rows - 1;
    }
  // Code offset type
  int new_row=CodeBit((left<last_left)?1:0, offset_type_dist);
  if (new_row)
    {
      // Begin a new row
      x_diff=get_diff(left-last_row_left,rel_loc_x_last);
      y_diff=get_diff(top-last_row_bottom,rel_loc_y_last);
      if (!encoding)
        {
          left = last_row_left + x_diff;
          top = last_row_bottom + y_diff;
          right = left + columns - 1;
          bottom = top - rows + 1;
        }
      last_left = last_row_left = left;
      last_right = right;
      last_bottom = last_row_bottom = bottom;
      fill_short_list(bottom);
    }
  else
    {
      // Same row
      x_diff=get_diff(left-last_right,rel_loc_x_current);
      y_diff=get_diff(bottom-last_bottom,rel_loc_y_current);
      if (!encoding)
        {
          left = last_right + x_diff;
          bottom = last_bottom + y_diff;
          right = left + columns - 1;
          top = bottom + rows - 1;
        }
      last_left = left;
      last_right = right;
      last_bottom = update_short_list(bottom);
    }
  // Store in blit record
  if (!encoding)
    {
      jblt->bottom = bottom - 1;
      jblt->left = left - 1;
    }
}

#ifndef NEED_DECODER_ONLY
void 
_JB2EncodeCodec::code_absolute_location(JB2Blit *jblt, int rows, int columns)
{
  // Check start record
  if (!gotstartrecordp)
    G_THROW("JB2Image.no_start");
  // Code TOP and LEFT
  CodeNum(jblt->left+1, 1, image_columns, abs_loc_x);
  CodeNum(jblt->bottom+rows-1+1, 1, image_rows, abs_loc_y);
}
#endif

void 
_JB2DecodeCodec::code_absolute_location(JB2Blit *jblt, int rows, int columns)
{
  // Check start record
  if (!gotstartrecordp)
    G_THROW("JB2Image.no_start");
  int left=CodeNum(1, image_columns, abs_loc_x);
  int top=CodeNum(1, image_rows, abs_loc_y);
  jblt->bottom = top - rows + 1 - 1;
  jblt->left = left - 1;
}

#ifndef NEED_DECODER_ONLY
void 
_JB2EncodeCodec::code_absolute_mark_size(GBitmap *bm, int border)
{
  CodeNum(bm->columns(), 0, BIGPOSITIVE, abs_size_x);
  CodeNum(bm->rows(), 0, BIGPOSITIVE, abs_size_y);
}
#endif

void 
_JB2DecodeCodec::code_absolute_mark_size(GBitmap *bm, int border)
{
  int xsize=CodeNum(0, BIGPOSITIVE, abs_size_x);
  int ysize=CodeNum(0, BIGPOSITIVE, abs_size_y);
  bm->init(ysize, xsize, border);
}

#ifndef NEED_DECODER_ONLY
void 
_JB2EncodeCodec::code_relative_mark_size(GBitmap *bm, int cw, int ch, int border)
{
  CodeNum(bm->columns()-cw, BIGNEGATIVE, BIGPOSITIVE, rel_size_x);
  CodeNum(bm->rows()-ch, BIGNEGATIVE, BIGPOSITIVE, rel_size_y);
}
#endif

void 
_JB2DecodeCodec::code_relative_mark_size(GBitmap *bm, int cw, int ch, int border)
{
  int xdiff=CodeNum(BIGNEGATIVE, BIGPOSITIVE, rel_size_x);
  int ydiff=CodeNum(BIGNEGATIVE, BIGPOSITIVE, rel_size_y);
  bm->init(ch + ydiff, cw + xdiff, border);
}




// CODE BITMAP DIRECTLY

static inline int
get_direct_context( unsigned char const * const up2,
                    unsigned char const * const up1,
                    unsigned char const * const up0,
                    const int column)
{
  return ( (up2[column - 1] << 9) |
              (up2[column    ] << 8) |
              (up2[column + 1] << 7) |
              (up1[column - 2] << 6) |
              (up1[column - 1] << 5) |
              (up1[column    ] << 4) |
              (up1[column + 1] << 3) |
              (up1[column + 2] << 2) |
              (up0[column - 2] << 1) |
              (up0[column - 1] << 0) );
}

static inline int
shift_direct_context(const int context, const int next,
                     unsigned char const * const up2,
                     unsigned char const * const up1,
                     unsigned char const * const up0,
                     const int column)
{
  return ( ((context << 1) & 0x37a) |
              (up1[column + 2] << 2)   |
              (up2[column + 1] << 7)   |
              (next << 0)              );
}

void 
_JB2Codec::code_bitmap_directly (GBitmap &bm)
{
  // Make sure bitmap will not be disturbed
  GMonitorLock lock(bm.monitor());
  // ensure borders are adequate
  bm.minborder(3);
  // initialize row pointers
  int dy = bm.rows() - 1;
  code_bitmap_directly(bm,bm.columns(),dy,bm[dy+2],bm[dy+1],bm[dy]);
}

#ifndef NEED_DECODER_ONLY
void 
_JB2EncodeCodec::code_bitmap_directly(
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
#endif

void 
_JB2DecodeCodec::code_bitmap_directly(
  GBitmap &bm,const int dw, int dy,
  unsigned char *up2, unsigned char *up1, unsigned char *up0 )
{
      // iterate on rows (decoding)      
      while (dy >= 0)
        {
          int context=get_direct_context(up2, up1, up0, 0);
          for(int dx=0;dx < dw;)
            {
              int n = zp.decoder(bitdist[context]);
              up0[dx++] = n;
              context=shift_direct_context(context, n, up2, up1, up0, dx);
            }
          // next row
          dy -= 1;
          up2 = up1;
          up1 = up0;
          up0 = bm[dy];
        }
#ifdef DEBUG
      bm.check_border();
#endif
}





// CODE BITMAP BY CROSS CODING


static inline int
get_cross_context( unsigned char const * const up1,
                   unsigned char const * const up0,
                   unsigned char const * const xup1,
                   unsigned char const * const xup0,
                   unsigned char const * const xdn1,
                   const int column )
{
  return ( ( up1[column - 1] << 10) |
              ( up1[column    ] <<  9) |
              ( up1[column + 1] <<  8) |
              ( up0[column - 1] <<  7) |
              (xup1[column    ] <<  6) |
              (xup0[column - 1] <<  5) |
              (xup0[column    ] <<  4) |
              (xup0[column + 1] <<  3) |
              (xdn1[column - 1] <<  2) |
              (xdn1[column    ] <<  1) |
              (xdn1[column + 1] <<  0) );
}


static inline int
shift_cross_context( const int context, const int n,
                     unsigned char const * const up1,
                     unsigned char const * const up0,
                     unsigned char const * const xup1,
                     unsigned char const * const xup0,
                     unsigned char const * const xdn1,
                     const int column )
{
  return ( ((context<<1) & 0x636)  |
              ( up1[column + 1] << 8) |
              (xup1[column    ] << 6) |
              (xup0[column + 1] << 3) |
              (xdn1[column + 1] << 0) |
              (n << 7)             );
}


void 
_JB2Codec::code_bitmap_by_cross_coding (GBitmap &bm, GBitmap *cbm, const int libno)
{
  // Make sure bitmaps will not be disturbed
  GBitmap copycbm;
  if (cbm->monitor())
    {
      // Perform a copy when the bitmap is explicitely shared
      GMonitorLock lock2(cbm->monitor());
      copycbm.init(*cbm);
      cbm = &copycbm;
    }
  GMonitorLock lock1(bm.monitor());
  // Center bitmaps
  const int cw = cbm->columns();
  const int dw = bm.columns();
  const int dh = bm.rows();
  const LibRect &l = libinfo[libno];
  const int xd2c = (dw/2 - dw + 1) - ((l.right - l.left + 1)/2 - l.right);
  const int yd2c = (dh/2 - dh + 1) - ((l.top - l.bottom + 1)/2 - l.top);
  // Ensure borders are adequate
  bm.minborder(2);
  cbm->minborder(2-xd2c);
  cbm->minborder(2+dw+xd2c-cw);
  // Initialize row pointers
  const int dy = dh - 1;
  const int cy = dy + yd2c;
#ifdef DEBUG
  bm.check_border();
  cbm->check_border();
#endif
  code_bitmap_by_cross_coding (bm,*cbm, xd2c, dw, dy, cy, bm[dy+1], bm[dy],
    (*cbm)[cy+1] + xd2c, (*cbm)[cy  ] + xd2c, (*cbm)[cy-1] + xd2c);
}

void 
_JB2EncodeCodec::code_bitmap_by_cross_coding (GBitmap &bm, GBitmap &cbm,
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
              const int n = up0[dx];
              zp.encoder(n, cbitdist[context]);
              dx += 1;
              context=shift_cross_context(context, n,  
                                  up1, up0, xup1, xup0, xdn1, dx);
            }
          // next row
          dy -= 1;
          cy -= 1;
          up1 = up0;
          up0 = bm[dy];
          xup1 = xup0;
          xup0 = xdn1;
          xdn1 = cbm[cy-1] + xd2c;
        }
}

void 
_JB2DecodeCodec::code_bitmap_by_cross_coding (GBitmap &bm, GBitmap &cbm,
  const int xd2c, const int dw, int dy, int cy,
  unsigned char *up1, unsigned char *up0, unsigned char *xup1, 
  unsigned char *xup0, unsigned char *xdn1 )
{
      // iterate on rows (decoding)      
      while (dy >= 0)
        {
          int context=get_cross_context(
                            up1, up0, xup1, xup0, xdn1, 0);
          for(int dx=0;dx < dw;)
            {
              const int n = zp.decoder(cbitdist[context]);
              up0[dx] = n;
              dx += 1;
              context=shift_cross_context(context, n,  
                                  up1, up0, xup1, xup0, xdn1, dx);
            }
          // next row
          dy -= 1;
          cy -= 1;
          up1 = up0;
          up0 = bm[dy];
          xup1 = xup0;
          xup0 = xdn1;
          xdn1 = cbm[cy-1] + xd2c;
#ifdef DEBUG
          bm.check_border();
#endif
        }
}




// CODE JB2DICT RECORD

void
_JB2Codec::code_record(int &rectype, JB2Dict *jim, JB2Shape *jshp)
{
  GBitmap *cbm = 0;
  GBitmap *bm = 0;
  int shapeno = -1;

  // Code record type
  code_record_type(rectype);
  
  // Pre-coding actions
  switch(rectype)
    {
    case NEW_MARK_LIBRARY_ONLY:
    case MATCHED_REFINE_LIBRARY_ONLY:
      if (!encoding) 
        {
          jshp->bits = new GBitmap;
          jshp->parent = -1;
        }
      bm = jshp->bits;
      break;
    }
  // Coding actions
  switch (rectype)
    {
    case START_OF_DATA:
      {
        code_image_size (jim);
        code_eventual_lossless_refinement ();
        if (! encoding)
          init_library(jim);
        break;
      }
    case NEW_MARK_LIBRARY_ONLY:
      {
        code_absolute_mark_size (bm, 4);
        code_bitmap_directly (*bm);
        break;
      }
    case MATCHED_REFINE_LIBRARY_ONLY:
      {
        int match = code_match_index (jshp->parent, jim);
        cbm = jim->get_shape(jshp->parent)->bits;
        LibRect &l = libinfo[match];
        code_relative_mark_size (bm, l.right-l.left+1, l.top-l.bottom+1, 4);
        code_bitmap_by_cross_coding (*bm, cbm, jshp->parent);
        break;
      }
    case PRESERVED_COMMENT:
      {
        code_comment(jim->comment);
        break;
      }
    case REQUIRED_DICT_OR_RESET:
      {
        if (! gotstartrecordp)
	  // Indicates need for a shape dictionary
	  code_inherited_shape_count(jim);
	else
	  // Reset all numerical contexts to zero
	  reset_numcoder();
        break;
      }
    case END_OF_DATA:
      {
        break;
      }
    default:
      {
        G_THROW("JB2Image.bad_type");
      }
    }
  // Post-coding action
  if (!encoding)
    {
      // add shape to dictionary
      switch(rectype)
        {
        case NEW_MARK_LIBRARY_ONLY:
        case MATCHED_REFINE_LIBRARY_ONLY:
          shapeno = jim->add_shape(*jshp);
          add_library(shapeno, jshp);
          break;
        }
      // make sure everything is compacted
      // decompaction will occur automatically when needed
      if (bm)
        bm->compress();
    }
}


// CODE JB2DICT

#ifndef NEED_DECODER_ONLY
void 
_JB2EncodeCodec::code(JB2Dict *jim)
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
#endif

void 
_JB2DecodeCodec::code(JB2Dict *jim)
{
      // -------------------------
      // THIS IS THE DECODING PART
      // -------------------------
      int rectype;
      JB2Shape tmpshape;
      do
        {
          code_record(rectype, jim, &tmpshape);        
        } 
      while(rectype != END_OF_DATA);
      if (!gotstartrecordp)
        G_THROW("JB2Image.no_start");
      jim->compress();
}



// CODE JB2IMAGE RECORD

void
_JB2Codec::code_record(int &rectype, JB2Image *jim, JB2Shape *jshp, JB2Blit *jblt)
{
  GBitmap *bm = 0;
  GBitmap *cbm;
  int shapeno = -1;
  int match;

  // Code record type
  code_record_type(rectype);
  
  // Pre-coding actions
  switch(rectype)
    {
    case NEW_MARK:
    case NEW_MARK_LIBRARY_ONLY:
    case NEW_MARK_IMAGE_ONLY:
    case MATCHED_REFINE:
    case MATCHED_REFINE_LIBRARY_ONLY:
    case MATCHED_REFINE_IMAGE_ONLY:
    case NON_MARK_DATA:
      if (!encoding) 
        {
          jshp->bits = new GBitmap;
          jshp->parent = -1;
          if (rectype == NON_MARK_DATA)
            jshp->parent = -2;
        }
      bm = jshp->bits;
      break;
    }
  // Coding actions
  switch (rectype)
    {
    case START_OF_DATA:
      {
        code_image_size (jim);
        code_eventual_lossless_refinement ();
        if (! encoding)
          init_library(jim);
        break;
      }
    case NEW_MARK:
      {
        code_absolute_mark_size (bm, 4);
        code_bitmap_directly (*bm);
        code_relative_location (jblt, bm->rows(), bm->columns() );
        break;
      }
    case NEW_MARK_LIBRARY_ONLY:
      {
        code_absolute_mark_size (bm, 4);
        code_bitmap_directly (*bm);
        break;
      }
    case NEW_MARK_IMAGE_ONLY:
      {
        code_absolute_mark_size (bm, 3);
        code_bitmap_directly (*bm);
        code_relative_location (jblt, bm->rows(), bm->columns() );
        break;
      }
    case MATCHED_REFINE:
      {
        match = code_match_index (jshp->parent, jim);
        cbm = jim->get_shape(jshp->parent)->bits;
        LibRect &l = libinfo[match];
        code_relative_mark_size (bm, l.right-l.left+1, l.top-l.bottom+1, 4); 
        code_bitmap_by_cross_coding (*bm, cbm, match);
        code_relative_location (jblt, bm->rows(), bm->columns() );
        break;
      }
    case MATCHED_REFINE_LIBRARY_ONLY:
      {
        match = code_match_index (jshp->parent, jim);
        cbm = jim->get_shape(jshp->parent)->bits;
        LibRect &l = libinfo[match];
        code_relative_mark_size (bm, l.right-l.left+1, l.top-l.bottom+1, 4);
        break;
      }
    case MATCHED_REFINE_IMAGE_ONLY:
      {
        match = code_match_index (jshp->parent, jim);
        cbm = jim->get_shape(jshp->parent)->bits;
        LibRect &l = libinfo[match];
        code_relative_mark_size (bm, l.right-l.left+1, l.top-l.bottom+1, 4);
        code_bitmap_by_cross_coding (*bm, cbm, match);
        code_relative_location (jblt, bm->rows(), bm->columns() );
        break;
      }
    case MATCHED_COPY:
      {
        int temp;
        if (encoding) temp = jblt->shapeno;
        match = code_match_index (temp, jim);
        if (!encoding) jblt->shapeno = temp;
        bm = jim->get_shape(jblt->shapeno)->bits;
        LibRect &l = libinfo[match];
        jblt->left += l.left;
        jblt->bottom += l.bottom;
        if (jim->reproduce_old_bug)
          code_relative_location (jblt, bm->rows(), bm->columns() );
        else
          code_relative_location (jblt, l.top-l.bottom+1, l.right-l.left+1 );
        jblt->left -= l.left;
        jblt->bottom -= l.bottom; 
        break;
      }
    case NON_MARK_DATA:
      {
        code_absolute_mark_size (bm, 3);
        code_bitmap_directly (*bm);
        code_absolute_location (jblt, bm->rows(), bm->columns() );
        break;
      }
    case PRESERVED_COMMENT:
      {
        code_comment(jim->comment);
        break;
      }
    case REQUIRED_DICT_OR_RESET:
      {
        if (! gotstartrecordp)
	  // Indicates need for a shape dictionary
	  code_inherited_shape_count(jim);
	else
	  // Reset all numerical contexts to zero
	  reset_numcoder();
        break;
      }
    case END_OF_DATA:
      {
        break;
      }
    default:
      {
        G_THROW("JB2Image.unknown_type");
      }
    }
  
  // Post-coding action
  if (!encoding)
    {
      // add shape to image
      switch(rectype)
        {
        case NEW_MARK:
        case NEW_MARK_LIBRARY_ONLY:
        case NEW_MARK_IMAGE_ONLY:
        case MATCHED_REFINE:
        case MATCHED_REFINE_LIBRARY_ONLY:
        case MATCHED_REFINE_IMAGE_ONLY:
        case NON_MARK_DATA:
          shapeno = jim->add_shape(*jshp);
          shape2lib.touch(shapeno);
          shape2lib[shapeno] = -1;
          break;
        }
      // add shape to library
      switch(rectype)
        {
        case NEW_MARK:
        case NEW_MARK_LIBRARY_ONLY:
        case MATCHED_REFINE:
        case MATCHED_REFINE_LIBRARY_ONLY:
          add_library(shapeno, jshp);
          break;
        }
      // make sure everything is compacted
      // decompaction will occur automatically on cross-coding bitmaps
      if (bm)
        bm->compress();
      // add blit to image
      switch (rectype)
        {
        case NEW_MARK:
        case NEW_MARK_IMAGE_ONLY:
        case MATCHED_REFINE:
        case MATCHED_REFINE_IMAGE_ONLY:
        case NON_MARK_DATA:
          jblt->shapeno = shapeno;
        case MATCHED_COPY:
          jim->add_blit(* jblt);
          break;
        }
    }
}


// CODE JB2IMAGE

#ifndef NEED_DECODER_ONLY
void 
_JB2EncodeCodec::code(JB2Image *jim)
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
#endif

void 
_JB2DecodeCodec::code(JB2Image *jim)
{
      // -------------------------
      // THIS IS THE DECODING PART
      // -------------------------
      int rectype;
      JB2Blit tmpblit;
      JB2Shape tmpshape;
      do
        {
          code_record(rectype, jim, &tmpshape, &tmpblit);        
        } 
      while(rectype!=END_OF_DATA);
      if (!gotstartrecordp)
        G_THROW("JB2Image.no_start");
      jim->compress();
}



////////////////////////////////////////
//// HELPERS
////////////////////////////////////////

#ifndef NEED_DECODER_ONLY
void 
_JB2Codec::encode_libonly_shape(JB2Image *jim, int shapeno )
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
#endif

void 
_JB2Codec::compute_bounding_box(GBitmap *bm, LibRect *lib)
{
  // First lock the stuff.
  GMonitorLock lock(bm->monitor());
  // Get size
  int w = bm->columns();
  int h = bm->rows();
  int s = bm->rowsize();
  int n;
  // Right border
  lib->right = w;
  while (--lib->right >= 0)
    {
      unsigned char *p = (*bm)[0] + lib->right;
      for (n=0; n<h; n++,p+=s) if (*p) break;
      if (n<h) break;
    }
  // Top border
  lib->top = h;
  while (--lib->top >= 0)
    {
      unsigned char *p = (*bm)[lib->top];
      for (n=0; n<w; n++,p++) if (*p) break;
      if (n<w) break;
    }
  // Left border
  lib->left = -1;
  while (++lib->left <= lib->right)
    {
      unsigned char *p = (*bm)[0] + lib->left;
      for (n=0; n<h; n++,p+=s) if (*p) break;
      if (n<h) break;
    }
  // Bottom border
  lib->bottom = -1;
  while (++lib->bottom <= lib->top)
    {
      unsigned char *p = (*bm)[lib->bottom];
      for (n=0; n<w; n++,p++) if (*p) break;
      if (n<w) break;
    }
}

