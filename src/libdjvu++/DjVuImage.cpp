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
//C- $Id: DjVuImage.cpp,v 1.22 1999-09-03 23:35:40 leonb Exp $


#ifdef __GNUC__
#pragma implementation
#endif

#include "DjVuImage.h"
#include "GScaler.h"
#include "DjVuDocument.h"
#include "GSmartPointer.h"
#include <stdarg.h>

//// DJVUIMAGE: CONSTRUCTION

DjVuImage::DjVuImage(void) {};

DjVuImage::DjVuImage(const GP<DjVuFile> & xfile) : file(xfile)
{
}

//// DJVUIMAGE: data collectors

GP<DjVuInfo>
DjVuImage::get_info(const GP<DjVuFile> & file) const
{
   if (file->info) return file->info;
   GPList<DjVuFile> list=file->get_included_files();
   for(GPosition pos=list;pos;++pos)
   {
      GP<DjVuInfo> info=get_info(list[pos]);
      if (info) return info;
   }
   return 0;
}

GP<DjVuAnno>
DjVuImage::get_anno(const GP<DjVuFile> & file) const
{
   if (file->anno) return file->anno;
   GPList<DjVuFile> list=file->get_included_files();
   for(GPosition pos=list;pos;++pos)
   {
      GP<DjVuAnno> anno=get_anno(list[pos]);
      if (anno) return anno;
   }
   return 0;
}

GP<IWPixmap>
DjVuImage::get_bg44(const GP<DjVuFile> & file) const
{
   if (file->bg44) return file->bg44;
   GPList<DjVuFile> list=file->get_included_files();
   for(GPosition pos=list;pos;++pos)
   {
      GP<IWPixmap> bg44=get_bg44(list[pos]);
      if (bg44) return bg44;
   }
   return 0;
}

GP<JB2Image>
DjVuImage::get_fgjb(const GP<DjVuFile> & file) const
{
   if (file->fgjb) return file->fgjb;
   GPList<DjVuFile> list=file->get_included_files();
   for(GPosition pos=list;pos;++pos)
   {
      GP<JB2Image> fgjb=get_fgjb(list[pos]);
      if (fgjb) return fgjb;
   }
   return 0;
}

GP<GPixmap>
DjVuImage::get_fgpm(const GP<DjVuFile> & file) const
{
   if (file->fgpm) return file->fgpm;
   GPList<DjVuFile> list=file->get_included_files();
   for(GPosition pos=list;pos;++pos)
   {
      GP<GPixmap> fgpm=get_fgpm(list[pos]);
      if (fgpm) return fgpm;
   }
   return 0;
}

GP<DjVuNavDir>
DjVuImage::get_dir(const GP<DjVuFile> & file) const
{
   if (file->dir) return file->dir;
   GPList<DjVuFile> list=file->get_included_files();
   for(GPosition pos=list;pos;++pos)
   {
      GP<DjVuNavDir> dir=get_dir(list[pos]);
      if (dir) return dir;
   }
   return 0;
}

GP<DjVuInfo>   
DjVuImage::get_info() const
{
   return (file && (!info || file->is_decoding())) ?
	  ((DjVuImage *) this)->info=get_info(file) : info;
}

GP<DjVuAnno>   
DjVuImage::get_anno() const
{
   return (file && (!anno || file->is_decoding())) ?
	  ((DjVuImage *) this)->anno=get_anno(file) : anno;
}

GP<IWPixmap>   
DjVuImage::get_bg44() const
{
   return (file && (!bg44 || file->is_decoding())) ?
	  ((DjVuImage *) this)->bg44=get_bg44(file) : bg44;
}

GP<JB2Image>   
DjVuImage::get_fgjb() const
{
   return (file && (!fgjb || file->is_decoding())) ?
	  ((DjVuImage *) this)->fgjb=get_fgjb(file) : fgjb;
}

GP<GPixmap>    
DjVuImage::get_fgpm() const
{
   return (file && (!fgpm || file->is_decoding())) ?
	  ((DjVuImage *) this)->fgpm=get_fgpm(file) : fgpm;
}

int
DjVuImage::get_width() const
{
   GP<DjVuInfo> info=get_info();
   return info ? info->width : 0;
}

