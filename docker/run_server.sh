#!/bin/bash

# error on unbound variable
set -u

readonly red="\033[31m"
readonly normal="\033[0m"
readonly thisdir=$(realpath $(dirname ${BASH_SOURCE[0]}))

readonly confdir="/workspace/server/msr"
readonly dlldir="/workspace/server/msr/dlls/"

[[ ${DOCKER_RUNNING:-} != "true" ]] && { echo "⚠️ This script only runs inside the container."; exit 1; }

echo -e "⚙️ Copying config files to ${red}msrebirth${normal} server directory <${confdir}>.\n"

# need full binary path because of fedora alias
/usr/bin/cp -f ${thisdir}/*.cfg ${confdir}

[[ -e /workspace/msrebirth/build/src/game/server/ms.so ]] && {
    echo -e "⚙️ Copying built ms.so file to ${red}msrebirth${normal} server directory <${dlldir}>.\n";
    /usr/bin/cp -f /workspace/msrebirth/build/src/game/server/ms.so ${dlldir}/ms.so;
}

echo -e "⚔️ Starting ${red}msrebirth${normal} server... \
       \n💡 If the error 'engine_i486.so: cannot enable executable stack' appears, run 'patchelf --clear-execstack /workspace/server/engine_i486.so'"

cd /workspace/server
exec ./msr.sh
