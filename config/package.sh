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
# $Id: package.sh,v 1.12 2000-11-02 01:08:33 bcr Exp $
# $Name:  $

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

i="./TOPDIR/SRCDIR/src/include/DjVuVersion.h"
version=`sed -n -e 's,.* DJVU_CVS_NAME[^0-9]*\([1-9][-_0-9A-Za-z]*\).*,\1,p' < "$i"`
if [ -z "$version" ] 
then 
  version=`sed -n -e 's,.* DJVU_VERSION  *"\(.*\)".*$,\1,p' -e 's,.*DJVU_CVS_REV.*Revision: \([0-9][.0-9]*\) .*,\1,p' < "$i"|tr '\n' '-'|sed 's,-$,,g'`
fi
if [ -z "$version" ] 
then
  echo "Version not found in TOPDIR/SRCDIR/src/include/DjVuVersion.h" 1>&2
  exit 1
fi
version=`echo $version|tr ' ' '_'`
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
      sed -e "s,@%VERSION%@,$version,g" -e "s,@%SYS%@,$SYS,g" -e "s,@%RTK%@,$RTK,g" < "$j" > "$k"
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
  for i in `if [ -n "$RTK" ] ;
      then
        sed -e "s,@%VERSION%@,$version,g" -e "s,@%SYS%@,$SYS,g" -e "s,@%RTK%@,$RTK,g" -e 's,%,@%%@,g' -e 's, ,@%s%@,g' < "$srcfilelist" ;
      else
        sed -e "s,@%VERSION%@,$version,g" -e "s,@%SYS%@,$SYS,g" -e "/@%RTK%@/d" -e 's,%,@%%@,g' -e 's, ,@%s%@,g' < "$srcfilelist" ;
      fi`
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
(
  cd "$name"
  tar xvf "$archive"
  rm -f "$archive"
)
tar cvvf "$name-$version-$SYS".tar "$name"
gzip -f "$name-$version-$SYS".tar
rm -rf "$name"
 

