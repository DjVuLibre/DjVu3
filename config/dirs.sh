# This script sets the variables:
#   TOPSRCDIR, TOPBUILDDIR, CONFIG_CACHE, and CONFIG_LOG

if [ -z "$CONFIG_DIR" ] ; then
  echo "You must source functions.sh" 1>&2
  exit 1
fi


if [ -z "$CONFIG_CACHE" ] ; then
  TOPSRCDIR=`cd "${CONFIG_DIR}/.." 1>>/dev/null 2>>/dev/null;pwd`
  PROJECT=`echo  $PROGRAM_NAME|sed 's,^.*-,,g'`
  if [ "$TOPSRCDIR" != "`pwd`" ] ; then
    TOPBUILDDIR="`pwd`"
  else
    if [ -z "$TOPBUILDPREFIX" ] ; then
    TOPBUILDDIR="`pwd`/build/${SYS}/$PROJECT"
    else
      TOPBUILDDIR="${TOPBUILDPREFIX}/${SYS}/PROJECT"
    fi
  fi
  CONFIG_CACHE="$TOPBUILDDIR"/config.cache
  CONFIG_LOG="$TOPBUILDDIR"/config.log
  if [ ! -d "$TOPBUILDDIR" ] ; then
    mkdirp "$TOPBUILDDIR"
  fi
  CONFIG_VARS=`echo TOPSRCDIR PROJECT TOPBUILDDIR CONFIG_LOG CONFIG_CACHE "${CONFIG_VARS}"`
fi

