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
# $Id: cc.sh,v 1.27 2001-07-24 17:52:02 bcr Exp $
# $Name:  $

# This rule sets the following variables:
#	CC, CCFLAGS, CCWARN, CCUNROLL

if [ -z "$CONFIG_DIR" ]
then
  echo "You must source functions.sh" 1>&2
  exit 1
fi

EGCS="egcs"
if [ -z "$CC_SET" ]
then
  (echo '#include <stdio.h>';echo 'int main(void) {puts("Hello World\n");return 0;}')|testfile $temp.c
  CCPIC_SET=""
  CCFLAGS=""
  CCSYMBOLIC=""
  CCPIC=""
  cc_is_gcc=""
  echon "Searching for C compiler ... "
  if [ -n "$CC" ] ; then
    if ( run "$CC" -c $temp.c ) ; then
      echo "$CC"
    else
      CC=""
    fi
  fi
  if [ -z "$CC" ]
  then
    if ( run "$EGCS" -c $temp.c ) ; then
      CC="$EGCS"
    elif ( run gcc -c $temp.c ) ; then
      CC=gcc
    elif ( run cc -c $temp.c ) ; then
      CC=cc
    elif ( run CC -c $temp.c ) ; then
      CC=CC
    else
      echo "none available"
      echo "Error: Can't find a C compiler" 1>&2
      exit 1
    fi
    echo "$CC"
  fi

  echon "Checking whether ${CC} -pipe works ... "
  if ( run $CC ${CCFLAGS} -pipe -c $temp.c )
  then
    echo yes
    CCPIPE="-pipe"
  else
    CCPIPE=""
    echo no
  fi
  CCFLAGS=`echo "${CCPIPE}" "${CCFLAGS}"`
  
  CCMMX=""
  m=`${uname} -m`
  if [ "${m}" = i686 ]
  then
    echon "Chesking whether ${CC} -mpentiumpro and -mmx work ... "
    if ( run $CC ${CCFLAGS} -mpentiumpro -c $temp.c) 
    then
      CCMMX="-mpentiumpro"
      if ( run $CC ${CCFLAGS} ${CCMMX} -mmx -c $temp.c )
      then
        CCMMX="$CCMMX -mmx"
        echo "yes, both work"
      else
        echo "just $CCMMX works"
      fi
    else
      echo no
    fi
    CCFLAGS=`echo "${CCMMX}" "${CCFLAGS}"`
  fi

  echon "Checking whether ${CC} is gcc ... "
  echo 'int main(void) { return __GNUC__;}' | testfile $temp.c
  CCOPT=""
  CCUNROLL=""
  CCWARN=""
  if ( run $CC $CCFLAGS -c $temp.c ) 
  then
    echo yes
    cc_is_gcc=yes
    CCWARN="-Wall"
    echon "Checking whether ${CC} -O3 works ... "
    if ( run $CC ${CCFLAGS} -O3 -c $temp.c )
    then
      echo yes
      CCOPT="-O3"
    else
      echo no
    fi
    echon "Checking whether ${CC} -funroll-loops works ... "
    if ( run $CC ${CCFLAGS} ${CCOPT} -funroll-loops -c $temp.c )
    then
      echo yes
      CCUNROLL="-funroll-loops"
    else
      echo no
    fi
  else
     echo no
  fi
  if [ -z "$CCOPT" ]
  then
    echon "Checking whether ${CC} -O works ... "
    if ( run $CC ${CCFLAGS} -O -c $temp.c )
    then
      echo yes
      CCOPT="-O"
    else
      echo no
      CCOPT=""
    fi
  fi

  "${rm}" -rf $temp.c $temp.o $temp.so
  CC_SET=true
  CONFIG_VARS=`echo CC CC_SET CCFLAGS CCOPT CCWARN CCUNROLL cc_is_gcc $CONFIG_VARS`
fi