int
DjVuImage::get_height() const
{
   GP<DjVuInfo> info=get_info();
   return info ? info->height : 0;
}

int
DjVuImage::get_version() const
{
   GP<DjVuInfo> info=get_info();
   return info ? info->version : DJVUVERSION;
}

int
DjVuImage::get_dpi() const
{
   GP<DjVuInfo> info=get_info();
   return info ? info->dpi : 300;
}

int
DjVuImage::get_rounded_dpi() const
{
   int dpi=get_dpi();
   if (dpi>700) return dpi;
  
   const int std_dpi[]={ 25, 50, 75, 100, 150, 300, 600 };
   const int std_dpis=sizeof(std_dpi)/sizeof(std_dpi[0]);
   int min_dist=abs(dpi-std_dpi[0]);
   int min_idx=0;
   for(int i=1;i<std_dpis;i++)
      if (abs(std_dpi[i]-dpi)<min_dist)
      {
         min_dist=abs(std_dpi[i]-dpi);
         min_idx=i;
      };
   return std_dpi[min_idx];
}

double
DjVuImage::get_gamma() const
{
   GP<DjVuInfo> info=get_info();
   return info ? info->gamma : 2.2;
}

GString
DjVuImage::get_mimetype() const
{
   return file->mimetype;
}


//// DJVUIMAGE: UTILITIES

GString 
DjVuImage::get_short_description() const
{
  GString msg = "Empty";
  int width = get_width();
  int height = get_height();
  if (width && height)
    if (file->file_size > 100)
      msg.format("%dx%d in %0.1f Kb", width, height, file->file_size/1024.0);
    else
      msg.format("%dx%d", width, height);
  return msg;
}

GString 
DjVuImage::get_long_description() const
{
  // Tab characters '\t' sometimes look really ugly.
  // This code replace them with spaces so that every column 
  // is really aligned.  Lines that do not contain tabs are 
  // left unchanged.
  GString result = file->description;
  // Loop on tabulations
  for(;;)
    {
      // Search position of first tab in each line
      int tab_num = 0;
      int tab_pos = 0;
      const char *s=result;
      for (s=result; *s; s++)
        {
          const char *line_start = s;
          for (; *s && *s!='\n' && *s!='\t'; s++) /**/;
          if (*s == '\t')
            {
              int pos = s-line_start;
              tab_pos = ( (pos>tab_pos) ? pos : tab_pos );
              tab_num += 1;
            }
          for (; *s && *s!='\n'; s++) /**/;
        }
      // Check whether all tabs have been removed
      if (tab_num <= 0)
        break;
      // Replace tab 
      GString tmp;
      char *d = tmp.getbuf(result.length() + tab_num * tab_pos + 1);
      s = result;
      for (s=result; *s; *d++=*s++)
        {
          const char *line_start = d;
          for (; *s && *s!='\n' && *s!='\t'; *d++=*s++) /**/;
          if (*s == '\t')
            for(s++; d<line_start+tab_pos+2; *d++=' ') /**/;
          for (; *s && *s!='\n'; *d++=*s++) /**/;
        }
      *d++ = 0;
      result = tmp;
    }
  // All tabs have been replaced.
  return result;
}

//// DJVUIMAGE: OLD-STYLE DECODING


class DjVuImageNotifier : public DjVuPort
{
  friend class DjVuImage;
  DjVuInterface  *notifier;
  GP<DataPool>	  stream_pool;
  GURL		  stream_url;
public:
  DjVuImageNotifier(DjVuInterface *notifier);
  GP<DataPool> request_data(const DjVuPort *src, const GURL & url);
  void notify_redisplay(const DjVuPort *);
  void notify_relayout(const DjVuPort *);
  void notify_chunk_done(const DjVuPort *, const char *);
};

DjVuImageNotifier::DjVuImageNotifier(DjVuInterface *notifier)
  : notifier(notifier)
{
}

GP<DataPool> 
DjVuImageNotifier::request_data(const DjVuPort *src, const GURL & url)
{
  if (url!=stream_url)
    THROW("This stream cannot be decoded the old way.");
  return stream_pool;
}

