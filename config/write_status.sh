#!/bin/ksh
#C-
#C- DjVu® Reference Library (v. 3.5)
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
# $Id: write_status.sh,v 1.18 2001-07-24 17:52:03 bcr Exp $
# $Name:  $

# This script writes the variables currently set to standard output.

if [ -z "$CONFIG_DIR" ]
then
  echo "You must source functions.sh" 1>&2
  exit 1
fi
if [ ! -z "$CONFIG_STATUS" ]
then
  p=`"${pwdcmd}"`
  echo "Writing the ${CONFIG_STATUS} file"|sed -e "s! ${p}[/]*! !" 
  if [ ! -d "$TOPBUILDDIR" ] ; then
    ${mkdirp} "$TOPBUILDDIR"
  fi
  (echo '#!'"${smartshell}"
   "${sed}" -e 's,+$,\\,g' -e "s!xTOPBUILDDIR!$TOPBUILDDIR!g" -e "s!xTOPSRCDIR!$TOPSRCDIR!g" <<\EOF
c=1;in="$1";out="$2";tmpA="$2~A";tmpB="$2~B";tmpC="$2~C"
SRCDIR=`dirname "$1"`
SRCDIR=`cd "$SRCDIR" 1>>/dev/null 2>>/dev/null;pwd`
BUILDDIR=`dirname "$2"`
BUILDDIR=`cd "$BUILDDIR" 1>>/dev/null 2>>/dev/null;pwd`
rm -f "$BUILDDIR/SRCDIR" "$BUILDDIR/TOPDIR" "xTOPBUILDDIR/SRCDIR" "xTOPBUILDDIR/TOPDIR"
TOPDIR=`echo "$BUILDDIR"|sed -e 's!^xTOPBUILDDIR/*!!' -e 's,^[.]//*,,g' -e 's,//*[.]$,,g' -e 's,/[.]/,/,g' -e 's,//*,/,g' -e 's,/[^/][^/]*,/..,g' -e 's,[^/][^/]*/,../,g'`
ln -s "$SRCDIR" "$BUILDDIR/SRCDIR"
ln -s "xTOPBUILDDIR" "$BUILDDIR/TOPDIR"
ln -s "." "xTOPBUILDDIR/TOPDIR"
ln -s "xTOPSRCDIR" "xTOPBUILDDIR/SRCDIR"
while [ $c != 0 ]
do
sed +
  -e 's,@%%@,@%%@p,g' +
  -e 's,!,@%%@e,g' +
  -e 's, ,@%%@s,g' +
  -e 's,	,@%%@t,g' +
  -e 's,\$,@%%@d,g' +
  -e 's,",@%%@q,g' +
  -e 's,'"'"',@%%@a,g' +
  -e 's,\\,@%%@z,g' +
  -e "s!@%TOPDIR%@!$TOPDIR!g" +
  -e "s!@%srcdir%@!$SRCDIR!g" +
  -e "s!@%builddir%@!$BUILDDIR!g" +
  -e "s!@%topsrcdir%@!$TOPSRCDIR!g" +
  -e "s!@%topbuilddir%@!$TOPBUILDDIR!g" +
EOF
  CONFIG_VARS=`sortlist $CONFIG_VARS`
  for i in $CONFIG_VARS ; do
    ss='echo $'"C_${i}"
    case "x`eval $ss`" in
    xreplace|xREPLACE)
      s='escape "$'"R_${i}"'"'
      ;;
    xprefix|xPREFIX)
      s='escape "$'P_"${i}"'" "$'"${i}"'"'
      ;;
    xappend|xAPPEND)
      s='escape "$'"${i}"'" "$'"A_${i}"'"'
      ;;
    *)
      s='escape "$'"${i}"'"'
      ;;
    esac 
    s=`eval "$s"`
    echo "-e 's!@%${i}%@!`nonl ${s}`!g' \\"
  done
# We include the following unescape rule twice because we allow recursive
# escapes in the variable substitutions.
  "${sed}" -e 's,+$,\\,g' <<\EOF
  -e 's,@%%@a,'"'"',g' +
  -e 's,@%%@q,",g' +
  -e 's,@%%@d,\$,g' +
  -e 's,@%%@t,	,g' +
  -e 's,@%%@s, ,g' +
  -e 's,@%%@e,!,g' +
  -e 's,@%%@p,@%%@,g' +
  -e 's,@%%@a,'"'"',g' +
  -e 's,@%%@q,",g' +
  -e 's,@%%@d,\$,g' +
  -e 's,@%%@t,	,g' +
  -e 's,@%%@s, ,g' +
  -e 's,@%%@e,!,g' +
  -e 's,@%%@p,@%%@,g' +
  -e 's,@%%@z,\\,g' +
