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
//C- $Id: cpaldjvu.cpp,v 1.5 2000-02-24 20:23:07 leonb Exp $


/** @name cpaldjvu

    {\bf Synopsis}
    \begin{verbatim}
        cpaldjvu [options] <inputppmfile> <outputdjvufile>
    \end{verbatim}

    {\bf Description}
    
    File #"cpaldjvu.cpp"# demonstrates a simple quasi-lossless compressor for
    low resolution, low color, images with a reduced number of colors (e.g
    screendumps).  It simply quantizes the image on a limited number of
    colors, uses the dominant color to construct a uniform background, then
    performs lossless jb2 compression for all remaining objects.  
    
    Options
    \begin{description}
    \item[-colors n]  Maximum number of colors during quantization (default 256)
    \item[-dpi n]     Resolution written into the output file (default 100).
    \item[-verbose]   Displays additional messages.
    \end{description}

    {\bf Remarks}

    This is an interesting alternative to GIF. It performs especially well on
    screendumps.  Compression ratios can get hurt when there are continuous
    tone segment in the image.  Demoting such segments from foreground to
    background is a pretty interesting project.  Dithered segments behave
    surprisingly well.

    @memo
    Simple lossless encoder for low resolution, low color images.
    @author
    L\'eon Bottou <leonb@research.att.com>
    @version
    #$Id: cpaldjvu.cpp,v 1.5 2000-02-24 20:23:07 leonb Exp $# */
//@{
//@}


#include "DjVuGlobal.h"
#include "GException.h"
#include "GSmartPointer.h"
#include "GContainer.h"
#include "ByteStream.h"
#include "IFFByteStream.h"
#include "GRect.h"
#include "GBitmap.h"
#include "JB2Image.h"
#include "DjVuPalette.h"
#include "IWImage.h"
#include "DjVuInfo.h"


#undef MIN
#undef MAX
inline int MIN(int a, int b) { return ( a<b ?a :b); }
inline int MAX(int a, int b) { return ( a>b ?a :b); }




// --------------------------------------------------
// COLOR CONNECTED COMPONENT ANALYSIS
// --------------------------------------------------

// -- A run of pixels with the same color
struct Run    
{ 
  short y;       // vertical coordinate
  short x1;      // first horizontal coordinate
  short x2;      // last horizontal coordinate
  short color;   // color id
  int ccid;      // component id
};


// -- A component descriptor
struct CC    
{
  GRect bb;      // bounding box
  int npix;      // number of black pixels
  int nrun;      // number of runs
  int frun;      // first run in cc ordered array of runs
  int color;     // color id
};


// -- An image composed of runs
class CCImage 
{
public:
  int height;            // Height of the image in pixels
  int width;             // Width of the image in pixels
  GTArray<Run> runs;     // Array of runs
  GTArray<CC>  ccs;      // Array of component descriptors
  int nregularccs;       // Number of regular ccs (set by merge_and_split_ccs)
  CCImage(int width, int height);
  void add_single_run(int y, int x1, int x2, int color, int ccid=0);
  GP<GBitmap> get_bitmap_for_cc(int ccid) const;
  void make_ccids_by_analysis();
  void make_ccs_from_ccids();
  void merge_and_split_ccs(int smallsize, int largesize);
  void sort_in_reading_order(); 
  void erase_cc(int ccid);
};


// -- Compares runs
static inline bool
operator <= (const Run &a, const Run &b)
{
  return (a.y<b.y) || (a.y==b.y && a.x1<=b.x1);
}


// -- Constructs CCImage and provide defaults
CCImage::CCImage(int width, int height)
  : height(height), width(width), nregularccs(0)
{
}


// -- Adds a run to the CCImage
inline void 
CCImage::add_single_run(int y, int x1, int x2, int color, int ccid=0)
{
  int index = runs.hbound();
  runs.touch(++index);
  Run& run = runs[index];
  run.y = y;
  run.x1 = x1;
  run.x2 = x2;
  run.color = color;
  run.ccid = ccid;
  
}


