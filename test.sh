#!/bin/sh
key=key
in=/dev/null
out=/dev/null
binary=no
verbose=no
all=no
while getopts :k:i:o:bva OPT; do
  case $OPT in
    k) key="$OPTARG" ;;
    i) in="$OPTARG" ;;
    o) out="$OPTARG" ;;
    b) binary=yes ;;
    v) verbose=yes ;;
    a) all=yes ;;
    \?) echo "Usage: $0 [-b] [-v] [-a] [-k name] [-i infile] [-o outfile] cmd..." ; exit 127 ;;
  esac
done
shift `expr $OPTIND - 1`
cmd="$*"

tmpout=$(mktemp)
tmperr=$(mktemp)
trap "rm -f '$tmpout' '$tmperr'" EXIT

eval "$cmd <'$in' >'$tmpout' 2>'$tmperr'"
status=$?

[ "$in" != /dev/null ] && cmd="$cmd < $in"

if cmp -s "$tmpout" "$out"; then
  echo "✅  OK 🎉 [$key] $cmd "
  if [ "$all" = yes ]; then
    if [ "$binary" = yes ]; then
      [ "$in" != /dev/null ] && echo "--- stdin" && od -c $in
      echo "--- stdout expected" && od -c "$out"
      echo "--- stdout" && od -c "$tmpout"
    else
      [ "$in" != /dev/null ] && echo "--- stdin" && cat $in
      echo "--- stdout expected" && cat "$out"
      echo "--- stdout" && cat "$tmpout"
    fi
    echo "--- exit status: $status"
  fi
  exit 0
else
  echo "❌  NG 😱 [$key] $cmd "
  if [ \( "$all" = yes \) -o \( "$verbose" = yes \) ]; then
    if [ "$binary" = yes ]; then
      [ "$in" != /dev/null ] && echo "--- stdin" && od -c $in
      echo "--- stdout expected" && od -c "$out"
      echo "--- stdout" && od -c "$tmpout"
    else
      [ "$in" != /dev/null ] && echo "--- stdin" && cat $in
      echo "--- stdout expected" && cat "$out"
      echo "--- stdout" && cat "$tmpout"
    fi
    echo "--- exit status: $status"
  fi
  exit 0   # exit 1 doesn't fit the make command...
fi
