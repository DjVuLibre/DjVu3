#!/bin/ksh
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
# $Id: commands.sh,v 1.15.2.1 2001-03-20 00:29:33 bcr Exp $
# $Name:  $

# This script finds all the commands
#   whence, true, pwdcmd, docxx, ranlib, ln, dirname, basename, mkdir, tar,
#   make, cmp, mv, cp, rm, sed, find, chmod, cat, ar, uname, grep, latex,
#   mkdirp, and tee
# This also sets the variable RULES_DIR
#

if [ -z "$CONFIG_DIR" ]
then
  echo "You must source functions.sh" 1>&2
  exit 1
fi

if [ -z "$RULES_DIR" ]
then
  if ( whence whence 1>>/dev/null 2>>/dev/null )
  then
    whence=whence
  else
    whence=which
  fi

  if ( ${whence} ksh 1>>/dev/null 2>>/dev/null )
  then
    smartshell="`${whence} ksh`"
  elif ( ${whence} bash 1>>/dev/null 2>>/dev/null )
  then
    smartshell="`${whence} bash`"
  else
    smartshell=/bin/sh
  fi

  if ( ${whence} true 1>>/dev/null 2>>/dev/null )
  then
    true="`${whence} true`"
  else
    true="echo>>/dev/null"
  fi

  if ( ${whence} pwd 1>>/dev/null 2>>/dev/null )
  then
    pwdcmd="`${whence} pwd`"
  else
    pwdcmd=pwd
  fi

  if ( ${whence} doc++ 1>>/dev/null 2>>/dev/null )
  then
    docxx="`${whence} doc++`"
  else
    docxx=doc++
  fi

  if ( ${whence} ranlib 1>>/dev/null 2>>/dev/null )
  then
    ranlib="`${whence} ranlib`"
  else
    ranlib="$true"
  fi

  for i in sed sort mkdir
  do
    if ( ${whence} "$i" 1>>/dev/null 2>>/dev/null )
    then
      s=`${whence} "$i"`
      eval "${i}='${s}'"
    else
      eval "${i}='${i}'"
    fi
  done

  if [ -z "$SUPPORTS_MKDIRP" ]
  then
    mkdirp="${CONFIG_DIR}/mkdirp.sh"
  else
    mkdirp="${mkdir} -p"
  fi

  system=`uname | tr A-Z a-z`

  makestlib="${CONFIG_DIR}/make_stlib.sh"
  case "$system" in 
    irix* )
      makeshlib="${CONFIG_DIR}/make_shlib.sh.irix"
      ;;
    * ) 
      makeshlib="${CONFIG_DIR}/make_shlib.sh"
      ;;
  esac

  docxxclean="${CONFIG_DIR}/doc++clean.pl"

  RULES_DIR=`cd ${CONFIG_DIR}/../rules/ 1>>/dev/null 2>>/dev/null;"${pwdcmd}"`
  CONFIG_VARS=`echo RULES_DIR ${CONFIG_VARS}`
  for i in `"${sed}" -n -e 's,^.*=@%\(.*\)%@,\1,p'<"${RULES_DIR}/commands"|"${sort}" -u`
  do
    CONFIG_VARS=`echo $i ${CONFIG_VARS}`
    s='$'"$i"
    eval 's="'"${s}"'"'
    if [ -z "$s" ]
    then
      if ( ${whence} "$i" 1>>/dev/null 2>>/dev/null )
      then
        s=`${whence} "$i"`
        eval "${i}='${s}'"
      else
        eval "${i}='${i}'"
      fi
    fi
  done
  if [ "x`$echo -n`" = "x-n" ]
  then
    ECHO_TYPE=SUFFIX
  else
    ECHO_TYPE=PREFIX
  fi
  ETAIL="@%%@z"
  ECHO="$echo"
  CONFIG_VARS="$CONFIG_VARS ECHO_TYPE ECHO"
fi

 
