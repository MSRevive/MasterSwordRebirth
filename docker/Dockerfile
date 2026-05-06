# syntax=docker/dockerfile:1
FROM fedora:latest as BASE_IMAGE

RUN dnf install -y su sudo cmake gcc gcc-c++ pkgconf-pkg-config libstdc++.i686 libstdc++-static.i686 gdb gdbserver strace file binutils patchelf htop nano procps-ng  \
    glibc.i686 glibc-devel.i686 \
    libatomic.i686 libatomic.x86_64 \
    glfw-devel.i686 \
    libglvnd-glx.i686 \
    mesa-libGL-devel.i686 \
    protobuf-devel.i686 \
    libcurl-devel.i686 \
    zlib-ng-compat-devel.i686 \
    gtest.i686 gtest-devel.i686


FROM BASE_IMAGE as MSR_BUILDER

# copy developer configs
COPY ./*.cfg ./*.sh /workspace/scripts/
RUN chmod -x /workspace/scripts/*.sh && \
    chmod u+x /workspace/scripts/install_server.sh && \
    chmod u+x /workspace/scripts/run_server.sh

# setup steam user
RUN useradd -m steam && \
    mkdir -p /workspace/server/ && \
    mkdir -p /home/steam/bin

WORKDIR /home/steam/bin
# install steamCMD and download HLDS (steam_legacy)
RUN curl -sqL "https://steamcdn-a.akamaihd.net/client/installer/steamcmd_linux.tar.gz" | tar zxvf - && \
    chown -R steam:steam /home/steam/bin

# we can check with this env if a script runs inside docker
ENV DOCKER_RUNNING=true

WORKDIR /workspace/
ENTRYPOINT ["bash"]
