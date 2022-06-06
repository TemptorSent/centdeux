#!/bin/sh
SCRIPTDIR="`dirname -- "$0";`"

if [ "$#" -lt 1 ] ; then
	printf "Requires filename to strip as argument.\n"
	exit 1
fi
if ! [ -e "$1" ] ; then
	printf "File %s not found\n" "$1"
	exit 1
fi

cat "$1" \
	| "${SCRIPTDIR}/stripMSB" \
	| strings -td \
	| sed -e 's/^[[:space:]]*//' \
	| awk '{printf("0x%08x:\t%s\n",$1,$2);}'
