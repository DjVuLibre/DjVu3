#!/bin/sh

documentcommand="documenttodjvu"
bitonalcommand="documenttodjvu"
electroniccommand=cpaldjvu

defaultprofile="--profile=clean"
defaultdpi="300"
outputdev="pnmraw"
args=""
j="$1"
case "$j" in
  "--"?*)
    while [ -n "$j" ] ; do
      case "$j" in
        "--profile="*)
           free=""
           profile="$j" ;;
        "--best"|"--best=t"*)
           best=true ;;
        "--free"|"--free=t"*)
           free=true ;;
        "--colors="*)
           free=true 
           args="$args '$j'" ;;
        "--tobitonal"|"--tobitonal=t"*)
           outputdev=pbmraw
           defaultprofile="--profile=bitonal"
           args="$args '$j'" ;;
        "--togray"|"--togray=t"*)
           outputdev=pgmraw
           args="$args '$j'" ;;
        "--dpi="*)
           dpi=`echo $j|sed 's,--dpi=,,'` ;;
        "--subsample="*|"--upsample="*|"--resize="*)
           best=true
           args="$args '$j'" ;;
        "--verbose"|"--verbose=t"*)
           verbose=true
           cargs=`echo $cargs $j`
           args="$args '$j'" ;;
        "--page-range="*|"--pages-per-dict=*")
           echo "Option $j is not supported by %0" 1>&2
           exit 1 ;;
        *)
           free=""
           args="$args '$j'" ;;
      esac
      shift
      case "$1" in
        "--"?*) j="$1" ;;
        *) j="" ;;
      esac
    done ;;
  *) ;;
esac

if [ -z "$profile" ] 
then
  if [ -z "$dpi" ] 
  then
    dpi=$defaultdpi
  fi
  profile="$defaultprofile$dpi"
elif [ -z "$dpi" ]
then
  dpi=`echo "$profile"|sed -n -e 's,.*\([1-6][0-5]0\)$,\1,p'`
  if [ -z "$dpi" ]
  then
    dpi=$defaultdpi  
  fi
fi
if [ -n "$free" ]
then
  $electroniccommand -help 2>>/dev/null 1>>/dev/null
  if [ $? -ne 10 ]
  then
    free=""
  fi
fi

if [ -n "$free" -o -z "$best" -o "$dpi" = "150" -o "$dpi" -gt 400 ]
then
  rdpi="$dpi"
else
  rdpi="`expr $dpi + $dpi`"
  args=`echo $args "--subsample=2"`
fi
if [ -n "$free" ]
then
  args="-dpi $rdpi "`echo "$args"|sed -e "s,'--togray[='][a-z]*,,g" -e "s,'--tobitonal[='][a-z]*,,g" -e "s,=,' ',g" -e "s,--,-,g"`
  djvucommand=$electroniccommand
else
  args="--dpi=$rdpi $profile --page-range=1 $args"
  if ( $documentcommand --help </dev/null 1>>/dev/null 2>>/dev/null )
  then
    djvucommand=$documentcommand
  else
    djvucommand=$bitonalcommand
  fi
fi

input="$1"
shift
output="$1"
shift
if [ -n "$1" ]
then
  echo "Usage: $0 [--best] [options] <input> <output>" 1>&2
  exit 1
fi
tmpdir="$output.tmp" 
name=`basename "$input" .ps`
name=`basename "$name" .pdf`
if [ -d "$tmpdir" ] 
then
  echo "$tmpdir" exists 1>&2
  exit 1
fi

mkdir "$tmpdir"
script="$tmpdir/run.sh"
sed 's,%-dollar-%,\$,g' <<+ > "$script"
#!/bin/sh
tmpfile=$tmpdir/gs$$.pnm
cat > "%-dollar-%tmpfile"
if [ -n "$verbose" ]
then
  echo "$djvucommand $args %-dollar-%tmpfile %-dollar-%1"
fi
"$djvucommand" $args "%-dollar-%tmpfile" "%-dollar-%1"
stat=%-dollar-%?
rm -f "%-dollar-%tmpfile"
exit %-dollar-%stat
+
chmod ugo+x "$script"
if [ -n "$verbose" ]
then
  echo "gs -dNOPAUSE -q -sDEVICE=$outputdev -r$rdpi -sOutputFile=|'$script' '$tmpdir/$name-%04d.djvu' $input"
fi
gs -dBATCH -dNOPAUSE -q "-sDEVICE=$outputdev" "-r$rdpi" -sOutputFile="|'$script' '$tmpdir/$name-%04d.djvu'" "$input"

if [ -d "$output" ] 
then
  combine="djvujoin"
  output="$output/index.djvu"
else
  combine="djvubundle"
fi
rm -f "$output"
if [ -n "$verbose" ]
then
  echo "$combine $cargs $tmpdir/$name-*.djvu $output"
fi
"$combine" $cargs "$tmpdir/$name-"*.djvu "$output"
rm -rf "$tmpdir"

