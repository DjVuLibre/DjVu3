//C-  -*- C++ -*-
//C-
//C-  Copyright (c) 1998 AT&T	
//C-  All Rights Reserved 
//C-
//C-  THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
//C-  The copyright notice above does not evidence any
//C-  actual or intended publication of such source code.
//C-
//C-  $Id: GException.h,v 1.3 1999-02-08 19:38:36 leonb Exp $


#ifndef _GEXCEPTION_H_
#define _GEXCEPTION_H_

/** @name GException.h

    Files #"GException.h"# and #"GException.cpp"# define a portable exception
    scheme used through the library. This scheme can use native C++ exception
    or an exception emulation based on #longjmp#/#setjmp#. A particular model
    can be forced a compile time by defining option #CPP_SUPPORTS_EXCEPTIONS#
    or #USE_EXCEPTION_EMULATION#.
    
    The exception emulation unfortunately is not able to call the proper
    destructors when an exception occurs. This is acceptable for simple
    command line program, but will cause memory leaks in any continuously
    running application (such as a browser). In addition, the exception
    emulation is not thread safe.  These are the two main reasons for using a
    compliant C++ compiler.  These are also compelling reasons to {\em only}
    use exception to signal an error condition which forces the library to
    discontinue execution.
    
    There are four macros for handling exceptions.  Macros #TRY#, #CATCH(ex)#
    and #ENDCATCH# must be used to define an exception catching
    block. Exceptions can be thrown at all times using macro
    #THROW(cause)#. An exception can be re-thrown from a catch block using
    macro #RETHROW#.
    
    Example:
    \begin{verbatim}
    TRY
      {
        // program lines including a possible THROW  
        // or calls to functions that perform a THROW
        THROW("message");
      }
    CATCH(ex) 
      {
        // ex is a \Ref{GException} object that you can print ...
        ex.perror();  
        // or rethrow to an outer exception handler
        RETHROW;
      }
    ENDCATCH;
    \end{verbatim} 

    @memo 
    Portable exceptions.
    @author 
    Leon Bottou <leonb@research.att.com> -- initial implementation.\\
    Andrei Erofeev <eaf@geocities.com> -- fixed message memory allocation.
    @version 
    #$Id: GException.h,v 1.3 1999-02-08 19:38:36 leonb Exp $# */
//@{

#include "DjVuGlobal.h"

#ifdef __GNUC__
#pragma interface
#endif

/** Base exception class.
    The library can use native C++ exception or an exception
    emulation based on #longjmp#/#setjmp#.  This model uses
    exception handling macros (see \Ref{GException.h}). 
    These macros represent all exceptions wich class GException.  
*/

class GException {
public:
  /** Constructs a GException.  Usually called by macro #THROW#.  Argument
      #cause# usually is a plain text error message which should not be relied
      upon by exception handlers.  As a convention however, string #"EOF"# is
      used when reaching an unexpected end-of-file condition and string
      #"STOP"# is used when the user interrupts the execution.  These strings
      can be tested by the exception handlers. Similar conventional strings may
      be defined in the future. They all will be small strings with only
      uppercase characters.  
      @param cause error message.  
      @param file file name, usually provided by macro #__FILE__#.  
      @param line line number, usually provided by macro #__LINE__#.  
      @param func function name, provided (in GCC) by macro #__PRETTY_FUNCTION__#. */
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
      pointer is never null. The string usually is a plain text error message
      and should not be relied upon by exception handlers. As a convention
      however, string #"EOF"# is used when reaching an unexpected end-of-file
      condition and string #"STOP"# is used when the user interrupts the
      execution. */
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

#define G_TRY        try
#define G_CATCH(n)   catch(GException &n) { 
#define G_ENDCATCH   }
#define G_RETHROW    throw
#ifdef __GNUG__
#define G_THROW(msg) throw \
  GException(msg, __FILE__, __LINE__, __PRETTY_FUNCTION__)
#else
#define G_THROW(msg) throw \
  GException(msg, __FILE__, __LINE__)
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
  static void emthrow(const GException &);
public:
  GExceptionHandler() { next = head; };
  ~GExceptionHandler() { head = next; };
};

#define G_TRY    do { GExceptionHandler __exh; \
                      if (!setjmp(__exh.jump)) \
                      { GExceptionHandler::head = &__exh;

#define G_CATCH(n) } else { GExceptionHandler::head = __exh.next; \
                            GException& n = __exh.current;

#define G_ENDCATCH } } while(0)

#define G_RETHROW  GExceptionHandler::emthrow(__exh.current)

#ifdef __GNUG__
#define G_THROW(msg) GExceptionHandler::emthrow \
  (GException(msg, __FILE__, __LINE__, __PRETTY_FUNCTION__))
#else
#define G_THROW(m) GExceptionHandler::emthrow \
  (GException(m, __FILE__, __LINE__))
#endif

#endif // !CPP_SUPPORTS_EXCEPTIONS


#undef TRY
#undef CATCH
#undef ENDCATCH
#undef RETHROW
#undef THROW
#define TRY G_TRY
#define CATCH G_CATCH
#define ENDCATCH G_ENDCATCH
#define RETHROW G_RETHROW
#define THROW G_THROW



// -------------- THE END
#endif
