//C-  -*- C++ -*-
//C- DjVu� Reference Library (v. 3.0)
//C- 
//C- Copyright � 1998-2000 LizardTech, Inc. All Rights Reserved.
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
// $Id: debug.h,v 1.12 2000-11-02 01:08:35 bcr Exp $
// $Name:  $



#ifndef _DEBUG_H_
#define _DEBUG_H_

#ifdef __GNUC__
#pragma interface
#endif

#include <stdio.h>
#ifdef WIN32
#include <atlbase.h> // USES_CONVERSION, A2CT macro
#include <windows.h> // OutputDebugString
#endif 
/** @name debug.h

    Files #"debug.h"# and #"debug.cpp"# implement means to print debug
    messages in a multithread safe way.  Message are also marked with a thread
    identifier.  Under Windows, debug messages are directly sent to the
    debugger using the Win32 function #OutputDebugString#.  Under Unix, debug
    messages are printed on the controlling terminal, preferably using device
    #/dev/tty#.

    The preprocessor variable #DEBUGLVL# defines which debug code is going to
    be compiled.  Selecting #-DDEBUGLVL=0# (the default) disables all
    debugging code.  Selecting a positive values (e.g. #-DDEBUGLVL=4#) enables
    more and more debugging code.

    Message output is controlled by the current debugging level (an integer
    between #0# and #DEBUGLVL#). Greater values enable more messages.  The
    initial debugging level is set to the maximum value.  The debugging level
    can be changed using macro \Ref{DEBUG_SET_LEVEL}.

    Message indentation can be modified using macro \Ref{DEBUG_MAKE_INDENT}.
    Messages are generated by macro \Ref{DEBUG_MSG} or its variants.  The
    argument of the macro can contain several components separated by operator
    #<<#, as demonstrated in the example below:
    \begin{verbatim}
    DEBUG_MSG("The value of a[" << n << "] is " << a[n] << '\n');
    \end{verbatim}

    One more preprocessor variable #RUNTIME_DEBUG_ONLY# enables compilation
    of debug code, but does not enable the debug messages automatically.
    In order to see them the program should use \Ref{DEBUG_SET_LEVEL} to
    change the level to anything greater than 0. Normally this happens when
    user specifies option #-debug# in the command line. Usage of
    #RUNTIME_DEBUG_ONLY# implies #DEBUGLVL=1# if not specified otherwise.

    Finally, #-DDEBUG# can be used instead of #-DDEBUGLVL=1#.

    {\bf Historical Comment} --- Debug macros are rarely used in the reference
    DjVu library because Leon thinks that debugging messages unnecessarily
    clutter the code.  Debug macros are used everywhere in the plugin code
    because Andrew thinks that code without debugging messages is close to
    useless.  No agreement could be reached. Neither could they agree on
    if cluttering header files with huge documentation chunks helps to
    improve code readability.

    @memo 
    Macros for printing debug messages.
    @version 
    #$Id: debug.h,v 1.12 2000-11-02 01:08:35 bcr Exp $#
    @author
    Andrew Erofeev <eaf@geocities.com> -- initial implementation \\
    Leon Bottou <leonb@research.att.com> -- cleanups */
//@{

#ifdef RUNTIME_DEBUG_ONLY
#ifndef DEBUG
#define DEBUG
#endif
#endif

#ifndef DEBUGLVL
#ifdef DEBUG
#define DEBUGLVL 1
#endif
#endif

#ifndef DEBUGLVL
#define DEBUGLVL 0
#endif

#if DEBUGLVL >= 1

#ifndef DEBUG
#define DEBUG
#endif

// ------------ SUPPORT

