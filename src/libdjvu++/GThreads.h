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
//C- $Id: GThreads.h,v 1.14 1999-03-17 19:24:58 leonb Exp $

#ifndef _GTHREADS_H_
#define _GTHREADS_H_


/** @name GThreads.h

    Files #"GThreads.h"# and #"GThreads.cpp"# implement common entry points
    for multithreading on multiple platforms.  Each execution thread is
    represented by an instance of class \Ref{GThread}.  Portions of code can
    be protected using class \Ref{GCriticalSection} or class
    \Ref{GCriticalSectionLock}. Synchronization is provided by class
    \Ref{GEvent}.

    The value of compiler symbol #THREADMODEL# selects an appropriate
    implementation for these classes. The current implementation supports
    the following values:
    \begin{description}
    \item[-DTHREADMODEL=NOTHREADS] Dummy implementation.  This is a
          good choice when the multithreading features are not required,
          because it minimizes the portability problems.  This is currently
          the default when compiling under Unix.
    \item[-DTHREADMODEL=WINTHREADS] Windows implementation.
          This is the default when compiling under Windows.
    \item[-DTHREADMODEL=MACTHREADS] Macintosh implementation,
          which is based on the MacOS cooperative model. The current 
          implementation does not yet fully support synchronization.
          This is the default when compiling under MacOS.
    \item[-DTHREADMODEL=POSIXTHREADS] Posix implementation.
          This implementation also supports DCE threads. The behavior of
          the code is subject to the quality of the system implementation of
          Posix threads.
    \item[-DTHREADMODEL=COTHREADS] Custom cooperative threads.
          These custom threads do not redefine system calls. Before executing
          a potentially blocking system function, each thread must explicitly
          check whether it is going to block and yield control explicitly if
          this is the case.  This code must be compiled with a patched version
          of egcs-1.1.1 \URL{http://egcs.cygnus.com}. The patch addresses
          exception thread-safety and is provided in #"@Tools/libgcc2.c.diff"#.
          Once you get the right compiler, this implementation is remarkably
          compact and portable. A variety of processors are supported,
          including mips, intel, sparc, hppa, and alpha.
    \item[-DTHREADMODEL=JRITHREADS] Java implementation hooks.
          Multi-threading within a Netscape plugin can be tricky.  A simple
          idea however consists of implementing the threading primitives in
          Java and to access them using JRI.  The classes just contain a
          JRIGlobalRef.  This is not a real implementation since everything
          (Java code, native functions, stubs, exception thread safety) must
          be addressed by the plugin source code. Performance may be an issue.
    \end{description}
    
    {\bf Portability}: The simultaneous use of threads and exceptions caused a
    lot of portability headaches under Unix.  We eventually decided to
    implement the COTHREADS cooperative threads (because preemptive threads
    have more problems) and to patch EGCS in order to make exception handling
    COTHREAD-safe.  We expect to make COTHREADs the default in future
    releases.

    {\bf ToDo}: For historical reasons, the interface is modeled after the
    Win32 model.  This is unfortunate because more and more systems adopt the
    simpler monitor approach.  It would be nice to change that some day.

    @memo
    Portable threads
    @author
    L\'eon Bottou <leonb@research.att.com> -- initial implementation.\\
    Praveen Guduru <praveen@sanskrit.lz.att.com> -- mac implementation.
    @version
    #$Id: GThreads.h,v 1.14 1999-03-17 19:24:58 leonb Exp $# */
//@{

#include "DjVuGlobal.h"

#ifdef __GNUC__
#pragma interface
#endif

#define NOTHREADS     0
#define COTHREADS     1
#define JRITHREADS    2
#define POSIXTHREADS  10
#define WINTHREADS    11
#define MACTHREADS    12

// Known platforms
#ifndef THREADMODEL
#if defined(WIN32)
#define THREADMODEL WINTHREADS
#endif
#if defined(macintosh)
#define THREADMODEL MACTHREADS
#endif
#endif
// Exception emulation is not thread safe
#include "GException.h"
#ifdef USE_EXCEPTION_EMULATION
#undef  THREADMODEL
#define THREADMODEL NOTHREADS
#endif
// Default is nothreads
#ifndef THREADMODEL
#define THREADMODEL NOTHREADS
#endif

