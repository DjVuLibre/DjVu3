//C-  -*- C++ -*-
//C-
//C-  Copyright (c) 1998 AT&T	
//C-  All Rights Reserved 
//C-
//C-  THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
//C-  The copyright notice above does not evidence any
//C-  actual or intended publication of such source code.
//C-
//C-  $Id: ZPCodec.h,v 1.4 1999-02-15 23:34:51 leonb Exp $


#ifndef _ZPCODEC_H
#define _ZPCODEC_H

#include "DjVuGlobal.h"
#include "ByteStream.h"

#ifdef __GNUC__
#pragma interface
#endif


/** @name ZPCodec.h
    
    Files #"ZPCodec.h"# and #"ZPCodec.cpp"# implement a fast binary adaptive
    quasi-arithmetic coder named ZP-Coder.  Because of its speed and
    convenience, the ZP-Coder is used in several parts of the DjVu reference
    library (See \Ref{BSByteStream.h}, \Ref{JB2Image.h}, \Ref{IWImage.h}).
    The following comments avoid the theory (see the historical remarks for
    useful pointers) and concentrate on the user perspective on the ZP-Coder.

    {\bf Introduction} ---
    Encoding consists of transforming a sequence of {\em message bits} into a
    sequence of {\em code bits}. Decoding consists of retrieving the message
    bits using only the code bits.  We can make the code smaller than the
    message as soon as we can predict a message bit on the basis of a {\em
    coding context} composed of previously encoded or decoded bits. If the
    prediction is always correct, we do not even need to encode the message
    bit. If the prediction is totally unreliable, we need to generate one code
    bit in order to unambiguously specify the message bit.  In other words,
    the more reliable the prediction, the more compression we get.

    The ZP-Coder handles prediction by means of {\em context variables} (see
    \Ref{BitContext}).  There must be a context variable for each possible
    combination of context bits.  Both the encoder and the decoder use same
    context variable for coding each message bit.  For instance, we can code a
    binary image by successively coding all the pixels (the message bits) in
    row and column order.  It is reasonable to assume that each pixel can be
    reasonably well predicted by looking at a few (say 10) neighboring pixels
    located above and to the left of the current pixel.  Since these 10 pixels
    make 1024 combinations, we need 1024 context variables. Each pixel is
    encoded using the context variable corresponding to the values of the 10
    neighboring pixels.  Each pixel will be decoded by specifying the same
    context variable corresponding to the values of these 10 pixels. This is
    possible because these 10 pixels (located above and to the left) have
    already been decoded and therefore are known by the decoder program.

    The context variables are initially set to zero, which mean that we do not
    know yet how to predict the current message bit on the basis of the
    context bits. While coding the message bits, the ZP-Coder automatically
    estimates the frequencies of #0#s and #1#s coded using each context
    variable.  These frequencies actually provide a prediction (the most
    probable bit value) and an estimation of the prediction reliability (how
    often the prediction was correct in the past).  All this statistical
    information is stored into the context variable after coding each bit.  In
    other words, the more we code bits within a particular context, the better
    the ZP-Coder adapts its prediction model, and the more compression we can
    obtain.

    All this adaptation works indeed because both the encoder program and the
    decoder program are always synchronized. Both the encoder and the decoder
    see the same message bits encoded (or decoded) with the same context
    variables.  Both the encoder and the decoder apply the same rules to
    update the context variables and improve the predictors.  Both the encoder
    and the decoder programs use the same predictors for any given message
    bit.  The decoder could not work if this was not the case.
    
    Just before encoding a message bit, all the context variables in the
    encoder program contain certain values. Just before decoding this message
    bit, all the context variables in the decoder program must contain the same
    values as for the encoder program.  This is guaranteed as long as
    each prediction only depends on already coded bits: {\em the coding context,
    on which the each prediction is based, must be composed of message bits which
    have already been coded. }

    {\bf Practice} ---
    Once you know how to organize the predictions (i.e. which coding context
    to use, how many context variables to initialize, etc.), using the
    ZP-Coder is straightforward (see \Ref{ZPCodec Examples}):
    \begin{itemize}
    \item The {\em encoder program} allocates context variables and
    initializes them to zero. It then constructs a \Ref{ZPCodec} object for
    encoding. For each message bit, the encoder program retrieves the context
    bits, selects a context variable on the basis of the context bits and
    calls member function \Ref{ZPCodec::encoder} with the message bit and a
    reference to the context variable.
    \item The {\em decoder program} allocates context variables and
    initializes them to zero. It then constructs a \Ref{ZPCodec} object for
    decoding. For each message bit, the decoder program retrieves the context
    bits, selects a context variable on the basis of the context bits and
    calls member function \Ref{ZPCodec::decoder} with a reference to the
    context variable. This function returns the message bit.
    \end{itemize}
    Functions #encoder# and #decoder# only require a few machine cycles to
    perform two essential tasks, namely {\em coding} and {\em context
    adaptation}.  Function #decoder# often returns after two arithmetic
    operations only.  To make your program fast, you just need to feed message
    bits and context variables fast enough.

    {\bf History} ---
    The ZP-Coder is similar in function and performance to the Q-Coder
    (Pennebaker, Mitchell, Langdon, Arps, IBM J. Res Dev. 32, 1988). An
    improved version of the Q-Coder, named QM-Coder, has been described in
    certain parts of the JPEG standard.  Unfortunate patent policies have made
    these coders very difficult to use in general purpose applications.  The
    Z-Coder was the result of re-interpreting quasi-arithmetic coding as an
    extension of Golomb codes (Bottou, Howard, Bengio, IEEE DCC 98, 1998
    \URL[DjVu]{http://www.research.att.com/~leonb/DJVU/bottou-howard-bengio}
    \URL[PostScript]{http://www.research.att.com/~leonb/PS/bottou-howard-bengio.ps.gz})
    This new interpretation provides a way to escape the QM-Coder patents.
    Unfortunately the initial Z-Coder is sailing dangerously close to another
    patent (The "Arithmetic MEL Coder").  Therefore we wrote the ZP-Coder
    (pronounce Zee-Prime Coder) which we believe is clear of legal problems.
    
    Needless to say, AT&T has patents pending for both the Z-Coder and the
    ZP-Coder. The good news however is that we can grant a license to use the
    ZP-Coder in Open Source code without further complication. See the 
    \Ref{AT&T Source Code License} for more information.

    @memo
    Binary adaptive quasi-arithmetic coder.
    @version
    #$Id: ZPCodec.h,v 1.4 1999-02-15 23:34:51 leonb Exp $#
    @author
    Leon Bottou <leonb@research.att.com> */
