#!/bin/sh -e

size="$1"
target="$2"

trap 'rm -f "$target".mask.png "$target".ter.png 2>/dev/null' EXIT

if [ -z "$size" -o -z "$target" ] ; then
	echo "Usage:" >&2
	echo "    $0 size target" >&2
	echo "E.g.:" >&2
	echo "    $0 256 test.png" >&2
	echo 1
fi

set -x

./diamond_square.py --size="$size" "$target".ter.png
./island_mask.py --width="$size" --height="$size" "$target".mask.png
composite -compose Multiply "$target".ter.png "$target".mask.png "$target"