// ----------------------------------------
// INCLUDES

#if THREADMODEL==WINTHREADS
#ifndef _WINDOWS_
#define WIN32_LEAN_AND_MEAN
#include "windows.h"
#endif
#endif

#if THREADMODEL==MACTHREADS
#include <threads.h>
#endif

#if THREADMODEL==POSIXTHREADS
#include <sys/types.h>
#include <sys/time.h>
#include <unistd.h>
#undef TRY
#undef CATCH
#define _CMA_NOWRAPPERS_
#include <pthread.h>
#if defined(G_TRY) && defined(G_CATCH)
#undef TRY
#undef CATCH
#define TRY G_TRY
#define CATCH G_CATCH
#endif 
#endif

#if THREADMODEL==JRITHREADS
#include "jri.h"
#endif

#if THREADMODEL==COTHREADS
#include <sys/types.h>
#include <sys/time.h>
#include <unistd.h>
#endif


// ----------------------------------------
// PORTABLE CLASSES


/** Thread class.  A multithreaded process is composed of a main execution
    thread and of several secondary threads.  Each secondary thread is
    represented by a #GThread# object.  The amount of memory required for the
    stack of a secondary thread is defined when the #GThread# object is
    constructed.  The execution thread is started when function
    \Ref{GThread::create} is called.  The execution can be terminated
    at all times by destroying the #GThread# object or calling
    \Ref{GThread::terminate}.

    Several static member functions control the thread scheduler.  Function
    \Ref{GThread::yield} relinquishes the processor to another thread.
    Function \Ref{GThread::select} (#COTHREADS# only) provides a thread-aware
    replacement for the well-known unix system call #select#.  

    {\bf Note} --- Both the copy constructor and the copy operator are declared
    as private members. It is therefore not possible to make multiple copies
    of instances of this class, as implied by the class semantic. */
class GThread {
public:
  /** Constructs a new thread object.  Memory is allocated for the
      thread, but the thread is not started. 
      Argument #stacksize# is used by the #COTHREADS# model only for
      specifying the amount of memory needed for the processor stack. A
      negative value will be replaced by a suitable default value (128Kb as of
      12/1998). A minimum value of 32Kb is silently enforced. */
  GThread(int stacksize = -1);
  /** Destructor. */
  ~GThread();
  /** Starts the thread. The new thread executes function #entry# with
      argument #arg#.  The thread terminates when the function returns.  A
      thread cannot be restarted after its termination. You must create a new
      #GThread# object. */
  int  create(void (*entry)(void*), void *arg);
  /** Terminates a thread with extreme prejudice. The thread is removed from
      the scheduling list.  Execution terminates regardless of the execution
      status of the thread function. Automatic variables may or may not be
      destroyed. This function must be considered as a last resort since
      memory may be lost. */
  void terminate();
  /** Causes the current thread to relinquish the processor.  The scheduler
      selects a thread ready to run and transfers control to that thread. The
      effect of #yield# heavily depends on the selected implementation.
      Function #yield# returns #-1# when the selected implementation does not
      provide an explicit way to relinquish the processor. This is often the
      case with preemptive multithreading models like #POSIXTHREADS# or
      #WINTHREADS#.  The scheduling code does not need such a feature.
      Function #yield# returns #+1# when only the current thread is ready to
      run.  Otherwise function #yield# returns #0#. */
  static int yield();
  /** Returns a value which uniquely identifies the current thread. */
  static void *current();
#if THREADMODEL==WINTHREADS
private:
  HANDLE hthr;
  DWORD  thrid;
#elif THREADMODEL==MACTHREADS
private:
  unsigned long thid;
#elif THREADMODEL==POSIXTHREADS
private:
  pthread_t hthr;
#elif THREADMODEL==JRITHREADS
private:
  JRIGlobalRef obj;
#elif THREADMODEL==COTHREADS
public:
  // Should be considered as private
  struct cotask *task;
  /** Replaces system call #select# (COTHREADS only).  The #COTHREADS# model
      does not redefine system function.  System functions therefore can
      potentially block the whole process (instead of blocking the current
      thread only) because the system is not aware of the #COTHREADS#
      scheduler.  The function #GThread::select# is a #COTHREADS#-aware
      replacement for the well known system function #select#.  You can also
      use #GThread::select# for making sure that calls to system functions
      will not block the entire process, as demonstrated below:
      \begin{verbatim}
      int 
      gthread_read(int fd, void *buffer, size_t len) 
      {
        fd_set rdset; 
        FD_ZERO(&rdset); 
        FD_SET(fd, &rdset);
        GThread::select(fd+1, &rdset, 0, 0, 0);
        return read(fd, buffer, len);
      }
      \end{verbatim} */
  static int select(int, fd_set*, fd_set*, fd_set*, struct timeval*);
  /** Install hooks in the scheduler (COTHREADS only).  The hook function
      #call# is called when a new thread is created (argument is
      #GThread::CallbackCreate#), when a thread terminates (argument is
      #GThread::CallbackTerminate#), or when thread is unblocked (argument is
      #GThread::CallbackUnblock#).  This callback can be useful in certain GUI
      toolkits where the most convenient method for scheduling the threads
      consists in setting a timer event that calls \Ref{GThread::yield}.  */
  static void set_scheduling_callback(void (*call)(int));
  enum { CallbackCreate, CallbackTerminate, CallbackUnblock };
#endif
public:
  // Should be considered as private
  void (*xentry)(void*);
  void  *xarg;
private:
  // Disable default members
  GThread(const GThread&);
  GThread& operator=(const GThread&);
};


