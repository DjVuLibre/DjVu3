//C-  -*- C++ -*-
//C-
//C-  Copyright (c) 1988 AT&T	
//C-  All Rights Reserved 
//C-
//C-  THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
//C-  The copyright notice above does not evidence any
//C-  actual or intended publication of such source code.
//C-
//C-  $Id: DejaVuDecoder.cpp,v 1.1.1.1 1999-01-22 00:40:19 leonb Exp $

// File "$Id: DejaVuDecoder.cpp,v 1.1.1.1 1999-01-22 00:40:19 leonb Exp $"
// - Author: Leon Bottou, 07/1997


#ifdef __GNUC__
#pragma implementation "DejaVuCodec.h"
#endif

#include "GException.h"
#include "GRect.h"
#include "GString.h"
#include "GPixmap.h"
#include "GBitmap.h"
#include "JB2Codec.h"
#include "ByteStream.h"
#include "IFFByteStream.h"
#include "DejaVuCodec.h"
#include "GScaler.h"
#ifdef HANDLE_JPEG_PIXMAP
#include "JPEGCodec.h"
#endif


// CLASS DEJAVUIMAGE

DejaVuImage::DejaVuImage()
: width(0), height(0),
  filesize(0), deltasize(0)
{
  memset(&info, -1, sizeof(info));
}


int          
DejaVuImage::get_version() const
{
  return (info.version_hi << 8) | (info.version_lo);
}


int          
DejaVuImage::get_dpi() const
{
  return (info.dpi_hi<<8) | (info.dpi_lo);
}


double
DejaVuImage::get_target_gamma() const
{
  return (double)info.gamma10 / 10.0;
}


unsigned int
DejaVuImage::get_memory_usage() const
{
  unsigned int usage = sizeof(DejaVuImage);
  // Components
  if (bgpm) 
    usage += bgpm->get_memory_usage();
  if (bg44) 
    usage += bg44->get_memory_usage();
  if (fgpm) 
    usage += fgpm->get_memory_usage();
  if (jb2stencil)
    usage += jb2stencil->get_memory_usage();
  // Strings
  usage += annotation.length();
  usage += mimetype.length();
  usage += desc.length();
  return usage;
}

int
DejaVuImage::get_suggested_scales(int maxscales, GRatio *scales)
{
  // Returns scales that are suggested for this image.
  // i.e. they are supported and quite efficient.
  int n = -1;
  // This is quite crude at the moment.
  if (mimetype == "image/iw44")
    {
      // Upsampling values
      if (++n<maxscales) scales[n]=GRatio(1,3);       
      if (++n<maxscales) scales[n]=GRatio(1,2);
      if (++n<maxscales) scales[n]=GRatio(2,3);
      if (++n<maxscales) scales[n]=GRatio(1); 
      if (++n<maxscales) scales[n]=GRatio(4,3); 
      if (++n<maxscales) scales[n]=GRatio(2); 
      if (++n<maxscales) scales[n]=GRatio(4);
      if (++n<maxscales) scales[n]=GRatio(6);
    }
  else
    {
      // Standard supsampling values
      if (++n<maxscales) scales[n]=GRatio(1); 
      if (++n<maxscales) scales[n]=GRatio(2); 
      if (++n<maxscales) scales[n]=GRatio(3); 
      if (++n<maxscales) scales[n]=GRatio(4);
      if (++n<maxscales) scales[n]=GRatio(6);
      if (++n<maxscales) scales[n]=GRatio(12);
    }
  return ++n;
}


GP<GBitmap>
DejaVuImage::get_bitmap(GRatio scale, int align)
{
  int supsample = scale.get_q();
  int subsample = scale.get_p();
  int swidth = (width*supsample+subsample-1)/subsample;
  int sheight = (height*supsample+subsample-1)/subsample;
  GRect rect(0,0,swidth,sheight);
  return get_bitmap(rect, scale, align);
}


GP<GBitmap>
DejaVuImage::get_bitmap(const GRect &rect, GRatio scale, int align)
{
  if (jb2stencil && scale.get_q()==1)
    return jb2stencil->get_bitmap(rect, scale.get_p(), align);
  return 0;
}


