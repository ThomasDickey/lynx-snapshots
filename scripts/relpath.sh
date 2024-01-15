#!/bin/sh
# Given two pathnames, print the first relative to the second.
#
# Adapted from
# https://unix.stackexchange.com/questions/573047/how-to-get-the-relative-path-between-two-directories

usage() {
	echo "usage: $0 source target" >&2
	exit 1
}

check_abs() {
	case "$1" in
	/*)
		;;
	*)
		echo "? not an absolute pathname: $1"
		usage
		;;
	esac
}

[ $# = 2 ] || usage
check_abs "$1"
check_abs "$2"

source="$1"
target="$2"
prefix=""

source="`echo "$source" | sed -e 's%/$%%'`/"
target="`echo "$target" | sed -e 's%/$%%'`/"
remain=`echo "$source" | sed -e 's%^'$target'%%'`
while [ -n "$target" ] && [ "$source" = "$remain" ]
do
	prefix="../$prefix"
	target=`echo "$target" | sed -e 's%/[^/]*/$%/%'`
	remain=`echo "$source" | sed -e 's%^'$target'%%'`
done
result="${prefix}${remain}"
[ -n "$result" ] || result="."

echo "$result"
