//C-  -*- C++ -*-
//C-
//C-  Copyright (c) 1998 AT&T	
//C-  All Rights Reserved 
//C-
//C-  THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
//C-  The copyright notice above does not evidence any
//C-  actual or intended publication of such source code.
//C-
//C-  $Id: IWImage.h,v 1.3 1999-02-11 14:33:11 leonb Exp $

#ifndef _IWIMAGE_H_
#define _IWIMAGE_H_


/** @name IWImage.h

    Files #"IWImage.h"# and #"IWImage.cpp"# implement the DjVu IW44 wavelet
    scheme for the compression of gray-level images (see class \Ref{IWBitmap})
    and color images (see class \Ref{IWPixmap}).  Programs \Ref{c44} and
    \Ref{d44} demonstrate how to encode and decode IW44 files.

    {\bf IW44 File Structure} --- The IW44 files are structured according to the
    EA IFF85 specifications (see \Ref{IFFByteStream.h}).  Gray level images
    consist of a single #"FORM:BM44"# chunk composed of an arbitrary number of
    #"BM44"# data chunks.  Color images consist of a single #"FORM:PM44"#
    chunk composed of an arbitrary number of #"PM44"# data chunks.  The
    successive #"PM44"# or #"BM44"# data chunks contain successive refinements
    of the encoded image.  Each chunk contains a certain number of "data
    slices".  The first chunk also contains a small image header.  You can use
    program \Ref{djvuinfo} to display all this structural information:
    \begin{verbatim}
    % djvuinfo lag.iw4
    lag.iw4:
      FORM:PM44 [62598] 
        PM44 [10807]              #1 - 74 slices - v1.2 (color) - 684x510
        PM44 [23583]              #2 - 13 slices 
        PM44 [28178]              #3 - 10 slices 
    \end{verbatim}

    {\bf Embedded IW44 Images} --- These IW44 data chunks can also appear within
    other contexts.  Files representing a DjVu page, for instance, consist of
    a single #"FORM:DJVU"# composite chunk.  This composite chunk may contain
    #"BG44"# chunks encoding the background layer and #"FG44"# chunks encoding
    the foreground color layer.  These #"BG44"# and #"FG44"# chunks are
    actually regular IW44 data chunks with a different chunk identifier.  This
    information too can be displayed using program \Ref{djvuinfo}.
    \begin{verbatim}
    % djvuinfo graham1.djvu 
    graham1.djvu:
      FORM:DJVU [32553] 
        INFO [5]            3156x2325, version 17
        Sjbz [17692] 
        BG44 [2570]         #1 - 74 slices - v1.2 (color) - 1052x775
        FG44 [1035]         #1 - 100 slices - v1.2 (color) - 263x194
        BG44 [3048]         #2 - 10 slices 
        BG44 [894]          #3 - 4 slices 
        BG44 [7247]         #4 - 9 slices 
    \end{verbatim}

    {\bf Performance} --- The main design objective for the DjVu wavelets
    consisted of allowing progressive rendering and smooth scrolling of large
    images with limited memory requirements.  Decoding functions process the
    compressed data and update a memory efficient representation of the
    wavelet coefficients.  Imaging function then can quickly render an
    arbitrary segment of the image using the available data.  Both process can
    be carried out in two threads of execution.  This design plays an
    important role in the DjVu system.  We have investigated various
    state-of-the-art wavelet compression schemes: although these schemes may
    achieve slightly smaller file sizes, the decoding functions did not even
    approach our requirements.  The IW44 wavelets reach these requirements
    today.  Little care however has been taken to make the IW44 encoder very
    lean.  This code uses two copies of the wavelet coefficient data structure
    (one for the raw coefficients, one for the quantized coefficients).  A
    more sophisticated implementation should considerably reduce the memory
    requirements.  Such an improvement could be a premature optimization
    however.

    {\bf Masking} --- When we create a DjVu image, we often know that certain
    pixels of the background image are going to be covered by foreground
    objects like text or drawings.  The DjVu IW44 wavelet decomposition
    routine can use an optional bilevel image named the mask.  Every non zero
    pixel in the mask means the value of the corresponding pixel in the
    background image is irrelevant.  The wavelet decomposition code will
    replace these masked pixels by a color value whose coding cost is minimal
    (see \URL{http://www.research.att.com/~leonb/DJVU/mask}).

    {\bf ToDo} --- There are many improvements to be made.  Besides modern
    quantization algorithms (such as treillis quantization and bitrate
    allocation), we should allow for more wavelet transform.  These
    improvements may be implemented in future version, if (and only if) they
    can meet our decoding constraints.  Future versions will probably split
    file #"IWCodec.cpp"# which currently contains everything
 
    @memo
    Wavelet encoded images.
    @author
    Leon Bottou <leonb@research.att.com>
    @version
    #$Id: IWImage.h,v 1.3 1999-02-11 14:33:11 leonb Exp $# */
