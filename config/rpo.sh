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
# $Id: rpo.sh,v 1.4 2000-11-02 01:08:33 bcr Exp $
# $Name:  $

# This script sets the variables:
#   CXXRPO_TEST,CXXRPO,CCRPO_TEST,CCRPO

if [ -z "$CONFIG_DIR" ] ; then
  echo "You must source functions.sh" 1>&2
  exit 1
fi

if [ ! -z "$CXX" ]
then
  if [ -z "$CXXRPO_TEST" ]
  then
    CXXRPO=""
    CXXRPO_TEST=true
    echon "Testing if ${CXX} supports option '-frepo' ... "
    if [ -z "$cxx_is_gcc" ] 
    then
      echo no.
      echo "The option -frepo is only supported with g++"
    else
      echo 'int main(void) {return 0;}' | testfile $temp.cpp
      run "$CXX" $CXXFLAGS -frepo $temp.cpp -o $temp
      if [ $? = 0 -a -z "`grep -i unrecognized $temp.out`" ]
      then
        echo yes.
        CXXRPO="-frepo"
      else
        echo no.
      fi
      echon "Searching rpo program ... "
      if [ -z "$rpo" -o "x$rpo" = "xyes" ] ; then rpo=rpo ; fi
      if ( run "$rpo" </dev/null ) 
      then
        echo "$rpo"
      else
        echo not found.
        echo  "-- You cannot use option -frepo without this program."
        CXXRPO=""
      fi
    fi
    CXXRPO_TEST=tested
    CONFIG_VARS=`echo rpo CXXRPO_TEST CXXRPO $CONFIG_VARS`
  fi
fi

if [ ! -z "$CC" ]
then
  if [ -z "$CCRPO_TEST" ]
  then
    CCRPO=""
    CCRPO_TEST=true
    echon "Testing if ${CC} supports option '-frepo' ... "
    if [ -z "$cc_is_gcc" ] 
    then
      echo no.
      echo "The option -frepo is only supported with gcc"
    else
      echo 'int main(void) {return 0;}' | testfile $temp.c
      run "$CC" $CCFLAGS -frepo $temp.c -o $temp
      if [ $? = 0 -a -z "`grep -i unrecognized $temp.out`" ]
      then
        echo yes.
        CCRPO="-frepo"
      else
        echo no.
      fi
      if [ -z "$CXXRPO" ]
      then
        echon "Searching rpo program ... "
        if [ -z "$rpo" -o "x$rpo" = "xyes" ] ; then rpo=rpo ; fi
        if ( run $rpo ) 
        then
          echo "$rpo"
        else
          echo not found.
          echo  "-- You cannot use option -frepo without this program."
          CCRPO=""
        fi
      fi
    fi
    CCRPO_TEST=tested
    CONFIG_VARS=`echo rpo CCRPO_TEST CCRPO $CONFIG_VARS`
  fi
fi

