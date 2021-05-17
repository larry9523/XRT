# Copyright (c) 2020 Xilinx, Inc.

FROM ubuntu:20.04

ENV DEBIAN_FRONTEND noninteractive

RUN apt-get update -y

RUN apt-get install -y sudo wget tzdata locales vim openjdk-11-jdk

RUN ln -fs /usr/share/zoneinfo/America/Los_Angeles /etc/localtime && dpkg-reconfigure --frontend noninteractive tzdata

RUN dpkg --add-architecture i386

RUN cd /tmp && wget https://github.com/Xilinx/XRT/raw/master/src/runtime_src/tools/scripts/xrtdeps.sh && chmod +x xrtdeps.sh && ./xrtdeps.sh -docker

RUN locale-gen en_US.UTF-8

ARG USER_ID
ARG USER_NAME
ARG GROUP_ID
ARG GROUP_NAME

# You may see errors like:
#   groupadd: group 'users' already exists
# This means that your default group is 'users' which conflicts with what is in 
# the docker image by default
# You can change the group for your image with:
#   docker-compose build --build-arg GROUP_NAME=`id -ng` --build-arg GROUP_ID=`id -g`
# Or you can request your default group change to hd or something similar, via: 
#   help@xilinx.com
RUN groupadd -g ${GROUP_ID} ${GROUP_NAME} || echo "Ignoring error, as ${GROUP_NAME} already exists"
RUN useradd -l -u ${USER_ID} -g ${GROUP_NAME} ${USER_NAME}
RUN echo "ALL ALL=(ALL) NOPASSWD: ALL" >> /etc/sudoers

# Create dirs needed for user
RUN mkdir -p /home/${USER_NAME} /tmp/gradle && chmod -R a+rw /home/${USER_NAME} /tmp/gradle

ENTRYPOINT ["bash", "-c"]