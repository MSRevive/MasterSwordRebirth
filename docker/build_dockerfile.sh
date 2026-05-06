#!/bin/bash

# error on unbound variable
set -u

readonly red="\033[31m"
readonly normal="\033[0m"

# check if docker is installed unless ${CONTAINER_BUILDER} is already set
[[ -z "${CONTAINER_BUILDER:-}" ]] && CONTAINER_BUILDER=$(which docker 2>/dev/null)
# if docker is not installed check for podman
[[ -z ${CONTAINER_BUILDER:-} ]] && CONTAINER_BUILDER=$(which podman 2>/dev/null)
# cannot run without either podman or docker, so better exit now
[[ -z ${CONTAINER_BUILDER:-} ]] && { echo "⚠️ Need to have either docker or podman installed!"; exit 1; }

# If you have just installed podman and want to run without root/sudo you need to assign extra subuids/subgids to your user and run podman system migrate like this:
#
# sudo usermod --add-subuids 100000-165535 --add-subgids 100000-165535 $USER
# podman system migrate

${CONTAINER_BUILDER} build . -t msrebirth || exit 1

echo -e "🏗️ Finished building ${red}msrebirth${normal} container"
