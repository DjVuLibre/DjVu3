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
//C- $Id: GException.h,v 1.19 2000-10-06 21:47:21 fcrary Exp $


#ifndef _GEXCEPTION_H_
#define _GEXCEPTION_H_

#ifdef __GNUC__
#pragma interface
#endif
#ifndef no_return
#ifdef __GNUC__
#define no_return __attribute__ ((noreturn))
#else
#define no_return
#endif
#endif

/** @name GException.h

    Files #"GException.h"# and #"GException.cpp"# define a portable exception
    scheme used through the DjVu Reference Library. This scheme can use native
    C++ exceptions or an exception emulation based on #longjmp#/#setjmp#. A
    particular model can be forced a compile time by defining option
    #CPP_SUPPORTS_EXCEPTIONS# or #USE_EXCEPTION_EMULATION#.
    
    This emulation code was motivated because many compilers did not properly
    support exceptions as mandated by the C++ standard documents. This
    emulation is now considered obsolete because (a) it is not able to call
    the proper destructors when an exception occurs, and (b) it is not thread
    safe.  Although all modern C++ compiler handle exception decently, the
    exception handling intrinsics are not always thread safe.  Therefore we
    urge programmers to {\em only} use exceptions to signal error conditions
    that force the library to discontinue execution.
    
    There are four macros for handling exceptions.  Macros #G_TRY#, #G_CATCH# and
    #G_ENDCATCH# are used to define an exception catching block.  Exceptions can
    be thrown at all times using macro #G_THROW(cause)#. An exception can be
    re-thrown from a catch block using macro #G_RETHROW#.
    
    Example:
    \begin{verbatim}
    G_TRY
      {
        // program lines which may result in a call to THROW()
        G_THROW("message");
      }
    G_CATCH(ex) 
      {
        // Variable ex refers to a GException object.
        ex.perror();  
        // You can rethrow the exception to an outer exception handler.
        G_RETHROW;
      }
    G_ENDCATCH;
    \end{verbatim} 

    @memo 
    Portable exceptions.
    @author 
    L\'eon Bottou <leonb@research.att.com> -- initial implementation.\\
    Andrei Erofeev <eaf@geocities.com> -- fixed message memory allocation.
    @version 
    #$Id: GException.h,v 1.19 2000-10-06 21:47:21 fcrary Exp $# */
//@{

#include "DjVuGlobal.h"


/** Exception class.  
    The library always uses macros #G_TRY#, #G_THROW#, #G_CATCH# and #G_ENDCATCH# for
    throwing and catching exceptions (see \Ref{GException.h}). These macros
    only deal with exceptions of type #GException#. */

class GException {
public:
  /** Constructs a GException.  This constructor is usually called by macro
      #THROW#.  Argument #cause# is a plain text error message. As a
      convention, string #"EOF"# is used when reaching an unexpected
      end-of-file condition and string #"STOP"# is used when the user
      interrupts the execution. The remaining arguments are usually provided
      by the predefined macros #__FILE__#, #__LINE__#, and (G++ and EGCS only)
      #__PRETTY_FUNCTION__#.  */
  GException (const char *cause, const char *file=0, int line=0, const char *func=0);

  /** Copy Constructor. */
  GException (const GException & exc);
  
  /** Null Constructor. */
  GException ();
  
  /** Destructor. */
  virtual ~GException(void);
  
  /** Copy Operator. */
  GException & operator=(const GException & exc);
  
  /** Prints an error message on stderr.
      @param msg: string incorporated into the error message. */
  void perror(const char *msg = 0) const;
  
  /** Returns the string describing the cause of the exception.  The returned
      pointer is never null.  Exception handlers should not rely on the value
      of the string #cause#.  As a convention however, string #"EOF"# is used
      when reaching an unexpected end-of-file condition and string #"STOP"# is
      used when the user interrupts the execution. These strings can be tested
      by the exception handlers. Similar conventional strings may be defined
      in the future. They all will be small strings with only uppercase
      characters. */
  const char* get_cause(void) const;

  /** Returns the function name from which the exception was thrown.
      A null pointer is returned if no function name is available. */
  const char* get_function(void) const { return func; };
  