// -- Performs color connected component analysis
void
CCImage::make_ccids_by_analysis()
{
  // Sort runs
  runs.sort();
  // Single Pass Connected Component Analysis (with unodes)
  int n;
  int p=0;
  GTArray<int> umap;
  for (n=0; n<=runs.hbound(); n++)
    {
      int y = runs[n].y;
      int x1 = runs[n].x1 - 1;
      int x2 = runs[n].x2 + 1;
      int color = runs[n].color;
      int id = (umap.hbound() + 1);
      // iterate over previous line runs
      if (p>0) p--;
      for(;runs[p].y < y-1;p++);
      for(;(runs[p].y < y) && (runs[p].x1 <= x2);p++ )
        {
          if ( runs[p].x2 >= x1 )
            {
              if (runs[p].color == color)
                {
                  // previous run touches current run and has same color
                  int oid = runs[p].ccid;
                  while (umap[oid] < oid)
                    oid = umap[oid];
                  if ((int)id > umap.hbound()) {
                    id = oid;
                  } else if (id < oid) {
                    umap[oid] = id;
                  } else {
                    umap[id] = oid;
                    id = oid;
                  }
                  // freshen previous run id
                  runs[p].ccid = id;
                }
              // stop if previous run goes past current run
              if (runs[p].x2 >= x2)
                break;
            }
        }
      // create new entry in umap
      runs[n].ccid = id;
      if (id > umap.hbound())
        {
          umap.touch(id);
          umap[id] = id;
        }
    }
  // Update umap and ccid
  for (n=0; n<=runs.hbound(); n++)
    {
      Run &run = runs[n];
      int ccid = run.ccid;
      while (umap[ccid] < ccid)
        ccid = umap[ccid];
      umap[run.ccid] = ccid;
      run.ccid = ccid;
    }
}


// -- Constructs the ``ccs'' array from run's ccids.
void
CCImage::make_ccs_from_ccids()
{
  int n;
  Run *pruns = runs;
  // Find maximal ccid
  int maxccid = -1;
  for (n=0; n<=runs.hbound(); n++)
    if (pruns[n].ccid > maxccid)
      maxccid = runs[n].ccid;
  GTArray<int> armap(0,maxccid);
  int *rmap = armap;
  // Renumber ccs 
  for (n=0; n<=maxccid; n++)
    armap[n] = -1;
  for (n=0; n<=runs.hbound(); n++)
    if (pruns[n].ccid >= 0)
      rmap[ pruns[n].ccid ] = 1;
  int nid = 0;
  for (n=0; n<=maxccid; n++)
    if (rmap[n] > 0)
      rmap[n] = nid++;
  // Adjust nregularccs (since ccs are renumbered)
  while (nregularccs>0 && rmap[nregularccs-1]<0)
    nregularccs -= 1;
  if (nregularccs>0)
    nregularccs = 1 + rmap[nregularccs-1];
  // Prepare cc descriptors
  ccs.resize(0,nid-1);
  for (n=0; n<nid; n++)
    ccs[n].nrun = 0;
  // Relabel runs
  for (n=0; n<=runs.hbound(); n++)
    {
      Run &run = pruns[n];
      if (run.ccid < 0) continue;  // runs with negative ccids are destroyed
      int oldccid = run.ccid;
      int newccid = rmap[oldccid];
      CC &cc = ccs[newccid];
      run.ccid = newccid;
      cc.nrun += 1;
    }
  // Compute positions for runs of cc
  int frun = 0;
  for (n=0; n<nid; n++) 
    {
      ccs[n].frun = rmap[n] = frun;
      frun += ccs[n].nrun;
    }
  // Copy runs
  GTArray<Run> rtmp;
  rtmp.steal(runs);
  Run *ptmp = rtmp;
  runs.resize(0,frun-1);
  pruns = runs;
  for (n=0; n<=rtmp.hbound(); n++)
    {
      int id = ptmp[n].ccid;
      if (id < 0) continue;
      int pos = rmap[id]++;
      pruns[pos] = ptmp[n];
    }
  // Finalize ccs
  for (n=0; n<nid; n++)
    {
      CC &cc = ccs[n];
      int npix = 0;
      runs.sort(cc.frun, cc.frun+cc.nrun-1);
      Run *run = &runs[cc.frun];
      int xmin = run->x1;
      int xmax = run->x2;
      int ymin = run->y;
      int ymax = run->y;
      cc.color = run->color;
      for (int i=0; i<cc.nrun; i++, run++)
        {
          if (run->x1 < xmin)  xmin = run->x1;
          if (run->x2 > xmax)  xmax = run->x2;
          if (run->y  < ymin)  ymin = run->y;
          if (run->y  > ymax)  ymax = run->y;
          npix += run->x2 - run->x1 + 1;
        }
      cc.npix = npix;
      cc.bb.xmin = xmin;
      cc.bb.ymin = ymin;
      cc.bb.xmax = xmax + 1;
      cc.bb.ymax = ymax + 1;
    }
}


