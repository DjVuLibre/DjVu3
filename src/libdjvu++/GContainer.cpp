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
//C- $Id: GContainer.cpp,v 1.6 1999-03-17 19:24:57 leonb Exp $


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

