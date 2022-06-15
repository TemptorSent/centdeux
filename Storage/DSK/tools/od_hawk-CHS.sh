#!/bin/sh

if [ "$#" -lt 1 ] ; then
	printf "Filename required\n"
	exit -1
fi

if [ "$#" -lt 2 ] ; then
	cmd="od -Ad -tx1 -v"
else
	cmd="od -Ad"
fi
while [ "$#" -gt 1 ] ; do
	cmd="${cmd} $1"
	shift
done

if ! [ -f "$1" ] ; then
	printf "File '%s' does not exist\n" "$1"
	exit -1
fi

${cmd} "$1" | awk '
	NR==1 {
		offset=index($0," ");
	};

	/^[0-9a-f]+/ {
		b=$1%400;
		sec=int($1/400);
		S=sec%16;
		H=(sec%32)/16;
		C=int(sec/32);
	};

	{
		printf("%06x(%03x,%01x,%02x)+%03x ",sec,C,H,S,b);
		printf("%s\n",substr($0,offset));
	};
'
