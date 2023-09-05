# GCC support can be specified at major, minor, or micro version
# (e.g. 8, 8.2 or 8.2.0).
# See https://hub.docker.com/r/library/gcc/ for all supported GCC
# tags from Docker Hub.
# See https://docs.docker.com/samples/library/gcc/ for more on how to use this image
#FROM gcc:12
FROM ubuntu:22.04

RUN  apt-get update \
	&& DEBIAN_FRONTEND=noninteractive \
	apt-get install --no-install-recommends -y \
 		# gcc libsdl1.2-compat-dev libsdl-ttf2.0-dev libsdl-image1.2-dev \ 
		# libasound2-dev \
		git vim gcc gdb cmake clang net-tools build-essential \
		libsdl1.2-dev libsdl-ttf2.0-dev libsdl-image1.2-dev libsdl-gfx1.2-dev \ 
		libboost-all-dev strace valgrind \
		# libasound2-dev \
 	&& rm -rf /var/lib/apt/lists/*

RUN groupadd -g 1000 goody
RUN useradd -d /userdata/system -s /bin/bash -m goody -u 1000 -g 1000