void
DjVuImageNotifier::notify_redisplay(const DjVuPort *)
{
  if (notifier)
    notifier->notify_redisplay();
}

void 
DjVuImageNotifier::notify_relayout(const DjVuPort *)
{
  if (notifier)
    notifier->notify_relayout();
}

void 
DjVuImageNotifier::notify_chunk_done(const DjVuPort *, const char *name)
{
  if (notifier)
    notifier->notify_chunk(name, "");
}

void
DjVuImage::decode(ByteStream & str, DjVuInterface *notifier)
{
  DEBUG_MSG("DjVuImage::decode(): decoding old way...\n");
  DEBUG_MAKE_INDENT(3);
  if (file) 
    THROW("To decode old way you should have been used default constructor.");
  
  GP<DjVuImageNotifier> pport = new DjVuImageNotifier(notifier);
  pport->stream_url="internal://fake_url_for_old_style_decoder";
  pport->stream_pool=new DataPool();
  int length;
  char buffer[1024];
  while((length=str.read(buffer, 1024)))
    pport->stream_pool->add_data(buffer, length);
  pport->stream_pool->set_eof();

  GP<DjVuDocument> doc = new DjVuDocument;
  doc->init(pport->stream_url, (DjVuImageNotifier*)pport);
  GP<DjVuImage> dimg=doc->get_page(-1);
  file=dimg->get_djvu_file();
  file->wait_for_finish();
  if (file->is_decode_stopped())
    THROW("STOP");
  if (file->is_decode_failed())
    THROW("EOF");  // a guess
  if (! file->is_decode_ok())
    THROW("Multiple errors while decoding");
  DEBUG_MSG("decode DONE\n");
}


//// DJVUIMAGE: CHECKING

static int
compute_red(int w, int h, int rw, int rh)
{
  for (int red=1; red<16; red++)
    if (((w+red-1)/red==rw) && ((h+red-1)/red==rh))
      return red;
  return 16;
}


int 
DjVuImage::is_legal_bilevel() const
{
  // Components
  GP<DjVuInfo> info = get_info();
  GP<JB2Image> fgjb = get_fgjb();
  GP<IWPixmap> bg44 = get_bg44();
  GP<GPixmap>  fgpm = get_fgpm();
  // Check info
  if (! info)
    return 0;
  int width = info->width;
  int height = info->height;
  if (! (width>0 && height>0))
    return 0;
  // Check fgjb
  if (!fgjb)
    return 0;
  if (fgjb->get_width()!=width || fgjb->get_height()!=height)
    return 0;
  // Check that color information is not present.
  if (bg44 || fgpm)
    return 0;
  // Ok.
  return 1;
}

int 
DjVuImage::is_legal_photo() const
{
  // Components
  GP<DjVuInfo> info = get_info();
  GP<JB2Image> fgjb = get_fgjb(); 
  GP<IWPixmap> bg44 = get_bg44();
  GP<GPixmap>  fgpm = get_fgpm();
  // Check info
  if (! info)
    return 0;
  int width = info->width;
  int height = info->height;
  if (! (width>0 && height>0))
    return 0;
  // Check bg44
  if (!bg44)
    return 0;
  if (bg44->get_width()!=width || bg44->get_height()!=height)
    return 0;
  // Check that extra information is not present.
  if (fgjb || fgpm)
    return 0;
  // Ok.
  return 1;
}

int 
DjVuImage::is_legal_compound() const
{
  // Components
  GP<DjVuInfo> info = get_info();
  GP<JB2Image> fgjb = get_fgjb();
  GP<IWPixmap> bg44 = get_bg44();
  GP<GPixmap>  fgpm = get_fgpm();
  // Check size
  if (! info)
    return 0;
  int width = info->width;
  int height = info->height;
  if (! (width>0 && height>0))
    return 0;
  // Check fgjb
  if (!fgjb)
    return 0;
  if (fgjb->get_width()!=width || fgjb->get_height()!=height)
    return 0;
  // Check background
  int bgred = 0;
  if (bg44)
    bgred = compute_red(width, height, bg44->get_width(), bg44->get_height());
  if (bgred<1 || bgred>12)
    return 0;
  // Check foreground colors
  int fgred = 0;
  if (fgpm)
    fgred = compute_red(width, height, fgpm->columns(), fgpm->rows());
  if (fgred<1 || fgred>12)
    return 0;
  // Check that all components are present
  if (fgjb && bgred && fgred)
    return 1;
  // Unrecognized
  return 0;
}