// -- Helper for merge_and_split_ccs
struct Grid_x_Color 
{
  short gridi;
  short gridj;
  int color;
};


// -- Helper for merge_and_split_ccs
static inline unsigned int
hash(const Grid_x_Color &x) 
{
  return (x.gridi<<16) ^ (x.gridj<<8) ^ x.color;
}


// -- Helper for merge_and_split_ccs
static inline bool
operator==(const Grid_x_Color &x, const Grid_x_Color &y)
{
  return (x.gridi==y.gridi) && (x.gridj==y.gridj) && (x.color==y.color);
}


// -- Helper for merge_and_split_ccs
static int
makeccid(const Grid_x_Color &x, GMap<Grid_x_Color,int> &map, int &ncc)
{
  GPosition p = map.contains(x);
  if (p) return map[p];
  return map[x] = ncc++;
}


// -- Merges small ccs of similar color and splits large ccs
void
CCImage::merge_and_split_ccs(int smallsize, int largesize)
{
  int ncc = ccs.size();
  int nruns = runs.size();
  int splitsize = largesize;
  if (ncc <= 0) return;
  // Associative map for storing merged ccids
  GMap<Grid_x_Color,int> map;
  nregularccs = ncc;
  // Set the correct ccids for the runs
  for (int ccid=0; ccid<ccs.size(); ccid++)
    {
      CC* cc = &ccs[ccid];
      if (cc->nrun <= 0) continue;
      Grid_x_Color key;
      key.color = cc->color;
      int ccheight = cc->bb.height();
      int ccwidth = cc->bb.width();
      if (ccheight<=smallsize && ccwidth<=smallsize)
        {
          key.gridi = (cc->bb.ymin+cc->bb.ymax)/splitsize/2;
          key.gridj = (cc->bb.xmin+cc->bb.xmax)/splitsize/2;
          int newccid = makeccid(key, map, ncc);
          for(int runid=cc->frun; runid<cc->frun+cc->nrun; runid++)
            runs[runid].ccid = newccid;
        }
      else if (ccheight>=largesize || ccwidth>=largesize)
        {
          for(int runid=cc->frun; runid<cc->frun+cc->nrun; runid++)
            {
              Run *r = & runs[runid];
              key.gridi = r->y/splitsize;
              key.gridj = r->x1/splitsize;
              int gridj_end = r->x2/splitsize;
              int gridj_span = gridj_end - key.gridj;
              r->ccid = makeccid(key, map, ncc);
              if (gridj_span>0)
                {
                  // truncate current run 
                  runs.touch(nruns+gridj_span-1);
                  r = &runs[runid];
                  int x = key.gridj*splitsize + splitsize;
                  int x_end = r->x2;
                  r->x2 = x-1;
                  // append additional runs to the runs array
                  while (++key.gridj < gridj_end)
                    {
                      Run& newrun = runs[nruns++];
                      newrun.y = r->y;
                      newrun.x1 = x;
                      x += splitsize;
                      newrun.x2 = x-1;
                      newrun.color = key.color;
                      newrun.ccid = makeccid(key, map, ncc);
                    }
                  // append last run to the run array
                  Run& newrun = runs[nruns++];
                  newrun.y = r->y;
                  newrun.x1 = x;
                  newrun.x2 = x_end;
                  newrun.color = key.color;
                  newrun.ccid = makeccid(key, map, ncc);
                }
            }
        }
    }
  // Recompute cc descriptors
  make_ccs_from_ccids();
}


// -- Helps sorting cc
static int 
top_edges_descending (const void *pa, const void *pb)
{
  if (((CC*) pa)->bb.ymax != ((CC*) pb)->bb.ymax)
    return (((CC*) pb)->bb.ymax - ((CC*) pa)->bb.ymax);
  if (((CC*) pa)->bb.xmin != ((CC*) pb)->bb.xmin)
    return (((CC*) pa)->bb.xmin - ((CC*) pb)->bb.xmin);
  return (((CC*) pa)->frun - ((CC*) pb)->frun);
}