GP<GPixmap>
DejaVuImage::get_background_pixmap(const GRect &rect, GRatio scale, double gamma)
{
  GP<GPixmap> pm = 0;
  // Compute gamma_correction
  double gamma_correction = 1.0;
  if (gamma > 0)
    gamma_correction = gamma / get_target_gamma();
  if (gamma_correction < 0.1)
    gamma_correction = 0.1;
  else if (gamma_correction > 10)
    gamma_correction = 10;
  
  // CASE1: Incremental BG IWPixmap
  if (bg44)
    {
      int red;
      int w = bg44->get_width();
      int h = bg44->get_height();
      // Avoid silly cases
      if (w==0 || h==0 || width==0 || height==0)
        return 0;
      // Determine how much bg44 is reduced
      for (red=1; red<=12; red++)
        if (w==(width+red-1)/red && h==(height+red-1)/red)
          break;
      if (red>12)
        return 0;
      // Unpack ratio
      // Compute scaling information
      int supsample = scale.get_q();
      int subsample = scale.get_p();
      int redsup = red*supsample;
      // Handle pure downsampling cases
      if (subsample == redsup)
        pm = bg44->get_pixmap(1,rect);
      else if (subsample == 2*redsup)
        pm = bg44->get_pixmap(2,rect);    
      else if (subsample == 4*redsup)
        pm = bg44->get_pixmap(4,rect); 
      else if (subsample == 8*redsup)
        pm = bg44->get_pixmap(8,rect); 
      // Handle fractional downsampling case
      else if (redsup*4 == subsample*3)
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
#ifdef FIXED_SCALING
      // Handle pure upsampling cases
      else if (redsup % subsample == 0)
        {
          int ups = redsup/subsample;
          GRect nrect = rect;
          GRect xrect = rect;
          xrect.xmin = xrect.xmin/ups;
          xrect.ymin = xrect.ymin/ups;
          xrect.xmax = (xrect.xmax+ups-1)/ups;
          xrect.ymax = (xrect.ymax+ups-1)/ups;
          nrect.translate(-xrect.xmin*ups, -xrect.ymin*ups);
          if (xrect.xmax > w) 
            xrect.xmax = w;
          if (xrect.ymax > h) 
            xrect.ymax = h;
          GP<GPixmap> ipm = bg44->get_pixmap(1,xrect);
          pm = new GPixmap();
          pm->upsample(ipm, ups, &nrect);
        }
      // Handle fractional upsampling case
      else if (redsup*2 == subsample*3)
        {
          GRect nrect = rect;
          GRect xrect = rect;
          xrect.xmin = (xrect.xmin/3)*2;
          xrect.ymin = (xrect.ymin/3)*2;
          xrect.xmax = ((xrect.xmax+2)/3)*2;
          xrect.ymax = ((xrect.ymax+2)/3)*2;
          nrect.translate(-xrect.xmin*3/2, -xrect.ymin*3/2);
          if (xrect.xmax > w) 
            xrect.xmax = w;
          if (xrect.ymax > h) 
            xrect.ymax = h;
          GP<GPixmap> ipm = bg44->get_pixmap(1,xrect);
          pm = new GPixmap();
          pm->upsample23(ipm, &nrect);
        }
#else
      // Handle all other cases with pixmapscaler
      else
        {
          // find suitable power of two
          int po2 = 16;
          while (po2>1 && subsample<po2*redsup)
            po2 >>= 1;
          // setup pixmap scaler
          int inw = (w+po2-1)/po2;
          int inh = (h+po2-1)/po2;
          int outw = (width*supsample+subsample-1)/subsample;
          int outh = (height*supsample+subsample-1)/subsample;
          GPixmapScaler ps(inw, inh, outw, outh);
          ps.set_horz_ratio(redsup*po2, subsample);
          ps.set_vert_ratio(redsup*po2, subsample);
          // run pixmap scaler
          GRect xrect;
          ps.get_input_rect(rect,xrect);
          GP<GPixmap> ipm = bg44->get_pixmap(po2,xrect);
          pm = new GPixmap();
          ps.scale(xrect, *ipm, rect, *pm);
        }
#endif
      // Apply gamma correction
      if (pm && gamma_correction!=1.0)
        pm->color_correct(gamma_correction);
      return pm;
    }

  // CASE2: Explicit BG pixmap
  if (bgpm)
    {
      int red;
      int w = bgpm->columns();
      int h = bgpm->rows();
      // Avoid silly cases
      if (w==0 || h==0 || width==0 || height==0)
        return 0;
      // Determine how much bgpm is reduced
      for (red=1; red<=12; red++)
        if (w==(width+red-1)/red && h==(height+red-1)/red)
          break;
      if (red>12)
        return 0;
      pm = new GPixmap;
      // Compute scaling information
      int supsample = scale.get_q();
      int subsample = scale.get_p();
      int redsup = red*supsample;
      // Handle pure downsampling cases
      if (subsample == redsup)
        pm->init(*bgpm, rect);
      else if (subsample == 2*redsup)
        pm->downsample(bgpm, 2, &rect);
      else if (subsample == 4*redsup)
        pm->downsample(bgpm, 4, &rect);
      else if (subsample == 8*redsup)
        pm = bg44->get_pixmap(8,rect); 
      // Handle fractional downsampling cases
      else if (redsup*4 == subsample*3)
        pm->downsample43(bgpm, &rect);
#ifdef FIXED_SCALING
      // Handle pure upsampling cases
      else if (redsup % subsample == 0)
        pm->upsample(bgpm, red/subsample, &rect);
      // Handle fractional upsampling
      else if (redsup*2 == subsample*3)
        pm->upsample23(bgpm, &rect);
      else
        return 0;
#else
      // Handle all other cases with pixmapscaler
      else
        {
          // setup pixmap scaler
          int outw = (width*supsample+subsample-1)/subsample;
          int outh = (height*supsample+subsample-1)/subsample;
          GPixmapScaler ps(w, h, outw, outh);
          ps.set_horz_ratio(redsup, subsample);
          ps.set_vert_ratio(redsup, subsample);
          // run pixmap scaler
          pm = new GPixmap();
          GRect xrect(0,0,w,h);
          ps.scale(xrect, *bgpm, rect, *pm);
        }
#endif
      // Apply gamma correction
      if (pm && gamma_correction!=1.0)
        pm->color_correct(gamma_correction);
      return pm;
    }
  // Failure
  return 0;
}



