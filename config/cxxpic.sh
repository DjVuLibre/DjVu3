#C-
#C- DjVu® Reference Library (v. 3.0)
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
# $Id: cxxpic.sh,v 1.8 2001-01-04 22:04:54 bcr Exp $
# $Name:  $

# This rule sets the following variables:
#	CXXSYMBOLIC, CXXPIC
# $Id: cxxpic.sh,v 1.8 2001-01-04 22:04:54 bcr Exp $

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

