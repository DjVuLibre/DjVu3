#!/bin/sh
#C-
#C- DjVu® Reference Library (v. 3.5)
#C- 
#C- Copyright © 2001 LizardTech, Inc. All Rights Reserved.
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
# $Id: build-gnu-gs.sh,v 1.3 2001-07-24 17:52:03 bcr Exp $
# $Name:  $
# This is a short script intended to allow building a packaged version
# of ghostscript.
gsversion=5.50

gs_src=gnu-gs-"$gsversion".tar.gz
jpeg_src=gnu-gs-"$gsversion"jpeg.tar.gz
png_src=gnu-gs-"$gsversion"libpng.tar.gz
zlib_src=gnu-gs-"$gsversion"zlib.tar.gz
stdfonts_src="gnu-gs-fonts-std-$gsversion.tar.gz"
otherfonts_src="gnu-gs-fonts-other-$gsversion.tar.gz"
url=""
for i in "$gs_src" "$jpeg_src" "$png_src" "$zlib_src" "$stdfonts_src" "$otherfonts_src" 
do
  if [ ! -r "$i" ]
  then
    xurl="ftp://ftp.cs.wisc.edu/pub/ghost/gnu/gs"`echo "$gsversion"|sed -e 's,[.],,g'`/
#   echo "missing $i"
#   if [ -z "$url" ]
#   then
#     echo "missing $i, attempting to retrieve it"
#      wget --http-user=ftp --http-passwd=`whoami`@ -O "$i" "$xurl/$i"
#      if ( gzip -t "$i" )
#      then
#        echo success
#      else
#       rm -f "$i"
#       url="$xurl"
#      fi
#    else
      echo "missing $i"
      url="$xurl"
#   fi    
  fi
done

if [ -n "$url" ]
then
  echo "Optain source code from $url"
  exit 1
fi

# Now we extract the source code
echo "extracting source code"
gunzip -c < "$gs_src"|tar xf -
cd "gs$gsversion"

# This is a dummy prefix that will be used for building.

prefix="C85B2BD9814D0D1C2309B4980F39A20ED139ADBE36CD7C4C1A735B8369A7EB877880734A1A765C98"
fullprefix="$prefix/$prefix/$prefix/$prefix/$prefix/$prefix/$prefix/$prefix/$prefix/$prefix"

echo "Building changeprefix program"
# This is a short program that will substitue the dummy prefix with a real one.
cat <<+ > changeprefix.c
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
int
main(int argc,char *argv[],char *env[])
{
  if(argc != 2)
  {
    fprintf(stderr,"Usage: %s <prefix>\n",argv[0]);
    exit(1);
  }else
  {
    static const char prefix[]="$fullprefix";
    const int len=strlen(argv[1]); 
    if(len>=sizeof(prefix))
    {
      fprintf(stderr,"Prefix can be no more than %u characters\n",sizeof(prefix));
      exit(1);
    }else if(chdir(argv[1])<0)
    {
      perror("chdir");
    }else
    {
      const int fd=open("bin/gs",O_RDWR,0777);
      size_t size,stopat,pos;
      char *buf=0,*ptr;
      if(fd<0)
      {
        perror("open");
        exit(1);
      }
#ifndef SEEK_END
#define SEEK_END 2
#endif
      size=lseek(fd,0,SEEK_END);
#ifndef SEEK_SET
#define SEEK_SET 1
#endif
      if(size<sizeof(prefix))
      {
        close(fd);
        exit(0);
      }
      lseek(fd,0,SEEK_SET);
      buf=(char *)mmap(0,size,PROT_READ|PROT_WRITE,MAP_SHARED,fd,0);
      if(!buf || (size_t)buf == (size_t)(-1))
      {
        perror("mmap");
        lseek(fd,0,SEEK_END);
        close(fd);
        exit(1);
      }
      lseek(fd,0,SEEK_END);
      close(fd);
      stopat=size-(len+1);
      for(pos=0,ptr=buf;pos<stopat;++pos,++ptr)
      {
        if(!strncmp(prefix,ptr,sizeof(prefix)-1))
        {
          char *s=strdup(ptr+sizeof(prefix)-(1+len));
          strncpy(s,argv[1],strlen(argv[1]));
          strcpy(ptr,s);
          free(s);
        }
      }
      if(msync(buf,size,MS_SYNC)<0)
      {
        perror("msync");
      }
      if(munmap(buf,size)<0)
      {
        perror("msync");
      }
    }
  }
  exit(0);
}
+
if ( gcc -g -o changeprefix changeprefix.c )
then
  rm -f changeprefix.c
