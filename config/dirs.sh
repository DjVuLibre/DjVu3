# This script sets the variables:
#   TOPSRCDIR, TOPBUILDDIR, CONFIG_CACHE, CONFIG_STATUS, and CONFIG_LOG

if [ -z "$CONFIG_DIR" ] ; then
  echo "You must source functions.sh" 1>&2
  exit 1
fi


if [ -z "$CONFIG_CACHE" ] ; then
  echo "Setting default paths"
  TOPSRCDIR=`cd "${CONFIG_DIR}/.." 1>>/dev/null 2>>/dev/null;"${pwdcmd}"`
  PROJECT=`echo  $PROGRAM_NAME|sed 's,^.*-,,g'`
  s=`${pwdcmd}`
  if [ "$TOPSRCDIR" != "$s" ] ; then
    TOPBUILDDIR="$s"
  else
    if [ -z "$TOPBUILDPREFIX" ] ; then
    TOPBUILDDIR="${s}/build/${SYS}/$PROJECT"
    else
      TOPBUILDDIR="${TOPBUILDPREFIX}/${SYS}/PROJECT"
    fi
  fi
  CONFIG_CACHE="$TOPBUILDDIR"/config.cache
  CONFIG_STATUS="$TOPBUILDDIR"/config.status
  CONFIG_LOG="$TOPBUILDDIR"/config.log
  if [ ! -d "$TOPBUILDDIR" ] ; then
    mkdirp "$TOPBUILDDIR"
  fi
  CONFIG_VARS=`echo TOPSRCDIR PROJECT TOPBUILDDIR CONFIG_LOG CONFIG_STATUS CONFIG_CACHE "${CONFIG_VARS}"`
fi
if [ -z "$PROJECT_FILLNAME" ]
then
  case "$PROJECT" in
	plugin)
		PROJECT_FULLNAME="NetscapePlugin";;
	*)
		PROJECT_FULLNAME="$PROJECT" ;;
  esac
fi
