//C-  -*- C++ -*-
//C-
//C-  Copyright © 1999-2001, LizardTech, Inc. All Rights Reserved.
//C-              Unauthorized use prohibited.
//C-
// 
// $Id: qd_thr_yielder.cpp,v 1.1 2001-05-29 22:05:31 bcr Exp $
// $Name:  $


#ifdef __GNUC__
#pragma implementation
#endif

#include "qd_thr_yielder.h"
#include "GThreads.h"
#include "GContainer.h"
#include "debug.h"
#include "throw_error.h"

#include <qsocketnotifier.h>
#include <qtimer.h>
#include <qobject.h>
#include <unistd.h>
#include <fcntl.h>

Helper	*	QDThrYielder::helper=0;
int		QDThrYielder::tasks=1;

//*****************************************************************************
//********************************* Static ************************************
//*****************************************************************************

class Static
{
public:
   Static(void)
   {
      GThread::set_scheduling_callback(QDThrYielder::schedulingCB);
   }
};

static Static s;

//*****************************************************************************
//********************************* Helper ************************************
//*****************************************************************************

class Helper : public QObject
{
   Q_OBJECT
private:
   GMap<int, void *>	read_sockets;	// GMap<int, QSocketNotifier *> in fact
   GMap<int, void *>	write_sockets;	// GMap<int, QSocketNotifier *> in fact
   GMap<int, void *>	except_sockets;	// GMap<int, QSocketNotifier *> in fact
   
   QTimer	timer;
private slots:
   void		slotTimeout(void);
   void		slotSocketNotifier(int);
public:
   void		setHooks(bool wait);
   
   Helper(QObject * parent=0, const char * name=0);
   ~Helper(void);
};

Helper::Helper(QObject * parent, const char * name) :
      QObject(parent, name)
{
   connect(&timer, SIGNAL(timeout(void)), this, SLOT(slotTimeout(void)));

   setHooks(false);
}

Helper::~Helper(void)
{
   GPosition pos;
   for(pos=read_sockets;pos;++pos)
      delete (QSocketNotifier *) read_sockets[pos];
   for(pos=write_sockets;pos;++pos)
      delete (QSocketNotifier *) write_sockets[pos];
   for(pos=except_sockets;pos;++pos)
      delete (QSocketNotifier *) except_sockets[pos];
}

void
Helper::setHooks(bool wait)
{
   if (!wait) timer.start(0, TRUE);
   else
   {
	 // Now check all file descriptors:
	 // Since QT is not using GThread::select(), it has no idea that other threads
	 // may have selected some file descriptors for reading/writing/etc. Here we're
	 // kind of fixing this feature with QSocketNotifiers.
      int nfds;
      unsigned long timeout;
      fd_set read_fds, write_fds, except_fds;
      GThread::get_select(nfds, &read_fds, &write_fds, &except_fds, timeout);
      if (timeout>0 && nfds>0)
      {
	 for(int i=0;i<nfds;i++)
	 {
	    GPosition pos;
	    if (FD_ISSET(i, &read_fds))
	    {
	       if (!read_sockets.contains(i))
	       {
		  QSocketNotifier * sn=new QSocketNotifier(i, QSocketNotifier::Read);
		  connect(sn, SIGNAL(activated(int)), this, SLOT(slotSocketNotifier(int)));
		  read_sockets[i]=sn;
	       }
	    } else if (read_sockets.contains(i, pos))
	    {
	       delete (QSocketNotifier *) read_sockets[pos];
	       read_sockets.del(pos);
	    }

	    if (FD_ISSET(i, &write_fds))
	    {
	       if (!write_sockets.contains(i))
	       {
		  QSocketNotifier * sn=new QSocketNotifier(i, QSocketNotifier::Write);
		  connect(sn, SIGNAL(activated(int)), this, SLOT(slotSocketNotifier(int)));
		  write_sockets[i]=sn;
	       }
	    } else if (write_sockets.contains(i, pos))
	    {
	       delete (QSocketNotifier *) write_sockets[pos];
	       write_sockets.del(pos);
	    }

	    if (FD_ISSET(i, &except_fds))
	    {
	       if (!except_sockets.contains(i))
	       {
		  QSocketNotifier * sn=new QSocketNotifier(i, QSocketNotifier::Exception);
		  connect(sn, SIGNAL(activated(int)), this, SLOT(slotSocketNotifier(int)));
		  except_sockets[i]=sn;
	       }
	    } else if (except_sockets.contains(i, pos))
	    {
	       delete (QSocketNotifier *) except_sockets[pos];
	       except_sockets.del(pos);
	    }
	 } // for(i=0;i<nfds;i++)
      } // if (timeout>0 && nfds>0)

      timer.start(timeout, TRUE);
   }
}

void
Helper::slotTimeout(void)
{
   setHooks(GThread::yield()!=0);
}

void
Helper::slotSocketNotifier(int)
{
   setHooks(GThread::yield()!=0);
}

//*****************************************************************************
//****************************** QDThrYielder **********************************
//*****************************************************************************

void
QDThrYielder::schedulingCB(int cmd)
{
   if (cmd==GThread::CallbackTerminate) tasks--;
   else
   {
      if (cmd==GThread::CallbackCreate) tasks++;
      if (helper) helper->setHooks(false);
   }
}

bool
QDThrYielder::isInitialized(void)
{
   return helper!=0;
}

void
QDThrYielder::initialize(void)
{
      // Note, that QT should be initialized by this moment.
   if (!helper) helper=new Helper(0, "qd_thr_yielder_helper");
}

int
QDThrYielder::getTasksNum(void)
{
   return tasks;
}

#include "qd_thr_yielder_moc.inc"
