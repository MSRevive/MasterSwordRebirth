#!/bin/bash

set -eu

cd $(dirname "${BASH_SOURCE[0]}")

path=$(realpath .)
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

echo "cmake --version"
cmake --version

echo "gcc -v"
gcc -v

./createall.sh -p $path -t $type

make -C $path/build
