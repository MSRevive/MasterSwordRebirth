#!/bin/bash

# error on unbound variable
set -u

[[ ${DOCKER_RUNNING:-} != "true" ]] && { echo "⚠️ This script only runs inside the container."; exit 1; }
[[ -z "${STEAM_USER:-}" ]] && { echo "⚠️ Need to export the <STEAM_USER> environment variable e.g. 'export STEAM_USER=whatever'."; exit 1; }

echo "⚙️ Running steamCMD"
sudo -u steam /home/steam/bin/steamcmd.sh +force_install_dir /workspace/server/ +login ${STEAM_USER} +app_update 1961680 validate +quit || { echo "⚠️ Installation failed!"; exit 1; }

# Fix execstack error: 'engine_i486.so: cannot enable executable stack as shared object'
patchelf --clear-execstack /workspace/server/engine_i486.so
