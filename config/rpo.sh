#C-
#C- DjVu® Reference Library (v. 3.0)
#C- 
#C- Copyright © 2000 LizardTech, Inc. All Rights Reserved.
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
#C- 
#
# $Id: rpo.sh,v 1.5 2000-11-09 20:15:05 jmw Exp $
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