else
  exit 1
fi

p="$prefix"
mkdir "$p"
while [ "$p" != "$fullprefix" ]
do
  p="$p/$prefix"
  mkdir "$p"
done
if [ ! -d "$fullprefix" ]
then
  echo "Failed to create prefix directory"
  exit 1
fi

#extract jpeg
echo "extracting jpeg source code"
if [ -d jpeg ]
then
  chmod -R 777 jpeg
  rm -r jpeg
fi
gunzip -c < "../$jpeg_src"|tar xf -
mv jpeg-* jpeg
chmod -R 777 jpeg

#extract png
echo "extracting png source code"
if [ -d libpng ]
then
  chmod -R 777 libpng
  rm -r libpng
fi
gunzip -c < "../$png_src"|tar xf -
mv libpng-* libpng
chmod -R 777 libpng

#extract zlib
echo "extracting zlib source code"
if [ -d zlib ] 
then
  chmod -R 777 zlib
  rm -r zlib 
fi
gunzip -c < "../$zlib_src"|tar xf -
mv zlib-* zlib
chmod -R 777 zlib

#Apply relevant diffs
echo "applying diffs to the source code"

(sed -e 's,%-dollar-%,\$,g' |patch -c dvx-gcc.mak)<<+ 
*** ./dvx-gcc.mak	Sun Feb 13 18:29:11 2000
--- ./dvx-gcc.mak	Thu Feb  1 10:49:42 2001
***************
*** 166,172 ****
  # Under DJGPP, since we strip the executable by default, we may as
  # well *not* use '-g'.
  
! # CFLAGS=-g -O %-dollar-%(XCFLAGS)
  CFLAGS=-O %-dollar-%(XCFLAGS)
  
  # Define platform flags for ld.
--- 166,172 ----
  # Under DJGPP, since we strip the executable by default, we may as
  # well *not* use '-g'.
  
! # CFLAGS=-O3 %-dollar-%(XCFLAGS)
  CFLAGS=-O %-dollar-%(XCFLAGS)
  
  # Define platform flags for ld.
+
(sed -e 's,%-dollar-%,\$,g' |patch -c ugcclib.mak)<<+ 
*** ./ugcclib.mak	Sun Feb 13 18:28:44 2000
--- ./ugcclib.mak	Thu Feb  1 10:49:42 2001
***************
*** 66,72 ****
  
  GCFLAGS=-Wall -Wcast-qual -Wpointer-arith -Wstrict-prototypes -Wwrite-strings -fno-common
  XCFLAGS=
! CFLAGS=-g -O %-dollar-%(GCFLAGS) %-dollar-%(XCFLAGS)
  LDFLAGS=%-dollar-%(XLDFLAGS)
  EXTRALIBS=
  XINCLUDE=-I/usr/local/X/include
--- 66,72 ----
  
  GCFLAGS=-Wall -Wcast-qual -Wpointer-arith -Wstrict-prototypes -Wwrite-strings -fno-common
  XCFLAGS=
! CFLAGS=-O3 %-dollar-%(GCFLAGS) %-dollar-%(XCFLAGS)
  LDFLAGS=%-dollar-%(XLDFLAGS)
  EXTRALIBS=
  XINCLUDE=-I/usr/local/X/include
+
(sed -e 's,%-dollar-%,\$,g' |patch -c unix-cc.mak)<<+ 
*** ./unix-cc.mak	Sun Feb 13 18:28:23 2000
--- ./unix-cc.mak	Thu Feb  1 10:49:42 2001
***************
*** 59,65 ****
  INSTALL_PROGRAM = %-dollar-%(INSTALL) -m 755
  INSTALL_DATA = %-dollar-%(INSTALL) -m 644
  
! prefix = /usr/local
  exec_prefix = %-dollar-%(prefix)
  bindir = %-dollar-%(exec_prefix)/bin
  scriptdir = %-dollar-%(bindir)
--- 59,65 ----
  INSTALL_PROGRAM = %-dollar-%(INSTALL) -m 755
  INSTALL_DATA = %-dollar-%(INSTALL) -m 644
  
! prefix = $fullprefix
  exec_prefix = %-dollar-%(prefix)
  bindir = %-dollar-%(exec_prefix)/bin
  scriptdir = %-dollar-%(bindir)
