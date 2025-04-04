#!/bin/bash

set -eu

cd $(dirname "${BASH_SOURCE[0]}")

./createall

make -C build

