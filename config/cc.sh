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
# $Id: cc.sh,v 1.22 2000-11-02 01:08:33 bcr Exp $
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

