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
//C- $Id: JPEGDecoder.cpp,v 1.4 1999-11-08 04:45:44 praveen Exp $


#include "JPEGDecoder.h"
#ifdef NEED_JPEG_DECODER

extern "C" {

struct djvu_error_mgr {
  struct jpeg_error_mgr pub;  /* "public" fields */

  jmp_buf setjmp_buffer;  /* for return to caller */
};

typedef struct djvu_error_mgr * djvu_error_ptr;

void jpeg_byte_stream_src(j_decompress_ptr, ByteStream *);

METHODDEF() void
djvu_error_exit (j_common_ptr cinfo)
{
  /* cinfo->err really points to a djvu_error_mgr struct, so coerce pointer */
  djvu_error_ptr djvuerr = (djvu_error_ptr) cinfo->err;

  /* Always display the message. */
  /* We could postpone this until after returning, if we chose. */
  (*cinfo->err->output_message) (cinfo);

  /* Return control to the setjmp point */
  longjmp(djvuerr->setjmp_buffer, 1);
}

}

GP<GPixmap>
JPEGDecoder::decode(ByteStream & bs )
{
  struct jpeg_decompress_struct cinfo;

  /* We use our private extension JPEG error handler. */
  struct djvu_error_mgr jerr;

  JSAMPARRAY buffer;    /* Output row buffer */
  int row_stride;   /* physical row width in output buffer */
  char tempBuf[50];
	int full_buf_size;
	int isGrey,i;

  cinfo.err = jpeg_std_error(&jerr.pub);

  jerr.pub.error_exit = djvu_error_exit;

  if (setjmp(jerr.setjmp_buffer)) {

    jpeg_destroy_decompress(&cinfo);
    return 0;
  }

  jpeg_create_decompress(&cinfo);

  jpeg_byte_stream_src(&cinfo, &bs);

  (void) jpeg_read_header(&cinfo, TRUE);

  jpeg_start_decompress(&cinfo);
  
  /* We may need to do some setup of our own at this point before reading
   * the data.  After jpeg_start_decompress() we have the correct scaled
   * output image dimensions available, as well as the output colormap
   * if we asked for color quantization.
   * In this example, we need to make an output work buffer of the right size.
   */

  /* JSAMPLEs per row in output buffer */
  row_stride = cinfo.output_width * cinfo.output_components;
	full_buf_size = row_stride * cinfo.output_height;

  /* Make a one-row-high sample array that will go away when done with image */
  buffer = (*cinfo.mem->alloc_sarray)
    ((j_common_ptr) &cinfo, JPOOL_IMAGE, row_stride, 1);

	sprintf(tempBuf,"P6\n%d %d\n%d\n",cinfo.output_width, 
		                             cinfo.output_height,255);
	MemoryByteStream outputBlock;
	outputBlock.write((char *)tempBuf,strlen(tempBuf));

	isGrey = ( cinfo.out_color_space == JCS_GRAYSCALE) ? 1 : 0; 

  while (cinfo.output_scanline < cinfo.output_height) {
    (void) jpeg_read_scanlines(&cinfo, buffer, 1);

			if ( isGrey == 1 ){
				for (i=0; i<row_stride; i++) {
					outputBlock.write8((char)buffer[0][i]); 
					outputBlock.write8((char)buffer[0][i]); 
					outputBlock.write8((char)buffer[0][i]); 
				}
			}else {
				for (i=0; i<row_stride; i++) 
					outputBlock.write8((char)buffer[0][i]); 
			}
  }

  (void) jpeg_finish_decompress(&cinfo);   

  jpeg_destroy_decompress(&cinfo);
	
	outputBlock.seek(0,SEEK_SET);
	GP<GPixmap> gp = new GPixmap(outputBlock);

  return gp; 
}         

/*** From here onwards code is to make ByteStream as the data
     source for the JPEG library */

extern "C" {

typedef struct {
  struct jpeg_source_mgr pub; /* public fields */

  ByteStream * byteStream;    /* source stream */
  JOCTET * buffer;    /* start of buffer */
  boolean start_of_stream;  
} byte_stream_src_mgr;
                

typedef byte_stream_src_mgr * byte_stream_src_ptr; 

#define INPUT_BUF_SIZE   4096

METHODDEF() void
init_source (j_decompress_ptr cinfo)
{
  byte_stream_src_ptr src = (byte_stream_src_ptr) cinfo->src;

  src->start_of_stream = TRUE;
}

METHODDEF() boolean
fill_input_buffer (j_decompress_ptr cinfo)
{
  byte_stream_src_ptr src = (byte_stream_src_ptr) cinfo->src;
  size_t nbytes;

  nbytes = src->byteStream->readall(src->buffer, INPUT_BUF_SIZE);

  if (nbytes <= 0) {
    if (src->start_of_stream) /* Treat empty input as fatal error */
      ERREXIT(cinfo, JERR_INPUT_EMPTY);
    WARNMS(cinfo, JWRN_JPEG_EOF);
    /* Insert a fake EOI marker */
    src->buffer[0] = (JOCTET) 0xFF;
    src->buffer[1] = (JOCTET) JPEG_EOI;
    nbytes = 2;
  }

  src->pub.next_input_byte = src->buffer;
  src->pub.bytes_in_buffer = nbytes;
  src->start_of_stream = FALSE; 

  return TRUE;
}


METHODDEF() void
skip_input_data (j_decompress_ptr cinfo, long num_bytes)
{
  byte_stream_src_ptr src = (byte_stream_src_ptr) cinfo->src;

  if (num_bytes > (long) src->pub.bytes_in_buffer) {

  	src->byteStream->seek((num_bytes - src->pub.bytes_in_buffer), SEEK_CUR);
    (void) fill_input_buffer(cinfo);
  }else{
		src->pub.bytes_in_buffer -= num_bytes;
  	src->pub.next_input_byte += num_bytes;
	}
}
                 
METHODDEF() void
term_source (j_decompress_ptr cinfo)
{
  /* no work necessary here */
}
    
GLOBAL() void
jpeg_byte_stream_src (j_decompress_ptr cinfo, ByteStream * bs)
{
  byte_stream_src_ptr src;

  if (cinfo->src == NULL) { /* first time for this JPEG object? */
    cinfo->src = (struct jpeg_source_mgr *)      
      (*cinfo->mem->alloc_small) ((j_common_ptr) cinfo, JPOOL_PERMANENT,
          SIZEOF(byte_stream_src_mgr));
    src = (byte_stream_src_ptr) cinfo->src;
    src->buffer = (JOCTET *)
      (*cinfo->mem->alloc_small) ((j_common_ptr) cinfo, JPOOL_PERMANENT,
          INPUT_BUF_SIZE * SIZEOF(JOCTET));
  }

  src = (byte_stream_src_ptr) cinfo->src;
  src->pub.init_source = init_source;
  src->pub.fill_input_buffer = fill_input_buffer;
  src->pub.skip_input_data = skip_input_data;
  src->pub.resync_to_restart = jpeg_resync_to_restart; /* use default method */
  src->pub.term_source = term_source;
  src->byteStream = bs;
  src->pub.bytes_in_buffer = 0; /* forces fill_input_buffer on first read */
  src->pub.next_input_byte = NULL; /* until buffer loaded */
}                                    

}


#endif