//@{


/** Context variable.  
    Variables of type #BitContext# hold a single byte describing how to encode
    or decode message bits with similar statistical properties.  This single
    byte simultaneously represents the current estimate of the bit probability
    distribution (which is determined by the frequencies of #1#s and #0#s
    already coded with this context) and the confidence in this estimate
    (which determines how fast the estimate can change.)

    A coding program typically allocates hundreds of context variables.  Each
    coding context is initialized to zero before encoding or decoding.  Value
    zero represents equal probabilities for #1#s and #0#s with a minimal
    confidence and therefore a maximum adaptation speed.  Each message bit is
    encoded using a coding context determined as a function of previously
    encoded message bits.  The decoder therefore can examine the previously
    decoded message bits and decode the current bit using the same context as
    the encoder.  This is critical for proper decoding.  
*/
typedef unsigned char  BitContext;


/** Performs ZP-Coder encoding and decoding.  A ZPCodec object must either
    constructed for encoding or for decoding.  The ZPCodec object is connected
    with a \Ref{ByteStream} object specified at construction time.  A ZPCodec
    object constructed for decoding reads code bits from the ByteStream and
    returns a message bit whenever function \Ref{decoder} is called.  A
    ZPCodec constructed for encoding processes the message bits provided by
    function \Ref{encoder} and writes the corresponding code bits to
    ByteStream #bs#.

    You should never access directly a ByteStream object connected to a valid
    ZPCodec object. The most direct way to access the ByteStream object
    consists of using the "pass-thru" versions of functions \Ref{encoder} and
    \Ref{decoder}.

    The ByteStream object can be accessed again after the destruction of the
    ZPCodec object.  Note that the encoder always flushes its internal buffers
    and writes a few final code bytes when the ZPCodec object is destroyed.
    Note also that the decoder often reads a few bytes beyond the last code byte
    written by the encoder.  This lag means that you must reposition the
    ByteStream after the destruction of the ZPCodec object and before re-using
    the ByteStream object (see \Ref{IFFByteStream}.)

    Please note also that the decoder has no way to reliably indicate the end
    of the message bit sequence.  The content of the message must be designed
    in a way which indicates when to stop decoding.  Simple ways to achieve
    this consists of announcing the message length at the beginning (like a
    pascal style string), or of defining a termination code (like a zero
    terminated string).  */

