#!/bin/sh
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
# $Id: package.sh,v 1.17 2001-02-02 23:05:53 bcr Exp $
# $Name:  $

# EXIT ON ERROR
set -e

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
version=`echo $version|sed -e 's, ,-,g'`
v=`echo "$version"|sed -n -e 's,^\([0-9]\).*,\1,p'`
if [ -z "$v" ]
then
  version="alpha"
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
if [ ! -d "packages" ] 
then
  mkdir packages
fi
mkdir "$packagedir"
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
 

