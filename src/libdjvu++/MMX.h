//C-  -*- C++ -*-
//C- DjVu® Reference Library (v. 3.0)
//C- 
//C- Copyright © 1999-2000 LizardTech, Inc. All Rights Reserved.
//C- 
//C- This software (the "Original Code") is subject to, and may be
//C- distributed under, the GNU General Public License, Version 2.
//C- The license should have accompanied the Original Code or you
//C- may obtain a copy of the license from the Free Software
//C- Foundation at http://www.fsf.org .
//C- 
//C- With respect to the Original Code, and subject to any third
//C- party intellectual property claims, LizardTech grants recipient
//C- a worldwide, royalty-free, non-exclusive license under patent
//C- claims infringed by making, using, or selling Original Code
//C- which are now or hereafter owned or controlled by LizardTech,
//C- but solely to the extent that any such patent is reasonably
//C- necessary to enable you to make, have made, practice, sell, or 
//C- otherwise dispose of Original Code (or portions thereof) and
//C- not to any greater extent that may be necessary to utilize
//C- further modifications or combinations.
//C- 
//C- The Original Code is provided "AS IS" WITHOUT WARRANTY OF ANY
//C- KIND, EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO
//C- ANY WARRANTY OF NON-INFRINGEMENT, OR ANY IMPLIED WARRANTY OF 
//C- MERCHANTIBILITY OF FITNESS FOR A PARTICULAR PURPOSE.
// 
// $Id: MMX.h,v 1.11 2000-11-02 01:08:35 bcr Exp $
// $Name:  $


#ifndef _MMX_H_
#define _MMX_H_

#include "DjVuGlobal.h"


/** @name MMX.h
    Files #"MMX.h"# and #"MMX.cpp"# implement basic routines for
    supporting the MMX instructions on x86.  Future instruction sets
    for other processors may be supported in this file as well.

    Macro #MMX# is defined if the compiler supports the X86-MMX instructions.
    It does not mean however that the processor supports the instruction set.
    Variable #MMXControl::mmxflag# must be used to decide whether MMX.
    instructions can be executed.  MMX instructions are entered in the middle
    of C++ code using the following macros.  Examples can be found in
    #"IWTransform.cpp"#.

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

    @memo
    Essential support for MMX.
    @version 
    #$Id: MMX.h,v 1.11 2000-11-02 01:08:35 bcr Exp $#
    @author: 
    L\'eon Bottou <leonb@research.att.com> -- initial implementation */
//@{


/** MMX Control. 
    Class #MMXControl# encapsulates a few static functions for 
    globally enabling or disabling MMX support. */

class MMXControl
{
 public:
  // MMX DETECTION
  /** Detects and enable MMX or similar technologies.  This function checks
      whether the CPU supports a vectorial instruction set (such as Intel's
      MMX) and enables them.  Returns a boolean indicating whether such an
      instruction set is available.  Speedups factors may vary. */
  static int enable_mmx();
  /** Disables MMX or similar technologies.  The transforms will then be
      performed using the baseline code. */
  static int disable_mmx();
  /** Contains a value greater than zero if the CPU supports vectorial
      instructions. A negative value means that you must call \Ref{enable_mmx}
      and test the value again. Direct access to this member should only be
      used to transfer the instruction flow to the vectorial branch of the
      code. Never modify the value of this variable.  Use #enable_mmx# or
      #disable_mmx# instead. */
  static int mmxflag;  // readonly
};

//@}




// ----------------------------------------
// GCC MMX MACROS

#ifndef NO_MMX

#if defined(__GNUC__) && defined(__i386__)
#define MMXemms \
  __asm__ volatile("emms" : : : "memory" ) 
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
#pragma warning( disable : 4799 )
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
#define MMX 1
#endif

#endif

// -----------
#endif