// -- Helps sorting cc
static int 
left_edges_ascending (const void *pa, const void *pb)
{
  if (((CC*) pa)->bb.xmin != ((CC*) pb)->bb.xmin)
    return (((CC*) pa)->bb.xmin - ((CC*) pb)->bb.xmin);
  if (((CC*) pb)->bb.ymax != ((CC*) pa)->bb.ymax)
    return (((CC*) pb)->bb.ymax - ((CC*) pa)->bb.ymax);
  return (((CC*) pa)->frun - ((CC*) pb)->frun);
}


// -- Helps sorting cc
static int 
integer_ascending (const void *pa, const void *pb)
{
  return ( *(int*)pb - *(int*)pa );
}


// -- Sort ccs in approximate reading order
void 
CCImage::sort_in_reading_order()
{
  if (nregularccs<2) return;
  CC *ccarray = new CC[nregularccs];
  // Copy existing ccarray (but segregate special ccs)
  int ccid;
  for(ccid=0; ccid<nregularccs; ccid++)
    ccarray[ccid] = ccs[ccid];
  // Sort the ccarray list into top-to-bottom order.
  qsort (ccarray, nregularccs, sizeof(CC), top_edges_descending);
  // Subdivide the ccarray list roughly into text lines
  int maxtopchange = width / 40;
  if (maxtopchange < 32) 
    maxtopchange = 32;
  // - Loop until processing all ccs
  int ccno = 0;
  int *bottoms = new int[nregularccs];
  while (ccno < nregularccs)
    {
      // - Gather first line approximation
      int nccno;
      int sublist_top = ccarray[ccno].bb.ymax-1;
      int sublist_bottom = ccarray[ccno].bb.ymin;
      for (nccno=ccno; nccno < nregularccs; nccno++)
        {
          if (ccarray[nccno].bb.ymax-1 < sublist_bottom) break;
          if (ccarray[nccno].bb.ymax-1 < sublist_top - maxtopchange) break;
          int bottom = ccarray[nccno].bb.ymin;
          bottoms[nccno-ccno] = bottom;
          if (bottom < sublist_bottom)
            sublist_bottom = bottom;
        }
      // - If more than one candidate cc for the line
      if (nccno > ccno + 1)
        {
          // - Compute median bottom
          qsort(bottoms, nccno-ccno, sizeof(int), integer_ascending);
          int bottom = bottoms[ (nccno-ccno-1)/2 ];
          // - Compose final line
          for (nccno=ccno; nccno < nregularccs; nccno++)
            if (ccarray[nccno].bb.ymax-1 < bottom)
              break;
          // - Sort final line
          qsort (ccarray+ccno, nccno-ccno, sizeof(CC), left_edges_ascending);
        }
      // - Next line
      ccno = nccno;
    }
  // Copy ccarray back and renumber the runs
  for(ccid=0; ccid<nregularccs; ccid++)
    {
      CC& cc = ccarray[ccid];
      ccs[ccid] = cc;
      for(int r=cc.frun; r<cc.frun+cc.nrun; r++)
        runs[r].ccid = ccid;
    }
  // Free memory
  delete [] bottoms;
  delete[] ccarray;
}


// -- Creates a bitmap for a particular component
GP<GBitmap>   
CCImage::get_bitmap_for_cc(const int ccid) const
{
  const CC &cc = ccs[ccid];
  const GRect &bb = cc.bb;
  GP<GBitmap> bits = new GBitmap(bb.height(), bb.width());
  const Run *prun = & runs[(int)cc.frun];
  for (int i=0; i<cc.nrun; i++,prun++)
    {
      if (prun->y<bb.ymin || prun->y>=bb.ymax)
        THROW("Internal error (y bounds)");
      if (prun->x1<bb.xmin || prun->x2>=bb.xmax)
        THROW("Internal error (x bounds)");
      unsigned char *row = (*bits)[prun->y - bb.ymin];
      for (int x=prun->x1; x<=prun->x2; x++)
        row[x - bb.xmin] = 1;
    }
  return bits;
}


