#!/bin/bash

set -eu

cd $(dirname "${BASH_SOURCE[0]}")

path=.
type=Release

while getopts "p:t:" opt; do
	case "${opt}" in
		p)
			path=${OPTARG}
			;;
		t)
			type=${OPTARG}
			;;
	esac
done

./createall.sh -p $path -t $type

cmake --build $path/build --config $type
