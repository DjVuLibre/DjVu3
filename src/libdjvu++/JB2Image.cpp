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
//C- $Id: JB2Image.cpp,v 1.7 1999-03-17 19:24:58 leonb Exp $


#ifdef __GNUC__
#pragma implementation
#endif

#include <string.h>
#include "JB2Image.h"
#include "GException.h"


////////////////////////////////////////
//// CLASS JB2CODEC:  DECLARATION
////////////////////////////////////////

// This class is accessed via the encode and decode
// functions of class JB2Image


// STRICT_PGH
// This macro conditionally selects code
// that complies strictly with PGH format.

#define STRICT_PGH 1


//**** Class JB2Codec
// This class implements both the JB2 coder and decoder.
// Contains all contextual information for encoding/decoding a JB2Image.


class JB2Image::JB2Codec {

public:
  // Constructor Destructor
  JB2Codec(ByteStream &bs, int encoding=0);
  ~JB2Codec();
  // Functions
  void code(JB2Image *jim);
protected:
  // Forbidden assignment
  JB2Codec(const JB2Codec &ref);
  JB2Codec& operator=(const JB2Codec &ref);
private:
  // Coder
  ZPCodec zp;
  int encoding;
  // NumCoder
  typedef unsigned int NumContext;
  static const int BIGPOSITIVE;
  static const int BIGNEGATIVE;
  static const int MAX_NCELLS;
  int cur_ncell;
  BitContext *bitcells;
  NumContext *leftcell;
  NumContext *rightcell;
  void CodeBit(int &bit, BitContext &ctx);
  void CodeNum(int &num, int lo, int hi, NumContext &ctx);
  // Info
  char refinementp;
  char gotstartrecordp;
  // Code comment
  NumContext dist_comment_byte;
  NumContext dist_comment_length;
  void code_comment(GString &comment);
  // Code values
  NumContext dist_record_type;
  NumContext dist_match_index;
  BitContext dist_refinement_flag;
  void code_eventual_lossless_refinement();
  void code_record_type(int &rectype);
  void code_match_index(int &index);
  GArray<int> shape2lib;
  GArray<int> lib2shape;
  // Code pairs
  NumContext abs_loc_x;
  NumContext abs_loc_y;
  NumContext abs_size_x;
  NumContext abs_size_y;
  NumContext image_size_dist;
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
  void code_image_size(JB2Image *jim);
  void code_relative_location(JB2Blit *jblt, int rows, int columns);
  void code_absolute_location(JB2Blit *jblt,  int rows, int columns);
  void code_absolute_mark_size(GBitmap *bm, int border=0);
  void code_relative_mark_size(GBitmap *bm, int cw, int ch, int border=0);
  int short_list[3];
  int short_list_pos;
  void fill_short_list(int v);
  int update_short_list(int v);
  // Code bitmaps
  BitContext bitdist[1024];
  BitContext cbitdist[2048];
  void code_bitmap_directly (GBitmap *bm);
  void code_bitmap_by_cross_coding (GBitmap *bm, GBitmap *cbm, int libno);
  // Code records
  void code_record(int &rectype, JB2Image *jim, JB2Shape *jshp, JB2Blit *jblt);
  // Private helpers
  void encode_libonly_shape(JB2Image *jim, int shapeno);
#ifdef STRICT_PGH
  struct LibRect { short top,left,right,bottom; };
  GArray<LibRect> libinfo;
  void compute_pgh_size(int libno, GBitmap *cbm);
#endif
};




////////////////////////////////////////
//// CLASS JB2IMAGE: IMPLEMENTATION
////////////////////////////////////////


JB2Image::JB2Image()
  : width(0), height(0)
{
}

void
JB2Image::init()
{
  width = height = 0;
  shapes.empty();
  blits.empty();
}

void
JB2Image::compress()
{
  for (int i=shapes.lbound(); i<=shapes.hbound(); i++)
    shapes[i].bits->compress();
}

