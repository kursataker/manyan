#!/bin/sh
# usage: bench/quick.sh [-workers N] ENGINE...
# Best of 5 wall-clock seconds of each engine on each bench spec, side by side.
workers=1
[ "$1" = "-workers" ] && { workers=$2; shift 2; }
root=$(cd "$(dirname "$0")/.." && pwd)
for spec in closure counters log mutex queue; do
  d=$(mktemp -d)
  python3 -c "import sys; sys.path.insert(0, '$root/tests'); import spec_runner, pathlib; spec_runner.stage(pathlib.Path('$root/bench/$spec.tla'), pathlib.Path('$d'))"
  printf "%-9s" $spec
  for eng in "$@"; do
    case $eng in /*) ;; *) eng=$PWD/$eng ;; esac
    best=$(cd "$d" && for i in 1 2 3 4 5; do /usr/bin/time -p "$eng" -workers $workers -config $spec.cfg $spec 2>&1 >/dev/null | awk '/real/{print $2}'; done | sort -n | head -1)
    printf " %8s" "$best"
  done
  echo
  rm -rf "$d"
done
