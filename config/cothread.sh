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
# $Id: cothread.sh,v 1.7 2000-11-02 01:08:33 bcr Exp $
# $Name:  $

# This script sets the variables:
#   CXXCOTHREAD_TEST,CXXCOTHREAD_TEST,CXXCOTHREAD,CXXCOTHREAD_UNSAFE,
#   CCCOTHREAD_TEST,CCCOTHREAD,CCCOTHREAD_UNSAFE

if [ -z "$CONFIG_DIR" ] ; then
  echo "You must source functions.sh" 1>&2
  exit 1
fi

if [ ! -z "$CXX" ]
then
  if [ -z "${CXXCOTHREAD_TEST}" ]
  then
    CXXCOTHREAD=""
    CXXCOTHREAD_UNSAFE=""
    if [ ! -z "$cxx_is_gcc" ] 
    then
      CXXCOTHREAD="-DTHREADMODEL=COTHREADS"
      echon "Testing exception handler patch for ${CXX} ... "
      testfile $temp.cpp <<\EOF
extern "C" void *(*__get_eh_context_ptr)(void);
extern "C" void *__new_eh_context(void);
void main() { __get_eh_context_ptr = &__new_eh_context; }
EOF
      if ( run "$CXX" $CXXFLAGS $temp.cpp -o $temp ) 
      then
        echo yes.
      else
        echo no.
        CXXCOTHREAD="$CXXCOTHREAD -DNO_LIBGCC_HOOKS"
        CXXCOTHREAD_UNSAFE=true
      fi
    fi
    CXXCOTHREAD_TEST=tested
    CONFIG_VARS=`echo CXXCOTHREAD_TEST CXXCOTHREAD CXXCOTHREAD_UNSAFE $CONFIG_VARS`
  fi
fi

if [ ! -z "$CC" ]
then
  if [ -z "${CCCOTHREAD_TEST}" ]
  then
    CCCOTHREAD=""
    CCCOTHREAD_UNSAFE=""
    if [ ! -z "$cc_is_gcc" ] 
    then
      CCCOTHREAD="-DTHREADMODEL=COTHREADS"
      echon "Testing exception handler patch for ${CC} ... "
      testfile $temp.c <<\EOF
void *(*__get_eh_context_ptr)(void);
void *__new_eh_context(void);
void main() { __get_eh_context_ptr = &__new_eh_context; }
EOF
      if ( run $CC $CCFLAGS $temp.c -o $temp ) 
      then
        echo yes.
      else
        echo no.
        CCCOTHREAD="$CCCOTHREAD -DNO_LIBGCC_HOOKS"
        CCCOTHREAD_UNSAFE=true
      fi
    fi
    CCCOTHREAD_TEST=tested
    CONFIG_VARS=`echo CCCOTHREAD_TEST CCCOTHREAD CCCOTHREAD_UNSAFE $CONFIG_VARS`
  fi
fi