unsigned int
JB2Image::get_memory_usage() const
{
  unsigned int usage = sizeof(JB2Image);
  usage += sizeof(JB2Blit) * blits.size();
  usage += sizeof(JB2Shape) * shapes.size();
  for (int i=shapes.lbound(); i<=shapes.hbound(); i++)
    if (shapes[i].bits)
      usage += shapes[i].bits->get_memory_usage();
  return usage;
}

void 
JB2Image::set_dimension(int awidth, int aheight)
{
  width = awidth;
  height = aheight;
}

int  
JB2Image::add_shape(const JB2Shape &shape)
{
  int index = shapes.size();
  shapes.touch(index);
  shapes[index] = shape;
  return index;
}

int  
JB2Image::add_blit(const JB2Blit &blit)
{
  int index = blits.size();
  blits.touch(index);
  blits[index] = blit;
  return index;
}

GP<GBitmap>
JB2Image::get_bitmap(int subsample, int align) const
{
  if (width==0 || height==0)
    THROW("Cannot create bitmap image for an unsized JB2Image object");
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
    THROW("Cannot create bitmap image for an unsized JB2Image object");
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


void 
JB2Image::encode(ByteStream &bs, int) const
{
  JB2Codec codec(bs, 1);
  codec.code((JB2Image*)this);
}

void 
JB2Image::decode(ByteStream &bs, int)
{
  init();
  JB2Codec codec(bs);
  codec.code(this);
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
#define LOSSLESS_REFINEMENT             (9)
#define PRESERVED_COMMENT               (10)
#define END_OF_DATA                     (11)


// STATIC DATA MEMBERS

const int JB2Image::JB2Codec::BIGPOSITIVE = 262142;
const int JB2Image::JB2Codec::BIGNEGATIVE = -262143;
const int JB2Image::JB2Codec::MAX_NCELLS = 20000;


// CONSTRUCTOR

JB2Image::JB2Codec::JB2Codec(ByteStream &bs, int encoding)
  : zp(bs, encoding),
    encoding(encoding),
    cur_ncell(0),
    bitcells(0),
    leftcell(0),
    rightcell(0),
    refinementp(0),
    gotstartrecordp(0),
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
  bitcells  = new BitContext[MAX_NCELLS+2];
  leftcell  = new NumContext[MAX_NCELLS+2];
  rightcell = new NumContext[MAX_NCELLS+2];
  bitcells[0] = 0; // dummy cell
  leftcell[0] = rightcell[0] = 0;
  bitcells[1] = 0; // overflow cell
  leftcell[1] = rightcell[1] = 1;
  cur_ncell = 2;
}


JB2Image::JB2Codec::~JB2Codec()
{
  delete [] bitcells;
  delete [] rightcell;
  delete [] leftcell;
}

// CODE NUMBERS

inline void 
JB2Image::JB2Codec::CodeBit(int &bit, BitContext &ctx)
{
  if (encoding)
    zp.encoder(bit, ctx);
  else
    bit = zp.decoder(ctx);
}

void 
JB2Image::JB2Codec::CodeNum(int &num, int low, int high, NumContext &ctx)
{

  int cutoff, decision, negative=0, phase, range, temp, v=0;
  NumContext *pctx = &ctx;
  // Check
  if ((int)ctx >= cur_ncell)
    THROW("(JB2Codec::CodeNum) Illegal NumContext");
  if (encoding)
    if (num < low || num > high)
      THROW("(JB2Codec::CodeNum) Number is outside the bounds.");
  // Initialize
  if (encoding) 
    v = num;
  // Start all phases
  phase = 1;
  cutoff = 0;
  range = 0xffffffff;
  while (range != 1)
    {
      if (! *pctx)
        {
          if (cur_ncell < MAX_NCELLS+2)
            {
              *pctx = cur_ncell ++;
              bitcells[*pctx] = 0;
              leftcell[*pctx] = rightcell[*pctx] = 0;
            }
          else 
            {
              *pctx = 1;
            }
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
  // Terminate
  if (! encoding)
    {
      if (negative)
        num = - cutoff - 1;
      else
        num = cutoff;
    }
}



// CODE COMMENTS

void 
JB2Image::JB2Codec::code_comment(GString &comment)
{
  // Encode size
  if (encoding)
    {
      int size = comment.length();
      CodeNum(size, 0, BIGPOSITIVE, dist_comment_length);
      for (int i=0; i<size; i++) 
        {
          int ch = comment[i];
          CodeNum(ch, 0, 255, dist_comment_byte);
        }
    }
  else
    {
      int size, ch;
      CodeNum(size, 0, BIGPOSITIVE, dist_comment_length);
      comment.empty();
      char *combuf = comment.getbuf(size);
      for (int i=0; i<size; i++) 
        {
          CodeNum(ch, 0, 255, dist_comment_byte);
          combuf[i] = ch;
        }
      comment.getbuf();
    }
}


// CODE SIMPLE VALUES

void 
JB2Image::JB2Codec::code_record_type(int &rectype)
{
  CodeNum(rectype, 
             START_OF_DATA, END_OF_DATA, 
             dist_record_type);
}

void
JB2Image::JB2Codec::code_eventual_lossless_refinement()
{
  int bit;
  if (encoding)
    bit = refinementp;
  CodeBit(bit, dist_refinement_flag);
  if (!encoding)
    refinementp = bit;
}

void 
JB2Image::JB2Codec::code_match_index(int &index)
{
  CodeNum(index,
             lib2shape.lbound(), lib2shape.hbound(), 
             dist_match_index);
}


// HANDLE SHORT LIST

void 
JB2Image::JB2Codec::fill_short_list(int v)
{
  short_list[0] = short_list[1] = short_list[2] = v;
  short_list_pos = 0;
}

int 
JB2Image::JB2Codec::update_short_list(int v)
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


void 
JB2Image::JB2Codec::code_image_size(JB2Image *jim)
{
  if (encoding)
    {
      image_columns = jim->get_width();
      image_rows = jim->get_height();
    }
  CodeNum(image_columns, 0, BIGPOSITIVE, image_size_dist);
  CodeNum(image_rows, 0, BIGPOSITIVE, image_size_dist);
  if (!encoding)
    {
      jim->set_dimension(image_columns, image_rows);
    }
  last_left = 1 + image_columns;
  last_row_left = 0;
  last_row_bottom = image_rows;
  last_right = 0;
  fill_short_list(last_row_bottom);
  gotstartrecordp = 1;
}

void 
JB2Image::JB2Codec::code_relative_location(JB2Blit *jblt, int rows, int columns)
{
  // Check start record
  if (!gotstartrecordp)
    THROW("Corrupted file: No start record");
  // Find location
  int bottom=0, left=0, top=0, right=0;
  int new_row, x_diff, y_diff;
  if (encoding)
    {
      left = jblt->left;
      bottom = jblt->bottom;
#ifdef STRICT_PGH
      left += 1;
      bottom += 1;
#endif
      right = left + columns - 1;
      top = bottom + rows - 1;
      if (left < last_left)
        new_row = 1;
      else
        new_row = 0;
    }
  // Code offset type
  CodeBit(new_row, offset_type_dist);
  if (new_row)
    {
      // Begin a new row
      if (encoding)
        {
          x_diff = left - last_row_left;
          y_diff = top - last_row_bottom;
        }
      CodeNum(x_diff, BIGNEGATIVE, BIGPOSITIVE, rel_loc_x_last);
      CodeNum(y_diff, BIGNEGATIVE, BIGPOSITIVE, rel_loc_y_last);
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
      if (encoding)
        {
          x_diff = left - last_right;
          y_diff = bottom - last_bottom;
        }
      CodeNum(x_diff, BIGNEGATIVE, BIGPOSITIVE, rel_loc_x_current);
      CodeNum(y_diff, BIGNEGATIVE, BIGPOSITIVE, rel_loc_y_current);
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
#ifdef STRICT_PGH
      bottom -= 1;
      left -= 1;
#endif
      jblt->bottom = bottom;
      jblt->left = left;
    }
}

void 
JB2Image::JB2Codec::code_absolute_location(JB2Blit *jblt, int rows, int columns)
{
  // Check start record
  if (!gotstartrecordp)
    THROW("Corrupted file: No start record");
  // Code TOP and LEFT
  int top, left;
  if (encoding)
    {
      top = jblt->bottom + rows - 1;
      left = jblt->left;
#ifdef STRICT_PGH
      top += 1;
      left += 1;
#endif
    }
  CodeNum(left, 1, image_columns, abs_loc_x);
  CodeNum(top, 1, image_rows, abs_loc_y);
  if (!encoding)
    {
      jblt->bottom = top - rows + 1;
      jblt->left = left;
#ifdef STRICT_PGH
      jblt->bottom -= 1;
      jblt->left -= 1;
#endif
    }
}

void 
JB2Image::JB2Codec::code_absolute_mark_size(GBitmap *bm, int border)
{
  int xsize, ysize;
  if (encoding) 
    {
      xsize = bm->columns();
      ysize = bm->rows();
    }
  CodeNum(xsize, 0, BIGPOSITIVE, abs_size_x);
  CodeNum(ysize, 0, BIGPOSITIVE, abs_size_y);
  if (!encoding)
    {
      bm->init(ysize, xsize, border);
    }
}

void 
JB2Image::JB2Codec::code_relative_mark_size(GBitmap *bm, int cw, int ch, int border)
{
  int xdiff, ydiff;
  if (encoding) 
    {
      xdiff = bm->columns() - cw;
      ydiff = bm->rows() - ch;
    }
  CodeNum(xdiff, BIGNEGATIVE, BIGPOSITIVE, rel_size_x);
  CodeNum(ydiff, BIGNEGATIVE, BIGPOSITIVE, rel_size_y);
  if (!encoding)
    {
      bm->init(ch + ydiff, cw + xdiff, border);
    }
}




// CODE BITMAP DIRECTLY

static inline void
get_direct_context(int &context,
                   unsigned char *up2,
                   unsigned char *up1,
                   unsigned char *up0,
                   int column)
{
  context = ( (up2[column - 1] << 9) |
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

static inline void
shift_direct_context(int &context, int next,
                     unsigned char *up2,
                     unsigned char *up1,
                     unsigned char *up0,
                     int column)
{
  context = ( ((context << 1) & 0x37a) |
              (up1[column + 2] << 2)   |
              (up2[column + 1] << 7)   |
              (next << 0)              );
}


void 
JB2Image::JB2Codec::code_bitmap_directly (GBitmap *bm)
{
  // ensure borders are adequate
  bm->minborder(3);
  // initialize row pointers
  int dw = bm->columns();
  int dh = bm->rows();
  int dy = dh - 1;
  unsigned char *up2 = (*bm)[dy+2];
  unsigned char *up1 = (*bm)[dy+1];
  unsigned char *up0 = (*bm)[dy  ];
  int context;
  int next;
  // test case
  if (encoding)
    {
      // iterate on rows (encoding)
      while (dy >= 0)
        {
          int dx = 0;
          get_direct_context(context, up2, up1, up0, 0);
          while (dx < dw)
            {
              next = up0[dx];
              zp.encoder(next, bitdist[context]);
              dx += 1;
              shift_direct_context(context, next, up2, up1, up0, dx);
            }
          // next row
          dy -= 1;
          up2 = up1;
          up1 = up0;
          up0 = (*bm)[dy];
        }
    }
  else
    {
      // iterate on rows (decoding)      
      while (dy >= 0)
        {
          int dx = 0;
          get_direct_context(context, up2, up1, up0, 0);
          while (dx < dw)
            {
              next = zp.decoder(bitdist[context]);
              up0[dx] = next;
              dx += 1;
              shift_direct_context(context, next, up2, up1, up0, dx);
            }
          // next row
          dy -= 1;
          up2 = up1;
          up1 = up0;
          up0 = (*bm)[dy];
        }
#ifdef DEBUG
      bm->check_border();
#endif
    }
}





// CODE BITMAP BY CROSS CODING


static inline void
get_cross_context( int &context,
                   unsigned char *up1,
                   unsigned char *up0,
                   unsigned char *xup1,
                   unsigned char *xup0,
                   unsigned char *xdn1,
                   int column )
{
  context = ( ( up1[column - 1] << 10) |
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


static inline void
shift_cross_context( int &context, int next,
                     unsigned char *up1,
                     unsigned char *up0,
                     unsigned char *xup1,
                     unsigned char *xup0,
                     unsigned char *xdn1,
                     int column )
{
  context = ( ((context<<1) & 0x636)  |
              ( up1[column + 1] << 8) |
              (xup1[column    ] << 6) |
              (xup0[column + 1] << 3) |
              (xdn1[column + 1] << 0) |
              (next << 7)             );
}


void 
JB2Image::JB2Codec::code_bitmap_by_cross_coding (GBitmap *bm, GBitmap *cbm, int libno)
{
  int cw = cbm->columns();
  int ch = cbm->rows();
  int dw = bm->columns();
  int dh = bm->rows();
  
  // center bitmaps
#ifdef STRICT_PGH
  LibRect &l = libinfo[libno];
  int xd2c = (dw/2 - dw + 1) - ((l.right - l.left + 1)/2 - l.right);
  int yd2c = (dh/2 - dh + 1) - ((l.top - l.bottom + 1)/2 - l.top);
#else
  int xd2c = (cw - cw/2) - (dw - dw/2);
  int yd2c = (ch - ch/2) - (dh - dh/2);
#endif

  // ensure borders are adequate
  bm->minborder(2);
  cbm->minborder(2-xd2c);
  cbm->minborder(2+dw+xd2c-cw);

  // initialize row pointers
  int next;
  int context;
  int dy = dh - 1;
  int cy = dy + yd2c;
  unsigned char *up1  =  (*bm)[dy+1];
  unsigned char *up0  =  (*bm)[dy  ];
  unsigned char *xup1 = (*cbm)[cy+1] + xd2c;
  unsigned char *xup0 = (*cbm)[cy  ] + xd2c;
  unsigned char *xdn1 = (*cbm)[cy-1] + xd2c;
#ifdef DEBUG
  bm->check_border();
  cbm->check_border();
#endif

  // test case
  if (encoding)
    {
      // iterate on rows (encoding)
      while (dy >= 0)
        {
          int dx = 0;
          get_cross_context(context, up1, up0, xup1, xup0, xdn1, 0);
          while (dx < dw)
            {
              next = up0[dx];
              zp.encoder(next, cbitdist[context]);
              dx += 1;
              shift_cross_context(context, next,  
                                  up1, up0, xup1, xup0, xdn1, dx);
            }
          // next row
          dy -= 1;
          cy -= 1;
          up1 = up0;
          up0 = (*bm)[dy];
          xup1 = xup0;
          xup0 = xdn1;
          xdn1 = (*cbm)[cy-1] + xd2c;
        }
    }
  else
    {
      // iterate on rows (decoding)      
      while (dy >= 0)
        {
          int dx = 0;
          get_cross_context(context, 
                            up1, up0, xup1, xup0, xdn1, 0);
          while (dx < dw)
            {
              next = zp.decoder(cbitdist[context]);
              up0[dx] = next;
              dx += 1;
              shift_cross_context(context, next,  
                                  up1, up0, xup1, xup0, xdn1, dx);
            }
          // next row
          dy -= 1;
          cy -= 1;
          up1 = up0;
          up0 = (*bm)[dy];
          xup1 = xup0;
          xup0 = xdn1;
          xdn1 = (*cbm)[cy-1] + xd2c;
#ifdef DEBUG
          bm->check_border();
#endif
        }
    }
}




// CODE RECORDS

void
JB2Image::JB2Codec::code_record(int &rectype, JB2Image *jim, JB2Shape *jshp, JB2Blit *jblt)
{
  GBitmap *bm = 0;
  GBitmap *cbm;
  int shapeno = -1;
  int indexno = -1;
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
          jshp->parent = (rectype==NON_MARK_DATA ? -2 : -1);
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
        break;
      }
    case NEW_MARK:
      {
        code_absolute_mark_size (bm, 4);
        code_bitmap_directly (bm);
        code_relative_location (jblt, bm->rows(), bm->columns() );
        break;
      }
    case NEW_MARK_LIBRARY_ONLY:
      {
        code_absolute_mark_size (bm, 4);
        code_bitmap_directly (bm);
        break;
      }
    case NEW_MARK_IMAGE_ONLY:
      {
        code_absolute_mark_size (bm, 3);
        code_bitmap_directly (bm);
        code_relative_location (jblt, bm->rows(), bm->columns() );
        break;
      }
    case MATCHED_REFINE:
      {
        if (encoding)
          match = shape2lib[jshp->parent];
        code_match_index (match);
        if (!encoding)
          jshp->parent = lib2shape[match];
        cbm = jim->get_shape(jshp->parent)->bits;
#ifdef STRICT_PGH
        LibRect &l = libinfo[match];
        code_relative_mark_size (bm, l.right-l.left+1, l.top-l.bottom+1, 4); 
#else
        code_relative_mark_size (bm, cbm->columns(), cbm->rows(), 4);
#endif
        code_bitmap_by_cross_coding (bm, cbm, match);
        code_relative_location (jblt, bm->rows(), bm->columns() );
        break;
      }
    case MATCHED_REFINE_LIBRARY_ONLY:
      {
        if (encoding)
          match = shape2lib[jshp->parent];
        code_match_index (match);
        if (!encoding)
          jshp->parent = lib2shape[match];
        cbm = jim->get_shape(jshp->parent)->bits;
#ifdef STRICT_PGH
        LibRect &l = libinfo[match];
        code_relative_mark_size (bm, l.right-l.left+1, l.top-l.bottom+1, 4);
#else
        code_relative_mark_size (bm, cbm->columns(), cbm->rows(), 4);
#endif
        code_bitmap_by_cross_coding (bm, cbm, match);
        break;
      }
    case MATCHED_REFINE_IMAGE_ONLY:
      {
        if (encoding)
          match = shape2lib[jshp->parent];
        code_match_index (match);
        if (!encoding)
          jshp->parent = lib2shape[match];
        cbm = jim->get_shape(jshp->parent)->bits;
#ifdef STRICT_PGH
        LibRect &l = libinfo[match];
        code_relative_mark_size (bm, l.right-l.left+1, l.top-l.bottom+1, 4);
#else
        code_relative_mark_size (bm, cbm->columns(), cbm->rows(), 2);
#endif
        code_bitmap_by_cross_coding (bm, cbm, match);
        code_relative_location (jblt, bm->rows(), bm->columns() );
        break;
      }
    case MATCHED_COPY:
      {
        if (encoding)
          match = shape2lib[(int)(jblt->shapeno)];
        code_match_index (match);
        if (!encoding)
          jblt->shapeno = lib2shape[match];
        bm = jim->get_shape(jblt->shapeno)->bits;
#ifdef STRICT_PGH
        LibRect &l = libinfo[match];
        jblt->left += l.left;
        jblt->bottom += l.bottom;
        // -- Theoretically we should do
        // code_relative_location (jblt, l.top-l.bottom+1, l.right-l.left+1 );
        //    but there is still something which does not match
        //    Paul's code. So we keep doing what the old plugin does
        //    because the new encoder does not use Paul's code either.
        code_relative_location (jblt, bm->rows(), bm->columns() );
        jblt->left -= l.left;
        jblt->bottom -= l.bottom; 
#else
        code_relative_location (jblt, bm->rows(), bm->columns() );
#endif
        break;
      }
    case NON_MARK_DATA:
      {
        code_absolute_mark_size (bm, 3);
        code_bitmap_directly (bm);
        code_absolute_location (jblt, bm->rows(), bm->columns() );
        break;
      }
    case PRESERVED_COMMENT:
      {
        code_comment(jim->comment);
        break;
      }
    case LOSSLESS_REFINEMENT:
    case END_OF_DATA:
      break;
    default:
      THROW("Corrupted file: Unknown record type");
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
          indexno = lib2shape.hbound() + 1;
          lib2shape.touch(indexno);
          lib2shape[indexno] = shapeno;
          shape2lib[shapeno] = indexno;
#ifdef STRICT_PGH
          compute_pgh_size(indexno, bm);
#endif
          break;
        }
      // save memory (almost zero impact on decompression time)
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
#ifdef TRACEJB2
      switch (rectype)
        {
        case START_OF_DATA:
          printf("P %d %d 0\n", 
                 jim->get_width(), jim->get_height());
          break;
        case NEW_MARK:
        case NEW_MARK_IMAGE_ONLY:
          printf("S %d %d -1 1 0 %d %d\n", 
                 jblt->left+1, jblt->bottom+bm->rows(), 
                 bm->columns(), bm->rows());
          break;
        case MATCHED_REFINE:
        case MATCHED_REFINE_IMAGE_ONLY:
          printf("S %d %d %d 1 1 %d %d\n", 
                 jblt->left+1, jblt->bottom+bm->rows(), match, 
                 bm->columns(), bm->rows());
          break;
        case MATCHED_COPY:
          printf("S %d %d %d 0 0 %d %d\n", 
                 jblt->left+1, jblt->bottom+bm->rows(), match, 
                 -1, -1);
          break;
        case NON_MARK_DATA:
          printf("N %d %d %d %d\n", 
                 jblt->left+1, jblt->bottom+bm->rows(), 
                 bm->columns(), bm->rows());
          break;
        }
#endif
    }
}


// CODE IMAGE

void 
JB2Image::JB2Codec::code(JB2Image *jim)
{
  // Easy check
  if (gotstartrecordp)
    THROW("Please use a fresh JB2Codec to code a new image");

  // Test case
  if (encoding)
    // -------------------------
    // THIS IS THE ENCODING PART
    {
      int i;
      int nshape = jim->get_shape_count();
      int nblit = jim->get_blit_count();
      // Initialize shape2lib 
      shape2lib.resize(0, nshape-1);
      for (i=0; i<nshape; i++)
        shape2lib[i] = -1;
      // Determine shapes that go into library
      //  shape2lib is -2 if used by one blit
      //  shape2lib is -3 if used by more than one blit
      //  shape2lib is -4 if used as a parent
      for (i=0; i<nblit; i++)
        {
          JB2Blit *jblt = jim->get_blit(i);
          int shapeno = jblt->shapeno;
          if (shape2lib[shapeno] >= -2) 
            shape2lib[shapeno] -= 1;
          shapeno = jim->get_shape(shapeno)->parent;
          while (shapeno>=0 && shape2lib[shapeno]>=-3)
            {
              shape2lib[shapeno] = -4;
              shapeno = jim->get_shape(shapeno)->parent;
            }
        }
      // Code start of data token
      int rectype = START_OF_DATA;
      code_record(rectype, jim, NULL, NULL);
      // Comment.
      rectype = PRESERVED_COMMENT;
      if (!! jim->comment)
        code_record(rectype, jim, NULL, NULL);
      // Encode every blit
      int blitno;
      for (blitno=0; blitno<nblit; blitno++)
        {
          JB2Blit *jblt = jim->get_blit(blitno);
          int shapeno = jblt->shapeno;
          JB2Shape *jshp = jim->get_shape(shapeno);
          // Progress indicator
          DJVU_PROGRESS("code_record", blitno*100/nblit);
          // Test if shape is already in library
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
              int libno = -1;
#ifdef LIBRARY_CONTAINS_MARKS // baseline
              if (jshp->parent >= -1)
                libno = lib2shape.hbound() + 1;
#endif
#ifdef LIBRARY_CONTAINS_SHARED // worse             
              if (shape2lib[shapeno] <= -3)
                libno = lib2shape.hbound() + 1;
#endif
#ifdef LIBRARY_CONTAINS_ALL // better
              libno = lib2shape.hbound() + 1;
#endif
              // Test all blit cases
              if (jshp->parent < -1 && libno<0)
                {
                  int rectype = NON_MARK_DATA;
                  code_record(rectype, jim, jshp, jblt);
                }
              else if (jshp->parent < 0)
                {
                  int rectype = ((libno>=0) ? NEW_MARK : NEW_MARK_IMAGE_ONLY );
                  code_record(rectype, jim, jshp, jblt);
                }
              else 
                {
                  int rectype = ((libno>=0) ? MATCHED_REFINE : MATCHED_REFINE_IMAGE_ONLY );
                  code_record(rectype, jim, jshp, jblt);
                }
              // Add shape to library
              if (libno >= 0) 
                {
                  lib2shape.touch(libno);
                  lib2shape[libno] = shapeno;
                  shape2lib[shapeno] = libno;
#ifdef STRICT_PGH
                  compute_pgh_size(libno, jshp->bits);
#endif
                }
            }
        }
      // Code end of data record
      rectype = END_OF_DATA;
      code_record(rectype, jim, NULL, NULL); 
      zp.ZPCodec::~ZPCodec();
      // Progress
      DJVU_PROGRESS("code_record", 999);
    }
  else
    // -------------------------
    // THIS IS THE DECODING PART
    {
      int rectype;
      JB2Blit tmpblit;
      JB2Shape tmpshape;
      for(;;) 
        {
          code_record(rectype, jim, &tmpshape, &tmpblit);        
          if (rectype == END_OF_DATA)
            break;
          if (rectype == LOSSLESS_REFINEMENT)
            break;
        } 
      if (!gotstartrecordp)
        THROW("Corrupted file: No start record");
      jim->compress();
    }
}


// HELPER FUNCTIONS

void 
JB2Image::JB2Codec::encode_libonly_shape(JB2Image *jim, int shapeno )
{
  // Recursively encode parent shape
  JB2Shape *jshp = jim->get_shape(shapeno);
  if (jshp->parent>=0 && shape2lib[jshp->parent]<0)
    encode_libonly_shape(jim, jshp->parent);
  // Test that library shape must be encoded
  if (shape2lib[shapeno] < 0)
    {
      // Allocate library entry
      int libno = lib2shape.hbound() + 1;
      // Code library entry
      int rectype = NEW_MARK_LIBRARY_ONLY;
      if (jshp->parent >= 0)
        rectype = MATCHED_REFINE_LIBRARY_ONLY;
      code_record(rectype, jim, jshp, NULL);      
      // Add shape to library
      lib2shape.touch(libno);
      lib2shape[libno] = shapeno;
      shape2lib[shapeno] = libno;
#ifdef STRICT_PGH
      compute_pgh_size(libno, jshp->bits);
#endif
    }
}


#ifdef STRICT_PGH
void 
JB2Image::JB2Codec::compute_pgh_size(int libno, GBitmap *cbm)
{
  int nrows = cbm->rows();
  int ncolumns = cbm->columns();
  int top, bottom, left, right;
  top = 0;
  bottom = nrows;
  right = 0;
  left = ncolumns;
  for (int r = 0; r < nrows; r++)
    {
      unsigned char *row = (*cbm)[r];
      for (int c = 0; c < ncolumns; c++)
        if (row[c])
          {
            if (c < left)   left = c;
            if (c > right)  right = c;
            if (r > top)    top = r;
            if (r < bottom) bottom = r;
          }
    }
  if (top < bottom)
    top = bottom = 0;
  if (right < left)
    right = left = 0;
  libinfo.touch(libno);
  libinfo[libno].right = right;
  libinfo[libno].left = left;
  libinfo[libno].top = top;
  libinfo[libno].bottom = bottom;
}
#endif