  /** Returns the file name from which the exception was thrown.
      A null pointer is returned if no file name is available. */
  const char* get_file(void) const { return file; };
  
  /** Returns the line number from which the exception was thrown.
      A zero is returned if no line number is available. */
  int get_line(void) const { return line; };
  
  //  Magic cause string
  static const char * const outofmemory;

private:
  const char *cause;
  const char *file;
  const char *func;
  int line;
};

//@}

#undef G_TRY
#undef G_CATCH
#undef G_CATCH_ALL
#undef G_ENDCATCH
#undef G_RETHROW
#undef G_THROW

// Check if compiler supports native exceptions
#if defined(_MSC_VER)
#define CPP_SUPPORTS_EXCEPTIONS
#endif
#if defined(__MWERKS__)
#define CPP_SUPPORTS_EXCEPTIONS
#endif
#if defined(__EXCEPTIONS)
#define CPP_SUPPORTS_EXCEPTIONS
#endif
// Decide which exception model to use
#ifndef CPP_SUPPORTS_EXCEPTIONS
#ifndef USE_EXCEPTION_EMULATION
#define USE_EXCEPTION_EMULATION
#endif
#endif


#ifndef USE_EXCEPTION_EMULATION

// Compiler supports ANSI C++ exceptions.
// Defined exception macros accordingly.

class GExceptionHandler {
public:
#ifndef NO_LIBGCC_HOOKS
  static void exthrow(const GException &) no_return;
#else
  static void exthrow(const GException ) no_return;
#endif /* NO_LIBGCC_HOOKS */
  static void rethrow(void) no_return;
};

#define G_TRY        try
#define G_CATCH(n)   catch(const GException &n) {
#define G_CATCH_ALL   catch(...) {
#define G_ENDCATCH   } 
#define G_RETHROW    GExceptionHandler::rethrow()
#define G_EMTHROW(ex)  GExceptionHandler::exthrow(ex)
#ifdef __GNUG__
#define G_THROW(msg) GExceptionHandler::exthrow \
  (GException(msg, __FILE__, __LINE__, __PRETTY_FUNCTION__))
#else
#define G_THROW(msg) GExceptionHandler::exthrow \
  (GException(msg, __FILE__, __LINE__))
#endif

#else // USE_EXCEPTION_EMULATION

// Compiler does not support ANSI C++ exceptions.
// Emulate with setjmp/longjmp.

#include <setjmp.h>

class GExceptionHandler {
public:
  jmp_buf jump;
  GExceptionHandler *next;
  GException current;
public:
  static GExceptionHandler *head;
  static void emthrow(const GException &) no_return;
public:
  GExceptionHandler() { next = head; };
  ~GExceptionHandler() { head = next; };
};

#define G_TRY    do { GExceptionHandler __exh; \
                      if (!setjmp(__exh.jump)) \
                      { GExceptionHandler::head = &__exh;

#define G_CATCH_ALL } else { GExceptionHandler::head = __exh.next; 
#define G_CATCH(n) G_CATCH_ALL const GException& n = __exh.current;

#define G_ENDCATCH } } while(0)

#define G_RETHROW    GExceptionHandler::emthrow(__exh.current)

#ifdef __GNUG__
#define G_THROW(msg) GExceptionHandler::emthrow \
  (GException(msg, __FILE__, __LINE__, __PRETTY_FUNCTION__)) 
#define G_EMTHROW(ex) GExceptionHandler::emthrow(ex)
#else
#define G_THROW(m) GExceptionHandler::emthrow \
  (GException(m, __FILE__, __LINE__)) no_return
#define G_EMTHROW(ex) GExceptionHandler::emthrow(ex) no_return
#endif

#endif // !CPP_SUPPORTS_EXCEPTIONS


inline void
G_EXTHROW
(const GException &ex,const char *msg=0,const char *file=0,int line=0,const char *func=0)
{
  G_EMTHROW( (msg||file||line||func)?
      GException(msg?msg:ex.get_cause(),
        file?file:ex.get_file(),
        line?line:ex.get_line(),
        func?func:ex.get_function())
  :ex);
}

inline void
G_EXTHROW
(const char msg[],const char *file=0,int line=0,const char *func=0)
{
  G_EMTHROW(GException(msg,file,line,func));
}

// -------------- THE END
#endif
