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
# $Id: dirs.sh,v 1.11 2001-01-09 17:29:22 bcr Exp $
# $Name:  $

# This script sets the variables:
#   TOPSRCDIR, TOPBUILDDIR, CONFIG_CACHE, CONFIG_STATUS, and CONFIG_LOG

if [ -z "$CONFIG_DIR" ] ; then
  echo "You must source functions.sh" 1>&2
  exit 1
fi


if [ -z "$CONFIG_CACHE" ] ; then
  echo "Setting default paths"
  TOPSRCDIR=`cd "${CONFIG_DIR}/.." 1>>/dev/null 2>>/dev/null;"${pwdcmd}"`
  PROJECT=`echo  $PROGRAM_NAME|sed 's,^.*-,,g'`
  s=`${pwdcmd}`
  if [ "$TOPSRCDIR" != "$s" ] ; then
    TOPBUILDDIR="$s"
  else
    if [ -z "$TOPBUILDPREFIX" ] ; then
    TOPBUILDDIR="${s}/build/${SYS}/$PROJECT"
    else
      TOPBUILDDIR="${TOPBUILDPREFIX}/${SYS}/PROJECT"
    fi
  fi
  CONFIG_CACHE="$TOPBUILDDIR"/config.cache
  CONFIG_STATUS="$TOPBUILDDIR"/config.status
  CONFIG_LOG="$TOPBUILDDIR"/config.log
  if [ ! -d "$TOPBUILDDIR" ] ; then
    ${mkdirp} "$TOPBUILDDIR"
  fi
  CONFIG_VARS=`echo TOPSRCDIR PROJECT TOPBUILDDIR CONFIG_LOG CONFIG_STATUS CONFIG_CACHE ${CONFIG_VARS}`
fi
if [ -z "$PROJECT_FILLNAME" ]
then
  case "$PROJECT" in
	plugin)
		PROJECT_FULLNAME="NetscapePlugin";;
	*)
		PROJECT_FULLNAME="$PROJECT" ;;
  esac
fi
