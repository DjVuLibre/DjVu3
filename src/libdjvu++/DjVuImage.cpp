//C-  -*- C++ -*-
//C- DjVu� Reference Library (v. 3.0)
//C- 
//C- Copyright � 1999-2001 LizardTech, Inc. All Rights Reserved.
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
// 
// $Id: DjVuImage.cpp,v 1.69 2001-04-12 18:50:50 fcrary Exp $
// $Name:  $

#ifdef __GNUC__
#pragma implementation
#endif

#include "DjVuImage.h"
#include "GScaler.h"
#include "DjVuDocument.h"
#include "DjVuPalette.h"
#include "GContainer.h"
#include "GSmartPointer.h"
#include "JB2Image.h"
#include "IW44Image.h"
#include "DataPool.h"
#include "ByteStream.h"
#include "GMapAreas.h"

#include "debug.h"
#include <stdarg.h>




//// DJVUIMAGE: CONSTRUCTION

DjVuImage::DjVuImage(void) 
: rotate_count(-1),relayout_sent(false)
{
}

void
DjVuImage::connect(const GP<DjVuFile> & xfile)
{
   file=xfile;
   DjVuPort::get_portcaster()->add_route(file, this);
}




//// DJVUIMAGE: DATA COLLECTORS

GP<DjVuInfo>
DjVuImage::get_info(const GP<DjVuFile> & file) const
{
   if (file->info)
   {
     if(rotate_count<0)
     {
       const_cast<DjVuImage *>(this)->init_rotate(*(file->info));
     }
     return file->info;
   }
   GPList<DjVuFile> list=file->get_included_files();
   for(GPosition pos=list;pos;++pos)
   {
      GP<DjVuInfo> info=get_info(list[pos]);
      if (info) 
      {
        if(rotate_count<0)
        {
          const_cast<DjVuImage *>(this)->init_rotate(*(file->info));
        }
        return info;
      }
   }
   return 0;
}

GP<IW44Image>
DjVuImage::get_bg44(const GP<DjVuFile> & file) const
{
   if (file->bg44) return file->bg44;
   GPList<DjVuFile> list=file->get_included_files();
   for(GPosition pos=list;pos;++pos)
   {
      GP<IW44Image> bg44=get_bg44(list[pos]);
      if (bg44) return bg44;
   }
   return 0;
}