//@{

#ifdef __GNUC__
#pragma interface
#endif

#include "DjVuGlobal.h"
#include "GRect.h"
#include "GSmartPointer.h"
#include "GBitmap.h"
#include "GPixmap.h"
#include "ByteStream.h"
#include "IFFByteStream.h"


class _IWMap;
class _IWBlock;
class _IWCodec;



/** IW44 encoding parameters.  
    This data structure gathers the quality specification parameters needed
    for encoding each chunk of an IW44 file.  Chunk data is generated until
    meeting either the slice target, the size target or the decibel target.  */

struct IWEncoderParms 
{
  /** Slice target.  Data generation for the current chunk stops if the total
      number of slices (in this chunk and all the previous chunks) reaches
      value #slice#.  The default value #0# has a special meaning: data will
      be generated regardless of the number of slices in the file. */
  int    slices;
  /** Size target.  Data generation for the current chunk stops if the total
      data size (in this chunk and all the previous chunks), expressed in
      bytes, reaches value #size#.  The default value #0# has a special
      meaning: data will be generated regardless of the file size. */
  int    bytes;
  /** Decibel target.  Data generation for the current chunk stops if the
      estimated luminance error, expressed in decibels, reaches value
      #decibel#.  The default value #0# has a special meaning: data will be
      generated regardless of the estimated luminance error.  Specifying value
      #0# in fact shortcuts the luminance error estimation and sensibly speeds
      up the encoding process.  */
  float  decibels;
  /** Constructor. Initializes the structure with the default values. */
  IWEncoderParms();
};



/** IW44 encoded gray-level image.  This class provided functions for managing
    a gray level image represented as a collection of IW44 wavelet
    coefficients.  The coefficients are stored in a memory efficient data
    structure.  Member function \Ref{get_bitmap} renders an arbitrary segment
    of the image into a \Ref{GBitmap}.  Member functions \Ref{decode_iff} and
    \Ref{encode_iff} read and write DjVu IW44 files (see \Ref{IWImage.h}).
    Both the copy constructor and the copy operator are declared as private
    members. It is therefore not possible to make multiple copies of instances
    of this class. */

