# This script writes the variables currently set to standard output.

if [ -z "$CONFIG_DIR" ] ; then
  echo "You must source functions.sh" 1>&2
  exit 1
fi

if [ -z "$CONFIG_READONLY" ] ; then
  p=`"${pwdcmd}"`
  echo "Writing the ${CONFIG_CACHE} file"|"${sed}" -e "s! ${p}[/]*! !" 
  if [ ! -d "$TOPBUILDDIR" ] ; then
    mkdirp "$TOPBUILDDIR"
  fi
  CONFIG_VARS=`sortlist $CONFIG_VARS`
  echo CONFIG_VARS="'$CONFIG_VARS'" > "${CONFIG_CACHE}"
  for i in $CONFIG_VARS ; do
    s='escape $'"${i}"
    echo "${i}='`eval $s`'" >> "${CONFIG_CACHE}"
  done
fi


