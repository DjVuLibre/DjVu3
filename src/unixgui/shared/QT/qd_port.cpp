//C-  -*- C++ -*-
//C-
//C- DjVu� Unix Viewer (v. 3.5)
//C- 
//C- Copyright � 1999-2001 LizardTech, Inc. All Rights Reserved.
//C- The DjVu Reference Library is protected by U.S. Pat. No.
//C- 6,058,214 and patents pending.
//C- 
//C- This software is subject to, and may be distributed under, the
//C- GNU General Public License, Version 2. The license should have
//C- accompanied the software or you may obtain a copy of the license
//C- from the Free Software Foundation at http://www.fsf.org .
//C- 
//C- The computer code originally released by LizardTech under this
//C- license and unmodified by other parties is deemed the "LizardTech
//C- Original Code."
//C- 
//C- With respect to the LizardTech Original Code ONLY, and subject
//C- to any third party intellectual property claims, LizardTech
//C- grants recipient a worldwide, royalty-free, non-exclusive license
//C- under patent claims now or hereafter owned or controlled by
//C- LizardTech that are infringed by making, using, or selling
//C- LizardTech Original Code, but solely to the extent that any such
//C- patent(s) is/are reasonably necessary to enable you to make, have
//C- made, practice, sell, or otherwise dispose of LizardTech Original
//C- Code (or portions thereof) and not to any greater extent that may
//C- be necessary to utilize further modifications or combinations.
//C- 
//C- The LizardTech Original Code is provided "AS IS" WITHOUT WARRANTY
//C- OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED
//C- TO ANY WARRANTY OF NON-INFRINGEMENT, OR ANY IMPLIED WARRANTY OF
//C- MERCHANTIBILITY OR FITNESS FOR A PARTICULAR PURPOSE.
//C-
// 
// $Id: qd_port.cpp,v 1.5 2001-10-17 19:09:17 docbill Exp $
// $Name:  $


#include "qd_port.h"
#include "debug.h"

#include <ctype.h>

#include "qt_fix.h"

//****************************************************************************
//******************************* QDPort::Port *******************************
//****************************************************************************

bool
QDPort::Port::notify_error(const DjVuPort * source, const GUTF8String &msg)
{
   GMonitorLock lock(&disabled);
   return disabled ? false : port->notify_error(source, msg);
}

bool
QDPort::Port::notify_status(const DjVuPort * source, const GUTF8String &msg)
{
   GMonitorLock lock(&disabled);
   return disabled ? false : port->notify_status(source, msg);
}

void
QDPort::Port::notify_redisplay(const DjVuImage * source)
{
   GMonitorLock lock(&disabled);
   if (!disabled)
      port->notify_redisplay(source);
}

void
QDPort::Port::notify_relayout(const DjVuImage * source)
{
   GMonitorLock lock(&disabled);
   if (!disabled)
      port->notify_relayout(source);
}

void
QDPort::Port::notify_chunk_done(const DjVuPort * source, const GUTF8String &qname)
{
//   const char * const name=qname;
   GMonitorLock lock(&disabled);
   if (!disabled)
      port->notify_chunk_done(source, qname);
}

void
QDPort::Port::notify_file_flags_changed(const DjVuFile * source,
					long set_mask, long clr_mask)
{
   GMonitorLock lock(&disabled);
   if (!disabled)
      port->notify_file_flags_changed(source, set_mask, clr_mask);
}

void
QDPort::Port::notify_doc_flags_changed(const DjVuDocument * source,
				       long set_mask, long clr_mask)
{
   GMonitorLock lock(&disabled);
   if (!disabled)
      port->notify_doc_flags_changed(source, set_mask, clr_mask);
}

void
QDPort::Port::notify_decode_progress(const DjVuPort * source, float done)
{
   GMonitorLock lock(&disabled);
   if (!disabled)
      port->notify_decode_progress(source, done);
}

//****************************************************************************
//********************************* QDPort ***********************************
//****************************************************************************

QDPort::~QDPort(void)
{
   port->disabled=1;
   port=0;
}

QDPort::QDPort(bool _watch_errors, bool _watch_status,
	       QObject * parent=0, const char * name=0) :
      QObject(parent, name),
      watch_errors(_watch_errors), watch_status(_watch_status)
{
   sig_error_on=sig_status_on=sig_redisplay_on=
   sig_relayout_on=sig_chunk_done_on=
   sig_file_flags_changed_on=sig_doc_flags_changed_on=
   sig_decode_progress_on=0;

   port=new Port(this);
   
   connect(&messenger, SIGNAL(sigGeneralMsg(const GUTF8String &)),
	   this, SLOT(slotGeneralMsg(const GUTF8String &)));
}