// -- Marks cc for deletion
void 
CCImage::erase_cc(int ccid)
{
  CC &cc = ccs[ccid];
  Run *r = &runs[cc.frun];
  int nr = cc.nrun;
  cc.nrun = 0;
  cc.npix = 0;
  while (--nr >= 0)
    (r++)->ccid = -1;  // will be deleted by make_ccs_from_ccids()
}




// --------------------------------------------------
// DEMOTION OF FOREGROUND CCS TO BACKGROUND STATUS
// --------------------------------------------------


// ISSUE: DEMOTION OF CCS (UNIMPLEMENTED) 

// The current code uses a single color for the background layer.  Many large,
// non matching, ccs however may be better encoded as part of the background
// layer.  A way to achieve this could be to consider each cc and evaluate the
// costs of coding it as foreground (does it match other ccs, does it comes
// with a complex geometry) or background (does its color blend smoothly with
// the surrounding parts of the background image).  One just needs then to
// remove the demoted ccs from the mask using CCImage::erase_cc() and
// recompute the ccs using CCImage::make_ccs_from_ccids().  Defining the
// compilation symbols BACKGROUND_SUBSAMPLING_FACTOR and
// PROGRESSIVE_BACKGROUND will enable code for computing the background from
// the input image and the provided mask.




// --------------------------------------------------
// LOSSLESS PATTERN MATCHING
// --------------------------------------------------

// ISSUE: LOSSY PATTERN MATCHING
// This is lossless because shapes with different colors
// can touch each other.  Modifying such shapes may result
// in unpleasant effects.  But not all shapes are like that...


// -- Candidate descriptor for pattern matching
struct MatchData 
{
  GBitmap *bits;       // bitmap pointer
  int area;            // number of black pixels
};


// -- Creates shape hierarchy and substitutions (lossless)
void
tune_jb2image(JB2Image *jimg, int refine_threshold=21)
{
  // Pattern matching data
  int nshapes = jimg->get_shape_count();
  GTArray<MatchData> library(nshapes);
  MatchData *lib = library;    // for faster access  
  // Loop on all shapes
  for (int current=0; current<nshapes; current++)
    {
      // Skip ``special shapes''
      lib[current].bits = 0;
      JB2Shape *jshp = jimg->get_shape(current);
      if (jshp->userdata || !jshp->bits) continue; 
      // Compute matchdata info
      GBitmap *bitmap = jshp->bits;
      int row;
      int rows = bitmap->rows();
      int column;
      int columns = bitmap->columns();
      int black_pixels = 0;
      for (row = rows - 1; row >= 0; row--) 
        for (column = 0; column < columns; column++) 
          if ((*bitmap)[row][column]) 
            black_pixels++;
      lib[current].bits = bitmap;
      lib[current].area = black_pixels;
      // Prepare for search
      int closest_match = -1;
      int best_score = (refine_threshold * rows * columns + 50) / 100;
      if (best_score < 2) best_score = 2;
      bitmap->minborder(2); // ensure sufficient borders
      // Search closest match
      for (int candidate = 0; candidate < current; candidate++) 
        {
          // Access candidate bitmap
          GBitmap *cross_bitmap = lib[candidate].bits;
          if (! cross_bitmap) continue;
          int cross_columns = cross_bitmap->columns();
          int cross_rows = cross_bitmap->rows();
          // Prune
          if (abs (lib[candidate].area - black_pixels) > best_score) continue;
          if (abs (cross_rows - rows) > 2) continue;
          if (abs (cross_columns - columns) > 2) continue;
          // Compute alignment (these are always +1, 0 or -1)
          int cross_column_adjust = (cross_columns  - cross_columns/2) - (columns - columns/2);
          int cross_row_adjust = (cross_rows  - cross_rows/2) - (rows - rows/2);
          // Ensure adequate borders
          cross_bitmap->minborder (2-cross_column_adjust);
          cross_bitmap->minborder (2+columns-cross_columns+cross_column_adjust);
          // Count pixel differences (including borders)
          int score = 0;
          unsigned char *p_row;
          unsigned char *p_cross_row;
          for (row = -1; row <= rows; row++) 
            {
              p_row = (*bitmap) [row];
              p_cross_row  = (*cross_bitmap)[row+cross_row_adjust] + cross_column_adjust;
              for (column = -1; column <= columns; column++) 
                if (p_row [column] != p_cross_row [column])
                  score ++;
              if (score >= best_score)  // prune
                break;
            }
          if (score < best_score) {
            best_score = score;
            closest_match = candidate;
          }
        }
      // Decide what to do with the match.
      if (closest_match >= 0)
        {
          // Mark the shape for cross-coding (``soft pattern matching'')
          jshp->parent = closest_match;
          // Exact match ==> Substitution
          if (best_score == 0)
            lib[current].bits = jshp->bits = 0;
        }
    }
  // Process shape substitutions
  for (int blitno=0; blitno<jimg->get_blit_count(); blitno++)
    {
      JB2Blit *jblt = jimg->get_blit(blitno);
      JB2Shape *jshp = jimg->get_shape(jblt->shapeno);
      if (!jshp->bits && jshp->parent>=0)
        jblt->shapeno = jshp->parent;
    }
}