class IWBitmap : public GPEnabled
{
public:
  /** Null constructor.  Constructs an empty IWBitmap object. This object does
      not contain anything meaningful. You must call function \Ref{init},
      \Ref{decode_iff} or \Ref{decode_chunk} to populate the wavelet
      coefficient data structure. */
  IWBitmap();
  /** Initializes an IWBitmap with image #bm#.  This constructor
      performs the wavelet decomposition of image #bm# and records the
      corresponding wavelet coefficient.  Argument #mask# is an optional
      bilevel image specifying the masked pixels (see \Ref{IWImage.h}). */
  void init(const GBitmap *bm, const GBitmap *mask=0);
  /** Convenience constructor. This constructors creates an empty IWBitmap
      and then calls function \Ref{init} above. */
  IWBitmap(const GBitmap *bm, const GBitmap *mask=0);
  // Virtual destructor
  ~IWBitmap();
  // ACCESS
  /** Returns the width of the IWBitmap image. */
  int get_width() const;
  /** Returns the height of the IWBitmap image. */
  int get_height() const;
  /** Reconstructs the complete image.  The reconstructed image
      is then returned as a GBitmap object. */
  GP<GBitmap> get_bitmap();
  /** Reconstructs a segment of the image at a given scale.  The subsampling
      ratio #subsample# must be a power of two between #1# and #32#.  Argument
      #rect# specifies which segment of the subsampled image should be
      reconstructed.  The reconstructed image is returned as a GBitmap object
      whose size is equal to the size of the rectangle #rect#. */
  GP<GBitmap> get_bitmap(int subsample, const GRect &rect);
  /** Returns the amount of memory used by the wavelet coefficients.  This
      amount of memory is expressed in bytes. */
  unsigned int get_memory_usage() const;
  /** Returns the filling ratio of the internal data structure.  Wavelet
      coefficients are stored in a sparse array.  This function tells what
      percentage of bins have been effectively alocated. */
  int get_percent_memory() const;
  // CODER
  /** Encodes one data chunk into ByteStream #bs#.  Parameter #parms# controls
      how much data is generated.  The chunk data is written to ByteStream
      #bs# with no IFF header.  Successive calls to #encode_chunk# encode
      successive chunks.  You must call #close_codec# after encoding the last
      chunk of a file. */
  int  encode_chunk(ByteStream &bs, const IWEncoderParms &parms);
  /** Writes a gray level image into DjVu IW44 file.  This function creates a
      composite chunk (identifier #FORM:BM44#) composed of #nchunks# chunks
      (identifier #BM44#).  Data for each chunk is generated with
      #encode_chunk# using the corresponding parameters in array #parms#. */
  void encode_iff(IFFByteStream &iff, int nchunks, const IWEncoderParms *parms);
  // DECODER
  /** Decodes one data chunk from ByteStream #bs#.  Successive calls to
      #decode_chunk# decode successive chunks.  You must call #close_codec#
      after decoding the last chunk of a file.  Note that function
      #get_bitmap# and #decode_chunk# may be called simultaneously from two
      execution threads. */
  int  decode_chunk(ByteStream &bs);
  /** Reads a DjVu IW44 file as a gray level image.  This function enters a
      composite chunk (identifier #FORM:BM44#), and decodes a maximum of
      #maxchunks# data chunks (identifier #BM44#).  Data for each chunk is
      processed using the function #decode_chunk#. */
  void decode_iff(IFFByteStream &iff, int maxchunks=999);
  // MISCELLANEOUS
  /** Resets the encoder/decoder state.  The first call to #decode_chunk# or
      #encode_chunk# initializes the coder for encoding or decoding.  Function
      #close_coder# must be called after processing the last chunk in order to
      reset the coder and release the associated memory. */
  void close_codec();  
  /** Returns the chunk serial number.  This function returns the serial
      number of the last chunk encoded with #encode_chunk# or decoded with
      #decode_chunk#. The first chunk always has serial number #1#. Successive
      chunks have increasing serial numbers.  Value #0# is returned is this
      function is called before calling #encode_chunk# or #decode_chunk# or
      after calling #close_codec#. */
  int get_serial();
  /** Sets the #dbfrac# parameter.  This function can be called before
      encoding the first IW44 data chunk.  Parameter #frac# modifies the
      decibel estimation algorithm in such a way that the decibel target only
      pertains to the average error of the fraction #frac# of the most
      misrepresented 32x32 pixel blocks.  Setting arguments #frac# to #1.0#
      restores the normal behavior.  */
  void parm_dbfrac(float frac);
private:
  // Parameter
  float db_frac;
  // Data
  _IWMap *ymap;
  _IWCodec *ycodec;
  int cslice;
  int cserial;
  int cbytes;
  // Disable assignment semantic
  IWBitmap(const IWBitmap &ref);
  IWBitmap& operator=(const IWBitmap &ref);
};



/** IW44 encoded color image. This class provided functions for managing a
    color image represented as a collection of IW44 wavelet coefficients.  The
    coefficients are stored in a memory efficient data structure.  Member
    function \Ref{get_pixmap} renders an arbitrary segment of the image into a
    \Ref{GPixmap}.  Member functions \Ref{decode_iff} and \Ref{encode_iff}
    read and write DjVu IW44 files (see \Ref{IWImage.h}).  Both the copy
    constructor and the copy operator are declared as private members. It is
    therefore not possible to make multiple copies of instances of this
    class. */

