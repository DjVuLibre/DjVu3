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
# $Id: cothread.sh,v 1.12 2001-07-24 17:52:02 bcr Exp $
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

