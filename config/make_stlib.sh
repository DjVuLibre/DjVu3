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
# $Id: make_stlib.sh,v 1.1.2.1 2001-03-21 23:47:03 bcr Exp $
# $Name:  $
if [ ! -r ./TOPDIR/config.cache ] ; then
  echo "Failed to find config.cache" 1>&2
  exit 1
fi
. ./TOPDIR/config.cache

unescape()
{
  (for i
   do
    echo "$i"
  done)|sed  -e 's,@%%@a,'"'"',g' -e 's,@%%@q,",g' -e 's,@%%@d,\$,g' -e 's,@%%@t,	,g' -e 's,@%%@s, ,g' -e 's,@%%@e,!,g' -e 's,@%%@p,@%%@,g' -e 's,@%%@z,\\,g'
}

arname="$1"
shift
rm -f "$arname"

libs=
objs=
for arg
do
  case $arg in
   *.a )
     if [ ! -r "$arg" ]
     then
	echo "$arg" does not exist 2>&1
        exit 1
     fi
     libs=`echo $libs "$w$ws$arg"`
     ;;
   *.o )
     if [ ! -r "$arg" ]
     then
	echo "$arg" does not exist 2>&1
        exit 1
     fi
     objs=`echo $objs "$arg"`
     ;;
   ?* )
     echo "Unrecognized argument $arg" 2>&1
     exit 1
     ;;
   esac
done

tmpdirs=
if [ -n "$libs" ] 
then
  j=0
  for i in "$libs"
  do
    tmp=libs-$j.tmp
    mkdir $tmp
    tmpdirs=`echo $tmpdirs $tmp`
    (cd $tmp;if [ ! -r "$i" ] ; then i="../$i" ; fi; ar x "$i")
    for k in $tmp/*.o ; do
      if [ -r "$k" ]
      then
        objs=`echo $objs $k`
      fi
    done
  done
fi

if [ -n "$objs" ] 
then
  if ( "$ar" cq "$arname" $objs )
  then
    if [ -n "$tmpdirs" ]
    then
      rm -rf $tmpdirs
    fi
    exec "$ranlib" "$arname"
  fi
fi
if [ -n "$tmpdirs" ]
then
  rm -rf $tmpdirs
fi
exit 1

