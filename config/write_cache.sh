# This script writes the variables currently set to standard output.

if [ -z "$SYS" ] ; then
  . `dirname $0`/sys.sh
fi

if [ -z "$CONFIG_READONLY" ] ; then

  if [ -z "$TOPBUILDDIR" ] ; then
    if [ -z "$TOPBUILDPREFIX" ] ; then
      TOPBUILDDIR="`pwd`/build/${SYS}"
    else
      TOPBUILDDIR="${TOPBUILDPREFIX}/${SYS}"
    fi
  fi

  while [ ! -d "$TOPBUILDDIR" ] ; do
    p="$TOPBUILDDIR/x/x"
    q="$TOPBUILDDIR/x"
    while [ "$p" != "$q" ] ; do
      p="$q"
      q=`dirname "$p"`
      if [ -d "$q" ] ; then
        q="$p"
      fi
    done
    if [ ! -d "$p" ] ; then
      mkdir "$p"
      if [ ! -d "$p" ] ; then
        exit 1
      fi
    fi
  done

  echo CONFIG_VARS="'$CONFIG_VARS'" > "${TOPBUILDDIR}/config.cache"
  for i in $CONFIG_VARS ; do
    echo "${i}='"`eval echo \"\$"$i"\"`"'" >> "${TOPBUILDDIR}/config.cache"
  done
fi