GP<GPixmap>
DjVuImage::get_bgpm(const GP<DjVuFile> & file) const
{
   if (file->bgpm) return file->bgpm;
   GPList<DjVuFile> list=file->get_included_files();
   for(GPosition pos=list;pos;++pos)
   {
      GP<GPixmap> bgpm=get_bgpm(list[pos]);
      if (bgpm) return bgpm;
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

GP<DjVuPalette>
DjVuImage::get_fgbc(const GP<DjVuFile> & file) const
{
   if (file->fgbc) return file->fgbc;
   GPList<DjVuFile> list=file->get_included_files();
   for(GPosition pos=list;pos;++pos)
   {
      GP<DjVuPalette> fgbc=get_fgbc(list[pos]);
      if (fgbc) return fgbc;
   }
   return 0;
}

GP<DjVuInfo>   
DjVuImage::get_info() const
{
   if (file)
   {
     return get_info(file);
   }else
   {
     return 0;
   }
}

GP<ByteStream>
DjVuImage::get_anno() const
{
   GP<ByteStream> out = ByteStream::create();
   ByteStream &mbs = *out;
   if (file) 
   {
     file->merge_anno(mbs);
   }
   mbs.seek(0);
   if(!mbs.size())
   {
     out=0;
   }
   return out;
}

GP<ByteStream>
DjVuImage::get_text() const
{
   GP<ByteStream> out = ByteStream::create();
   ByteStream &mbs = *out;
   if (file) 
   {
     file->get_text(mbs);
   }
   mbs.seek(0);
   if(!mbs.size())
   {
     out=0;
   }
   return out;
}

GP<IW44Image>   
DjVuImage::get_bg44() const
{
   if (file) return get_bg44(file);
   else return 0;
}

GP<GPixmap>   
DjVuImage::get_bgpm() const
{
   if (file) return get_bgpm(file);
   else return 0;
}

GP<JB2Image>   
DjVuImage::get_fgjb() const
{
   if (file) return get_fgjb(file);
   else return 0;
}

GP<GPixmap>    
DjVuImage::get_fgpm() const
{
   if (file) return get_fgpm(file);
   else return 0;
}

GP<DjVuPalette>    
DjVuImage::get_fgbc() const
{
   if (file) return get_fgbc(file);
   else return 0;
}

int
DjVuImage::get_width() const
{
   GP<DjVuInfo> info=get_info();
   return info?((rotate_count&1)?(info->height):(info->width)):0;
}

int
DjVuImage::get_height() const
{
   GP<DjVuInfo> info=get_info();
   return info?((rotate_count&1)?(info->width):(info->height)):0;
}

int
DjVuImage::get_real_width() const
{
   GP<DjVuInfo> info=get_info();
   return info ? info->width : 0;
}

int
DjVuImage::get_real_height() const
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
   return (get_dpi()+5)/10*10;
   
      /* This code used to round the reported dpi to 25, 50, 75, 100, 150,
	 300, and 600. Now we just round the dpi to 10ths and return it
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
   */
}

double
DjVuImage::get_gamma() const
{
   GP<DjVuInfo> info=get_info();
   return info ? info->gamma : 2.2;
}

GUTF8String
DjVuImage::get_mimetype() const
{
   return file ? file->mimetype : GUTF8String();
}


//// DJVUIMAGE: UTILITIES

GUTF8String 
DjVuImage::get_short_description() const
{
  GUTF8String msg = "Empty";
  int width = get_width();
  int height = get_height();
  if (width && height)
    if (file && file->file_size>100)
      msg.format("%dx%d in %0.1f Kb", width, height, file->file_size/1024.0);
    else
      msg.format("%dx%d", width, height);
  return msg;
}

GUTF8String 
DjVuImage::get_long_description() const
{
  // Tab characters '\t' sometimes look really ugly.
  // This code replace them with spaces so that every column 
  // is really aligned.  Lines that do not contain tabs are 
  // left unchanged.
  GUTF8String result = file ? file->description : GUTF8String();
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
      GUTF8String tmp;
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

void
DjVuImage::notify_chunk_done(const DjVuPort *, const char * name)
{
   if (!relayout_sent &&
     ( GUTF8String::ncmp("INFO",name, 4) ||
       GUTF8String::ncmp("PMxx",name, 2) ||
       GUTF8String::ncmp("BMxx",name, 2)  ) )
   {
      DjVuPort::get_portcaster()->notify_relayout(this);
      relayout_sent=true;
   } 
   else if (GUTF8String::ncmp("Sxxx", name, 1) ||
            GUTF8String::ncmp("BGxx", name, 2) ||
            GUTF8String::ncmp("FGxx", name, 2) ||
            GUTF8String::ncmp("BMxx", name, 2) ||
            GUTF8String::ncmp("PMxx", name, 2)  )
     DjVuPort::get_portcaster()->notify_redisplay(this);
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
  void notify_chunk_done(const DjVuPort *, const char *);
  void notify_redisplay(const class DjVuImage * source);
  void notify_relayout(const class DjVuImage * source);
};

DjVuImageNotifier::DjVuImageNotifier(DjVuInterface *notifier)
  : notifier(notifier)
{
}

GP<DataPool> 
DjVuImageNotifier::request_data(const DjVuPort *src, const GURL & url)
{
  if (url!=stream_url)
    G_THROW( ERR_MSG("DjVuImage.not_decode") );
  return stream_pool;
}

void 
DjVuImageNotifier::notify_redisplay(const class DjVuImage * source)
{
  if (notifier)
    notifier->notify_redisplay();
}

void 
DjVuImageNotifier::notify_relayout(const class DjVuImage * source)
{
  if (notifier)
    notifier->notify_relayout();
}

void 
DjVuImageNotifier::notify_chunk_done(const DjVuPort *, const char *name)
{
  if (notifier)
    notifier->notify_chunk(name, "" );
}

void
DjVuImage::decode(ByteStream & str, DjVuInterface *notifier)
{
  DEBUG_MSG("DjVuImage::decode(): decoding old way...\n");
  DEBUG_MAKE_INDENT(3);
  if (file) 
    G_THROW( ERR_MSG("DjVuImage.bad_call") );
  GP<DjVuImageNotifier> pport = new DjVuImageNotifier(notifier);
  pport->stream_url=GURL::UTF8("internal://fake/fake.djvu");
  pport->stream_pool=DataPool::create();
  // Get all the data first
  int length;
  char buffer[1024];
  while((length=str.read(buffer, 1024)))
    pport->stream_pool->add_data(buffer, length);
  pport->stream_pool->set_eof();
  GP<DjVuDocument> doc = DjVuDocument::create_wait(pport->stream_url, (DjVuImageNotifier*)pport);
  GP<DjVuImage> dimg=doc->get_page(-1, true, (DjVuImageNotifier*)pport);
  file=dimg->get_djvu_file();
  if (file->is_decode_stopped())
    G_THROW("STOP");
  if (file->is_decode_failed())
    G_THROW( ERR_MSG("EOF") );  // a guess
  if (!file->is_decode_ok())
    G_THROW( ERR_MSG("DjVuImage.mult_error") );
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
  GP<IW44Image> bg44 = get_bg44();
  GP<GPixmap>  bgpm = get_bgpm();
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
  if (bg44 || bgpm || fgpm)
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
  GP<IW44Image> bg44 = get_bg44();
  GP<GPixmap>  bgpm = get_bgpm();
  GP<GPixmap>  fgpm = get_fgpm();
  // Check info
  if (! info)
    return 0;
  int width = info->width;
  int height = info->height;
  if (! (width>0 && height>0))
    return 0;
  // Check that extra information is not present.
  if (fgjb || fgpm)
    return 0;
  // Check bg44
  if (bg44 && bg44->get_width()==width && bg44->get_height()==height)
    return 1;
  // Check bgpm
  if (bgpm && (int)bgpm->columns()==width && (int)bgpm->rows()==height)
    return 1;
  // Ok.
  return 0;
}

int 
DjVuImage::is_legal_compound() const
{
  // Components
  GP<DjVuInfo>     info = get_info();
  GP<JB2Image>     fgjb = get_fgjb();
  GP<IW44Image>     bg44 = get_bg44();
  GP<GPixmap>      bgpm = get_bgpm();
  GP<GPixmap>      fgpm = get_fgpm();
  GP<DjVuPalette>  fgbc = get_fgbc();
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
  else if (bgpm)
    bgred = compute_red(width, height, bgpm->columns(), bgpm->rows());
  if (bgred<1 || bgred>12)
    return 0;
  // Check foreground colors
  int fgred = 0;
  if (fgbc)
    fgred = 1;
  else if (fgpm)
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
  int width = get_real_width();
  int height = get_real_height();
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
  
  GP<DjVuInfo> info = get_info();
  int width = get_real_width();
  int height = get_real_height();


  if (width<=0 || height<=0 || !info) return 0;
  // Compute gamma_correction
  double gamma_correction = 1.0;
  if (gamma > 0)
    gamma_correction = gamma / info->gamma;
  if (gamma_correction < 0.1)
    gamma_correction = 0.1;
  else if (gamma_correction > 10)
    gamma_correction = 10;
  
  // CASE1: Incremental BG IW44Image
  GP<IW44Image> bg44 = get_bg44();
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
          pm = GPixmap::create();
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
          GP<GPixmapScaler> gps=GPixmapScaler::create(inw, inh, outw, outh);
          GPixmapScaler &ps=*gps;
          ps.set_horz_ratio(red*po2, subsample);
          ps.set_vert_ratio(red*po2, subsample);
          // run pixmap scaler
          GRect xrect;
          ps.get_input_rect(rect,xrect);
          GP<GPixmap> ipm = bg44->get_pixmap(po2,xrect);
          pm = GPixmap::create();
          ps.scale(xrect, *ipm, rect, *pm);
        }
      // Apply gamma correction
      if (pm && gamma_correction!=1.0)
        pm->color_correct(gamma_correction);
      return pm;
    }

  // CASE 2: Raw background pixmap
  GP<GPixmap>  bgpm = get_bgpm();
  if (bgpm)
    {
      int w = bgpm->columns();
      int h = bgpm->rows();
      // Avoid silly cases
      if (w==0 || h==0 || width==0 || height==0)
        return 0;
      // Determine how much bgpm is reduced
      int red = compute_red(width,height,w,h);
      if (red<1 || red>12)
        return 0;
      // Handle pure downsampling cases
      int ratio = subsample/red;
      if (subsample==ratio*red && ratio>=1)
        {
          pm = GPixmap::create();
          if (ratio == 1)
            pm->init(*bgpm, rect);
          else if (ratio > 1)
            pm->downsample(bgpm, ratio, &rect);
        }
      // Handle all other cases with pixmapscaler
      else
        {
          // setup pixmap scaler
          int outw = (width+subsample-1)/subsample;
          int outh = (height+subsample-1)/subsample;
          GP<GPixmapScaler> gps=GPixmapScaler::create(w, h, outw, outh);
          GPixmapScaler &ps=*gps;
          ps.set_horz_ratio(red, subsample);
          ps.set_vert_ratio(red, subsample);
          // run pixmap scaler
          pm = GPixmap::create();
          GRect xrect(0,0,w,h);
          ps.scale(xrect, *bgpm, rect, *pm);
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
  // Warping and blending. 
  if (!pm)
    return 0;
  // Access components
  
  GP<DjVuInfo> info = get_info();
  int width = get_real_width();
  int height = get_real_height();


  if (width<=0 || height<=0 || !info) return 0;
  GP<JB2Image> fgjb = get_fgjb();
  GP<GPixmap> fgpm = get_fgpm();
  GP<DjVuPalette> fgbc = get_fgbc();
  
  // Compute gamma_correction
  double gamma_correction = 1.0;
  if (gamma > 0)
    gamma_correction = gamma / info->gamma;
  if (gamma_correction < 0.1)
    gamma_correction = 0.1;
  else if (gamma_correction > 10)
    gamma_correction = 10;

  // Compute alpha map and relevant JB2Image components
  GList<int> components;
  GP<GBitmap> bm;
  if (fgjb)
    {
      JB2Image *jimg = fgjb;
      if (! (width && height && 
             jimg->get_width() == width && 
             jimg->get_height() == height ) )
        return 0;
      // Decode bitmap
      bm = GBitmap::create(rect.height(), rect.width());
      bm->set_grays(1+subsample*subsample);
      int rxmin = rect.xmin * subsample;
      int rymin = rect.ymin * subsample;
      for (int blitno = 0; blitno < jimg->get_blit_count(); blitno++)
        {
          const JB2Blit *pblit = jimg->get_blit(blitno);
          const JB2Shape  &pshape = jimg->get_shape(pblit->shapeno);
          if (pshape.bits &&
              pblit->left <= rect.xmax * subsample &&
              pblit->bottom <= rect.ymax * subsample &&
              pblit->left + (int)pshape.bits->columns() >= rect.xmin * subsample &&
              pblit->bottom + (int)pshape.bits->rows() >= rect.ymin * subsample )
            {
              // Record component list
              if (fgbc) components.append(blitno);
              // Blit
              bm->blit(pshape.bits, 
                       pblit->left - rxmin, pblit->bottom - rymin, 
                       subsample);
            }
        }
    }


  // TWO LAYER MODEL
  if (bm && fgbc)
    {
      // Perform attenuation from scratch
      pm->attenuate(bm, 0, 0);
      // Check that fgbc has the correct size
      JB2Image *jimg = fgjb;
      DjVuPalette *fg = fgbc;
      if (jimg->get_blit_count() != fg->colordata.size())
        return 0;
      // Copy and color correct palette
      int palettesize = fg->size();
      GTArray<GPixel> colors(0,palettesize-1);
      for (int i=0; i<palettesize; i++)
        fg->index_to_color(i, colors[i]);
      GPixmap::color_correct(gamma_correction, colors, palettesize);
      // Blit all components (one color at a time)
      while (components.size() > 0)
        {
          GPosition nullpos;
          GPosition pos = components;
          int lastx = 0;
          int colorindex = fg->colordata[components[pos]];
          if (colorindex >= palettesize)
            G_THROW( ERR_MSG("DjVuImage.corrupted") );
          // Gather relevant components and relevant rectangle
          GList<int> compset;
          GRect comprect;
          while (pos)
            {
              int blitno = components[pos];
              const JB2Blit *pblit = jimg->get_blit(blitno);
              if (pblit->left < lastx) break; 
              lastx = pblit->left;
              if (fg->colordata[blitno] == colorindex)
                {
                  const JB2Shape  &pshape = jimg->get_shape(pblit->shapeno);
                  GRect rect(pblit->left, pblit->bottom, 
                             pshape.bits->columns(), pshape.bits->rows());
                  comprect.recthull(comprect, rect);
                  compset.insert_before(nullpos, components, pos);
                  continue;
                }
              ++pos;
            }
          // Round alpha map rectangle
          comprect.xmin = comprect.xmin / subsample;
          comprect.ymin = comprect.ymin / subsample;
          comprect.xmax = (comprect.xmax+subsample-1) / subsample;
          comprect.ymax = (comprect.ymax+subsample-1) / subsample;
          comprect.intersect(comprect, rect);
          // Compute alpha map for that color
          bm = 0;
          bm = GBitmap::create(comprect.height(), comprect.width());
          bm->set_grays(1+subsample*subsample);
          int rxmin = comprect.xmin * subsample;
          int rymin = comprect.ymin * subsample;
          for (pos=compset; pos; ++pos)
            {
              int blitno = compset[pos];
              const JB2Blit *pblit = jimg->get_blit(blitno);
              const JB2Shape  &pshape = jimg->get_shape(pblit->shapeno);
              bm->blit(pshape.bits, 
                       pblit->left - rxmin, pblit->bottom - rymin, 
                       subsample);
            }
          // Blend color into background pixmap
          pm->blit(bm, comprect.xmin-rect.xmin, comprect.ymin-rect.ymin, &colors[colorindex]);
        }
      return 1;
    }


  // THREE LAYER MODEL
  if (bm && fgpm)
    {
      // This follows fig. 4 in Adelson "Layered representations for image
      // coding" (1991) http://www-bcs.mit.edu/people/adelson/papers.html.
      // The properly warped background is already in PM.  The properly warped
      // alpha map is already in BM.  We just have to warp the foreground and
      // perform alpha blending.
#ifdef SIMPLE_THREE_LAYER_RENDERING
      int w = fgpm->columns();
      int h = fgpm->rows();
      // Determine foreground reduction
      int red = compute_red(width,height, w, h);
      if (red<1 || red>12)
        return 0;
      // Warp foreground pixmap
      GPixmapScaler ps(w,h,width/subsample+1,height/subsample+1);
      ps.set_horz_ratio(red,subsample);
      ps.set_vert_ratio(red,subsample);
      GP<GPixmap> nfg = new GPixmap;
      GRect provided(0,0,w,h);
      ps.scale(provided, *fgpm, rect, *nfg);
      // Attenuate background and blit
      nfg->color_correct(gamma_correction);
      pm->blend(bm, 0, 0, nfg); // blend == attenuate + blit
      return 1;
#else 
      // Things are now a little bit more complex because the convenient
      // function GPixmap::stencil() simultaneously upsamples the foreground 
      // by an integer factor and performs the alpha blending.  We have
      // to determine how and when this facility can be used.
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
          // Must pre-warp foreground pixmap
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
              GP<GPixmapScaler> gps=GPixmapScaler::create(w,h,desw,desh);
              GPixmapScaler &ps=*gps;
              ps.set_horz_ratio(red, wantedred);
              ps.set_vert_ratio(red, wantedred);
              nfg = GPixmap::create();
              GRect provided(0,0,w,h);
              GRect desired(0,0,desw,desh);
              ps.scale(provided, *fgpm, desired, *nfg);
            }
          // Use combined warp+blend function
          pm->stencil(bm, nfg, supersample, &rect, gamma_correction);
          // Cache
          tagimage = this;
          tagfgpm = fgpm;
          cachednfg = nfg;
          return 1;
        }
#endif
    }
  
  // FAILURE
  return 0;
}


