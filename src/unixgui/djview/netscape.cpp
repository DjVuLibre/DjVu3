//C-  -*- C++ -*-
//C-
//C-  Copyright © 1999-2001, LizardTech, Inc. All Rights Reserved.
//C-              Unauthorized use prohibited.
//C-
// 
// $Id: netscape.cpp,v 1.1 2001-05-29 22:05:29 bcr Exp $
// $Name:  $


#ifdef __GNUC__
#pragma implementation
#endif

#include "ZPCodec.h"		// Hates to be included after QT stuff

#include "netscape.h"
#include "debug.h"
#include "throw_error.h"
#include "dispatch.h"
#include "exc_msg.h"
#include "io.h"

//  #ifdef DJVU_EDITOR
//  #include "djvu_terminator.h"
//  #endif

#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/time.h>
#include <iostream.h>
#include <arpa/inet.h>

#include <qsocketnotifier.h>
#include <qapplication.h>

#include "qt_fix.h"

// This object is here just to provide a way to connect to socket notifiers
class QDWatcher : public QObject
{
   Q_OBJECT
public slots:
   void		slotInput(int);
public:
   QDWatcher(void) {}
   ~QDWatcher(void) {}
};

int	pipe_read=3, pipe_write=4;
int	rev_pipe=5;
struct timeval	* exit_tv=0;

static QSocketNotifier	*input_sn;
static QDWatcher * watcher;

// This is the thread, which will be processing stream-related requests.
// The point is to minimize the chance of dead locks: With this thread the system will
// work find even if the main thread requests data, which is not available.
// There still may be dead locks though because requests like NPP_New/NPP_AttachWindow,
// etc. must be processed by the main thread sychronously.
static GThread	str_thread;

void
PipesAreDead(void)
{
   DEBUG_MSG("PipesAreDead(): Killing the rest of pipes and removing input handlers\n");
   delete input_sn; input_sn=0;
   close(pipe_read); pipe_read=-1;
   close(pipe_write); pipe_write=-1;
   close(rev_pipe); rev_pipe=-1;
      // Don't close the watcher here: you may be called from its slot.
      // delete watcher; watcher=0;

   if (qApp) qApp->exit(1);
   else exit(1);
}

void
QDWatcher::slotInput(int)
{
   DEBUG_MSG("QDWatcher::slotInput() called: smth is in the netscape pipe\n");
   try
   {
      Dispatch();
   } catch(PipeError & exc)
   {
      cerr << exc.get_cause() << "\n";
      PipesAreDead();
   } catch(const GException & exc)
   {
      cerr << exc.get_cause() << "\n";
   }
}

void
WorkWithNetscape(void)
{
   DEBUG_MSG("WorkWithNetscape(): working together with netscape\n");

   WriteString(pipe_write, "Here am I");	// Plugin is waiting for this

//  #ifdef DJVU_EDITOR
//     DjVuTerminator::AddPipe(pipe_read);
//     DjVuTerminator::AddPipe(pipe_write);
//     DjVuTerminator::AddPipe(rev_pipe);
//  #endif

   exit_tv=new timeval;
   exit_tv->tv_sec=INACTIVITY_TIMEOUT/1000;
   exit_tv->tv_usec=0;
   
   while(true)
   {
      if (qApp)
      {
	 if (!watcher)
	 {
	    watcher=new QDWatcher();
	    input_sn=new QSocketNotifier(pipe_read, QSocketNotifier::Read,
					 watcher, "input_sn");
	    input_sn->setEnabled(TRUE);
	    QObject::connect(input_sn, SIGNAL(activated(int)),
			     watcher, SLOT(slotInput(int)));
	 }
	 int rc=qApp->exec();
	 DEBUG_MSG("QApplication::exec() returned rc=" << rc << "\n");
	 exit(rc);
      }
      fd_set read_fds;
      FD_ZERO(&read_fds);
      FD_SET(pipe_read, &read_fds);
      int rc=select(pipe_read+1, &read_fds, 0, 0, exit_tv);
      if (rc<0 && errno!=EINTR)
        ThrowError("WorkWithNetscape", "Failed to listen on pipe");
      if (rc==0)
        {
          DEBUG_MSG("Shutdown timeout => exit\n");
          exit(0);
        }
      if (rc>0 && FD_ISSET(pipe_read, &read_fds)) 
        Dispatch();
   }
}

#include "netscape_moc.inc"
