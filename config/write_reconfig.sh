# This saves the command line to a reconfigure file.

if [ -z "$CONFIG_DIR" ] ; then
  echo "You must source functions.sh" 1>&2
  exit 1
fi

# --- save command line
echo '#!'"$smartshell" > "$TOPBUILDDIR/reconfigure"
echo "'$TOPSRCDIR/$PROGRAM_NAME' \\" >> "$TOPBUILDDIR/reconfigure"
set - $CONFIGURATION_OPTIONS
while [ $# != 0 ] ; do
  echon "'$1' " >> "$TOPBUILDDIR/reconfigure"
  shift
done
echo " "  >> "$TOPBUILDDIR/reconfigure"
chmod 755 "$TOPBUILDDIR/reconfigure"