GP<GPixmap>
DjVuImage::get_fg_pixmap(const GRect &rect, 
                         int subsample, double gamma) const
{
  // Obtain white background pixmap
  GP<GPixmap> pm;
  // Access components
  const int width = get_real_width();
  const int height = get_real_height();
  if (width && height)
  {
    pm = GPixmap::create(rect.height(),rect.width(), &GPixel::WHITE);
    if (!stencil(pm, rect, subsample, gamma))
      pm=0;
  }
  return pm;
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
          const GRect &inrect, const GRect &inall, int align )
{
    GRect rect=inrect;
    GRect all=inall;
///* rotate code
    if( dimg.get_rotate()%4 )
    {
        GRectMapper mapper;

        GRect input, output;

        input = GRect(0,0,all.width(), all.height());
        if( dimg.get_rotate() & 1 )
            output = GRect(0,0,all.height(), all.width());
        else
            output = GRect(0,0,all.width(), all.height());

        mapper.clear();
        mapper.set_input(input);
        mapper.set_output(output);
        mapper.rotate((4-dimg.get_rotate())%4);

        mapper.map(rect);
        mapper.map(all);
    }
///* rotate code ends

  // Sanity
  if (! ( all.contains(rect.xmin, rect.ymin) &&
          all.contains(rect.xmax-1, rect.ymax-1) ))
    G_THROW( ERR_MSG("DjVuImage.bad_rect") );
  // Check for integral reduction
  int red;
  int w = dimg.get_real_width();
  int h = dimg.get_real_height();

  int rw = all.width();
  int rh = all.height();
  GRect zrect = rect; 
  zrect.translate(-all.xmin, -all.ymin);
  for (red=1; red<=15; red++)
    if (rw*red>w-red && rw*red<w+red && rh*red>h-red && rh*red<h+red)
    {
        GP<GBitmap> bm=(dimg.*get)(zrect, red, align);
        if(bm)
            return bm->rotate((4-dimg.get_rotate())%4);
        else
	        return NULL;
    }
  // Find best reduction
  for (red=15; red>1; red--)
    if ( (rw*red < w && rh*red < h) ||
         (rw*red*3 < w || rh*red*3 < h) )
      break;
  // Setup bitmap scaler
  if (! (w && h)) return 0;
  GP<GBitmapScaler> gbs=GBitmapScaler::create();
  GBitmapScaler &bs=*gbs;
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
  GP<GBitmap> bm = GBitmap::create(zrect.height(), zrect.width(), border);
  bs.scale(srect, *sbm, zrect, *bm);
  if( bm )
      return bm->rotate((4-dimg.get_rotate())%4);
  else
      return NULL;
}

