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
# $Id: make_shlib.sh,v 1.15.2.1 2001-03-20 00:29:33 bcr Exp $
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

linkname="$1"
shift
outputname="$1"
shift
soname="$1"
shift

rm -f $linkname $outputname

libs=
w=`unescape "$WHOLEARCHIVE"` 
ws=`unescape "$WHOLEARCHIVESEP"`
if [ -z "$ws" ] ; then
  ws=" "
fi
nw=`unescape "$NOWHOLEARCHIVE"` 

isw=
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
     isw=true
     ;;
   */lib*.so )
     if [ -n "$isw" ]
     then
       libs=`echo $libs "$nw"`
       isw=
     fi
     libs=`echo $libs "$arg"`
     ;;
   ?* )
     if [ -n "$isw" ]
     then
       libs=`echo $libs "$nw"`
       isw=
     fi
     libs=`echo $libs "$arg"`
     ;;
   esac
done

cxx=`unescape "$CXX"`
cxxsymbolic=`unescape "$CXXSYMBOLIC"`
cxxthreads=`unescape "$CXXTHREADS"`
name=`unescape "$soname"`

echo "" > "dummy$$.c"
"$cxx" -c "dummy$$.c" -o "dummy$$.o" 
echo "$cxx $cxxsymbolic $cxxthreads -o $outputname dummy$$.o -Wl,-soname,$name $libs"
"$cxx" $cxxsymbolic $cxxthreads -o `basename "$outputname"` "dummy$$.o" "-Wl,-soname,$name" $libs
rm -f "dummy$$.o" "dummy$$.c"
`unescape "$mv"` `basename $outputname` "$outputname"

if [ -f "$outputname" ] 
then
  `unescape "$mv"` "$outputname" "$linkname"
  LN=`unescape "$ln"`
  outputdir=`dirname $outputname`
  if [ ! -f "$outputdir/$name" ]
  then
    (cd $outputdir;"$LN" -s `basename "$linkname"` "$name")
  fi 
  exec `unescape "$LN"` -s `basename "$linkname"` "$outputname"
else
  exit 1
fi