/** Mutual exclusion class.  Class #GCriticalSection# provides an efficient
    way to protect segment of codes ({\em critical sections}) which should
    not be simultaneously executed by two threads. Only one thread can own a
    critical section object at a given time.  Function \Ref{lock} causes the
    current thread to own the critical section object (or wait as long as
    necessary). Function \Ref{unlock} is used to release the critical section
    object.  Critical section in the code are easily protected by creating a
    critical section object, locking this object at beginning of the critical
    section code, and unlocking the object at the end of the critical section
    code. Class \Ref{GCriticalSectionLock} provides a convenient way to do
    this effectively.

    {\bf Note}. Both the copy constructor and the copy operator are declared
    as private members. It is therefore not possible to make multiple copies of
    instances of this class, as implied by the class semantic. */

class GCriticalSection {
#if THREADMODEL==WINTHREADS
  BOOL ok;
  CRITICAL_SECTION cs;
#elif THREADMODEL==POSIXTHREADS
  int ok;
  int count;
  pthread_t locker;
  pthread_mutex_t mutex;
#elif THREADMODEL==JRITHREADS
private:
  JRIGlobalRef obj;
#elif THREADMODEL==COTHREADS
  int ok;
  int count;
  struct cotask *locker;
#endif
public:
  GCriticalSection();
  ~GCriticalSection();
  /** Locks a critical section. Only one thread can own a critical section at
      a given time. If the critical section is already owned by another
      thread, the thread waits until the critical section is released.
      Otherwise the locking counter is incremented and the current thread is
      given ownership of the critical section object. */
  void lock();
  /** Unlocks a critical section object. If the critical section is owned by
      the current thread, the locking counter is decremented. The critical
      section is released when the counter reaches zero. */
  void unlock();
private:
  // Disable default members
  GCriticalSection(const GCriticalSection&);
  GCriticalSection& operator=(const GCriticalSection&);
};

/** Thread synchronization class. Class #GEvent# provides a simple way to
    synchronize several threads.  An event object can be either {\em signaled}
    or {\em non-signaled}. Event objects are initially {\em
    non-signaled}. Function \Ref{set} turns an event object to {\em signaled}.
    Function \Ref{wait} waits until the event object is {\em signaled} and
    then turns it to {\em non-signaled}.

    {\bf Note}. Both the copy constructor and the copy operator are declared
    as private members. It is therefore not possible to make multiple copies
    of instances of this class, as implied by the class semantic. */

