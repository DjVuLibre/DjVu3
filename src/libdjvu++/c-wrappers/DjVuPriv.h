#include "DjVuGlobal.h"
#include "DjVuAPI.h"
#include <stdlib.h> 

/* This is an strictly inline version of a priv struct for djvu_image's,
 * so we don't have library dependancies...
 */
struct _djvu_image_priv 
{
  djvu_image &img;
  bool isMalloc;

  _djvu_image_priv(djvu_image &ximg,bool xisMalloc=true)
  : img(ximg),isMalloc(xisMalloc)
  {
    img.priv=this;
    img.data=img.start_alloc=(unsigned char *)
      (img.start_alloc?img.start_alloc:(isMalloc?
        _djvu_malloc(img.datasize):new unsigned char [img.datasize]));
  }

  ~_djvu_image_priv()
  {
    if(isMalloc&&img.start_alloc)
      _djvu_free(img.start_alloc);
    else
      delete [] img.start_alloc;
    img.data=img.start_alloc=0;
    img.priv=0;
  }

  inline void *
  realloc(size_t sz)
  {
    void *data=0;
    if((isMalloc||!img.start_alloc)&&(data=::_djvu_realloc(img.start_alloc,sz)))
    {
      size_t offset=(size_t)img.data-(size_t)img.start_alloc;
      img.data=(img.start_alloc=(unsigned char *)data)+offset;
      img.datasize=sz;
    }
    return data;
  }

  static inline int
  GetRowTDLRNR(const djvu_image &img,const int row,const unsigned char *&startptr,const unsigned char *&stopptr)
  {
    stopptr=(startptr=img.data+img.rowsize*row)+img.pixsize*img.w;
    return (int)(img.pixsize);
  }
  static inline int
  GetRowTDRLNR(const djvu_image &img,const int row,const unsigned char *&startptr,const unsigned char *&stopptr)
  {
    startptr=(stopptr=img.data+img.rowsize*row-img.pixsize)+img.pixsize*img.w;
    return -(int)(img.pixsize);
  }
  static inline int
  GetRowBULRNR(const djvu_image &img,const int row,const unsigned char *&startptr,const unsigned char *&stopptr)
  {
    stopptr=(startptr=img.data+img.rowsize*(img.h-row-1))+img.pixsize*img.w;
    return (int)(img.pixsize);
  }
  static inline int
  GetRowBURLNR(const djvu_image &img,const int row,const unsigned char *&startptr,const unsigned char *&stopptr)
  {
    startptr=(stopptr=img.data+img.rowsize*(img.h-row-1)-img.pixsize)+img.pixsize*img.w;
    return -(int)(img.pixsize);
  }
  static inline int
  GetRowTDLRCW(const djvu_image &img,const int row,const unsigned char *&startptr,const unsigned char *&stopptr)
  {
    stopptr=(startptr=img.data+img.pixsize*(img.w-row-1))+img.rowsize*img.h;
    return (int)(img.rowsize);
  }
  static inline int
  GetRowTDRLCW(const djvu_image &img,const int row,const unsigned char *&startptr,const unsigned char *&stopptr)
  {
    startptr=(stopptr=img.data+img.pixsize*(img.w-row-1)-img.rowsize)+img.rowsize*img.h;
    return -(int)(img.rowsize);
  }
  static inline int
  GetRowBULRCW(const djvu_image &img,const int row,const unsigned char *&startptr,const unsigned char *&stopptr)
  {
    startptr=(stopptr=img.data+img.pixsize*row-img.rowsize)+img.rowsize*img.h;
    return -(int)(img.rowsize);
  }
  static inline int
  GetRowBURLCW(const djvu_image &img,const int row,const unsigned char *&startptr,const unsigned char *&stopptr)
  {
    stopptr=(startptr=img.data+img.pixsize*row)+img.rowsize*img.h;
    return (int)(img.rowsize);
  }
  static inline int
  GetRowTDLRNR(djvu_image &img,const int row,unsigned char *&startptr,unsigned char *&stopptr)
  {
    stopptr=(startptr=img.data+img.rowsize*row)+img.pixsize*img.w;
    return (int)(img.pixsize);
  }
  static inline int
  GetRowTDRLNR(djvu_image &img,const int row,unsigned char *&startptr,unsigned char *&stopptr)
  {
    startptr=(stopptr=img.data+img.rowsize*row-img.pixsize)+img.pixsize*img.w;
    return -(int)(img.pixsize);
  }
  static inline int
  GetRowBULRNR(djvu_image &img,const int row,unsigned char *&startptr,unsigned char *&stopptr)
  {
    stopptr=(startptr=img.data+img.rowsize*(img.h-row-1))+img.pixsize*img.w;
    return (int)(img.pixsize);
  }
  static inline int
  GetRowBURLNR(djvu_image &img,const int row,unsigned char *&startptr,unsigned char *&stopptr)
  {
    startptr=(stopptr=img.data+img.rowsize*(img.h-row-1)-img.pixsize)+img.pixsize*img.w;
    return -(int)(img.pixsize);
  }
  static inline int
  GetRowTDLRCW(djvu_image &img,const int row,unsigned char *&startptr,unsigned char *&stopptr)
  {
    stopptr=(startptr=img.data+img.pixsize*(img.w-row-1))+img.rowsize*img.h;
    return (int)(img.rowsize);
  }
  static inline int
  GetRowTDRLCW(djvu_image &img,const int row,unsigned char *&startptr,unsigned char *&stopptr)
  {
    startptr=(stopptr=img.data+img.pixsize*(img.w-row-1)-img.rowsize)+img.rowsize*img.h;
    return -(int)(img.rowsize);
  }
  static inline int
  GetRowBULRCW(djvu_image &img,const int row,unsigned char *&startptr,unsigned char *&stopptr)
  {
    startptr=(stopptr=img.data+img.pixsize*row-img.rowsize)+img.rowsize*img.h;
    return -(int)(img.rowsize);
  }
  static inline int
  GetRowBURLCW(djvu_image &img,const int row,unsigned char *&startptr,unsigned char *&stopptr)
  {
    stopptr=(startptr=img.data+img.pixsize*row)+img.rowsize*img.h;
    return (int)(img.rowsize);
  }
};

/* This can safely be used on user allocated images...
 */
static inline void
_djvu_image_free(djvu_image &img)
{
  if(img.priv)
  {
    delete img.priv;
  }
  img.flags=(djvu_flags)0;
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
    if(img->priv)
      delete img;
  }
}


