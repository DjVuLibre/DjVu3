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
name=`unescape "$soname"`

echo "$cxx $cxxsymbolic -o $outputname -Wl,-soname,$name $libs"
"$cxx" "$cxxsymbolic" -o "$outputname" "-Wl,-soname,$name" $libs

if [ -x "$outputname" ] 
then
  `unescape "$mv"` "$outputname" "$linkname"
  LN=`unescape "$ln"`
  outputdir=`dirname $outputname`
  if [ ! -x "$outputdir/$name" ]
  then
    "$LN" `basename "$linkname"` "$outputdir/$name"
  fi 
  exec `unescape "$LN"` -s `basename "$linkname"` "$outputname"
else
  exit 1
fi