class ZPCodec {
public:
  ~ZPCodec();
  /** Constructs a ZP-Coder.  If argument #encoding# is zero, the ZP-Coder
      object will read code bits from the ByteStream #bs# and return a message
      bit whenever function #decoder# is called.  If argument #encoding# is
      non zero, the ZP-Coder object will process the message bits provided by
      function #encoder# and write code bits to ByteStream #bs#. */
  ZPCodec (ByteStream &bs, int encoding=0);
  /** Encodes bit #bit# using context variable #ctx#.  Argument #bit# must be
      #0# or #1#. This function should only be used with ZP-Coder objects
      created for encoding. It may modify the contents of variable #ctx# in
      order to perform context adaptation. */
  void encoder(int bit, BitContext &ctx);
  /** Decodes a bit using context variable #ctx#. This function should only be
      used with ZP-Coder objects created for decoding. It may modify the
      contents of variable #ctx# in order to perform context adaptation. */
  int  decoder(BitContext &ctx);
  /** Encodes bit #bit# without compression (pass-thru encoder).  Argument
      #bit# must be #0# or #1#. No compression will be applied. Calling this
      function always increases the length of the code bit sequence by one
      bit. */
  void encoder(int bit);
  /** Decodes a bit without compression (pass-thru decoder).  This function
      retrieves bits encoded with the pass-thru encoder. */
  int  decoder();
#ifdef ZPCODEC_BITCOUNT
  /** Counter for code bits (requires #-DZPCODEC_BITCOUNT#). This member
      variable is available when the ZP-Coder is compiled with option
      #-DZPCODEC_BITCOUNT#.  Variable #bitcount# counts the number of code
      bits generated by the encoder since the construction of the object.
      This variable can be used to evaluate how many code bits are spent on
      various components of the message. */
  int bitcount;
#endif
  // Table management (advanced stuff)
  struct Table { 
    unsigned short p;
    unsigned short m;
    BitContext     up;
    BitContext     dn;
  };
  void newtable(ZPCodec::Table *table);
  BitContext state(float prob1);
  // Non-adaptive encoder/decoder
  void encoder_nolearn(int pix, BitContext &ctx);
  int  decoder_nolearn(BitContext &ctx);
protected:
  // coder status
  ByteStream *bs;               // Where the data goes/comes from
  char encoding;                // Direction (0=decoding, 1=encoding)
  unsigned char byte;
  unsigned char scount;
  unsigned char delay;
  unsigned int  a;
  unsigned int  code;
  unsigned int  fence;
  unsigned int  subend;
  unsigned int  buffer;
  unsigned int  nrun;
  // table
  unsigned int  p[256];
  unsigned int  m[256];
  BitContext    up[256];
  BitContext    dn[256];
  // machine independent ffz
  char          ffzt[256];
  // encoder private
  void einit (void);
  void eflush (void);
  void outbit(int bit);
  void emit(int b);
  void encode_mps(BitContext &ctx, unsigned int z);
  void encode_lps(BitContext &ctx, unsigned int z);
  void encode_mps_simple(unsigned int z);
  void encode_lps_simple(unsigned int z);
  void encode_mps_nolearn(unsigned int z);
  void encode_lps_nolearn(unsigned int z);
  // decoder private
  void dinit(void);
  void preload(void);
  int  ffz(unsigned int x);
  int  decode_sub(BitContext &ctx, unsigned int z);
  int  decode_sub_simple(int mps, unsigned int z);
  int  decode_sub_nolearn(int mps, unsigned int z);
private:
  // no copy allowed (hate c++)
  ZPCodec(const ZPCodec&);
  ZPCodec& operator=(const ZPCodec&);
};