void
QDPort::connectNotify(const char *s)
{
   const char * ptr=strchr(s, '(');
   int len=ptr-s;
   if (!strncmp(s, SIGNAL(sigNotifyError(const GP<DjVuPort> &, const GUTF8String &)), len))
      sig_error_on=1;
   if (!strncmp(s, SIGNAL(sigNotifyStatus(const GP<DjVuPort> &, const QString &)), len))
      sig_status_on=1;
   if (!strncmp(s, SIGNAL(sigNotifyRedisplay(const GP<DjVuImage> &)), len))
      sig_redisplay_on=1;
   if (!strncmp(s, SIGNAL(sigNotifyRelayout(const GP<DjVuImage> &)), len))
      sig_relayout_on=1;
   if (!strncmp(s, SIGNAL(sigNotifyChunkDone(const GP<DjVuPort> &, const GUTF8String &)), len))
      sig_chunk_done_on=1;
   if (!strncmp(s, SIGNAL(sigNotifyFileFlagsChanged(const GP<DjVuFile> &, long, long)), len))
      sig_file_flags_changed_on=1;
   if (!strncmp(s, SIGNAL(sigNotifyDocFlagsChanged(const GP<DjVuDocument> &, long, long)), len))
      sig_doc_flags_changed_on=1;
   if (!strncmp(s, SIGNAL(sigNotifyDecodeProgress(const GP<DjVuPort> &, float)), len))
      sig_decode_progress_on=1;
}

void
QDPort::disconnectNotify(const char *s)
{
   const char * ptr=strchr(s, '(');
   int len=ptr-s;
   if (!strncmp(s, SIGNAL(sigNotifyError(const GP<DjVuPort> &, const GUTF8String &)), len))
      sig_error_on=0;
   if (!strncmp(s, SIGNAL(sigNotifyStatus(const GP<DjVuPort> &, const QString &)), len))
      sig_status_on=0;
   if (!strncmp(s, SIGNAL(sigNotifyRedisplay(const GP<DjVuImage> &)), len))
      sig_redisplay_on=0;
   if (!strncmp(s, SIGNAL(sigNotifyRelayout(const GP<DjVuImage> &)), len))
      sig_relayout_on=0;
   if (!strncmp(s, SIGNAL(sigNotifyChunkDone(const GP<DjVuPort> &, const GUTF8String &)), len))
      sig_chunk_done_on=0;
   if (!strncmp(s, SIGNAL(sigNotifyFileFlagsChanged(const GP<DjVuFile> &, long, long)), len))
      sig_file_flags_changed_on=0;
   if (!strncmp(s, SIGNAL(sigNotifyDocFlagsChanged(const GP<DjVuDocument> &, long, long)), len))
      sig_doc_flags_changed_on=0;
   if (!strncmp(s, SIGNAL(sigNotifyDecodeProgress(const GP<DjVuPort> &, float)), len))
      sig_decode_progress_on=0;
}

void
QDPort::slotGeneralMsg(const GUTF8String &qmsg)
{
   DEBUG_MSG("QDPort::slotGeneralMsg()\n");
   DEBUG_MAKE_INDENT(3);
   
   const char * msg=qmsg;
   const char * ptr;
   for(ptr=msg;*ptr && !isspace(*ptr);ptr++);
   GUTF8String call_name=GUTF8String(msg, ptr-msg);

   DEBUG_MSG("QDPort::slotGeneralMsg(): call_name=" << call_name << "\n");
   
   while(*ptr && isspace(*ptr)) ptr++;
   msg=ptr;
   while(*ptr && !isspace(*ptr)) ptr++;
   DEBUG_MSG("QDPort::slotGeneralMsg(): source=" << atol(GUTF8String(msg, ptr-msg)) << "\n");
   DjVuPort * source=(DjVuPort *) atol(GUTF8String(msg, ptr-msg));
   GP<DjVuPort> src;
   {
      GCriticalSectionLock lock(&src_lock);
      for(GPosition pos=src_list;pos;++pos)
	 if (src_list[pos]==source)
	 {
	    src=src_list[pos];
	    src_list.del(pos);
	    break;
	 }
   }
   if (src)
   {
      if (call_name=="notify_error")
      {
	 while(*ptr && isspace(*ptr)) ptr++;
	 emit sigNotifyError(src, ptr);
      } else if (call_name=="notify_status")
      {
	 while(*ptr && isspace(*ptr)) ptr++;
	 emit sigNotifyStatus(src, ptr);
      } else if (call_name=="notify_redisplay")
      {
	 emit sigNotifyRedisplay((DjVuImage *) source);
      } else if (call_name=="notify_relayout")
      {
	 emit sigNotifyRelayout((DjVuImage *) source);
      } else if (call_name=="notify_chunk_done")
      {
	 while(*ptr && isspace(*ptr)) ptr++;
	 emit sigNotifyChunkDone(src, ptr);
      } else if (call_name=="notify_file_flags_changed")
      {
	 while(*ptr && isspace(*ptr)) ptr++;
	 long set_mask, clr_mask;
	 sscanf(ptr, "%ld %ld", &set_mask, &clr_mask);
	 //emit sigNotifyFileFlagsChanged((DjVuFile *) source, set_mask, clr_mask);
	 emit sigNotifyFileFlagsChanged((GP<DjVuFile> &)source, set_mask, clr_mask);
      } else if (call_name=="notify_doc_flags_changed")
      {
	 while(*ptr && isspace(*ptr)) ptr++;
	 long set_mask, clr_mask;
	 sscanf(ptr, "%ld %ld", &set_mask, &clr_mask);
	 emit sigNotifyDocFlagsChanged((DjVuDocument *) source, set_mask, clr_mask);
      } else if (call_name=="notify_decode_progress")
      {
	 while(*ptr && isspace(*ptr)) ptr++;
	 emit sigNotifyDecodeProgress(src, atof(ptr));
      } else G_THROW("Internal error: Unknown call name read from the pipe.");
   };
}