//// DJVUIMAGE: LOW LEVEL RENDERING

GP<GBitmap>
DjVuImage::get_bitmap(const GRect &rect, 
                      int subsample, int align) const
{
  // Access image size
  int width = get_width();
  int height = get_height();
  GP<JB2Image> fgjb = get_fgjb();
  if ( width && height && fgjb && 
       (fgjb->get_width() == width) && 
       (fgjb->get_height() == height) ) 
    {
      return fgjb->get_bitmap(rect, subsample, align);
    }
  return 0;
}

GP<GPixmap>
DjVuImage::get_bg_pixmap(const GRect &rect, 
                         int subsample, double gamma) const
{
  GP<GPixmap> pm = 0;
  // Access image size
  int width = get_width();
  int height = get_height();
  GP<DjVuInfo> info = get_info();
  if (width<=0 || height<=0 || !info) return 0;
  GP<IWPixmap> bg44 = get_bg44();
  // Compute gamma_correction
  double gamma_correction = 1.0;
  if (gamma > 0)
    gamma_correction = gamma / info->gamma;
  if (gamma_correction < 0.1)
    gamma_correction = 0.1;
  else if (gamma_correction > 10)
    gamma_correction = 10;
  
  // CASE1: Incremental BG IWPixmap
  if (bg44)
    {
      int w = bg44->get_width();
      int h = bg44->get_height();
      // Avoid silly cases
      if (w==0 || h==0 || width==0 || height==0)
        return 0;
      // Determine how much bg44 is reduced
      int red = compute_red(width,height,w,h);
      if (red<1 || red>12)
        return 0;
      // Handle pure downsampling cases
      if (subsample == red)
        pm = bg44->get_pixmap(1,rect);
      else if (subsample == 2*red)
        pm = bg44->get_pixmap(2,rect);    
      else if (subsample == 4*red)
        pm = bg44->get_pixmap(4,rect); 
      else if (subsample == 8*red)
        pm = bg44->get_pixmap(8,rect); 
      // Handle fractional downsampling case
      else if (red*4 == subsample*3)
        {
          GRect nrect = rect;
          GRect xrect = rect;
          xrect.xmin = (xrect.xmin/3)*4;
          xrect.ymin = (xrect.ymin/3)*4;
          xrect.xmax = ((xrect.xmax+2)/3)*4;
          xrect.ymax = ((xrect.ymax+2)/3)*4;
          nrect.translate(-xrect.xmin*3/4, -xrect.ymin*3/4);
          if (xrect.xmax > w) 
            xrect.xmax = w;
          if (xrect.ymax > h) 
            xrect.ymax = h;
          GP<GPixmap> ipm = bg44->get_pixmap(1,xrect);
          pm = new GPixmap();
          pm->downsample43(ipm, &nrect);
        }
      // Handle all other cases with pixmapscaler
      else
        {
          // find suitable power of two
          int po2 = 16;
          while (po2>1 && subsample<po2*red)
            po2 >>= 1;
          // setup pixmap scaler
          int inw = (w+po2-1)/po2;
          int inh = (h+po2-1)/po2;
          int outw = (width+subsample-1)/subsample;
          int outh = (height+subsample-1)/subsample;
          GPixmapScaler ps(inw, inh, outw, outh);
          ps.set_horz_ratio(red*po2, subsample);
          ps.set_vert_ratio(red*po2, subsample);
          // run pixmap scaler
          GRect xrect;
          ps.get_input_rect(rect,xrect);
          GP<GPixmap> ipm = bg44->get_pixmap(po2,xrect);
          pm = new GPixmap();
          ps.scale(xrect, *ipm, rect, *pm);
        }
      // Apply gamma correction
      if (pm && gamma_correction!=1.0)
        pm->color_correct(gamma_correction);
      return pm;
    }

  // FAILURE
  return 0;
}