// --------------------------------------------------
// MAIN COMPRESSION ROUTINE
// --------------------------------------------------


// -- Options for low color compression
struct cpaldjvuopts
{
  int ncolors;
  int dpi;
  bool verbose;
};


// -- Compresses low color pixmap.
void 
cpaldjvu(const GPixmap &input, const char *fileout, const cpaldjvuopts &opts)
{
  int w = input.columns();
  int h = input.rows();
  int dpi = MAX(200, MIN(900, opts.dpi));
  int largesize = MIN(500, MAX(64, dpi));
  int smallsize = MAX(2, dpi/150);

  // Compute optimal palette and quantize input pixmap
  DjVuPalette pal;
  int bgindex = pal.compute_pixmap_palette(input, opts.ncolors);
  if (opts.verbose)
    fprintf(stderr,"cpaldjvu: image %dx%d quantized to %d colors\n", 
            w, h, pal.size());
  GPixel bgcolor;
  pal.index_to_color(bgindex, bgcolor);
  if (opts.verbose)
    fprintf(stderr,"cpaldjvu: background color is #%02x%02x%02x\n", 
            bgcolor.r, bgcolor.g, bgcolor.b);

  // Fill CCImage with color runs
  CCImage rimg(w, h);
  for (int y=0; y<h; y++)
    {
      int x = 0;
      const GPixel *row = input[y];
      while (x<w)
        {
          int x1 = x;
          int index = pal.color_to_index(row[x++]);
          while (x<w && pal.color_to_index(row[x])==index) { x++; }
          if (index != bgindex)
            rimg.add_single_run(y, x1, x-1, index);
        }
    }
  if (opts.verbose)
    fprintf(stderr,"cpaldjvu: %d color runs\n", 
            rimg.runs.size());

  // Perform Color Connected Component Analysis
  rimg.make_ccids_by_analysis();                  // Obtain ccids
  rimg.make_ccs_from_ccids();                     // Compute cc descriptors
  if (opts.verbose)
    fprintf(stderr,"cpaldjvu: %d ccs\n", rimg.ccs.size());
  rimg.merge_and_split_ccs(smallsize,largesize);  // Eliminates gross ccs
  if (opts.verbose)
    fprintf(stderr,"cpaldjvu: %d ccs after merging/splitting\n", 
            rimg.ccs.size());
  rimg.sort_in_reading_order();                   // Sort cc descriptors
  
  // Create JB2Image and fill colordata
  JB2Image jimg;
  jimg.set_dimension(w, h);
  int nccs = rimg.ccs.size();
  for (int ccid=0; ccid<nccs; ccid++)
    {
      JB2Shape shape;
      JB2Blit  blit;
      shape.parent = -1;
      shape.userdata = (ccid >= rimg.nregularccs);
      shape.bits = rimg.get_bitmap_for_cc(ccid);
      shape.bits->compress();
      CC& cc = rimg.ccs[ccid];
      blit.shapeno = jimg.add_shape(shape);
      blit.left = cc.bb.xmin;
      blit.bottom = cc.bb.ymin;
      int blitno = jimg.add_blit(blit);
      pal.colordata.touch(blitno);
      pal.colordata[blitno] = cc.color;
    }
  
  // Organize JB2Image
  tune_jb2image(&jimg);
  if (opts.verbose)
    {
      int nshape=0, nrefine=0;
      for (int i=0; i<jimg.get_shape_count(); i++) {
        if (!jimg.get_shape(i)->bits) continue;
        if (jimg.get_shape(i)->parent >= 0) nrefine++; 
        nshape++; 
      }
      fprintf(stderr,"cpaldjvu: %d shapes after matching (%d are cross-coded)\n", 
              nshape, nrefine);
    }
  
  // Create background image
  IWPixmap iwimage;
#ifdef BACKGROUND_SUBSAMPLING_FACTOR
  // -- we may create the background by masking and subsampling
  GPixmap inputsub;
  GP<GBitmap> mask = jimg.get_bitmap(BACKGROUND_SUBSAMPLING_FACTOR);
  inputsub.downsample(&input, BACKGROUND_SUBSAMPLING_FACTOR);
  iwimage.init(&inputsub, mask);
#else
  // -- but who cares since the background is uniform.
  GPixmap inputsub((h+11)/12, (w+11)/12, &bgcolor);
  iwimage.init(&inputsub);
#endif

  // Assemble DJVU file
  StdioByteStream obs(fileout, "wb");
  IFFByteStream iff(obs);
  // -- main composite chunk
  iff.put_chunk("FORM:DJVU");
  // -- ``INFO'' chunk
  iff.put_chunk("INFO");
  DjVuInfo info;
  info.height = h;
  info.width = w;
  info.dpi = opts.dpi;
  info.encode(iff);
  iff.close_chunk();
  // -- ``Sjbz'' chunk
  iff.put_chunk("Sjbz");
  jimg.encode(iff);
  iff.close_chunk();
  // -- ``FGbz'' chunk
  iff.put_chunk("FGbz");
  pal.encode(iff);
  iff.close_chunk();
  // -- ``BG44'' chunk
  IWEncoderParms iwparms;
#ifdef PROGRESSIVE_BACKGROUND
  // ----- we may use several chunks to enable progressive rendering ...
  iff.put_chunk("BG44");
  iwparms.slices = 74;
  iwimage.encode_chunk(iff, iwparms);
  iff.close_chunk();
  iff.put_chunk("BG44");
  iwparms.slices = 87;
  iwimage.encode_chunk(iff, iwparms);
  iff.close_chunk();
#endif
  // ----- but who cares when the background is so small.
  iff.put_chunk("BG44");
  iwparms.slices = 97;
  iwimage.encode_chunk(iff, iwparms);
  iff.close_chunk();
  // -- terminate main composite chunk
  iff.close_chunk();
  // Finished!
}  