EOF
  "${sed}" -e 's,+$,\\,g' <<EOF
  -e 's!^%%%[ 	]*include[ 	]*<\(.*\)>!%%%include "${RULES_DIR}/\1"!g' +
  -e '/^%%%[ 	]*BEGIN_SYS=(${SYS})/d' +
  -e '/^%%%[ 	]*END_SYS=(${SYS})/d' +
  -e '/^%%%[ 	]*BEGIN_SYS=(/,/^%%%[ 	]*END_SYS=(/d' +
  -e '/^%%%[ 	]*BEGIN_SYS!=(${SYS})/,/^%%%[ 	]*END_SYS!=(${SYS})/d' +
  -e '/^%%%[ 	]*BEGIN_SYS!=(/d' +
  -e '/^%%%[ 	]*END_SYS!=(/d' +
  -e '/^%%%[ 	]*BEGIN_IFDEF([^@]*$/,/^%%%[ 	]*END_IFDEF([^@]*$/d' +
  -e '/^%%%[ 	]*BEGIN_IFNDEF(.*@%.*)/,/^%%%[ 	]*END_IFNDEF(.*@%.*)/d' +
  -e '/^%%%[ 	]*BEGIN_IFDEF/d' +
  -e '/^%%%[ 	]*END_IFDEF/d' +
  -e '/^%%%[ 	]*BEGIN_IFNDEF/d' +
  -e '/^%%%[ 	]*END_IFNDEF/d' +
  -e 's,@%[^@]*%@,,g' +
  -e '/^%%%[ 	]*BEGIN_REQUIRE()/,/^%%%[ 	]*END_REQUIRE()/d' +
  -e '/^%%%[ 	]*BEGIN_REQUIRE([&][&]/,/^%%%[ 	]*END_REQUIRE([&][&]/d' +
  -e '/^%%%[ 	]*BEGIN_REQUIRE([^)]*[&][&])/,/^%%%[ 	]*END_REQUIRE([^)]*[&][&])/d' +
  -e '/^%%%[ 	]*BEGIN_REQUIRE([^)]*[&][&][&][&]/,/^%%%[ 	]*END_REQUIRE([^)]*[&][&][&][&]/d' +
  -e '/^%%%[ 	]*BEGIN_REQUIRE!([^)]/,/^%%%[ 	]*END_REQUIRE!([^)]/d' +
  -e '/^%%%[ 	]*BEGIN_/d' +
  -e '/^%%%[ 	]*END_/d' +
EOF
  "${sed}" -e 's,+$,\\,g' <<\EOF
  "$in" | tee "$out"| sed -n -e 's, ,!,g' -e 's,^%%%[!	]*include[!	]*"\(.*\)"[!	]*$,\1,p' > "$tmpA"
  c=`wc -l<"$tmpA"`
  if [ $c != 0 ]
  then
    echo "sed -n \\" > $tmpB
    for i in `cat $tmpA` ; do
      s=`echo "$i"|sed -n -e 's,^/,,p'`
      j=`echo "$i"|sed -e 's,!, ,g'`
      k=`echo "$i"|sed -e 's,/,\\\\/,g'`
      if [ -z "$s" ]
      then
        if [ -r "$BUILDDIR/$j" ] ; then
          j="$BUILDDIR/$j"
        elif [ -r "$SRCDIR/$j" ] ; then
          j="$SRCDIR/$j"
        fi
      fi
      (sed -e 's,$,\\,g' <<EOF2
-e '/^%%%[ 	]*include[ 	]*"$k"/r $j' 
-e '/^%%%[ 	]*include[ 	]*"$k"/d' 
EOF2
) >> "$tmpB"
    done
#    (echo "sed \\";cat $tmpA;sed -e 's,^\(-e '"'"'/.*/\)r '"'"'`echo .*`,\1d'"'"',p' < "$tmpA") > "$tmpB"
    echo "-e /^/p < '$out'  > '$tmpA'" >> "$tmpB"
    . "$tmpB"
    in="$tmpA"
    tmpA="$tmpB"
    tmpB="$tmpC"
    tmpC="$in"
  fi
  done
  if [ "$out" != "$2" ]
  then
    mv "$out" "$2"
  fi
  rm -f "$tmpA" "$tmpB" "$tmpC"

EOF
) > "$CONFIG_STATUS"
  "${chmod}" 755 "${CONFIG_STATUS}"
  WROTE_STATUS=true
fi