int  
DejaVuImage::apply_stencil(GPixmap *pm, const GRect &rect, GRatio scale, double gamma)
{
  // Compute scaling information
  int subsample = scale.get_p();
  int supsample = scale.get_q();
  if (supsample != 1) return 0;
  // Compute gamma_correction
  double gamma_correction = 1.0;
  if (gamma > 0)
    gamma_correction = gamma / get_target_gamma();
  if (gamma_correction < 0.1)
    gamma_correction = 0.1;
  else if (gamma_correction > 10)
    gamma_correction = 10;

  // CASE1: JB2 stencil and FG pixmap
  if (jb2stencil && fgpm)
    {
      GP<GPixmap> fg = fgpm;
      GP<GBitmap> bm = get_bitmap(rect, scale);
      if (bm && pm)
        {
          int red;
          int w = fg->columns();
          int h = fg->rows();
          // Determine foreground reduction
          for (red=1; red<=12; red++)
            if (w==(width+red-1)/red && h==(height+red-1)/red)
              break;
          if (red>12)
            return 0;
          // Refuse
          // Perform further reduction if necessary
          if (red+red<subsample)
            {
              GP<GPixmap> nfg = new GPixmap;
              int ds = (subsample+red+red-1)/(red+red);
              nfg->downsample(fg, ds);
              fg = nfg;
              red = red*ds;
              w = fg->columns();
              h = fg->rows();
            }
          // Try supersampling foreground pixmap by an integer factor
          int supersample = ( red>subsample ? red/subsample : 1);
          int wantedred = supersample*subsample;
          // Try simple foreground upsampling
          if (red == wantedred)
            {
              pm->stencil(bm, fg, supersample, &rect, gamma_correction);
              return 1;
            }
          else 
            {
              // Must rescale foreground pixmap first
              int desw = (w*red+wantedred-1)/wantedred;
              int desh = (h*red+wantedred-1)/wantedred;
              GPixmapScaler ps(w,h,desw,desh);
              ps.set_horz_ratio(red, wantedred);
              ps.set_vert_ratio(red, wantedred);
              GP<GPixmap> nfg = new GPixmap;
              GRect provided(0,0,w,h);
              GRect desired(0,0,desw,desh);
              ps.scale(provided, *fg, desired, *nfg);
              pm->stencil(bm, nfg, supersample, &rect, gamma_correction);
              return 1;
            }
        }
    }
  
  // CASE2: JB2 stencil but no color
  if (jb2stencil)
    {
      GP<GBitmap> bm = get_bitmap(rect, scale);
      if (bm && pm)
        {
          GPixmap black(1, 1, &GPixel::BLACK);
          int pms = pm->rows() + pm->columns();
          pm->stencil(bm, &black, pms, 0, gamma_correction);
          return 1;
        }
    }
  // FAILURE
  return 0;
}