// --------------------------------------------------
// MAIN
// --------------------------------------------------


void
usage()
{
  fprintf(stderr,"Usage: cpaldjvu [options] <inputppmfile> <outputdjvufile>\n"
          "Options are:\n"
          "   -colors n    Maximum number of colors during quantization (default 256).\n"
          "   -dpi n       Resolution written into the output file (default 100).\n"
          "   -verbose     Displays additional messages.\n" );
  exit(10);
}


int 
main(int argc, const char **argv)
{
  TRY
    {
      GString inputppmfile;
      GString outputdjvufile;
      // Defaults
      cpaldjvuopts opts;
      opts.dpi = 100;
      opts.ncolors = 256;
      opts.verbose = false;
      // Parse options
      for (int i=1; i<argc; i++)
        {
          GString arg = argv[i];
          if (arg == "-colors" && i+1<argc)
            {
              char *end;
              opts.ncolors = strtol(argv[++i], &end, 10);
              if (*end || opts.ncolors<2 || opts.ncolors>1024)
                usage();
            }
          else if (arg == "-dpi" && i+1<argc)
            {
              char *end;
              opts.dpi = strtol(argv[++i], &end, 10);
              if (*end || opts.dpi<25 || opts.dpi>144000)
                usage();
            }
          else if (arg == "-verbose")
            opts.verbose = true;
          else if (arg[0] == '-')
            usage();
          else if (!inputppmfile)
            inputppmfile = arg;
          else if (!outputdjvufile)
            outputdjvufile = arg;
          else
            usage();
        }
      if (!inputppmfile || !outputdjvufile)
        usage();
      // Load and run
      StdioByteStream ibs(inputppmfile,"rb");
      GPixmap input(ibs);
      cpaldjvu(input, outputdjvufile, opts);
    }
  CATCH(ex)
    {
      ex.perror();
      exit(1);
    }
  ENDCATCH;
  return 0;
}