bool
QDPort::notify_error(const DjVuPort * source, const GUTF8String &qmsg)
{
   if (sig_error_on && watch_errors)
   {
      {
	 GCriticalSectionLock lock(&src_lock);
	 src_list.append((DjVuPort *) source);
      }
      char src_str[128];
      sprintf(src_str, "%ld", (long) source);
      GUTF8String mesg=GUTF8String("notify_error ")+src_str+" "+qmsg;
      messenger.generalMsg(mesg);
      return true;
   }
   return false;
}

bool
QDPort::notify_status(const DjVuPort * source, const GUTF8String &msg)
{
   if (sig_status_on && watch_status)
   {
      {
	 GCriticalSectionLock lock(&src_lock);
	 src_list.append((DjVuPort *) source);
      }
      char src_str[128];
      sprintf(src_str, "%ld", (long) source);
      GUTF8String mesg=GUTF8String("notify_status ")+src_str+" "+msg;
      messenger.generalMsg(mesg);
      return true;
   }
   return false;
}

void
QDPort::notify_redisplay(const DjVuImage * source)
{
   if (sig_redisplay_on)
   {
      {
	 GCriticalSectionLock lock(&src_lock);
	 src_list.append((DjVuPort *) source);
      }
      GUTF8String src_str;
      src_str.format("notify_redisplay %ld",(long)source);
      messenger.generalMsg(src_str);
   }
}

void
QDPort::notify_relayout(const DjVuImage * source)
{
   if (sig_relayout_on)
   {
      {
	 GCriticalSectionLock lock(&src_lock);
	 src_list.append((DjVuPort *) source);
      }
      GUTF8String src_str;
      src_str.format("notify_relayout %ld",(long)source);
      messenger.generalMsg(src_str);
   }
}

void
QDPort::notify_chunk_done(const DjVuPort * source, const GUTF8String &name)
{
   if (sig_chunk_done_on)
   {
      {
	 GCriticalSectionLock lock(&src_lock);
	 src_list.append((DjVuPort *) source);
      }
      GUTF8String src_str;
      src_str.format("notify_chunk_done %ld %s",(long)source,(const char *)name);
      messenger.generalMsg(src_str);
   }
}

void
QDPort::notify_file_flags_changed(const DjVuFile * source,
				  long set_mask, long clr_mask)
{
   if (sig_file_flags_changed_on)
   {
      {
	 GCriticalSectionLock lock(&src_lock);
	 src_list.append((DjVuPort *) source);
      }
      GUTF8String src_str;
      src_str.format("notify_file_flags_changed %ld %ld %ld", (long) source, set_mask, clr_mask);
      messenger.generalMsg(src_str);
   }
}

void
QDPort::notify_doc_flags_changed(const DjVuDocument * source,
				 long set_mask, long clr_mask)
{
   if (sig_doc_flags_changed_on)
   {
      {
	 GCriticalSectionLock lock(&src_lock);
	 src_list.append((DjVuPort *) source);
      }
      GUTF8String src_str;
      src_str.format("notify_doc_flags_changed %ld %ld %ld", (long) source, set_mask, clr_mask);
      messenger.generalMsg(src_str);
   }
}

void
QDPort::notify_decode_progress(const DjVuPort * source, float done)
{
   if (sig_decode_progress_on)
   {
      {
	 GCriticalSectionLock lock(&src_lock);
	 src_list.append((DjVuPort *) source);
      }
      GUTF8String src_str;
      src_str.format("notify_decode_progress %ld %g",(long)source,done);
      messenger.generalMsg(src_str);
   }
}