class GEvent {
#if THREADMODEL==WINTHREADS
  HANDLE hev;
#elif THREADMODEL==POSIXTHREADS
  pthread_mutex_t mutex;
  pthread_cond_t cond;
  char status;
#elif THREADMODEL==JRITHREADS
private:
  JRIGlobalRef obj;
#elif THREADMODEL==COTHREADS
  int ok;
  int status;
#endif
public:
  GEvent();
  ~GEvent();
  /** Sets the event object to {\em signaled}. */
  void set();
  /** Waits until the event object is {\em signaled}.  If the event object was
      {\em signaled} before, the event object is turned to {\em non signaled}
      and this function returns immediately. If the event object was {\em not
      signaled}, this function waits until another thread calls function
      \Ref{set}, then turns the object to the {\em non-signaled} state and
      returns. */
  void wait();
  /** Waits until the event object is {\em signaled} with a timeout.  This
      function is similar to \Ref{GEvent::wait} but will return after
      #timeout# milliseconds, regardless of the status of the event object.  */
  void wait(int timeout);
private:
  // Disable default members
  GEvent(const GEvent&);
  GEvent& operator=(const GEvent&);
};


// ----------------------------------------
// WIN32 INLINES

#if THREADMODEL==WINTHREADS

inline
GCriticalSection::GCriticalSection()
  : ok(FALSE)
{ 
  InitializeCriticalSection(&cs); 
  ok=TRUE; 
}

inline 
GCriticalSection::~GCriticalSection()
{ 
  ok=FALSE; 
  DeleteCriticalSection(&cs); 
}

inline void 
GCriticalSection::lock() 
{
  if (ok) 
    EnterCriticalSection(&cs);
}

inline void 
GCriticalSection::unlock() 
{
  if (ok) 
    LeaveCriticalSection(&cs);
}

inline
GEvent::GEvent()
{
  hev = CreateEvent(NULL,FALSE,FALSE,NULL);
}

inline
GEvent::~GEvent()
{
  CloseHandle(hev);
}

inline void 
GEvent::set()
{
  SetEvent(hev);
}

inline void 
GEvent::wait()
{
  WaitForSingleObject(hev,INFINITE);
}

inline void 
GEvent::wait(int timeout)
{
  WaitForSingleObject(hev, timeout);
}

#endif  //WINTHREADS


// ----------------------------------------
// NOTHREADS INLINES

#if THREADMODEL==NOTHREADS
inline GThread::GThread(int stacksize) {}
inline GThread::~GThread(void) {}
inline void GThread::terminate() {}
inline int GThread::yield() { return -1; }
inline void* GThread::current() { return 0; }
inline GCriticalSection::GCriticalSection() {}
inline GCriticalSection::~GCriticalSection() {}
inline void GCriticalSection::lock() {}
inline void GCriticalSection::unlock() {}
inline GEvent::GEvent() {}
inline GEvent::~GEvent() {}
inline void GEvent::set() {}
inline void GEvent::wait() {}
inline void GEvent::wait(int timeout) {}
#endif // NOTHREADS


// ----------------------------------------
// SCOPE LOCK


/** Wrapper for mutually exclusive code.
    This class locks a specified critical section (see \Ref{GCriticalSection})
    at construction time and unlocks it at destruction time. It provides a
    convenient way to take advantage of the C++ implicit destruction of
    automatic variables in order to make sure that the critical section is
    unlocked when exiting the protected code.  The following code will release
    the lock when the execution thread leaves the protected scope, either
    because the protected code has executed successfully, or because an
    exception was thrown.
    \begin{verbatim}
      {      -- protected scope
         static GCriticalSection theSection;
         GCriticalSectionLock lock(&theSection)
         ... -- protected code
      }
    \end{verbatim} 
*/
class GCriticalSectionLock 
{
private:
  GCriticalSection *gsec;
public:
  /** Constructor. Locks the critical section object #gsec#. */
  GCriticalSectionLock(GCriticalSection *gsec);
  /** Destructor. Releases the associated critical section. */
  ~GCriticalSectionLock();
};

inline 
GCriticalSectionLock::GCriticalSectionLock(GCriticalSection *gsec)
  : gsec(gsec) 
{ 
  gsec->lock(); 
}

inline  
GCriticalSectionLock::~GCriticalSectionLock()
{
  gsec->unlock();
}


//@}

// ----------------------------------------
#endif //_GTHREADS_H_

