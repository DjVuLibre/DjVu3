//C-  -*- C++ -*-
//C-
//C-  Copyright © 1999-2001, LizardTech, Inc. All Rights Reserved.
//C-              Unauthorized use prohibited.
//C-
// 
// $Id: netscape.h,v 1.1 2001-05-29 22:05:29 bcr Exp $
// $Name:  $

 
#ifndef HDR_NETSCAPE
#define HDR_NETSCAPE

#ifdef __GNUC__
#pragma interface
#endif

#include <time.h>

// In 5 minutes of inactivity the program will exit. IN MILLISECONDS.
#define INACTIVITY_TIMEOUT	5*60*1000

extern int		pipe_read, pipe_write;
extern int		rev_pipe;
extern int		pipe_str_read, pipe_str_write;
extern struct timeval	* exit_tv;

void WorkWithNetscape(void);
void PipesAreDead(void);

#endif