GP<GPixmap>
DejaVuImage::get_foreground_pixmap(const GRect &rect, GRatio scale, double gamma)
{
  // Obtain white background pixmap
  GP<GPixmap> pm = 0;
  if (width && height)
    pm = new GPixmap(rect.height(),rect.width(), &GPixel::WHITE);
  // Apply stencil
  if (apply_stencil(pm, rect, scale, gamma))
    return pm;
  return 0;
}


GP<GPixmap>
DejaVuImage::get_color_pixmap(const GRect &rect, GRatio scale, double gamma)
{
  // Straightforward
  GP<GPixmap> pm = get_background_pixmap(rect, scale, gamma);
  // Avoid ugly intermediate display
  if (jb2stencil && !fgpm)
    return 0;
  // Superimpose foreground
  apply_stencil(pm, rect, scale, gamma);
  return pm;
}


inline int 
DejaVuImage::is_color(void)
{
  // Quite silly predicate
  if (!width && !height) 
    return 0;
  int w=10, h=10;
  if (w>width) w=width;
  if (h>height) h=height;
  GRect grect(0, 0, w, h);
  return get_color_pixmap(grect, 1)!=0;
}


GString 
DejaVuImage::get_short_description()
{
  GString msg = "empty";
  if (width && height)
    if (filesize > 2048)
      msg.format("%dx%d in %0.1f Kb", width, height, (filesize+512)/1024.0);
    else
      msg.format("%dx%d", width, height);      
  return msg;
}


GString 
DejaVuImage::get_long_description()
{
      // '\t' in the description look really ugly. What we want to do here is to
      // replace '\t' with spaces so that every column is really aligned
   GString result=desc;
   while(1)	// Loop over tabs. Each iteration removes first tab from each line
   {
      int tab_pos=0;
      
      const char * ptr=result;
      const char * line_start=result;
      int done_smth=0;
      while(1)	// Loop over lines
      {
	 for(;*ptr && *ptr!='\n';ptr++)
	    if (*ptr=='\t')
	    {
	       if (tab_pos<ptr-line_start) tab_pos=ptr-line_start;
	       while(*ptr && *ptr!='\n') ptr++;
	       done_smth=1;
	       break;
	    };
	 if (!*ptr) break;
	 line_start=++ptr;
      };
      if (!done_smth) break;

	 // Now modify the string to replace the first tab in each line with spaces
      GString tmp;
      tmp=result+result;	// To avoid malloc for every added char
      int cnt=0;
      ptr=result;
      line_start=result;
      while(1)
      {
	 for(;*ptr && *ptr!='\n';ptr++)
	    if (*ptr=='\t')
	    {
	       for(int i=0;i<tab_pos-(ptr-line_start)+2;i++) tmp.setat(cnt++, ' ');
	       for(ptr++;*ptr && *ptr!='\n';ptr++) tmp.setat(cnt++, *ptr);
	       break;
	    } else tmp.setat(cnt++, *ptr);
	 if (!*ptr) break;
	 tmp.setat(cnt++, '\n');
	 line_start=++ptr;
      };
      tmp.setat(cnt, 0);
      result=tmp;
   };
   return result;
}


void 
DejaVuImage::add_description(const char *s)
{
  GString str = desc;
  desc = str + s;
}



// DEJAVUDECODER

DejaVuDecoder::DejaVuDecoder(ByteStream &bs)
  : iff(0), bs(bs)
{
}


DejaVuDecoder::~DejaVuDecoder()
{
   if (iff) 
     delete iff; 
   iff = 0;
}