class Debug // DJVU_CLASS
{
private:
  int    id;
  int    block;
  int    indent;
  void   format(const char *fmt, ... );
public:
  // construction
  Debug();
  ~Debug();
  // access
  static void   set_debug_level(int lvl);
  static void   set_debug_file(const char *fname);
  static void	set_debug_file(FILE * file);
  void          modify_indent(int rindent);
  static Debug& lock(int lvl, int noindent);
  void          unlock();
  // printing
  Debug &	operator<<(bool b);
  Debug &	operator<<(char c);
  Debug &	operator<<(unsigned char c);
  Debug &	operator<<(int i);
  Debug &	operator<<(unsigned int i);
  Debug &	operator<<(short int i);
  Debug &	operator<<(unsigned short int i);
  Debug &	operator<<(long i);
  Debug &	operator<<(unsigned long i);
  Debug &	operator<<(const char * const ptr);
  Debug &	operator<<(const unsigned char * const ptr);
  Debug &	operator<<(float f);
  Debug &	operator<<(double d);
  Debug &	operator<<(const void * const p);
};

class DebugIndent // DJVU_CLASS
{
private:
  int inc;
public:
  DebugIndent(int inc=2);
  ~DebugIndent();
//#define DEBUG_MAKE_INDENT_2(x, y) DebugIndent debug_indent ## y ## (x)
//#define DEBUG_MAKE_INDENT_1(x, y) DEBUG_MAKE_INDENT_2(x, y)
#define DEBUG_MAKE_INDENT_1(x, y) DebugIndent debug_indent ## y ## (x)
};

// ------------ MAIN MACROS

/** Indents all messages in the current scope. */
#define DEBUG_MAKE_INDENT(x)     DEBUG_MAKE_INDENT_1(x, __LINE__)
/** Sets the current debugging level. */
#define DEBUG_SET_LEVEL(level)   Debug::set_debug_level(level)

#define DEBUG_MSG_LVL(level,x)   { ( Debug::lock(level,0) << x ).unlock(); }
#define DEBUG_MSGN_LVL(level,x)  { ( Debug::lock(level,1) << x ).unlock(); }
#define DEBUG_MSGF_LVL(level,x)  { ( Debug::lock(level,1) << x ).unlock(); }

#else /* DEBUGLVL <= 0 */

#define DEBUG_MAKE_INDENT(x)
#define DEBUG_SET_LEVEL(level)
#define DEBUG_MSG_LVL(level,x)
#define DEBUG_MSGN_LVL(level,x)

#endif


// ------------ EAF MACROS

#if ( DEBUGLVL >= 1 )
/** Generates a level 1 message */
#define DEBUG1_MSG(x)  DEBUG_MSG_LVL(1,x)
#define DEBUG1_MSGF(x) DEBUG_MSGF_LVL(1,x)
#else
#define DEBUG1_MSG(x)
#define DEBUG1_MSGF(x)
#endif
#if ( DEBUGLVL >= 2 )
/** Generates a level 2 message */
#define DEBUG2_MSG(x)  DEBUG_MSG_LVL(2,x)
#define DEBUG2_MSGF(x) DEBUG_MSGF_LVL(2,x)
#else
#define DEBUG2_MSG(x)
#define DEBUG2_MSGF(x)
#endif
#if ( DEBUGLVL >= 3 )
/** Generates a level 3 message */
#define DEBUG3_MSG(x)  DEBUG_MSG_LVL(3,x)
#define DEBUG3_MSGF(x) DEBUG_MSGF_LVL(3,x)
#else
#define DEBUG3_MSG(x)
#define DEBUG3_MSGF(x)
#endif
#if ( DEBUGLVL >= 4 )
/** Generates a level 4 message */
#define DEBUG4_MSG(x)  DEBUG_MSG_LVL(4,x)
#define DEBUG4_MSGF(x) DEBUG_MSGF_LVL(4,x)
#else
#define DEBUG4_MSG(x)
#define DEBUG4_MSGF(x)
#endif

#define DEBUG_RUNTIME_SET_LEVEL(level) DEBUG_SET_LEVEL(level)
/** Generates a level 1 message. */
#define DEBUG_MSG(x)  DEBUG1_MSG(x)
/** Generates a level 1 message without indentation. */
#define DEBUG_MSGF(x) DEBUG1_MSGF(x)
/** Generates a level 1 message terminated with a newline. */
#define DEBUG_MSGN(x) DEBUG_MSG(x<<'\n')

//@}

// ------------ THE END
#endif
