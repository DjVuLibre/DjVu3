#!/bin/sh
name="$1"
if [ -z "$name" ] 
then
  echo "Usage: $0 <packagename>" 1>&2
  exit 1
fi
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
packagedir="packages/$name"
if [ ! -d  "./SRCDIR/$packagedir" ] 
then
  echo "'TOPDIR/SRCDIR/$packagedir' does not exist." 1>&2
  exit 1
fi
issource="$2"
if [ -n "$issource" ] 
then
  if [ ! -d "./SRCDIR/$packagedir/source" ]
  then
    echo "'TOPDIR/SRCDIR/$packagedir/source' does not exist." 1>&2
    exit 1
  fi
  srcdir="./SRCDIR/$packagedir/source"
  srcdir=`cd $srcdir 1>>/dev/null 2>>/dev/null;pwd`
else
  srcdir="./SRCDIR/$packagedir"
  srcdir=`cd $srcdir 1>>/dev/null 2>>/dev/null;pwd`
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
if [ -n "$issource" ]
then
  SYS=source
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

thisdir=`pwd`

version=`grep 'SDKVERSION=' ./SRCDIR/rules/versions|sed -e 's,.*=,,g' -e 's,[ 	]*$,,g'`
if [ -z "$version" ] 
then
  echo "Version not found in TOPDIR/SRCDIR/rules/versions" 1>&2
  exit 1
fi
srcfilelist="$srcdir/archive.list"
filelist="$packagedir/archive.list"
if [ ! -r "$srcfilelist" ]
then
  echo "Archive list $srcfilelist does not exist." 1>&2
  exit 1
fi

echo "Copying files"
rm -rf "$packagedir"
mkdir packages 2>>/dev/null
mkdir "$packagedir" 2>>/dev/null
abspackagedir=`cd "$packagedir" 1>>/dev/null 2>>/dev/null;pwd`
for i in `cd $srcdir 2>>/dev/null 1>>/dev/null;ls|sed -e 's,%,@%%@,g' -e 's, ,@%s%@,g'` 
do
  i=`echo "$i"|sed -e 's,@%s%@, ,g' -e 's,@%%@,%,g'`
  j="$srcdir/$i"
  if [ ! -d "$j" ]
  then
    if [ "$j" != "$srcfilelist" ] 
    then
      k="$packagedir/$i"
      cp "$j" "$k"
      chmod u+w "$k"
      sed -e "s,@%VERSION%@,$version,g" -e "s,@%SYS%@,$SYS,g" < "$j" > "$k"
      chmod u-w "$k"
    fi
  fi
done
rm -f "$filelist" "$packagedir/CVS"
archive=`cd "$packagedir" 1>>/dev/null 2>>/dev/null;pwd`/archive.tar
(
  if [ -n "$issource" ]
  then
    cd ./SRCDIR
  fi
  action="cvf"
  for i in `sed -e "s,@%VERSION%@,$version,g" -e 's,%,@%%@,g' -e 's, ,@%s%@,g' < "$srcfilelist"` 
  do
    i=`echo "$i"|sed -e 's,@%s%@, ,g' -e 's,@%%@,%,g'`
    if [ -d "./$i" ]
    then
      for j in `find "$i" -name CVS -prune -o -type f -print|sed -e 's,%,@%%@,g' -e 's, ,@%s%@,g'` ; do
        j=`echo "$j"|sed -e 's,@%s%@, ,g' -e 's,@%%@,%,g'`
        tar "$action" "$archive" "$j"
        action=rvf
      done
    elif [ -d "./SRCDIR/$i" ]
    then
      (
        cd ./SRCDIR;
        for j in `find "$i" -name CVS -prune -o -type f -print|sed -e 's,%,@%%@,g' -e 's, ,@%s%@,g'` ; do
          j=`echo "$j"|sed -e 's,@%s%@, ,g' -e 's,@%%@,%,g'`
          tar "$action" "$archive" "$j"
          action=rvf
        done
      )
    else
      (
        if [ -r "./examples/$i" ] 
        then
          cd examples
        elif [ -r "./SRCDIR/examples/$i" ] 
        then
          cd ./SRCDIR/examples
        fi
        if [ -r "$i" ] 
        then
          tar "$action" "$archive" "$i"
        elif [ -r "./SRCDIR/$i" ]
        then
          (cd ./SRCDIR;tar "$action" "$archive" "$i")
        else
	  j=`echo $i|sed 's,^.\/.*,,g'`
	  if [ -n "$j" ]
          then
            echo "$i does not exist." 1>&2
            rm -rf "$abspackagedir"
            exit 1
          else
            echo "skipping $i"
          fi
        fi
      )
    fi
    if [ ! -d "$abspackagedir" ]
    then
      exit 1
    fi
    if [ -r "$archive" ]
    then
      action="rvf"
    fi
  done
)
cd packages
tar cvvf "$name-$version-$SYS".tar "$name"
gzip -f "$name-$version-$SYS".tar
rm -rf "$name"
 