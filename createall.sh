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

cmake -S . -B $path/build -DCMAKE_BUILD_TYPE=$type -DCMAKE_TOOLCHAIN_FILE=$path/cmake/LinuxToolchain.cmake --fresh
cmake -S utils -B $path/utils/build -DCMAKE_BUILD_TYPE=$type -DCMAKE_TOOLCHAIN_FILE=$path/cmake/LinuxToolchain.cmake --fresh
