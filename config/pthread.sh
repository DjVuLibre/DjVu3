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
# $Id: pthread.sh,v 1.5 2000-11-02 01:08:33 bcr Exp $
# $Name:  $

# This script sets the variables:
#   CXXPTHREAD,CXXPTHREAD_LIB,CCPTHREAD,CCPTHREAD_LIB

if [ -z "$CONFIG_DIR" ] ; then
  echo "You must source functions.sh" 1>&2
  exit 1
fi

if [ ! -z "$CXX" ]
then
  if [ -z "${CXXPTHREAD}" ]
  then
    echo 'int main(void) {return 0;}' | testfile $temp.cpp
    echon "Checking if ${CXX} -pthread works ... "
    run $CXX $CXXFLAGS -pthread $temp.cpp -o $temp
    CXXPTHREAD="-DTHREADMODEL=POSIXTHREADS"
    CXXPTHREAD_LIB=""
    if [ $? = 0 -a -z "`grep -i unrecognized $temp.out`" ]
    then
      echo yes.
      CXXPTHREAD="$CXXPTHREAD -pthread"
    else
      echo no.
      echon "Checking if ${CXX} -thread works ... "
      run $CXX $CXXFLAGS -threads $temp.cpp -o $temp
      if [ $? = 0 -a -z "`grep -i unrecognized $temp.out`" ]
      then
        echo yes.
        CXXPTHREAD="$CXXPTHREAD -threads"
      else
        echo no.
        CXXPTHREAD="$CXXPTHREAD -D_REENTRANT"
        CXXPTHREAD_LIB="-lpthread"
      fi
    fi
    CONFIG_VARS=`echo CXXPTHREAD CXXPTHREAD_LIB $CONFIG_VARS`
  fi
fi

if [ ! -z "$CC" ]
then
  if [ -z "${CCPTHREAD}" ]
  then
    echo 'int main(void) {return 0;}' | testfile $temp.c
    echon "Checking if ${CC} -pthread works ... "
    run $CC $CCFLAGS -pthread $temp.c -o $temp
    CCPTHREAD="-DTHREADMODEL=POSIXTHREADS"
    CCPTHREAD_LIB=""
    if [ $? = 0 -a -z "`grep -i unrecognized $temp.out`" ]
    then
      echo yes.
      CCPTHREAD="$CCPTHREAD -pthread"
    else
      echo no.
      echon "Checking if ${CC} -threads works ... "
      run $CC $CCFLAGS -threads $temp.cpp -o $temp
      if [ $? = 0 -a -z "`grep -i unrecognized $temp.out`" ]
      then
        echo yes.
        CCPTHREAD="$CCPTHREAD -threads"
      else
        echo no.
        CCPTHREAD="$CCPTHREAD -D_REENTRANT"
        CCPTHREAD_LIB="-lpthread"
      fi
    fi
    CONFIG_VARS=`echo CCPTHREAD CCPTHREAD_LIB $CONFIG_VARS`
  fi
fi

