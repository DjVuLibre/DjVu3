#!/bin/ksh
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
# $Id: cxx.sh,v 1.29 2001-03-30 23:31:21 bcr Exp $
# $Name:  $

# This rule sets the following variables:
#	CXX, CXXFLAGS, CXXSYMBOLIC, CXXPIC, CXXUNROLL, CXXWARN
# $Id: cxx.sh,v 1.29 2001-03-30 23:31:21 bcr Exp $

if [ -z "$CONFIG_DIR" ]
then
  echo "You must source functions.sh" 1>&2
  exit 1
fi

ECXX="eg++"
if [ -z "$CXX_SET" ]
then
  echo 'extern "C" {void exit(int);};void foo(void) {exit(0);}' |testfile $temp.cpp
  
  CXXFLAGS=""
  CXXPIC_SET=""
  cxx_is_gcc=
  echon "Searching for C++ compiler ... "
  if [ -n "$CXX" ]
  then
    if ( run $CXX -c $temp.cpp )
    then
      echo "$CXX"
    else
      CXX=""
    fi
  fi
  if [ -z "$CXX" ]
  then  
    if ( run $EGXX -c $temp.cpp )
    then
        CXX="$EGXX"
    elif ( run c++ -c $temp.cpp )
    then
      CXX=c++
    elif ( run g++ -c $temp.cpp )
    then
      CXX=g++
    elif ( run CC -c $temp.cpp )
    then
      CXX=CC
    else 
      echo "none available"
      echo "Error: Can't find a C++ compiler" 1>&2
      exit 1
    fi
    echo "$CXX"
  fi

  echon "Checking if ${CXX} -pipe works ... "
  CXXPIPE=""
  if ( run $CXX ${CXXFLAGS} -pipe -c $temp.cpp ) ; then
    CXXPIPE="-pipe"
    echo yes
  else
    echo no
  fi
  CXXFLAGS=`echo "${CXXPIPE}" "${CXXFLAGS}"`

  CXXMMX=""
  if [ `uname -m` = i686 ]
  then
    echon "Checking ${CXX} supports pentium optimizations ... "
    check_compile_flags CXXMMX $temp.cpp "-mpentiumpro -mmx" "-mpentiumpro"
    if [ -z "$CXXMMX" ] ; then
      echo "none"
    else
      echo "$CXXMMX"
      CXXFLAGS="${CXXMMX} ${CXXFLAGS}"
    fi
  fi

  echon "Checking whether ${CXX} is gcc ... "
  echo 'int main(void) { return __GNUG__;}' | testfile $temp.cpp
  CXXOPT=""
  CXXUNROLL=""
  CXXWARN=""
  if ( run $CXX $CXXFLAGS -c $temp.cpp ) 
  then
    echo yes
    cxx_is_gcc=yes
    CXXWARN="-Wall"
    echon "Checking whether ${CXX} -O3 works ... "
    if ( run $CXX ${CXXFLAGS} -O3 -c $temp.cpp ) ; then
      echo yes
      CXXOPT="-O3"
    else
      echo no
    fi
    echon "Checking whether ${CXX} -funroll-loops works ... "
    if ( run $CXX ${CXXFLAGS} ${CXXOPT} -funroll-loops -c $temp.cpp )
    then
      echo yes
      CXXUNROLL="-funroll-loops"
    else
      echo no
    fi
  else
    echo no
  fi
  if [ -z "$CXXOPT" ] ; then
    echon "Checking whether ${CXX} -O works ... "
    if ( run $CXX ${CXXFLAGS} -O -c $temp.cpp ) ; then
      echo yes
      CXXOPT="-O"
    else
      echo no
      CXXOPT=""
    fi
  fi

  CXX_SET=true
  "${rm}" -rf $temp.cpp $temp.so $temp.o
  CONFIG_VARS=`echo CXX_SET CXX CXXFLAGS CXXOPT CXXUNROLL CXXWARN CXXSYMBOLIC cxx_is_gcc $CONFIG_VARS`
fi