void 
DejaVuDecoder::decode(DejaVuImage * dimg,
                      void (* callback)(void *, int, const char *, const char *),
                      void * arg)
{
  GString chunk_name, message;
  int do_redraw, call_again;
  do
     {
       decode(dimg, &chunk_name, &message, &do_redraw, &call_again);
       if (callback)
         callback(arg, do_redraw, chunk_name,
                  message.length() ? (const char *) message : 0);
     } 
   while(call_again);
}


void 
DejaVuDecoder::decode(DejaVuImage * dimg, GString * chunk_name,
                      GString * message, int * do_redraw, int * call_again)
{
  // Decode iff form header
  if (! iff)
    {
      GString chkid;
      iff=new IFFByteStream(&bs);
      if (! iff->get_chunk(chkid))
        THROW("EOF");
      // Auto-determine MIME type
      if (chkid == "FORM:DJVU")
        dimg->mimetype = "image/djvu";
      else if (chkid == "FORM:PM44" || chkid == "FORM:BM44" )
        dimg->mimetype = "image/iw44";
      else
        THROW("DejaVu decoder: a DJVU or IW44 image was expected");
    }
  // Call decoder according to detected MIME type
  if (dimg->mimetype == "image/djvu")
    decode_djvu(dimg, chunk_name, message, do_redraw, call_again);    
  else if (dimg->mimetype == "image/iw44")
    decode_iw44(dimg, chunk_name, message, do_redraw, call_again);
  else
    THROW("DejaVu Decoder: unknown MIME type");
}


void
DejaVuDecoder::decode_iw44(DejaVuImage *dimg, GString * chunk_name,
			   GString * message, int * do_redraw, int * call_again)
{
  GString desc;
  GString chkid;
  int chksize;
  if (!iff)
    THROW("DejaVu decoder: internal error");
  // Decode one chunk
  IWPixmap *img44 = dimg->bg44;
  if (( chksize = iff->get_chunk(chkid) ))
    {
      *chunk_name=chkid;
      *message="";
      *do_redraw=0;
      *call_again=0;
      if (chkid=="PM44" || chkid=="BM44")
        {
          // Create IWPixmap
          if (! img44 )
            img44 = new IWPixmap;
          // Decode Chunk
          img44->decode_chunk(*iff);
          // Update Dimg
          if (! dimg->bg44)
            {
              dimg->bg44 = img44;
              dimg->width = img44->get_width();
              dimg->height = img44->get_height();
              memset((void*)&dimg->info, 0, sizeof(dimg->info));
              dimg->info.version_lo = (DEJAVUVERSION & 0xff);
              dimg->info.version_hi = ((DEJAVUVERSION >> 8) & 0xff);
              dimg->info.gamma10 = 22;
              dimg->info.dpi_hi = 0;
              dimg->info.dpi_lo = 100;
              desc.format("IW44 Image (%dx%d) :\n\n", 
                          img44->get_width(), img44->get_height() );
              dimg->add_description(desc);
            }
          desc.format(" %0.1f Kb\t'%s'\tIW44 image (part %d)\n", 
                      (chksize+512)/1024.0, (const char*)chkid, img44->get_serial());
          dimg->add_description(desc);
          *message = desc;
          *call_again=1;
          *do_redraw = 1;
        }
      //--- CHUNK "ANTa"
      else if (chkid == "ANTa")
        {
          GString ant;
          char buffer[1024];
          int length;
          while ((length=iff->readall(buffer, 1024)))
            ant += (const char *) GString(buffer, length);
          dimg->annotation=ant;
          // Add description
          GString desc;
          desc.format(" %0.1f Kb\t'%s'\tPage annotation.\n",
                      (chksize+512)/1024.0, (const char*)chkid);
          dimg->add_description(desc);
          *message=desc;
          *do_redraw=0;
          *call_again=1;
        }
      //--- Unknown chunk
      else
        {
          GString desc;
          desc.format(" %0.1f Kb\t'%s'\tUnknown chunk.\n", 
                      (chksize+512)/1024.0, (const char*)chkid);
          dimg->add_description(desc);
          *message=desc;
          *do_redraw=0;
          *call_again=1;
        }
      iff->close_chunk();
      return;
    }
  if (img44)
    {
      dimg->filesize = iff->tell();
      int rawsize = dimg->width*dimg->height * 3;
      desc.format("\nCompression ratio : %0.1f  (%0.1f Kb)\n", 
                  (float) rawsize/dimg->filesize, (dimg->filesize+512)/1024.0 );
      dimg->add_description(desc);
    }
  *chunk_name="PM44";
  *message="";
  *do_redraw=1;
  *call_again=0;
}



