# This script finds all the commands
#   whence, true, pwdcmd, docxx, ranlib, ln, dirname, basename, mkdir, tar,
#   make, cmp, mv, cp, rm, sed, find, chmod, cat, ar, uname, grep, latex,
#   mkdirp, and tee
# This also sets the variable RULES_DIR
#

if [ -z "$CONFIG_DIR" ]
then
  echo "You must source functions.sh" 1>&2
  exit 1
fi

if [ -z "$RULES_DIR" ]
then
  if ( whence whence 1>>/dev/null 2>>/dev/null )
  then
    whence=whence
  else
    whence=which
  fi

  if ( ${whence} true 1>>/dev/null 2>>/dev/null )
  then
    true="`${whence} true`"
  else
    true="echo>>/dev/null"
  fi

  if ( ${whence} pwd 1>>/dev/null 2>>/dev/null )
  then
    pwdcmd="`${whence} pwd`"
  else
    pwdcmd=pwd
  fi

  if ( ${whence} doc++ 1>>/dev/null 2>>/dev/null )
  then
    docxx="`${whence} doc++`"
  else
    docxx=doc++
  fi

  if ( ${whence} ranlib 1>>/dev/null 2>>/dev/null )
  then
    ranlib="`${whence} ranlib`"
  else
    ranlib="$true"
  fi

  for i in sed sort mkdir
  do
    if ( ${whence} "$i" 1>>/dev/null 2>>/dev/null )
    then
      s=`${whence} "$i"`
      eval "${i}='${s}'"
    else
      eval "${i}='${i}'"
    fi
  done

  if [ -z "$SUPPORTS_MKDIRP" ]
  then
    mkdirp="${CONFIG_DIR}/mkdirp.sh"
  else
    mkdirp="${mkdir} -p"
  fi

  RULES_DIR=`cd ${CONFIG_DIR}/../rules/ 1>>/dev/null 2>>/dev/null;"${pwdcmd}"`
  CONFIG_VARS=`echo RULES_DIR ${CONFIG_VARS}`
  for i in `"${sed}" -n -e 's,^.*=@%\(.*\)%@,\1,p'<"${RULES_DIR}/commands"|"${sort}" -u`
  do
    CONFIG_VARS=`echo $i ${CONFIG_VARS}`
    s='$'"$i"
    eval 's="'"${s}"'"'
    if [ -z "$s" ]
    then
      if ( ${whence} "$i" 1>>/dev/null 2>>/dev/null )
      then
        s=`${whence} "$i"`
        eval "${i}='${s}'"
      else
        eval "${i}='${i}'"
      fi
    fi
  done

fi

 
