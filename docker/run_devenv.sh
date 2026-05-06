#!/bin/bash

# error on unbound variable
set -u

readonly red="\033[31m"
readonly normal="\033[0m"
readonly thisdir=$(realpath $(dirname ${BASH_SOURCE[0]}))

# check if docker is installed unless ${CONTAINER_BUILDER} is already set
[[ -z "${CONTAINER_BUILDER:-}" ]] && CONTAINER_BUILDER=$(which docker 2>/dev/null)
# if docker is not installed check for podman
[[ -z ${CONTAINER_BUILDER:-} ]] && CONTAINER_BUILDER=$(which podman 2>/dev/null)
# cannot run without either podman or docker, so better exit now
[[ -z ${CONTAINER_BUILDER:-} ]] && { echo "⚠️ Need to have either docker or podman installed!"; exit 1; }

readonly builder=$(basename ${CONTAINER_BUILDER})

# check if the image was built
[[ -z "$(${CONTAINER_BUILDER} images -q msrebirth:latest)" ]] && { echo "⚠️ Need to build the ${red}msrebirth${normal} image first!"; exit 1; }


# check if the container was never started before
if [[ -z "$(${CONTAINER_BUILDER} container inspect msrebirth --format '{{.Id}}' 2>/dev/null)" ]]; then
    echo -e "⚔️ Launching clean ${red}msrebirth${normal} container...\n"
    mkdir -p ${thisdir}/hlds
    ${CONTAINER_BUILDER} run -it --name msrebirth --hostname msrebirth-debug --net=host -v $(dirname ${thisdir}):/workspace/msrebirth -v ${thisdir}/hlds:/workspace/server msrebirth:latest
else
    echo -e "⚔️ Launching existing ${red}msrebirth${normal} container... \
    \n💡 Run '${builder} container rm msrebirth' before starting this script to start clean or to start a new version.\n"

    ${CONTAINER_BUILDER} start --attach --interactive msrebirth
fi
