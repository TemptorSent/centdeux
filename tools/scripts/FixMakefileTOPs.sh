#!/bin/sh

optPRETEND=0;
optVERBOSE=0;
optQUIET=0;

if ! test -f ./common/common.mk ; then
	printf "Must be run from top level of build tree.\n"
	exit -1
fi

while getopts nv flag
do
	case "${flag}" in
		n) optPRETEND=1;;
		v) optVERBOSE=1;;
		q) optQUIET=1;;
	esac
done

for file in $(grep -l 'TOP.*=' `find . -type f -name Makefile`) ; do
	[ "${optQUIET}" -eq "1" ] || printf 'Updating %s' "${file}"
	TOP=$(dirname ${file} | sed -e 's|/[^/]*|/..|g' -e 's|^\./|TOP := |')
	[ "${optVERBOSE}" -eq "1" ] && printf ': %s' "${TOP}"
	[ "${optQUIET}" -eq "1" ] || printf '\n'
	[ "${optPRETEND}" -eq "0" ] && sed -e 's|TOP.*=.*|'"${TOP}"'|' -i ${file}
done
