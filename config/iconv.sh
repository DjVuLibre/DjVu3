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
# $Id: iconv.sh,v 1.4 2001-08-24 15:40:30 docbill Exp $
# $Name:  $

# This script sets the variables:
#   CXXPTHREAD,CXXPTHREAD_LIB,CCPTHREAD,CCPTHREAD_LIB

if [ -z "$CXX_SET" ] ; then
  echo "You must source cxx.sh" 1>&2
  exit 1
fi

if [ -z "${ICONV_TEST}" ]
then
  ICONV_TEST=true
  (echo '#include <iconv.h>'
  echo '#include <unistd.h>'
  echo '#include <stdlib.h>'
  echo 'int main(int argc,char *[],char *[])'
  echo '{iconv_t cv=iconv_open("UTF-8","UTF-8");exit(0);}'
  )|testfile $temp.cpp
  echon "Checking for iconv in the iconv.h header file ... "
  check_compile_flags HAS_ICONV $temp.cpp -DHAS_ICONV=1
  if [ $? = 0 ]
  then
    echo yes
    (echo '#include <iconv.h>'
     echo 'int main(int argc,char *[],char *[])'
     echo '{iconv_t cv;char *a;size_t i;iconv(cv,&a,&i,&a,&i);;exit(0);}'
    )|testfile $temp.cpp
    check_compile_flags HAS_ICONV $temp.cpp -DHAS_ICONV=2
    if [ $? = 0 ]
    then
      add_defs HAS_ICONV 2
    else
      add_defs HAS_ICONV 1
    fi
  else
    add_undefs HAS_ICONV
    echo no
  fi
  CONFIG_VARS=`echo ICONV_TEST $CONFIG_VARS`
fi