class IWPixmap : public GPEnabled
{
public:
  /** Null constructor.  Constructs an empty IWBitmap object. This object does
      not contain anything meaningful.  You must call function \Ref{init},
      \Ref{decode_iff} or \Ref{decode_chunk} to populate the wavelet
      coefficient data structure. */
  IWPixmap();
  /** Chrominance processing selector.  The following constants may be used as
      argument to the following \Ref{IWPixmap} constructor to indicate how the
      chrominance information should be processed. */
  enum CRCBMode { 
    /// Disables chrominance encoding. 
    CRCBnone, 
    /// Selects half resolution chrominance.
    CRCBhalf, 
    /// Selects full resolution chrominance.
    CRCBnormal, 
    /// Selects full resolution and zero encoding delay.
    CRCBfull };
  /** Initializes an IWPixmap with color image #bm#.  This constructor
      performs the wavelet decomposition of image #bm# and records the
      corresponding wavelet coefficient.  Argument #mask# is an optional
      bilevel image specifying the masked pixels (see \Ref{IWImage.h}).
      Argument #crcbmode# specifies how the chrominance information should be
      encoded (see \Ref{CRCBMode}). */
  void init(const GPixmap *bm, const GBitmap *mask=0, CRCBMode crcbmode=CRCBnormal);
  /** Convenience constructor. This constructors creates an empty IWBitmap
      and then calls function \Ref{init} above. */
  IWPixmap(const GPixmap *bm, const GBitmap *mask=0, CRCBMode crcbmode=CRCBnormal );
  // Virtual destructor.
  ~IWPixmap();
  // ACCESS
  /** Returns the width of the IWPixmap image. */
  int get_width() const;
  /** Returns the height of the IWPixmap image. */
  int get_height() const;
  /** Reconstructs the complete image.  The reconstructed image
      is then returned as a GPixmap object. */
  GP<GPixmap> get_pixmap();
  /** Reconstructs a segment of the image at a given scale.  The subsampling
      ratio #subsample# must be a power of two between #1# and #32#.  Argument
      #rect# specifies which segment of the subsampled image should be
      reconstructed.  The reconstructed image is returned as a GPixmap object
      whose size is equal to the size of the rectangle #rect#. */
  GP<GPixmap> get_pixmap(int subsample, const GRect &rect);
  /** Returns the amount of memory used by the wavelet coefficients.  This
      amount of memory is expressed in bytes. */
  unsigned int get_memory_usage() const;
  /** Returns the filling ratio of the internal data structure.  Wavelet
      coefficients are stored in a sparse array.  This function tells what
      percentage of bins have been effectively alocated. */
  int get_percent_memory() const;
  // CODER
  /** Encodes one data chunk into ByteStream #bs#.  Parameter #parms# controls
      how much data is generated.  The chunk data is written to ByteStream
      #bs# with no IFF header.  Successive calls to #encode_chunk# encode
      successive chunks.  You must call #close_codec# after encoding the last
      chunk of a file. */
  int  encode_chunk(ByteStream &bs, const IWEncoderParms &parms);
  /** Writes a color image into a DjVu IW44 file.  This function creates a
      composite chunk (identifier #FORM:PM44#) composed of #nchunks# chunks
      (identifier #PM44#).  Data for each chunk is generated with
      #encode_chunk# using the corresponding parameters in array #parms#. */
  void encode_iff(IFFByteStream &iff, int nchunks, const IWEncoderParms *parms);
  // DECODER
  /** Decodes one data chunk from ByteStream #bs#.  Successive calls to
      #decode_chunk# decode successive chunks.  You must call #close_codec#
      after decoding the last chunk of a file.  Note that function
      #get_bitmap# and #decode_chunk# may be called simultaneously from two
      execution threads. */
  int  decode_chunk(ByteStream &bs);
  /** Reads a DjVu IW44 file as a color image.  This function enters a
      composite chunk (identifier #FORM:PM44# or #FORM:BM44#), and decodes a
      maximum of #maxchunks# data chunks (identifier #PM44# or #BM44#).  Data
      for each chunk is processed using the function #decode_chunk#. */
  void decode_iff(IFFByteStream &iff, int maxchunks=999);
  // MISCELLANEOUS
  /** Resets the encoder/decoder state.  The first call to #decode_chunk# or
      #encode_chunk# initializes the coder for encoding or decoding.  Function
      #close_coder# must be called after processing the last chunk in order to
      reset the coder and release the associated memory. */
  void close_codec();  
  /** Returns the chunk serial number.  This function returns the serial
      number of the last chunk encoded with #encode_chunk# or decoded with
      #decode_chunk#. The first chunk always has serial number #1#. Successive
      chunks have increasing serial numbers.  Value #0# is returned is this
      function is called before calling #encode_chunk# or #decode_chunk# or
      after calling #close_codec#. */
  int  get_serial();
  /** Sets the chrominance delay parameter.  This function can be called
      before encoding the first IW44 data chunk.  Parameter #parm# is an
      encoding delay which reduces the bitrate associated with the
      chrominance information. The default chrominance encoding delay is 10. */
  int  parm_crcbdelay(int parm);
  /** Sets the #dbfrac# parameter.  This function can be called before
      encoding the first IW44 data chunk.  Parameter #frac# modifies the
      decibel estimation algorithm in such a way that the decibel target only
      pertains to the average error of the fraction #frac# of the most
      misrepresented 32x32 pixel blocks.  Setting arguments #frac# to #1.0#
      restores the normal behavior.  */
  void parm_dbfrac(float frac);
private:
  // Parameter
  int   crcb_delay;
  int   crcb_half;
  float db_frac;
  // Data
  _IWMap *ymap, *cbmap, *crmap;
  _IWCodec *ycodec, *cbcodec, *crcodec;
  int cslice;
  int cserial;
  int cbytes;
  // Disable assignment semantic
  IWPixmap(const IWPixmap &ref);
  IWPixmap& operator=(const IWPixmap &ref);
};




//@}


#endif
