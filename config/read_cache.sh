# This script reads the variables from the cache file.

if [ -z "${CONFIG_DIR}" ] ; then
  echo "You must source functions.sh" 1>&2
  exit 1
fi
if [ -z "${CONFIG_CACHE}" ] ; then
  . ${CONFIG_DIR}/dirs.sh
fi

if [ -r "${CONFIG_CACHE}" ] ; then
  p=`"${pwdcmd}"`
  echon `echo "Checking the values in ${CONFIG_CACHE} ..."|sed -e "s! ${p}[/]*! !"` 
  THISSYS="$SYS"
  CONFIG_VARS=`sortlist $CONFIG_VARS`
  (echo CONFIG_VARS="'$CONFIG_VARS'"
  for i in $CONFIG_VARS ; do
    s='echo $'"${i}"
    echo "${i}='`eval $s`'"
    eval "${i}=''"
  done ) > $temp
  . "${CONFIG_CACHE}"
  if [ "x${SYS}" != "x${THISSYS}" ] ; then
    echo "no good"
    for i in $CONFIG_VARS ; do
      eval "${i}=''"
    done
    . "$temp"
  else
    echo good
    for i in $CONFIG_VARS ; do
      s='unescape $'"${i}"
      eval "${i}="'"`'"$s"'`"'
    done
  fi
fi

