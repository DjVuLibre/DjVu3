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
# $Id: cc-wrapper.sh,v 1.1 2001-09-05 23:25:34 docbill Exp $
# $Name:  $

# This rule sets the following variables:
#	CC, CCFLAGS, CCWARN, CCUNROLL

if [ -z "$CC_SET$CXX_SET" ]
then
  echo "You must source cc.sh" 1>&2
  exit 1
fi

if [ -z "$WRAPPER_SET" ]
then
  if [ ! -d buildtools ]
  then
    mkdir buildtools
  fi
  if [ -n "$CC" ]
  then
    echon "Building c wrapper ... "
    xcc="$CC"
    if [ ! -r "$xcc" ]
    then
      xcc=`which "$xcc"`
    fi
    if ( run $CC ${CCFLAGS} ${CONFIG_DIR}/build-wrapper.c -DCOMMAND=\""$xcc"\" -o cc-wrapper ) 
    then
      echo built.
      if ( run mv cc-wrapper `pwd`/buildtools/cc-wrapper )
      then
        CC=`pwd`/buildtools/cc-wrapper
      fi
      echon "Building installtool ... "
      if ( run $CC ${CCFLAGS} ${CONFIG_DIR}/build-wrapper.c -o installer ) 
      then
        echo built.
        if ( run mv installer `pwd`/buildtools/installer )
        then
          INSTALLER=`pwd`/buildtools/installer
        fi
      else
        echo failed.
      fi
      WRAPPER_SET=true
    else
      echo failed.
    fi
  fi
  if [ -n "$CXX" ]
  then
    xcxx="$CXX"
    if [ ! -r "$xcxx" ]
    then
      xcxx=`which "$xcxx"`
    fi
    echon "Building c++ wrapper ... "
    if ( run $xcxx ${CXXFLAGS} ${CONFIG_DIR}/build-wrapper.c -DCOMMAND=\""$xcxx"\" -o c++-wrapper )
    then
      echo built.
      ( run mv c++-wrapper `pwd`/buildtools/c++-wrapper )
      CXX=`pwd`/buildtools/c++-wrapper
      if [ -z "$WRAPPER_SET" ]
      then
        echon "Building installtool ... "
        if ( run $CXX ${CXXFLAGS} ${CONFIG_DIR}/build-wrapper.c -o installer ) 
        then
          echo built.
          if ( run mv installer `pwd`/buildtools/installer )
          then
            INSTALLER=`pwd`/buildtools/installer
          fi
        else
          echo failed.
        fi
      fi
    else
      echo failed.
    fi
  fi
  WRAPPER_SET=true
  CONFIG_VARS=`echo WRAPPER_SET INSTALLER $CONFIG_VARS`
fi