static GP<GPixmap>
do_pixmap(const DjVuImage &dimg, PImager get,
          const GRect &inrect, const GRect &inall, double gamma )
{

    GRect rect=inrect;
    GRect all=inall;
///* rotate code
    if( dimg.get_rotate()%4 )
    {
        GRectMapper mapper;

        GRect input, output;

        input = GRect(0,0,all.width(), all.height());
        if( dimg.get_rotate() & 1 )
            output = GRect(0,0,all.height(), all.width());
        else
            output = GRect(0,0,all.width(), all.height());

        mapper.clear();
        mapper.set_input(input);
        mapper.set_output(output);
        mapper.rotate((4-dimg.get_rotate())%4);

        mapper.map(rect);
        mapper.map(all);
    }
///* rotate code ends

  // Sanity
  if (! ( all.contains(rect.xmin, rect.ymin) &&
          all.contains(rect.xmax-1, rect.ymax-1) ))
    G_THROW( ERR_MSG("DjVuImage.bad_rect2") );
  // Check for integral reduction
  int red, w=0, h=0, rw=0, rh=0;
  w = dimg.get_real_width();
  h = dimg.get_real_height();
  

  rw = all.width();
  rh = all.height();
  GRect zrect = rect; 
  zrect.translate(-all.xmin, -all.ymin);
  for (red=1; red<=15; red++)
    if (rw*red>w-red && rw*red<w+red && rh*red>h-red && rh*red<h+red)
    {
        GP<GPixmap> pm = (dimg.*get)(zrect, red, gamma);
        if( pm ) 
            return pm->rotate((4-dimg.get_rotate())%4);
        else
            return NULL;
    }
  // These reductions usually go faster (improve!)
  static int fastred[] = { 12,6,4,3,2,1 };
  // Find best reduction
  for (int i=0; (red=fastred[i])>1; i++)
    if ( (rw*red < w && rh*red < h) ||
         (rw*red*3 < w || rh*red*3 < h) )
      break;
  // Setup pixmap scaler
  if (w<0 || h<0) return 0;
  GP<GPixmapScaler> gps=GPixmapScaler::create();
  GPixmapScaler &ps=*gps;
  ps.set_input_size( (w+red-1)/red, (h+red-1)/red );
  ps.set_output_size( rw, rh );
  ps.set_horz_ratio( rw*red, w );
  ps.set_vert_ratio( rh*red, h );
  // Scale
  GRect srect;
  ps.get_input_rect(zrect, srect);
  GP<GPixmap> spm = (dimg.*get)(srect, red, gamma);
  if (!spm) return 0;
  GP<GPixmap> pm = GPixmap::create();
  ps.scale(srect, *spm, zrect, *pm);
  if(pm)
      return pm->rotate((4-dimg.get_rotate())%4);
  else
      return NULL;
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

int 
DjVuImage::get_rotate() const
{
  return (rotate_count<0)?0:rotate_count;
}

void
DjVuImage::init_rotate(const DjVuInfo &info)
{ 
  rotate_count=4;
  for(int a=(int)(info.orientation);(a!=(int)GRect::BULRNR)&&(a!=(int)GRect::BURLNR);--rotate_count)
  {
    a^=((a&(int)GRect::ROTATE90_CW)
      ?(int)(GRect::BOTTOM_UP|GRect::MIRROR|GRect::ROTATE90_CW)
      :(int)(GRect::ROTATE90_CW));
  }
  rotate_count%=4;
}

void DjVuImage::set_rotate(int count) 
{ 
  rotate_count=((count%4)+4)%4;
}

GP<DjVuAnno> 
DjVuImage::get_decoded_anno()
{
    GP<DjVuAnno> djvuanno = DjVuAnno::create();
    GP<ByteStream> bs=get_anno();
    if( bs )
    {
        djvuanno->decode(bs);
       
        const int rotate_count=get_rotate(); 
        if( rotate_count % 4 )
        {   
            ///map hyperlinks correctly for rotation           
            GRect input, output;
            input = GRect(0,0,get_width(), get_height());
            output = GRect(0,0,  get_real_width(), get_real_height());

            GRectMapper mapper;
            mapper.clear();
            mapper.set_input(input);
            mapper.set_output(output);               
            mapper.rotate((4-rotate_count)%4);

            GPList<GMapArea> &list=djvuanno->ant->map_areas;
            for(GPosition pos=list;pos;++pos)
            {
                list[pos]->unmap(mapper);
            }
        }
        return djvuanno;
    }
    else
        return NULL;
}


void
DjVuImage::map(GRect &rect) const
{
    GRect input, output;
    const int rotate_count=get_rotate(); 
    if(rotate_count%4)
    {  
        input = GRect(0,0,get_width(), get_height());
        output = GRect(0,0, get_real_width(), get_real_height());

        GRectMapper mapper;
        mapper.clear();
        mapper.set_input(input);
        mapper.set_output(output);               
        mapper.rotate((4-rotate_count)%4);
        mapper.map(rect);
    }
}

void
DjVuImage::unmap(GRect &rect) const
{
    GRect input, output;
    const int rotate_count=get_rotate(); 
    if(rotate_count%4)
    {  
        input = GRect(0,0,get_width(), get_height());
        output = GRect(0,0, get_real_width(), get_real_height());

        GRectMapper mapper;
        mapper.clear();
        mapper.set_input(input);
        mapper.set_output(output);               
        mapper.rotate((4-rotate_count)%4);
        mapper.unmap(rect);
    }
}

void
DjVuImage::map(int &x, int &y) const
{
    GRect input, output;
    const int rotate_count=get_rotate(); 
    if(rotate_count%4)
    {  
        input = GRect(0,0,get_width(), get_height());
        output = GRect(0,0, get_real_width(), get_real_height());

        GRectMapper mapper;
        mapper.clear();
        mapper.set_input(input);
        mapper.set_output(output);               
        mapper.rotate((4-rotate_count)%4);
        mapper.map(x, y);
    }
}

void
DjVuImage::unmap(int &x, int &y) const
{
    GRect input, output;
    const int rotate_count=get_rotate(); 
    if(rotate_count%4)
    {  
        input = GRect(0,0,get_width(), get_height());
        output = GRect(0,0, get_real_width(), get_real_height());

        GRectMapper mapper;
        mapper.clear();
        mapper.set_input(input);
        mapper.set_output(output);               
        mapper.rotate((4-rotate_count)%4);
        mapper.unmap(x, y);
    }
}