// INLINE CODE

inline void 
ZPCodec::encoder(int bit, BitContext &ctx) 
{
  unsigned int z = a + p[ctx];
  if (bit != (ctx & 1))
    encode_lps(ctx, z);
  else if (z >= 0x8000)
    encode_mps(ctx, z);
  else
    a = z;
}

inline int
ZPCodec::decoder(BitContext &ctx) 
{
  unsigned int z = a + p[ctx];
  if (z <= fence) 
    { a = z; return (ctx&1); } 
  return decode_sub(ctx, z);
}

inline void 
ZPCodec::encoder_nolearn(int bit, BitContext &ctx) 
{
  unsigned int z = a + p[ctx];
  if (bit != (ctx & 1))
    encode_lps_nolearn(z);
  else if (z >= 0x8000)
    encode_mps_nolearn(z);
  else
    a = z;
}

inline int
ZPCodec::decoder_nolearn(BitContext &ctx) 
{
  unsigned int z = a + p[ctx];
  if (z <= fence) 
    { a = z; return (ctx&1); } 
  return decode_sub_nolearn( (ctx&1), z);
}

inline void 
ZPCodec::encoder(int bit)
{
  if (bit)
    encode_lps_simple(0x8000 + (a>>1));
  else
    encode_mps_simple(0x8000 + (a>>1));
}

inline int
ZPCodec::decoder()
{
  return decode_sub_simple(0, 0x8000 + (a>>1));
}

// ------------ THE END
#endif


// ------------ ADDITIONAL DOCUMENTATION