void
DejaVuDecoder::decode_djvu(DejaVuImage *dimg, GString * chunk_name,
			   GString * message, int * do_redraw, int * call_again)
{
  GString chkid;
  int chksize;
  if (!iff)
    THROW("DejaVu decoder: internal error");
  // Decode one chunk
  if (( chksize = iff->get_chunk(chkid) ))
    {
      *chunk_name=chkid;
      *message="";
      *do_redraw=0;
      *call_again=0;
      //--- CHUNK "INFO"
      if (chkid == "INFO")
        {
          if (dimg->width || dimg->height)
            THROW("DjVu Decoder: Corrupted file (Duplicate INFO chunk)"); 
          // Read chunk data into info field
          memset((void*)&dimg->info, 0, sizeof(dimg->info));
          int size = iff->readall((void*)&dimg->info, sizeof(dimg->info));
          // Interpret size information
          if (size < 4)
            THROW("DjVu Decoder: Corrupted file (Truncated INFO chunk)");
          dimg->width = (dimg->info.width_hi<<8) + dimg->info.width_lo;
          dimg->height = (dimg->info.height_hi<<8) + dimg->info.height_lo;
          if (!dimg->width || !dimg->height)
            THROW("DjVu Decoder: Corrupted file (Zero dimension in INFO chunk)");
          if (!dimg->info.version_lo && !dimg->info.version_hi)
            THROW("DjVu Decoder: Corrupted file (No version number in INFO chunk)");
          // Fixup leftovers from the past
          if (dimg->info.version_hi == 0xff)
            { dimg->info.version_hi = 0; }
          if (dimg->info.dpi_hi == 0xff)
            { dimg->info.dpi_lo = dimg->info.dpi_hi = 0; }
          // Setup default values
          if (dimg->info.dpi_lo == 0 && dimg->info.dpi_hi == 0)
            { dimg->info.dpi_hi = 1;  dimg->info.dpi_lo = 300-256; }
          if (dimg->info.gamma10 < 3 || dimg->info.gamma10 > 50)
            { dimg->info.gamma10 = 22; }
          // Refuse to decode files that are much more modern than the library
          if (dimg->get_version() >= DEJAVUVERSION_TOO_NEW)
            return;
          // Add description
          GString desc;
          desc.format("DjVu Page (%dx%d) :\n\n", dimg->width, dimg->height);
          dimg->add_description(desc);
          desc.format(" %0.1f Kb\t'%s'\tPage information.\n", 
                      (chksize+512)/1024.0, (const char*)chkid);
          dimg->add_description(desc);
          *message=desc;
          *do_redraw=1;
          *call_again=1;
        }
      //--- CHUNK "ANTa"
      else if (chkid == "ANTa")
        {
          GString ant;
          char buffer[1024];
          int length;
          while ((length=iff->readall(buffer, 1024)))
            ant += (const char *) GString(buffer, length);
          dimg->annotation=ant;
          // Add description
          GString desc;
          desc.format(" %0.1f Kb\t'%s'\tPage annotation.\n",
                      (chksize+512)/1024.0, (const char*)chkid);
          dimg->add_description(desc);
          *message=desc;
          *do_redraw=0;
          *call_again=1;
        }
      //--- CHUNK "BG44"
      else if (chkid == "BG44")
        {
          if (!dimg->width || !dimg->height)
            THROW("DjVu Decoder: Corrupted data (Missing INFO chunk)");
          if (dimg->bgpm)
            THROW("DjVu Decoder: Corrupted data (Duplicate BGxx chunk)");
          GP<IWPixmap> bg44 = dimg->bg44;
          if (! bg44)
            bg44 = new IWPixmap;
          if (! bg44->decode_chunk(*iff))
            bg44->close_codec();
          // Check pixmap size
          int w = bg44->get_width();
          int h = bg44->get_height();
          int red;
          for (red=1; red<=12; red++)
            if ((dimg->width+red-1)/red == w)
              if ((dimg->height+red-1)/red == h)
                break;
          if (red>12)
            THROW("DjVu Decoder: Corrupted data (Incorrect size in BG44 chunk)\n");
          // Update image
          if (! dimg->bg44) 
            dimg->bg44 = bg44;
          // Add description
          GString desc;
          int basedpi = dimg->get_dpi();
          if (! basedpi) basedpi = 300;
          desc.format(" %0.1f Kb\t'%s'\tIW44 background, %d dpi, part %d.\n", 
                      (chksize+512)/1024.0, (const char*)chkid, 
                      basedpi/red, dimg->bg44->get_serial());
          dimg->add_description(desc);
          *message=desc;
          *do_redraw=1;
          *call_again=1;
        }
      //--- CHUNK "BGjp"
      else if (chkid == "BGjp")
        {
          if (!dimg->width || !dimg->height)
            THROW("DjVu Decoder: Corrupted data (Missing INFO chunk)");
          if (dimg->bgpm || dimg->bg44)
            THROW("DjVu Decoder: Corrupted data (Duplicate BGxx chunk)");
#ifdef HANDLE_JPEG_PIXMAP
          // Read chunk data
          GP<GPixmap> pm = new GPixmap;
          JPEGDecoder jpdecoder(iff);
          jpdecoder.decode(pm);
          // Check pixmap size
          int w = pm->columns();
          int h = pm->rows();
          int red;
          for (red=1; red<=12; red++)
            if ((dimg->width+red-1)/red == w)
              if ((dimg->height+red-1)/red == h)
                break;
          if (red>12)
            THROW("DjVu Decoder: Corrupted data (Incorrect size in FGjp chunk)\n");
          // Store pixmap
          dimg->bgpm = pm;
          // Add description
          GString desc;
          int basedpi = dimg->get_dpi();
          if (! basedpi) basedpi = 300;
          desc.format(" %0.1f Kb\t'%s'\tJPEG background, %d dpi.\n", 
                      (chksize+512)/1024.0, (const char*)chkid, basedpi/red);
#else
          GString desc;
          desc.format(" %0.1f Kb\t'%s'\tJPEG background (OBSOLETE).\n", 
                      (chksize+512)/1024.0, (const char*)chkid);
#endif
          dimg->add_description(desc);
          *message=desc;
          *do_redraw=1;
          *call_again=1;
        }
      //--- CHUNK "FG44"
      else if (chkid == "FG44")
        {
          if (!dimg->width || !dimg->height)
            THROW("DjVu Decoder: Corrupted data (Missing INFO chunk)");
          if (dimg->fgpm)
            THROW("DjVu Decoder: Corrupted data (Duplicate FGxx chunk)");
          GP<IWPixmap> fg44 = new IWPixmap;
          fg44->decode_chunk(*iff);
          GP<GPixmap> pm = fg44->get_pixmap();
          // Check pixmap size
          int w = pm->columns();
          int h = pm->rows();
          int red;
          for (red=1; red<=12; red++)
            if ((dimg->width+red-1)/red == w)
              if ((dimg->height+red-1)/red == h)
                break;
          if (red>12)
            THROW("DjVu Decoder: Corrupted data (Incorrect size in FG44 chunk)\n");
          // Update image
          dimg->fgpm = pm;
          // Add description
          GString desc;
          int basedpi = dimg->get_dpi();
          if (! basedpi) basedpi = 300;
          desc.format(" %0.1f Kb\t'%s'\tIW44 foreground, %d dpi.\n", 
                      (chksize+512)/1024.0, (const char*)chkid, basedpi/red );
          dimg->add_description(desc);
          *message=desc;
          *do_redraw=1;
          *call_again=1;
        }
      //--- CHUNK "FGjp"
      else if (chkid == "FGjp")
        {
          if (!dimg->width || !dimg->height)
            THROW("DjVu Decoder: Corrupted file (Missing INFO chunk)");
          if (dimg->fgpm)
            THROW("DjVu Decoder: Corrupted data (Duplicate FGxx chunk)");
#ifdef HANDLE_JPEG_PIXMAP
          // Read chunk data
          GP<GPixmap> pm = new GPixmap;
          JPEGDecoder jpdecoder(iff);
          jpdecoder.decode(pm);
          // Check pixmap size
          int w = pm->columns();
          int h = pm->rows();
          int red;
          for (red=1; red<=12; red++)
            if ((dimg->width+red-1)/red == w)
              if ((dimg->height+red-1)/red == h)
                break;
          if (red>12)
            THROW("DjVu Decoder: Corrupted data (Incorrect size in FGjp chunk)\n");
          // Update image
          dimg->fgpm = pm;
          // Add description
          GString desc;
          int basedpi = dimg->get_dpi();
          if (! basedpi) basedpi = 300;
          desc.format(" %0.1f Kb\t'%s'\tJPEG foreground, %d dpi.\n", 
                      (chksize+512)/1024.0, (const char*)chkid, basedpi/red);
#else
          GString desc;
          desc.format(" %0.1f Kb\t'%s'\tJPEG foreground (OBSOLETE).\n", 
                      (chksize+512)/1024.0, (const char*)chkid);
#endif  
          dimg->add_description(desc);
          *message=desc;
          *do_redraw=1;
          *call_again=1;
        }
      //--- CHUNK "SJbz"
      else if (chkid == "Sjbz")
        {
          if (!dimg->width || !dimg->height)
            THROW("DjVu Decoder: Corrupted file (Missing INFO chunk)");
          if (dimg->jb2stencil)
            THROW("DjVu Decoder: Corrupted data (Duplicate Sxxx chunk)");
          // Decode chunk data
          GP<JB2Image> jimg = new JB2Image;
          jimg->decode(*iff);
          // Check stencil size
          if (jimg->get_width()!=dimg->width || jimg->get_height()!=dimg->height)
            THROW("DjVu Decoder: Corrupted data (JB2 Stencil size is incorrect)");
          // Update image
          dimg->jb2stencil = jimg;
          // Add description
          GString desc;
          int basedpi = dimg->get_dpi();
          if (! basedpi) basedpi = 300;
          desc.format(" %0.1f Kb\t'%s'\tJB2 stencil, %d dpi.\n", 
                      (chksize+512)/1024.0, (const char*)chkid, basedpi);
          dimg->add_description(desc);
          *message=desc;
          *do_redraw=1;
          *call_again=1;
        }
      //--- CHUNK "DIjp"
      else if (chkid == "DIjp")
        {
          GString desc;
          desc.format(" %0.1f Kb\t'%s'\tJPEG delta (OBSOLETE).\n", 
                      (chksize+512)/1024.0, (const char*)chkid);
          dimg->add_description(desc);
          *message=desc;
          *do_redraw=1;
          *call_again=1;
        }
      //--- UNKNOWN CHUNK
      else
        {
          // Add description
          GString desc;
          desc.format(" %0.1f Kb\t'%s'\tUnknown chunk.\n", 
                      (chksize+512)/1024.0, (const char*)chkid);
          dimg->add_description(desc);
          *message=desc;
          *do_redraw=0;
          *call_again=1;
        }
      iff->close_chunk();
      return;
    }
  // Close codec in bg44
  if (dimg->bg44)
    dimg->bg44->close_codec();
  // Add info about compression ratio
  if (dimg && dimg->width && dimg->height)
    {
      GString desc = "\nCompression ratio :\t";
      dimg->filesize = iff->tell();
      int rawsize = 3*dimg->width*dimg->height;
      if (dimg->deltasize)
        {
          desc.format("\nCompression (without delta) :\t%0.1f (%0.1f Kb)\n", 
                      (float) rawsize / (dimg->filesize - dimg->deltasize),
                      (dimg->filesize - dimg->deltasize + 512) / 1024.0 );
          dimg->add_description(desc);
          desc.format("Compression (with delta) :   \t%0.1f (%0.1f Kb)\n", 
                      (float) rawsize / (dimg->filesize),
                      (dimg->filesize + 512) / 1024.0 );
          dimg->add_description(desc);
        }
      else
        {
          desc.format("\nCompression ratio : %0.1f (%0.1f Kb)\n", 
                      (float) rawsize / dimg->filesize, (dimg->filesize + 512)/1024.0 );
          dimg->add_description(desc);
        }
    }
  *chunk_name="";
  *message="";
  *do_redraw=0;
  *call_again=0;
}

