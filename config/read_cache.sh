# This script reads the variables from the cache file.

if [ -z "$SYS" ] ; then
  `dirname $0`/sys.sh
fi

if [ -z "$TOPBUILDDIR" ] ; then
  if [ -z "$TOPBUILDPREFIX" ] ; then
    TOPBUILDDIR="`pwd`/build/${SYS}"
  else
    TOPBUILDDIR="${TOPBUILDPREFIX}/${SYS}"
  fi
fi

if [ -r "${TOPBUILDDIR}/config.cache" ] ; then
  THISSYS="$SYS"
  SYS=""
  . ${TOPBUILDDIR}/config.cache
  if [ "x${SYS}" != "x${THISSYS}" ] ; then
    for i in $CONFIG_VARS ; do
      eval `$i=''`
    done
    SYS="$THISSYS"
  fi
fi

