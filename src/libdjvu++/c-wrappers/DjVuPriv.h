/*C-  -*- C -*-
 *C-
 *C- Copyright (c) 2000, LizardTech, Inc.  All Rights Reserved.
 *C-
 *C- $Id: DjVuPriv.h,v 1.11 2000-09-18 17:10:36 bcr Exp $
 */

#ifndef _DJVU_PRIV_H_
#define _DJVU_PRIV_H_ true
#include "DjVuGlobal.h"
#include "DjVuAPI.h"
#include <stdlib.h> 
#include <stdio.h>

static inline void _djvu_image_free(djvu_image &img);

/* This is an strictly inline version of a priv struct for djvu_image's,
 * so we don't have library dependencies...
 */
struct _djvu_image_priv 
{
  djvu_image *img;
  bool isMalloc;

  _djvu_image_priv(djvu_image &ximg,bool xisMalloc=true)
  : img(&ximg),isMalloc(xisMalloc)
  {
    ximg.priv=this;
    ximg.data=ximg.start_alloc=(unsigned char *)
      (ximg.start_alloc?ximg.start_alloc:(isMalloc?
        _djvu_malloc(ximg.datasize):new unsigned char [ximg.datasize]));
  }

  ~_djvu_image_priv()
  {
    if(img->priv == this)
    {
      if(isMalloc&&img->start_alloc)
        _djvu_free(img->start_alloc);
      else
        delete [] img->start_alloc;
      img->data=img->start_alloc=0;
      img->priv=0;
    }
  }

  inline void *
  realloc(size_t sz)
  {
    unsigned char *data=0,*olddata=img->start_alloc;
    const size_t s=((img->datasize)>sz)?(sz):(img->datasize);
    if(isMalloc)
    {
      data=(unsigned char *)::_djvu_realloc(olddata,sz);
    }else if(((s+s)>=sz)&&(data=new unsigned char [sz?sz:1])&&olddata)
    {
      for(size_t i=0;i<s;i++)
        data[i]=olddata[i];
      delete [] olddata;
    }
    if(data)
    {
      size_t offset=(size_t)(img->data)-(size_t)(img->start_alloc);
      img->data=(img->start_alloc=(unsigned char *)data)+offset;
      img->datasize=sz;
    }
    return data?data:((sz!=s)?olddata:0);
  }

  djvu_image *moveto(djvu_image *ximg) 
  {
    _djvu_image_free(*ximg);
    *ximg=*img;
    img->priv=0;
    _djvu_image_free(*img);
    djvu_image *x=img;
    img=ximg;
    return x;
  }

  static inline int
  GetRowTDLRNR(const djvu_image &img,const int row,const unsigned char *&startptr,const unsigned char *&stopptr)
  {
    stopptr=(startptr=img.data+img.rowsize*row)+img.pixsize*img.w;
    return (int)(img.pixsize);
  }
  static inline int
  GetRowTDLRNR(djvu_image &img,const int row,unsigned char *&startptr,unsigned char *&stopptr)
  {
    stopptr=(startptr=img.data+img.rowsize*row)+img.pixsize*img.w;
    return (int)(img.pixsize);
  }

  static inline int
  GetRowBULRNR(const djvu_image &img,const int row,const unsigned char *&startptr,const unsigned char *&stopptr)
  {
    stopptr=(startptr=img.data+img.rowsize*(img.h-row-1))+img.pixsize*img.w;
    return (int)(img.pixsize);
  }
  static inline int
  GetRowBULRNR(djvu_image &img,const int row,unsigned char *&startptr,unsigned char *&stopptr)
  {
    stopptr=(startptr=img.data+img.rowsize*(img.h-row-1))+img.pixsize*img.w;
    return (int)(img.pixsize);
  }

