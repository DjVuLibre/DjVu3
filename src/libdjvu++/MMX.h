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
//C- $Id: MMX.h,v 1.3 1999-06-08 16:03:34 leonb Exp $

#ifndef _MMX_H_
#define _MMX_H_

#include "DjVuGlobal.h"
#include "GPixmap.h"

/** @name MMX.h
    Files #"MMX.h"# and #"MMX.cpp"# implement basic routines for
    supporting the MMX instructions on x86.  Future instruction sets
    for other processors may be supported in this file as well.

    Macro #MMX# is defined if the compiler supports the X86-MMX instructions.
    It does not mean however that the processor supports the instruction set.
    Function #MMXControl::ok# must be used to decide whether MMX instructions
    can be executed.  MMX instructions are entered in the middle of C++ code
    using the following macros.
    \begin{description}
    \item[MMXrr( insn, srcreg, dstreg)] 
       Encode a register to register MMX instruction 
       (e.g. #paddw# or #punpcklwd#).
    \item[MMXar( insn, addr, dstreg )]
       Encode a memory to register MMX instruction 
       (e.g. #moveq# from memory).
    \item[MMXra( insn, srcreg, addr )]
       Encode a register to memory MMX instruction 
       (e.g. #moveq# to memory).
    \item[MMXir( insn, imm, dstreg )]
       Encode a immediate to register MMX instruction 
       (e.g #psraw#).
    \item[MMXemms]
       Execute the #EMMS# instruction to reset the FPU state.
    \end{description}

    Examples can be found in #"IWTransform.h"#.  

    @memo
    Essential support for MMX.
    @version 
    #$Id: MMX.h,v 1.3 1999-06-08 16:03:34 leonb Exp $#
    @author: 
    L\'eon Bottou <leonb@research.att.com> -- initial implementation 
*/    
//@{


/** MMX Control. 
    Class #MMXControl# encapsulates a few static functions for 
    globally enabling or disabling MMX support. */

class MMXControl
{
 public:
  // MMX DETECTION
  /** Detects and enable MMX or similar technologies.  This function chects
      whether a specialized implementations of the IW44 transform is available
      (such as the MMX implementation) and enables it.  Returns a boolean
      indicating whether such an implementation is available.  Speedups
      factors may vary. */
  static int enable_mmx();
  /** Disables MMX or similar technologies.  The transforms will then be
      performed using the baseline code. */
  static int disable_mmx();
  /** Contains a value greater than zero if MMX code can be executed.  A
      negative value means that you must call \Ref{enable_mmx} and test the
      value again. Direct access to this member should only be used to
      transfer the instruction flow to the MMX branch of the code. Do not
      write into this variable. */
  static int mmxflag;
};

//@}




// ----------------------------------------
// GCC MMX MACROS

#if defined(__GNUC__) && defined(__i386__)
#define MMXemms \
  __asm__ volatile("emms") 
#define MMXrr(op,src,dst) \
  __asm__ volatile( #op ## " %%" ## #src ## ",%%" ## #dst : : : "memory") 
#define MMXir(op,imm,dst) \
  __asm__ volatile( #op ## " %0,%%" ## #dst : : "i" (imm) : "memory") 
#define MMXar(op,addr,dst) \
  __asm__ volatile( #op ## " %0,%%" ## #dst : : "rm" (*(int*)(addr)) : "memory") 
#define MMXra(op,src,addr) \
  __asm__ volatile( #op ## " %%" #src ## ",%0" : : "rm" (*(int*)(addr)) : "memory") 
#define MMX 1
#endif


// ----------------------------------------
// MSVC MMX MACROS

#if defined(_MSC_VER) && defined(_M_IX86)
// Compiler option /GM is required
#pragma warning( 4799 : disable )
#define MMXemms \
  __asm { emms }
#define MMXrr(op,src,dst) \
  __asm { op dst,src }
#define MMXir(op,imm,dst) \
  __asm { op dst,imm }
#define MMXar(op,addr,dst) \
  { register __int64 var=*(__int64*)(addr); __asm { op dst,var } }
#define MMXra(op,src,addr) \
  { register __int64 var; __asm { op [var],src };  *(__int64*)addr = var; } 
// Probably not as efficient as GCC macros
// Not thoroughly tested.
#define MMX 1
#endif


// -----------
#endif
