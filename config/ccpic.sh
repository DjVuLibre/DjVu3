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
# $Id: ccpic.sh,v 1.6 2000-11-02 01:08:33 bcr Exp $
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

