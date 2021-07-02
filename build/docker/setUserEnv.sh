#!/bin/bash

#This Scripts sets the user env variables to run the gradle tasks in XRT IPU docker container

export XDOCK_USER=${USER}
export XDOCK_UID=$(id -u ${USER})
export XDOCK_GROUP=${GROUP}
export XDOCK_GID=$(id -g ${USER})
