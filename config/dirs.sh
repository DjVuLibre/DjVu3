#C- -*- C -*-
#C-
#C- DjVu® Reference Library (v. 3.0)
#C- 
#C- Copyright © 2000 LizardTech, Inc. All Rights Reserved.
#C- 
#C- This software (the "Original Code") is subject to, and may be
#C- distributed under, the GNU General Public License, Version 2.
#C- The license should have accompanied the Original Code or you
#C- may obtain a copy of the license from the Free Software
#C- Foundation at http://www.fsf.org .
#C- 
#C- With respect to the Original Code, and subject to any third
#C- party intellectual property claims, LizardTech grants recipient
#C- a worldwide, royalty-free, non-exclusive license under patent
#C- claims infringed by making, using, or selling Original Code
#C- which are now or hereafter owned or controlled by LizardTech,
#C- but solely to the extent that any such patent is reasonably
#C- necessary to enable you to make, have made, practice, sell, or 
#C- otherwise dispose of Original Code (or portions thereof) and
#C- not to any greater extent that may be necessary to utilize
#C- further modifications or combinations.
#C- 
#C- The Original Code is provided "AS IS" WITHOUT WARRANTY OF ANY
#C- KIND, EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO
#C- ANY WARRANTY OF NON-INFRINGEMENT, OR ANY IMPLIED WARRANTY OF 
#C- MERCHANTIBILITY OF FITNESS FOR A PARTICULAR PURPOSE.
#
# $Id: dirs.sh,v 1.8 2000-11-02 01:08:33 bcr Exp $
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
