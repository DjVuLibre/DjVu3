//C-  -*- C++ -*-
//C-
//C-  Copyright (c) 1998 AT&T	
//C-  All Rights Reserved 
//C-
//C-  THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
//C-  The copyright notice above does not evidence any
//C-  actual or intended publication of such source code.
//C-
//C-  $Id: GContainer.cpp,v 1.3 1999-02-11 14:33:11 leonb Exp $


/* Put this into *one* file, which instantiates all the required containers
#ifdef __GNUC__
#pragma implementation
#endif
*/


#include "GContainer.h"

// Implemetation of class GCONTAINERBASE

GContainerBase::~GContainerBase()
{
}

void 
GContainerBase::first(GPosition &pos) const 
{ 
  pos=firstpos(); 
}

void 
GContainerBase::last(GPosition &pos) const 
{ 
  pos=lastpos(); 
}