+
(sed -e 's,%-dollar-%,\$,g' |patch -c unix-gcc.mak)<<+ 
*** ./unix-gcc.mak	Sun Feb 13 18:28:41 2000
--- ./unix-gcc.mak	Thu Feb  1 10:49:42 2001
***************
*** 59,65 ****
  INSTALL_PROGRAM = %-dollar-%(INSTALL) -m 755
  INSTALL_DATA = %-dollar-%(INSTALL) -m 644
  
! prefix = /usr/local
  exec_prefix = %-dollar-%(prefix)
  bindir = %-dollar-%(exec_prefix)/bin
  scriptdir = %-dollar-%(bindir)
--- 59,65 ----
  INSTALL_PROGRAM = %-dollar-%(INSTALL) -m 755
  INSTALL_DATA = %-dollar-%(INSTALL) -m 644
  
! prefix = $fullprefix
  exec_prefix = %-dollar-%(prefix)
  bindir = %-dollar-%(exec_prefix)/bin
  scriptdir = %-dollar-%(bindir)
***************
*** 195,201 ****
  # Define the added flags for standard, debugging, and profiling builds.
  
  CFLAGS_STANDARD=-O2
! CFLAGS_DEBUG=-g -O
  CFLAGS_PROFILE=-pg -O2
  
  # Define the other compilation flags.  Add at most one of the following:
--- 195,201 ----
  # Define the added flags for standard, debugging, and profiling builds.
  
  CFLAGS_STANDARD=-O2
! CFLAGS_DEBUG=-O3
  CFLAGS_PROFILE=-pg -O2
  
  # Define the other compilation flags.  Add at most one of the following:
+
(sed -e 's,%-dollar-%,\$,g' |patch -c unixansi.mak)<<+ 
*** ./unixansi.mak	Sun Feb 13 18:28:50 2000
--- ./unixansi.mak	Thu Feb  1 10:49:42 2001
***************
*** 59,65 ****
  INSTALL_PROGRAM = %-dollar-%(INSTALL) -m 755
  INSTALL_DATA = %-dollar-%(INSTALL) -m 644
  
! prefix = /usr/local
  exec_prefix = %-dollar-%(prefix)
  bindir = %-dollar-%(exec_prefix)/bin
  scriptdir = %-dollar-%(bindir)
--- 59,65 ----
  INSTALL_PROGRAM = %-dollar-%(INSTALL) -m 755
  INSTALL_DATA = %-dollar-%(INSTALL) -m 644
  
! prefix = $fullprefix
  exec_prefix = %-dollar-%(prefix)
  bindir = %-dollar-%(exec_prefix)/bin
  scriptdir = %-dollar-%(bindir)
+

echo "Building ......"
make 

echo "doing dummy install"
make install

echo "renaming dummy install directory"
name=gnu-gs-$gsversion-`uname -s`-`uname -m`
if [ -d "../$name" ]
then
  chmod -R 777 "../$name"
  rm -r "../$name"
fi
mv -f "$fullprefix" "../$name"
rm -rf "$prefix"

echo "creating install.sh script"
(sed 's,%-dollar-%,\$,g' > "../$name"/install.sh) <<+  
#!/bin/sh
if [ -d "$name" ]
then
  cd "$name"
fi
if [ -n "%-dollar-%1" ]
then
  prefix="%-dollar-%1"
fi
if [ ! -d "%-dollar-%prefix" ]
then
  echo "Usage: %-dollar-%0 <prefix>"
  echo "For example: %-dollar-%0 /usr/local"
  exit 1
fi
tar cf  - share man bin |(cd "%-dollar-%prefix";tar xvvf -)
echo "./changeprefix %-dollar-%prefix"
exec ./changeprefix "%-dollar-%prefix"
+
chmod 555 "../$name/install.sh"

echo "extracting fonts"
for i in "$stdfonts_src" "$otherfonts_src" ; do
  gunzip -c < "../$i"|(cd "../$name/share/ghostscript";tar xf -)
done

echo "moving the changeprefix command"
mv changeprefix "../$name/changeprefix"
echo "copying documentation"
cp *.htm "../$name/."
echo "copying license"
cp COPYING "../$name/COPYING"

# echo "creating archive" 
# (cd ..;tar cf - `basename "../$name"`)|gzip -c > "../$name".tar.gz

# echo "deleting dummy install directory"
# chmod -R 777 "../$name"
# rm -r "../$name"

cp "../$name/install.sh" ../install.sh
echo "Run ./install.sh <prefix> to install ghostscript"

