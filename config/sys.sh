#C-
#C- DjVu® Reference Library (v. 3.0)
#C- 
#C- Copyright © 2000 LizardTech, Inc. All Rights Reserved.
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
#C- 
#
# $Id: sys.sh,v 1.21 2000-11-09 20:15:05 jmw Exp $
# $Name:  $

# This sets the variable SYS INCS JOBJ and DEFS
#

if [ -z "$CONFIG_DIR" ] ; then
  echo "You must source functions.sh" 1>&2
  exit 1
fi

if [ -z "$SYS_SET" ] ; then
  echon "Checking system type ... "
  if [ -z "$SYS" ] ; then 
    SYS=`"${uname}" -s`
  fi
  if [ -z "$PROC" ] ; then 
    PROC=`"${uname}" -p`
  fi
  DEFS="-DUNIX"
  INCS=" "
  JOBJ=" "
  SECURITYMODE="-D_LOCK_ID_"
  SENTINAL_NEED_LIB=""
  SENTINAL=""
  WHOLEARCHIVESEP=" "
  WHOLEARCHIVE="-Wl,--whole-archive"
  NOWHOLEARCHIVE="-Wl,--no-whole-archive"
  LIBDL=""
  LIBC=""
  if [ "$SYS" = "Linux" ] ; then
    if [ -r /usr/lib/libdl.a ] ; then
      LIBDL=/usr/lib/libdl.a
    elif [ -r /lib/libdl.a ] ; then
      LIBDL=/lib/libdl.a
    elif [ -r /usr/lib/libdl.so ] ; then
      LIBDL=/usr/lib/libdl.so
    elif [ -r /lib/libdl.so ] ; then
      LIBDL=/lib/libdl.so
    fi
    if [ -r /lib/libc.so.6 ] ; then
      LIBC="libc.so.6"
      SYS=linux-libc6
    elif [ -r /usr/lib/libc.so.6 ] ; then
      LIBC="libc.so.6"
      SYS=linux-libc6
    else
      SYS=linux-libc5
    fi
    SENTINAL=src/3rd-party/sentinal_lm_60/linux
    RTK="src/3rd-party/rtk3/$SYS"
  elif [ "$SYS" = "SunOS" ] ; then
    s=`"${uname}" -r|"${sed}" 's,\(5.[4-9]\)[.0-9]*,SOLARIS,'`
    WHOLEARCHIVESEP=","
    if [ "x$s" = xSOLARIS ] ; then
      SYS=Solaris
      if [ x$PROC = xi386 ]
      then
        SYS=Solaris-i386
      elif [ x$PROC = xsparc ]
      then
        SYS=Solaris-sparc
        SENTINAL=src/3rd-party/sentinal_lm_60/sol2sprc
	SENTINAL_NEED_LIB="/usr/lib/libsocket.a /usr/lib/libnsl.a"
      fi
    fi
  elif [ "$SYS" = "IRIX64" ] ; then
    WHOLEARCHIVE="-Wl,-all"
    NOWHOLEARCHIVE="-Wl,-notall"
    SENTINAL=src/3rd-party/sentinal_lm_60/irixn32
    strip=""
  fi
  if [ -n "$SENTINAL" ]
  then
    if [ ! -d "${CONFIG_DIR}/../$SENTINAL" ]
    then 
      SENTINAL=""
      SENTINAL_NEED_LIB=""
    fi
  fi
  SYS_SET=true
  echo "$SYS"
  CONFIG_VARS=`echo SYS SYS_SET LIBC LIBDL DEFS INCS JOBJ WHOLEARCHIVE WHOLEARCHIVESEP NOWHOLEARCHIVE SENTINAL SENTINAL_NEED_LIB RTK $CONFIG_VARS`
fi

