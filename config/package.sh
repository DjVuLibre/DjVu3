#!/bin/sh
name="$1"
if [ ! -d ./TOPDIR ]
then
  echo "TOPDIR does not exist." 1>&2
  exit 1
fi
cd ./TOPDIR
if [ ! -d ./SRCDIR ]
then
  echo "./TOPDIR/SRCDIR does not exist." 1>&2
  exit 1
fi
if [ ! -d  "./SRCDIR/packages/$name" ] 
then
  echo "'TOPDIR/SRCDIR/packages/$name' does not exist." 1>&2
  exit 1
fi
if [ ! -r ./config.cache ]
then
  echo "TOPDIR/config.cache does not exist" 1>&2
  exit 1
fi
. ./config.cache
if [ -z "$SYS" ] 
then
  echo "System type not found in TOPDIR/config.cache" 1>&2
  exit 1
fi
for i in $PACKAGES none
do
  if [ "$i" = "$name" ]
  then
    echo "Got $i"
    gotit=yes
  fi
done
if [ -z "$gotit" ]
then
  echo "This build was not configured to build the package '$name'." 1>&2
  exit 1
fi 

packagedir="packages/$name"
filelist="$packagedir/archive.list"
thisdir=`pwd`

version=`grep 'SDKVERSION=' ./SRCDIR/rules/versions|sed -e 's,.*=,,g' -e 's,[ 	]*$,,g'`
if [ -z "$version" ] 
then
  echo "Version not found in TOPDIR/SRCDIR/rules/versions" 1>&2
  exit 1
fi
if [ -z "$name" ] 
then
  echo "Usage: $0 <packagename>" 1>&2
  exit 1
fi
if [ ! -r "./SRCDIR/$filelist" ]
then
  echo "Archive list $filelist does not exist." 1>&2
  exit 1
fi

echo "Copying files"
rm -rf "$packagedir"
mkdir packages 2>>/dev/null
mkdir "$packagedir" 2>>/dev/null
for i in `cd ./SRCDIR;echo $packagedir/*` 
do
  if [ ! -d "./SRCDIR/$i" ]
  then
    if [ "$i" != "$filelist" ] 
    then
      cp "./SRCDIR/$i" "$i"
      chmod u+w "$i"
      sed -e "s,@%VERSION%@,$version,g" -e "s,@%SYS%@,$SYS,g" < "./SRCDIR/$i" > "$i"
      chmod u-w "$i"
    fi
  fi
done
rm -f "$filelist" "$packagedir/CVS"
archive=`cd "$packagedir" 1>>/dev/null 2>>/dev/null;pwd`/archive.tar
action="cvf"
for i in `sed "s,@%VERSION%@,$version,g"< "./SRCDIR/$filelist"` 
do
  if [ -d "./$i" ]
  then
    find "$i" -name CVS -prune -o -type f -exec tar "$action" "$archive" \{\} \;
  elif [ -d "./SRCDIR/$i" ]
  then
    (cd ./SRCDIR;find "$i" -name CVS -prune -o -type f -exec tar "$action" "$archive" \{\} \;)
  elif [ -r "$i" ] 
  then
    tar "$action" "$archive" "$i"
  elif [ -r "./SRCDIR/$i" ]
  then
    (cd ./SRCDIR;tar "$action" "$archive" "$i")
  else
    echo "$i does not exist." 1>&2
    rm -rf "$packagedir"
    exit 1
  fi
  action="rvf"
done
cd packages
tar cvvf "$name-$version-$SYS".tar "$name"
gzip -f "$name-$version-$SYS".tar
rm -rf "$name"
 