int  
DjVuImage::stencil(GPixmap *pm, const GRect &rect,
		   int subsample, double gamma) const
{
  // Access components
  int width = get_width();
  int height = get_height();
  GP<DjVuInfo> info = get_info();
  if (width<=0 || height<=0 || !info) return 0;
  GP<JB2Image> fgjb = get_fgjb();
  GP<GPixmap> fgpm = get_fgpm();
  // Compute gamma_correction
  double gamma_correction = 1.0;
  if (gamma > 0)
    gamma_correction = gamma / info->gamma;
  if (gamma_correction < 0.1)
    gamma_correction = 0.1;
  else if (gamma_correction > 10)
    gamma_correction = 10;

  // CASE1: JB2 stencil and FG pixmap
  if (fgjb && fgpm)
    {
      GP<GBitmap> bm = get_bitmap(rect, subsample);
      if (bm && pm)
        {
          int w = fgpm->columns();
          int h = fgpm->rows();
          // Determine foreground reduction
          int red = compute_red(width,height,w,h);
          if (red<1 || red>12)
            return 0;
          // Try supersampling foreground pixmap by an integer factor
          int supersample = ( red>subsample ? red/subsample : 1);
          int wantedred = supersample*subsample;
          // Try simple foreground upsampling
          if (red == wantedred)
            {
              // Simple foreground upsampling is enough.
              pm->stencil(bm, fgpm, supersample, &rect, gamma_correction);
              return 1;
            }
          else 
            {
              // Must rescale foreground pixmap first
              GP<GPixmap> nfg;
              int desw = (w*red+wantedred-1)/wantedred;
              int desh = (h*red+wantedred-1)/wantedred;
              // Cache rescaled fgpm for speed
              static const DjVuImage *tagimage  = 0;
              static const GPixmap *tagfgpm   = 0;
              static GP<GPixmap> cachednfg = 0;
              // Check whether cached fgpm applies.
              if ( cachednfg && this==tagimage && fgpm==tagfgpm
                   && desw==(int)cachednfg->columns()
                   && desh==(int)cachednfg->rows() )
                {
                  nfg = cachednfg;
                }
              else
                {
                  GPixmapScaler ps(w,h,desw,desh);
                  ps.set_horz_ratio(red, wantedred);
                  ps.set_vert_ratio(red, wantedred);
                  nfg = new GPixmap;
                  GRect provided(0,0,w,h);
                  GRect desired(0,0,desw,desh);
                  ps.scale(provided, *fgpm, desired, *nfg);
                }
              // Compute
              pm->stencil(bm, nfg, supersample, &rect, gamma_correction);
              // Cache
              tagimage = this;
              tagfgpm = fgpm;
              cachednfg = nfg;
              return 1;
            }
        }
    }
  // FAILURE
  return 0;
}

GP<GPixmap>
DjVuImage::get_fg_pixmap(const GRect &rect, 
                         int subsample, double gamma) const
{
  // Obtain white background pixmap
  GP<GPixmap> pm = 0;
  // Access components
  int width = get_width();
  int height = get_height();
  if (width && height)
    {
      pm = new GPixmap(rect.height(),rect.width(), &GPixel::WHITE);
      if (stencil(pm, rect, subsample, gamma))
        return pm;
    }
  return 0;
}

GP<GPixmap>
DjVuImage::get_pixmap(const GRect &rect, int subsample, double gamma) const
{
  // Get background
  GP<GPixmap> pm = get_bg_pixmap(rect, subsample, gamma);
  // Superpose foreground
  if (! stencil(pm, rect, subsample, gamma))
    // Avoid ugly progressive display (hack)
    if (get_fgjb()) return 0;
  // Return
  return pm;
}


//// DJVUIMAGE: RENDERING (ARBITRARY SCALE)

typedef GP<GBitmap>(DjVuImage::*BImager)(const GRect &, int, int) const;
typedef GP<GPixmap>(DjVuImage::*PImager)(const GRect &, int, double) const;

