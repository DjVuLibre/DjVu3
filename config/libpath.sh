#!/bin/sh
#C-
#C- DjVu® Reference Library (v. 3.0)
#C- 
#C- Copyright © 2000-2001 LizardTech, Inc. All Rights Reserved.
#C- The DjVu Reference Library is protected by U.S. Pat. No.
#C- 6,058,214 and patents pending.
#C- 
#C- This software is subject to, and may be distributed under, the
#C- GNU General Public License, Version 2. The license should have
#C- accompanied the software or you may obtain a copy of the license
#C- from the Free Software Foundation at http://www.fsf.org .
#C- 
#C- The computer code originally released by LizardTech under this
#C- license and unmodified by other parties is deemed the "LizardTech
#C- Original Code."
#C- 
#C- With respect to the LizardTech Original Code ONLY, and subject
#C- to any third party intellectual property claims, LizardTech
#C- grants recipient a worldwide, royalty-free, non-exclusive license
#C- under patent claims now or hereafter owned or controlled by
#C- LizardTech that are infringed by making, using, or selling
#C- LizardTech Original Code, but solely to the extent that any such
#C- patent(s) is/are reasonably necessary to enable you to make, have
#C- made, practice, sell, or otherwise dispose of LizardTech Original
#C- Code (or portions thereof) and not to any greater extent that may
#C- be necessary to utilize further modifications or combinations.
#C- 
#C- The LizardTech Original Code is provided "AS IS" WITHOUT WARRANTY
#C- OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED
#C- TO ANY WARRANTY OF NON-INFRINGEMENT, OR ANY IMPLIED WARRANTY OF
#C- MERCHANTIBILITY OR FITNESS FOR A PARTICULAR PURPOSE.
#
# $Id: libpath.sh,v 1.7 2001-01-09 17:29:22 bcr Exp $
# $Name:  $

# This sets the variable SYS and DEFS
#

if [ -z "$CONFIG_DIR" ] ; then
  echo "You must source functions.sh" 1>&2
  exit 1
fi

if [ -z "$LIBPATHS" ]
then
  LIBPATHS=/usr/X11R6/lib:/usr/openwin/lib:/usr/lib:/lib
  if [ ! -z "$prefix" ] ; then
    LIBPATHS="$prefix/lib:$LIBPATHS"
  fi
  if [ ! -z "$PROJECT_PREFIX" ] ; then
    LIBPATHS="$PROJECT_PREFIX/lib:$LIBPATHS"
  fi
  if [ ! -z "$LD_LIBRARY_PATH" ] ; then
    LIBPATHS="$LD_LIBRARY_PATH:$LIBPATHS"
  fi
  if [ -r /etc/ld.so.conf ] ; then
    for i in `grep -v '^ *$' < /etc/ld.so.conf`; do
      if [ -d "$i" ] ; then
        LIBPATHS="$LIBPATHS:$i"
      fi
    done
  fi
  LIBPATHS=`escape "$LIBPATHS"|sed -e 's,:, ,g'`
  INCPATHS=`echo "$LIBPATHS"|sed -e 's,$,:,g' -e 's,lib:,include:,g' -e 's,:$,,g'`
  SOPATHS=`echo "$LIBPATHS"|sed -e 's,:/usr/openwin/lib:,:,g'`
  CONFIG_VARS=`echo LIBPATHS INCPATHS SOPATHS $CONFIG_VARS`
fi

