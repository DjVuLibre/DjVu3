#!/bin/sh
if [ ! -r ./TOPDIR/config.cache ] ; then
  echo "Failed to find config.cache" 1>&2
  exit 1
fi
. ./TOPDIR/config.cache
linkname="$1"
shift
outputname="$1"
shift
soname="$1"
shift

rm -f $linkname $outputname

for arg
do
  case $arg in
   *.a )
     if [ ! -r "$arg" ] ; then exit 0 ; fi 
     libs=`echo $libs "$WHOLEARCHIVE$arg"`
     ;;
   ?* )
     libs=`echo $libs "$NOWHOLEARCHIVE" $arg`
     ;;
   esac
done

$CXX -o "$outputname" "-Wl,-soname,$soname" $libs -lm -lc
status=$?
if [ $status -ne 0 ]
then
  exec "$LN" -s "$outputname" "$linkname"
else
  exit $status
fi

