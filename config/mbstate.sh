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
# $Id: mbstate.sh,v 1.1 2001-08-23 23:45:23 docbill Exp $
# $Name:  $

# This script sets the variables:
#   CXXPTHREAD,CXXPTHREAD_LIB,CCPTHREAD,CCPTHREAD_LIB

if [ -z "$CXX_SET" ] ; then
  echo "You must source cxx.sh" 1>&2
  exit 1
fi

if [ -z "${WCHAR_TEST}" ]
then
  WCHAR_TEST=true
  (echo '#include <wchar.h>'
  echo 'wchar_t *dummy;'
  )|testfile $temp.cpp
  echon "Checking for wchar.h ... "
  check_compile_flags HAS_WCHAR $temp.cpp -DHAS_WCHAR=1
  if [ $? = 0 ]
  then
    echo yes
    add_defs HAS_WCHAR 1
    (echo '#include <wchar.h>'
    echo 'int main(int argc,char *[],char *[])'
    echo '{mbstate_t ps;exit(mbrtowc(0,0,0,&ps));}'
    )|testfile $temp.cpp
    echon "Checking for mbstate_t in the wchar.h header file ... "
    check_compile_flags HAS_MBSTATE $temp.cpp -DHAS_MBSTATE=1
    if [ $? = 0 ]
    then
      echo yes
      add_defs HAS_MBSTATE 1
    else
      add_undefs HAS_MBSTATE
      echo no
    fi
  else
    add_undefs HAS_WCHAR
    echo no
  fi
  CONFIG_VARS=`echo WCHAR_TEST $CONFIG_VARS`
fi

