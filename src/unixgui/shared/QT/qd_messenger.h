//C-  -*- C++ -*-
//C-
//C-  Copyright � 1999-2001, LizardTech, Inc. All Rights Reserved.
//C-              Unauthorized use prohibited.
//C-
// 
// $Id: qd_messenger.h,v 1.1 2001-05-29 22:05:30 bcr Exp $
// $Name:  $


#ifndef HDR_QD_MESSENGER
#define HDR_QD_MESSENGER

#ifdef __GNUC__
#pragma interface
#endif

#include "GThreads.h"
#include "GContainer.h"
#include "debug.h"
#include "GURL.h"

#include <qobject.h>

// This class actually does synchronization between two threads by maintaining
// a pipe and queue of messages. There can be just one queue per application
// QDMessenger will be able to use the same queue as they provide their
// address in every queue message so that the QDMessageQueue knows whom
// to direct a message to.
//
// In some cases it's necessary to look ahead in the queue for other events
// for the same QDMessenger. Here is an explanation "why":
//
// "It's important to do so at least for REDRAW events since usually
// Decoder works pretty fast and manages to decode everything by the
// moment when visualizer finishes redrawing the B&W picture. While
// decoding the decoder sends a bunch of REDRAW requests thru this
// messenger. Unfortunately, if we process it one-by-one then no event
// compression will be observed as QT seems to pay much more attention
// to XEvents (generated by x11Redraw) than to active SocketNotifiers.
// Thus, instead of processing another pipe REDRAW command (and thus
// sending another Expose event, which could be merged with the previous
// one), QT notices Expose events returned from XServer and processes
// it immediately. So we don't let QT return to event loop and do it.
// We process all pending commands at once"
//
// So, there is a "look_ahead" setting for every QDMessenger, that the queue
// may deal with. Every time when smth is read from the pipe, the queue
// senses the QDMessenger which should take the message and, if look_ahead
// is TRUE, it will look thru the queue for other messages for the same
// QDMessenger until all of them are processed.

class QDMessageQueue : public QObject
{
   Q_OBJECT
public:
   class Item
   {
   public:
      Item	* next;

      class QDMessenger	* messenger;
      int	cmd;
      int	req;
      GUTF8String title, msg;
      GUTF8String url, target;

      Item(class QDMessenger * _messenger, int _cmd) :
	    next(0), messenger(_messenger), cmd(_cmd) {};
   };
private:
   int			fd[2];
   GCriticalSection	queue_lock;
   Item			* queue_head;
private slots:
   void		notifierActivated(int);
public:
   void		addQueueItem(Item * item);
   void		doOneQueueItem(void);
   void		clearMessenger(class QDMessenger * messenger);

   QDMessageQueue(QObject * parent=0, const char * name=0);
   virtual ~QDMessageQueue(void);
};

class QDMessenger : public QObject
{
   Q_OBJECT
private:
      // CMD_LAYOUT and CMD_REDRAW could have been implemented via
      // CMD_GENERAL_REQ as they don't require any arguments, but it's
      // nice to have these requests in separate
   enum CMD { SHOW_STATUS=0, LAYOUT, REDRAW,
	      SHOW_ERROR, GET_URL, GENERAL_REQ, GENERAL_MSG };

   static QDMessageQueue	* queue;
   bool		look_ahead;
signals:
   void		sigRedraw(void);
   void		sigLayout(void);
   void		sigShowError(const GUTF8String &, const GUTF8String &);
   void		sigShowStatus(const QString &);
   void		sigGetURL(const GURL & url, const GUTF8String &target);
   void		sigGeneralReq(int);
   void		sigGeneralMsg(const GUTF8String &);
public:
   void		showStatus(const QString &status);
   void		layout(void);
   void		redraw(void);
   void		showError(const GUTF8String &title, const GUTF8String &msg);
   void		getURL(const GURL & url, const GUTF8String &target);
   void		generalReq(int req);
   void		generalMsg(const GUTF8String &msg);

      // Internal: called from QDMessageQueue::notifierActivated()
   void		sendSignal(QDMessageQueue::Item * item);

   void		setLookAhead(bool la) { look_ahead=la; }
   bool		getLookAhead(void) const { return look_ahead; }
   
      // The reason to pass parent here is to provide automatic destruction
      // of the messenger when parent passes away.
      // You may safely pass parent=0 if you don't forget to destroy the
      // messenger yourself
   QDMessenger(QWidget * parent=0, const char * name=0);
   ~QDMessenger(void);
};

#endif
