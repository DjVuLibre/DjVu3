//C-  -*- C++ -*-
//C-
//C-  Copyright © 1999-2001, LizardTech, Inc. All Rights Reserved.
//C-              Unauthorized use prohibited.
//C-
// 
// $Id: io.h,v 1.1 2001-08-08 17:38:05 docbill Exp $
// $Name:  $


#ifndef HDR_IO
#define HDR_IO

#ifdef __GNUC__
#pragma interface
#endif

#include "GString.h"
#include "Arrays.h"

#ifndef PLUGIN
#include "exc_sys.h"
#endif

#define OK_STRING	"OK"
#define ERR_STRING	"ERR"

#define CMD_SHUTDOWN		0
#define CMD_NEW			1
#define CMD_DETACH_WINDOW	2
#define CMD_ATTACH_WINDOW	3
#define CMD_RESIZE		4
#define CMD_DESTROY		5
#define CMD_PRINT		6
#define CMD_NEW_STREAM		7
#define CMD_WRITE		8
#define CMD_DESTROY_STREAM	9
#define CMD_SHOW_STATUS		10
#define CMD_GET_URL		11
#define CMD_GET_URL_NOTIFY	12
#define CMD_URL_NOTIFY		13
#define CMD_HANDSHAKE		14

#ifndef PLUGIN
DECLARE_EXCEPTION(PipeError, "PipeError", SystemError);
#define PIPE_ERROR(func, msg) PipeError(func, msg, 0, __FILE__, __LINE__)
#endif

void	WriteString(int fd, const char * str);
void	WriteInteger(int fd, int var);
void	WriteDouble(int fd, double var);
void	WritePointer(int fd, const void * ptr);
void	WriteArray(int fd, const TArray<char> & array);

// Note: refresh_cb() works only for the plugin (-DPLUGIN)
// Otherwise it affects nothing.
GUTF8String 	ReadString(int fd, int refresh_pipe=-1, void (* refresh_cb)(void)=0);
int     	ReadInteger(int fd, int refresh_pipe=-1, void (* refresh_cb)(void)=0);
double		ReadDouble(int fd, int refresh_pipe=-1, void (* refresh_cb)(void)=0);
void *  	ReadPointer(int fd, int refresh_pipe=-1, void (* refresh_cb)(void)=0);
TArray<char>	ReadArray(int fd, int refresh_pipe=-1, void (* refresh_cb)(void)=0);
void            ReadResult(int fd, const char * cmd_name, 
                           int refresh_pipe=-1, void (* refresh_cb)(void)=0);

#endif