  static inline int
  GetRowTDRLNR(const djvu_image &img,const int row,const unsigned char *&startptr,const unsigned char *&stopptr)
  {
    startptr=(stopptr=img.data+img.rowsize*row-img.pixsize)+img.pixsize*img.w;
    return -(int)(img.pixsize);
  }
  static inline int
  GetRowTDRLNR(djvu_image &img,const int row,unsigned char *&startptr,unsigned char *&stopptr)
  {
    startptr=(stopptr=img.data+img.rowsize*row-img.pixsize)+img.pixsize*img.w;
    return -(int)(img.pixsize);
  }

  static inline int
  GetRowBURLNR(const djvu_image &img,const int row,const unsigned char *&startptr,const unsigned char *&stopptr)
  {
    startptr=(stopptr=img.data+img.rowsize*(img.h-row-1)-img.pixsize)+img.pixsize*img.w;
    return -(int)(img.pixsize);
  }
  static inline int
  GetRowBURLNR(djvu_image &img,const int row,unsigned char *&startptr,unsigned char *&stopptr)
  {
    startptr=(stopptr=img.data+img.rowsize*(img.h-row-1)-img.pixsize)+img.pixsize*img.w;
    return -(int)(img.pixsize);
  }

  static inline int
  GetRowTDLRCW(const djvu_image &img,const int row,const unsigned char *&startptr,const unsigned char *&stopptr)
  {
    startptr=(stopptr=img.data+img.pixsize*row-img.rowsize)+img.rowsize*img.h;
    return -(int)(img.rowsize);
  }
  static inline int
  GetRowTDLRCW(djvu_image &img,const int row,unsigned char *&startptr,unsigned char *&stopptr)
  {
    startptr=(stopptr=img.data+img.pixsize*row-img.rowsize)+img.rowsize*img.h;
    return -(int)(img.rowsize);
  }

  static inline int
  GetRowBULRCW(const djvu_image &img,const int row,const unsigned char *&startptr,const unsigned char *&stopptr)
  {
    stopptr=(startptr=img.data+img.pixsize*row)+img.rowsize*img.h;
    return (int)(img.rowsize);
  }
  static inline int
  GetRowBULRCW(djvu_image &img,const int row,unsigned char *&startptr,unsigned char *&stopptr)
  {
    stopptr=(startptr=img.data+img.pixsize*row)+img.rowsize*img.h;
    return (int)(img.rowsize);
  }

  static inline int
  GetRowTDRLCW(const djvu_image &img,const int row,const unsigned char *&startptr,const unsigned char *&stopptr)
  {
    startptr=(stopptr=img.data+img.pixsize*(img.w-row-1)-img.rowsize)+img.rowsize*img.h;
    return -(int)(img.rowsize);
  }
  static inline int
  GetRowTDRLCW(djvu_image &img,const int row,unsigned char *&startptr,unsigned char *&stopptr)
  {
    startptr=(stopptr=img.data+img.pixsize*(img.w-row-1)-img.rowsize)+img.rowsize*img.h;
    return -(int)(img.rowsize);
  }

  static inline int
  GetRowBURLCW(const djvu_image &img,const int row,const unsigned char *&startptr,const unsigned char *&stopptr)
  {
    stopptr=(startptr=img.data+img.pixsize*(img.w-row-1))+img.rowsize*img.h;
    return (int)(img.rowsize);
  }
  static inline int
  GetRowBURLCW(djvu_image &img,const int row,unsigned char *&startptr,unsigned char *&stopptr)
  {
    stopptr=(startptr=img.data+img.pixsize*(img.w-row-1))+img.rowsize*img.h;
    return (int)(img.rowsize);
  }
};

/* This can safely be used on user allocated images...
 */
static inline void
_djvu_image_free(djvu_image &img)
{
  if(img.priv)
    delete img.priv;
  img.type=DJVU_RLE;
  img.orientation=0;
  img.w=img.h=0;
  img.pixsize=img.rowsize=img.datasize=0;
}

/* This is dangerous to use with user allocated images...
 */
static inline void
_djvu_image_free(djvu_image *img)
{
  if(img)
  {
    _djvu_image_free(*img);
    delete img;
  }
}

#endif /* _DJVU_PRIV_H_ */

