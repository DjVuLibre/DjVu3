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
# $Id: cxxpic.sh,v 1.6 2000-11-02 01:08:33 bcr Exp $
# $Name:  $

# This rule sets the following variables:
#	CXXSYMBOLIC, CXXPIC
# $Id: cxxpic.sh,v 1.6 2000-11-02 01:08:33 bcr Exp $

if [ -z "$CXX_SET" ] ; then
  echo "You must source cxx.sh" 1>&2
  exit 1
fi

if [ -z "$CXXPIC_SET" ] ; then
  CXXSYMBOLIC=""
  CXXPIC=""
  echo 'extern "C" {void exit(int);};void foo(void) {exit(0);}' |testfile $temp.cpp
  
  echon "Checking ${CXX} symbolic option ... "
  SYSTEMGXX=`echo $SYS | tr A-Z a-z `-$cxx_is_gcc
  case $SYSTEMGXX in
    linux-*)
      TESTCXXSYMBOLIC="-shared "
      TESTCXXPIC="-fPIC"
      ;;
    solaris-yes)
      if [ -z "$CROSSCOMPILER" ] ; then 
        TESTCXXSYMBOLIC="-shared "
      else
        TESTCXXSYMBOLIC="-shared "
      fi
      TESTCXXPIC="-fPIC"
      ;;
    solaris-*)
      if [ -z "$CROSSCOMPILER" ] ; then 
        TESTCXXSYMBOLIC="-shared "
      else
        TESTCXXSYMBOLIC="-shared "
      fi
      TESTCXXPIC="-fPIC"
      ;;
    irix*-*)
      TESTCXXSYMBOLIC="-shared "
      TESTCXXPIC=""
      ;;
    aix*-*)
      TESTCXXSYMBOLIC="-r "
      TESTCXXPIC="-bM\:SRE"
      ;;
  esac

  check_shared_link_flags CXXSYMBOLIC $temp.cpp "$TESTCXXSYMBOLIC"
  if [ -z "$CXXSYMBOLIC" ] ; then
    echo "none"
  else
    echo "$CXXSYMBOLIC"
  fi
	
  echon "Checking whether ${CXX} $TESTCXXPIC works ... "
  check_compile_flags CXXPIC $temp.cpp $TESTCXXPIC
  if [ $? = 0 ]
  then
    echo yes
  else
    echo no
  fi

  CXXPIC_SET=true
  "${rm}" -rf $temp.cpp $temp.so $temp.o
  CONFIG_VARS=`echo CXXPIC_SET CXXSYMBOLIC CXXPIC $CONFIG_VARS`
fi

