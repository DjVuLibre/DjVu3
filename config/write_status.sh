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
    mkdirp "$TOPBUILDDIR"
  fi
  ("${sed}" -e 's,+$,\\,g' -e 's,!!,#!,g' <<\EOF
!!/bin/sh
c=1;in="$1";out="$2";tmpA="$2~A";tmpB="$2~B";tmpC="$3~C"
SRCDIR=`dirname "$1"`
SRCDIR=`cd "$SRCDIR" 1>>/dev/null 2>>/dev/null;pwd`
BUILDDIR=`dirname "$2"`
BUILDDIR=`cd "$BUILDDIR" 1>>/dev/null 2>>/dev/null;pwd`
while [ $c != 0 ]
do
sed +
  -e "s!@%srcdir%@!$SRCDIR!g" +
  -e "s!@%builddir%@!$BUILDDIR!g" +
EOF
  for i in $CONFIG_VARS ; do
    ss='echo $'"C_${i}"
    case "x`eval $ss`" in
    xreplace|xREPLACE)
      s='echo "$'"R_${i}"'"'
      ;;
    xprefix|xPREFIX)
      s='echo "$'P_"${i}"'" "$'"${i}"'"'
      ;;
    xappend|xAPPEND)
      s='echo "$'"${i}"'" "$'"A_${i}"'"'
      ;;
    *)
      s='echo "$'"${i}"'"'
      ;;
    esac 
    echo "-e 's!@%${i}%@!`eval $s`!g' \\"
  done
  "${sed}" -e 's,+$,\\,g' <<EOF
  -e 's!^%%%[ 	]*include[ 	]*<\(.*\)>!%%%include "${RULES_DIR}/\1"!g' +
  -e '/^%%%[ 	]*BEGIN_SYS=(${SYS})/d' +
  -e '/^%%%[ 	]*END_SYS=(${SYS})/d' +
  -e '/^%%%[ 	]*BEGIN_SYS=(/,/^%%%[ 	]*END_SYS=(/d' +
  -e '/^%%%[ 	]*BEGIN_SYS!=(${SYS})/,/^%%%[ 	]*END_SYS!=(${SYS})/d' +
  -e '/^%%%[ 	]*BEGIN_SYS!=(/d' +
  -e '/^%%%[ 	]*END_SYS!=(/d' +
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

