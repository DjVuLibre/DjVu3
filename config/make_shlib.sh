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
# $Id: make_shlib.sh,v 1.12 2000-11-02 01:08:33 bcr Exp $
# $Name:  $

#!/bin/ksh
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

