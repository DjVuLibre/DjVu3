#!/bin/ksh
#C-
#C- DjVu® Reference Library (v. 3.5)
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
# $Id: ccpic.sh,v 1.11 2001-07-24 17:52:02 bcr Exp $
# $Name:  $

# This rule sets the following variables:
#	CCSYMBOLIC, CCPIC

if [ -z "$CC_SET" ]
then
  echo "You must source cc.sh" 1>&2
  exit 1
fi

EGCS="egcs"
if [ -z "$CCPIC_SET" ]
then
  CCSYMBOLIC=""
  CCPIC=""
  (echo '#include <stdio.h>';echo 'int main(void) {puts("Hello World\n");return 0;}')|testfile $temp.c

  echon "Checking ${CC} symbolic option ... "
  CCSYMBOLIC=""
#  s=`( cd "$tempdir" 2>>/dev/null;${CC} ${CCFLAGS} -symbolic -c $temp.c 2>&1)|"${grep}" 'unrecognized option'`
#  if [ -z "$s" ]
#  then
#    CCSYMBOLIC='-symbolic'
#  else 
    SYSTEMGCC=`echo $SYS | tr A-Z a-z `-$cc_is_gcc
    case $SYSTEMGCC in
      linux-*) 
        TESTCCSYMBOLIC="-shared"
        TESTCCPIC="-fPIC"
        ;;
      solaris-*-yes)
        if [ -z "$CROSSCOMPILER" ] ; then 
          TESTCCSYMBOLIC="-shared -L/usr/lib -R/usr/lib "
        else
          TESTCCSYMBOLIC="-shared -Wl,-rpath,/usr/lib:/usr/ccs/lib:/usr/openwin/lib "
        fi
        TESTCCPIC="-fPIC"
        ;;
      solaris-*)
        if [ -z "$CROSSCOMPILER" ] ; then 
          TESTCCSYMBOLIC="-G -L/usr/lib -R/usr/lib "
        else
          TESTCCSYMBOLIC="-shared -Wl,-rpath,/usr/lib:/usr/ccs/lib:/usr/openwin/lib "
        fi
        TESTCCPIC="-K PIC"
        ;;
      irix*-*) 
        TESTCCSYMBOLIC="-shared"
        TESTCCPIC=""
        ;;
      aix*-*) 
        TESTCCSYMBOLIC="-r"
        TESTCCPIC="-bM\:SRE"
	;;
     esac

    check_shared_link_flags CCSYMBOLIC $temp.c "$TESTCCSYMBOLIC"
#  fi
  if [ -z "$CCSYMBOLIC" ]
  then
    echo "none"
  else
    echo "$CCSYMBOLIC"
  fi

  echon "Checking whether ${CC} ${TESTCCPIC} works ... "
  if ( run $CC ${CCFLAGS} $TESTCCPIC -c $temp.c )
  then
    CCPIC=$TESTCCPIC
    echo yes
  else
    echo no
  fi

  "${rm}" -rf $temp.c $temp.o $temp.so
  CCPIC_SET=true
  CONFIG_VARS=`echo CCPIC_SET CCSYMBOLIC CCPIC $CONFIG_VARS`
fi