static GP<GBitmap>
do_bitmap(const DjVuImage &dimg, BImager get,
          const GRect &rect, const GRect &all, int align )
{
  // Sanity
  if (! ( all.contains(rect.xmin, rect.ymin) &&
          all.contains(rect.xmax-1, rect.ymax-1) ))
    THROW("(DjVuImage::get_pixmap) Illegal target rectangles");
  // Check for integral reduction
  int red;
  int w = dimg.get_width();
  int h = dimg.get_height();
  int rw = all.width();
  int rh = all.height();
  GRect zrect = rect; 
  zrect.translate(-all.xmin, -all.ymin);
  for (red=1; red<=15; red++)
    if (rw*red>w-red && rw*red<w+red && rh*red>h-red && rh*red<h+red)
      return (dimg.*get)(zrect, red, align);
  // Find best reduction
  for (red=15; red>1; red--)
    if ( (rw*red < w && rh*red < h) ||
         (rw*red*3 < w || rh*red*3 < h) )
      break;
  // Setup bitmap scaler
  if (! (w && h)) return 0;
  GBitmapScaler bs;
  bs.set_input_size( (w+red-1)/red, (h+red-1)/red );
  bs.set_output_size( rw, rh );
  bs.set_horz_ratio( rw*red, w );
  bs.set_vert_ratio( rh*red, h );
  // Scale
  GRect srect;
  bs.get_input_rect(zrect, srect);
  GP<GBitmap> sbm = (dimg.*get)(srect, red, 1);
  if (!sbm) return 0;
  int border = ((zrect.width() + align - 1) & ~(align - 1)) - zrect.width();
  GP<GBitmap> bm = new GBitmap(zrect.height(), zrect.width(), border);
  bs.scale(srect, *sbm, zrect, *bm);
  return bm;
}

static GP<GPixmap>
do_pixmap(const DjVuImage &dimg, PImager get,
          const GRect &rect, const GRect &all, double gamma )
{
  // Sanity
  if (! ( all.contains(rect.xmin, rect.ymin) &&
          all.contains(rect.xmax-1, rect.ymax-1) ))
    THROW("(DjVuImage::get_pixmap) Illegal target rectangles");
  // Check for integral reduction
  int red, w=0, h=0, rw=0, rh=0;
  w = dimg.get_width();
  h = dimg.get_height();
  rw = all.width();
  rh = all.height();
  GRect zrect = rect; 
  zrect.translate(-all.xmin, -all.ymin);
  for (red=1; red<=15; red++)
    if (rw*red>w-red && rw*red<w+red && rh*red>h-red && rh*red<h+red)
      return (dimg.*get)(zrect, red, gamma);
  // These reductions usually go faster (improve!)
  static int fastred[] = { 12,6,4,3,2,1 };
  // Find best reduction
  for (int i=0; (red=fastred[i])>1; i++)
    if ( (rw*red < w && rh*red < h) ||
         (rw*red*3 < w || rh*red*3 < h) )
      break;
  // Setup pixmap scaler
  if (w<0 || h<0) return 0;
  GPixmapScaler ps;
  ps.set_input_size( (w+red-1)/red, (h+red-1)/red );
  ps.set_output_size( rw, rh );
  ps.set_horz_ratio( rw*red, w );
  ps.set_vert_ratio( rh*red, h );
  // Scale
  GRect srect;
  ps.get_input_rect(zrect, srect);
  GP<GPixmap> spm = (dimg.*get)(srect, red, gamma);
  if (!spm) return 0;
  GP<GPixmap> pm = new GPixmap;
  ps.scale(srect, *spm, zrect, *pm);
  return pm;
}

GP<GPixmap>  
DjVuImage::get_pixmap(const GRect &rect, const GRect &all, double gamma) const
{
  return do_pixmap(*this, & DjVuImage::get_pixmap, rect, all, gamma);
}

GP<GBitmap>  
DjVuImage::get_bitmap(const GRect &rect, const GRect &all, int align) const
{
  return do_bitmap(*this, & DjVuImage::get_bitmap, rect, all, align);
}

GP<GPixmap>  
DjVuImage::get_bg_pixmap(const GRect &rect, const GRect &all, double gamma) const
{
  return do_pixmap(*this, & DjVuImage::get_bg_pixmap, rect, all, gamma);
}

GP<GPixmap>  
DjVuImage::get_fg_pixmap(const GRect &rect, const GRect &all, double gamma) const
{
  return do_pixmap(*this, & DjVuImage::get_fg_pixmap, rect, all, gamma);
}
