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
# $Id: dlopen.sh,v 1.3 2001-08-24 00:05:47 docbill Exp $
# $Name:  $

# This script sets the variables:
#   CXXPTHREAD,CXXPTHREAD_LIB,CCPTHREAD,CCPTHREAD_LIB

if [ -z "$CXX_SET" ] ; then
  echo "You must source cxx.sh" 1>&2
  exit 1
fi

if [ -z "${DLOPEN_TEST}" ]
then
  DLOPEN_TEST=true
  (echo '#include <dlfcn.h>'
  echo '#include <unistd.h>'
  echo 'int main(int argc,char *[],char *[]){dlopen("foo",RTLD_LAZY);exit(0);}')|testfile $temp.cpp
  echon "Checking for dlopen in the dlfcn.h header file ... "
  check_compile_flags HAS_DLOPEN $temp.cpp -DHAS_DLOPEN=1
  if [ $? = 0 ]
  then
    echo yes
    add_defs HAS_DLOPEN 1
    search_for_library dl dlopen /usr/lib/libdl.a /lib/libdl.a /usr/lib/libdl.so /lib/libdl.so -ldl
    LIBDL="$libdl"
  else
    add_undefs HAS_DLOPEN
    echo no
  fi
  CONFIG_VARS=`echo LIBDL DLOPEN_TEST $CONFIG_VARS`
#  if [ -z "${CXXPTHREAD}" ]
#  then
#    echo 'int main(void) {return 0;}' | testfile $temp.cpp
#    echon "Checking if ${CXX} -pthread works ... "
#    run $CXX $CXXFLAGS -pthread $temp.cpp -o $temp
#    CXXPTHREAD="-DTHREADMODEL=POSIXTHREADS"
#    CXXPTHREAD_LIB=""
#    if [ $? = 0 -a -z "`grep -i unrecognized $temp.out`" ]
#    then
#      echo yes.
#      CXXPTHREAD="$CXXPTHREAD -pthread"
#    else
#      echo no.
#      echon "Checking if ${CXX} -thread works ... "
#      run $CXX $CXXFLAGS -threads $temp.cpp -o $temp
#      if [ $? = 0 -a -z "`grep -i unrecognized $temp.out`" ]
#      then
#        echo yes.
#        CXXPTHREAD="$CXXPTHREAD -threads"
#      else
#        echo no.
#        CXXPTHREAD="$CXXPTHREAD -D_REENTRANT"
#        CXXPTHREAD_LIB="-lpthread"
#      fi
#    fi
#    CONFIG_VARS=`echo CXXPTHREAD CXXPTHREAD_LIB $CONFIG_VARS`
#  fi
fi