/** @name ZPCodec Examples
    
    Binary adaptive coders are efficient and very flexible.  Unfortunate
    intellectual property issues however have limited their popularity.  As a
    consequence, few programmers have a direct experience of using such a
    coding device.  The few examples provided in this section demonstrate how
    we think the ZP-Coder should be used.
    
    {\bf Encoding Multivalued Symbols} ---
    Since the ZP-Coder is a strictly binary coder, every message must be
    reduced to a sequence of bits (#0#s or #1#s).  It is often convenient to
    consider that a message is a sequence of symbols taking more than two
    values.  For instance, a character string may be a sequence of bytes, and
    each byte can take 256 values.  Each byte of course is composed of eight
    bits that we can encode in sequence.  The real issue however consists of
    deciding how we will use context variables in order to let the ZP-Coder
    learn the probability distribution of the byte values.

    The most significant bit #b0# decides whether the byte is in range 0..127
    or in range 128..255.  We let the ZP-Coder learn how to predict this bit
    by allocating one context variable for it.  The second most significant
    byte #b1# has two distinct meanings depending of bit #b0#.  If bit #b0# is
    #0#, bit #b1# decides whether the byte is in range 0..63 or 64..127.  If
    bit #b0# is #1#, bit #b1# decides whether the byte is in range 128..191 or
    192..255.  The prediction for bit #b1# must therefore depend on the value
    of #b0#.  This is why we will allocate two context variables for this bit.
    If bit #b0# is #0#, we will use the first variable; if bit #b0# is #1#, we
    will use the second variable.  The next bit #b2# has four meanings and
    therefore we will use four context variables, etc.  This analysis leads to
    a total of #1+2+4+...+128# = #255# context variables for encoding one
    byte.  This encoding procedure can be understood as a binary decision
    tree with a dedicated context variable for predicting each decision.
    \begin{verbatim}
    [>=128]----n---[>=64?]----n----[>31?]  ... 
           \              `---y----[>95?]  ...
            \
             `--y---[>=192?]----n---[>=160?] ...
                            `---y---[>=224?] ...
    \end{verbatim}
    The following decoding function illustrates a very compact way to
    implement such a decision tree.  Argument #ctx# points to an array of 255
    #BitContext# variables.  Macro #REPEAT8# is a shorthand notation for eight
    repetitions of its argument.  
    \begin{verbatim}
    int decode_8_bits(ZPCodec &zp, BitContext *ctx )
    {
      int n = 1;
      REPEAT8( { n = (n<<1) | (zp.decoder(ctx[n-1])); } );
      return n & 0xff;
    }
    \end{verbatim}
    The binary representation of variable #n# is always composed of a #1#
    followed by whichever bits have been decoded so far. This extra bit #1# in
    fact is a nice trick to flatten out the tree structure and directly
    address the array of context variables.  Bit #b0# is decoded using the
    first context variable since #n# is initially #1#.  Bit #b1# is decoded
    using one of the next two variables in the array, since #n# is either #2#
    (#10# in binary) or #3# (#11# in binary).  Bit #b2# will be decoded using
    one of the next four variables, since #n# ranges from #4# (#100# in
    binary) to #7# (#111# in binary).  The final result is given by removing
    the extra #1# in variable #n#.

    The corresponding encoding function is almost as compact. Argument #ctx#
    again is an array of 255 #BitContext# variables.  Each bit of byte #x# is
    encoded and shifted into variable #n# as in the decoding function.
    Variable #x# in fact contains the bits to be encoded. Variable #n#
    contains a #1# followed by the already encoded bits.
    \begin{verbatim}
    void encode_8_bits(ZPCodec &zp, int x, BitContext *ctx )
    {
      int n = 1;
      REPEAT8( { int b=(x&0x80) ? 1 : 0;  x=(x<<1);
                 zp.encoder(b,ctx[n-1]);  n=(n<<1)|(b); } );
    }
    \end{verbatim}
    The ZP-Coder automatically adjusts the content of the context variables
    while coding (recall the context variable argument is passed to functions
    #encoder# and #decoder# by reference).  The whole array of 255 context
    variables can be understood as a "byte context variable".  The estimated
    probability of each byte value is indeed the product of the estimated
    probabilities of the eight binary decisions that lead to that value in the
    decision tree.  All these probabilities are adapted by the underlying
    adaptation algorithm of the ZP-Coder.

    {\bf Application} ---
    We consider now a simple applications consisting of encoding the
    horizontal and vertical coordinates of a cloud of points. Each coordinate
    requires one byte.  The following function illustrates a possible
    implementation:
    \begin{verbatim}
    void encode_points(const char *filename, int n, int *x, int *y)
    {
       StdioByteStream bs(filename, "wb");
       bs.write32(n);             // Write number of points.
       ZPCodec zp(bs, 1);         // Construct encoder and context vars.
       BitContext ctxX[255], ctxY[255];
       memset(ctxX, 0, sizeof(ctxX));
       memset(ctxY, 0, sizeof(ctxY));
       for (int i=0; i<n; i++) {  // Encode coordinates.
          encode_8_bits(zp, x[i], ctxX);
          encode_8_bits(zp, y[i], ctxY);
       }
    }
    \end{verbatim}
    The decoding function is very similar to the encoding function:
    \begin{verbatim}
    int decode_points(const char *filename, int *x, int *y)
    {
       StdioByteStream bs(filename, "rb");
       int n = bs.read32();      // Read number of points.
       ZPCodec zp(bs, 0);        // Construct decoder and context vars.
       BitContext ctxX[255], ctxY[255];
       memset(ctxX, 0, sizeof(ctxX));
       memset(ctxY, 0, sizeof(ctxY));
       for (int i=0; i<n; i++) { // Decode coordinates.
         x[i] = decode_8_bits(zp, ctxX);
         y[i] = decode_8_bits(zp, ctxY);
       }
       return n;                 // Return number of points.
    }
    \end{verbatim}
    The ZP-Coder automatically estimates the probability distributions of both
    the horizontal and vertical coordinates. These estimates are used to
    efficiently encode the point coordinates.  This particular implementation
    is a good option if we assume that the order of the points is significant
    and that successive points are independent.  It would be much smarter
    otherwise to sort the points and encode relative displacements between
    successive points.


    {\bf Huffman Coding Tricks} ---
    Programmer with an experience of Huffman codes should rejoice because this
    experience will prove very useful when working with the ZP-Coder.  Huffman
    codes also organize the symbol values as a decision tree. The tree is
    balanced in such a way that each decision is as unpredictable as possible
    (i.e. both branches must be equally probable).  This is very close to the
    ZP-Coder technique described above.  Since we allocate one context
    variable for each decision, our tree need not be balanced: the context
    variable will track the decision statistics and the ZP-Coder will
    compensate optimally.

    There are good reasons however to avoid unbalanced tree with the ZP-Coder.
    Frequent symbol values may be located quite deep in a poorly balanced
    tree.  This increases the average number of message bits (the number of
    decisions) required to code a symbol.  The ZP-Coder will be called more
    often, making the coding program quite slower.  Furthermore, each message
    bit is encoded using an estimated distribution.  All these useless message
    bits mean that the ZP-Coder has more distributions to adapt.  All this
    extra adaptation work can increase the file size.

    Huffman codes are very fast when the tree structure is fixed beforehand.
    Such {\em static Huffman codes} are unfortunately not very efficient
    because the tree never matches the actual data distribution.  This is why
    such programs almost always define a data dependent tree structure.  This
    structure must then be encoded in the file since the decoder must know it
    before decoding the symbols.  Static Huffman codes however become very
    efficient when decisions are encoded with the ZP-Coder.  The tree
    structure represents a priori knowledge about the distribution of the
    symbol values.  Small data discrepancies will be addressed transparently
    by the ZP-Coder.


    {\bf Encoding Numbers} ---
    This technique is illustrated with the following number encoding problem.
    The multivalued technique described above is not practical with large
    numbers because the decision tree has too many nodes and requires too many
    context variables.  This problem can be solved by using a priori knowledge
    about the probability distribution of our numbers.

    Assume for instance that the distribution is symetrical and that small
    numbers are much more probable than large numbers.  We will first group
    our numbers into several sets.  Each number is coded by first coding which
    set contains the number and then coding a position within the set.  Each
    set contains #2^n# numbers that we consider roughly equiprobable.  Since
    the most probable values occur much more often, we want to model their
    probabily more precisely. Therefore we use small sets for the most
    probable values and large sets for the least probable values, as
    demonstrated below.
    \begin{verbatim} 
    A---------------- {0}                                 (size=1)
     `------B---C---- {1}            or {-1}              (size=1)
             \   `--- {2,3}          or {-2,-3}           (size=2)
              D------ {4...131}      or {-4...-131}       (size=128)
               `----- {132...32899}  or {-132...-32899}   (size=32768)
    \end{verbatim}
    We then organize a decision tree for coding the set identifier.  This
    decision tree is balanced using whatever a priori knowledge we have about
    the probability distribution of the number values, just like a static
    Huffman tree.  Each decision (except the sign decision) is then coded
    using a dedicated context variable.
    \begin{verbatim}
        if (! zp.decoder(ctx_A)) {             // decision A
           return 0;
        } else {
           if (! zp.decoder(ctx_B)) {          // + decision B
             if (! zp.decoder(ctx_C)) {        // ++ decision C
               if (! zp.decoder())             // +++ sign decision
                 return +1;
               else
                 return -1;
             } else {
               if (! zp.decoder())             // +++ sign decision
                 return + 2 + zp.decoder();
               else
                 return - 2 - zp.decoder();
             }
           } else {
             if (! zp.decoder(ctx_D)) {        // ++ decision D
               if (! zp.decoder())             // +++ sign decision
                 return + 4 + decode_7_bits(zp);
               else
                 return - 4 - decode_7_bits(zp);
             } else {
               if (! zp.decoder())             // +++ sign decision
                 return + 132 + decode_15_bits(zp);
               else
                 return - 132 - decode_15_bits(zp);
             }
           }
        } 
   \end{verbatim}
   Note that the call #zp.decoder()# for coding the sign decision does not use
   a context variable.  This is a "pass-thru" variant of \Ref{decoder} which
   bypasses the ZP-Coder and just reads a bit from the code sequence.  There
   is a corresponding "pass-thru" version of \Ref{encoder} for encoding such
   bits.  Similarly, functions #decode_7_bits# and #decode_15_bits# do not
   take an array of context variables because, unlike function #decode_8_bits#
   listed above, they are based on the pass-thru decoder instead of the
   regular decoder.

   The ZP-Coder will not learn the probabilities of the numbers within a set
   since no context variables have been allocated for that.  This could be
   improved by allocating additional context variables for encoding the
   position within the smaller sets and using the regular decoding functions
   instead of the pass-thru variants.  Only experimentation can tell what
   works best for your problem.


   {\bf Understanding Adaptation} ---
   We have so far explained that the ZP-Coder adaptation algorithm is able to
   quickly estimate of the probability distribution of the message bits coded
   using a particular context variable.  It is also able to track slow
   variations when the actual probabilities change while coding.
   
   Let us consider the "cloud of points" application presented above.  Suppose
   that we first code points located towards the left side and then slowly
   move towards points located on the right side.  The ZP-Coder will first
   estimate that the X coordinates are rather on the left side. This
   estimation will be progressively revised after seing more points on the
   right side.  Such an ordering of the points obviously violates the point
   independence assumption on which our code is based.  Despite our inexact
   assumptions, the tracking mechanism allows for better prediction of the X
   coordinates and therefore better compression.

   This is not however a perfect solution. The ZP-Coder tracks the changes
   because every point seems to be a little bit more on the right side than
   suggested by the previous points.  The ZP-Coder coding algorithm is always a
   little bit misadjusted and we always lose a little bit on file size.  This
   is hardly a problem when the probabilities drift slowly. This can be very
   significant if the probabilities change drastically.

   Adaptation is always associated with a small loss of efficiency.  The
   ZP-Coder updates the probability model whenever it suspects, {\em after
   coding}, that the current settings were not optimal.  The model will be
   better next time, but a little bit of harm has been done.  The design of
   ZP-Coder of course minimizes this effect as much as possible.  Yet you will
   pay a price if you ask too much to the adaptation algorithm.  If you have
   millions of context variables, it will be difficult to learn them all.  If
   the probability distributions change drastically while coding, it will be
   difficult to track the changes fast enough.

   Adaptation on the other hand is a great simplification.  A good data
   compression program must (a) represent the data in order to make its
   predictability apparent, and (b) perform the predictions and generate the
   code bits.  The ZP-Coder is an efficient and effortless solution for
   implementing task (b).


   {\bf Practical Tricks} ---
   Sometimes you write an encoding program and a decoding program.
   Unfortunately there is a bug: the decoding program decodes half the file
   and then just outputs garbage.  There is a simple way to locate the
   problem.  In the encoding program, after each call to #encoder#, print the
   encoded bit and the value of the context variable.  In the decoding
   program, after each call to #decoder#, print the decoded bit and the value
   of the context variable.  Both program should print exactly the same thing.
   When you find the difference, you find the bug.
   
   @memo Suggestions for efficiently using the ZP-Coder.  */
//@}
