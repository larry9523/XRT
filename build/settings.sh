#/bin/bash
##############################################################
# Copyright (c) 1986-2021 Xilinx, Inc.  All rights reserved. #
##############################################################
root=$(readlink -f $(dirname ${BASH_SOURCE[0]}))
arch=$(uname -m)

host=`hostname`
OSDIST=`lsb_release -i |awk -F: '{print tolower($2)}' | tr -d ' \t'`
OSREL=`lsb_release -r |awk -F: '{print tolower($2)}' |tr -d ' \t'`
default_dir=${arch}/centos-default

# Setup XRT environment variables
if [[ $OSDIST == "ubuntu" ]]; then
    
    # if [[ $OSREL != "16.04" ]] &&  [[ $OSREL != "18.04" ]] &&  [[ $OSREL != "20.04" ]]; then
    #     echo "Ubuntu $OSREL detected (${host}), minimal setup"
    #     set_path_only=1
    # fi
    OSDIST="ubuntu"
fi

if [[ $OSDIST == "centos" ]] || [[ $OSDIST == "redhat"* ]]; then
    # if [[ $OSREL != "7.4"* ]] &&  [[ $OSREL != "7.5"* ]] &&  [[ $OSREL != "7.6"* ]] &&  [[ $OSREL != "7.7"* ]] &&  [[ $OSREL != "7.8"* ]] &&  [[ $OSREL != "7.9"* ]] &&  [[ $OSREL != "8.1"* ]] &&  [[ $OSREL != "8.2"* ]] &&  [[ $OSREL != "8.3"* ]] &&  [[ $OSREL != "8.4"* ]] ; then
    #     echo "Centos/RHEL $OSREL detected (${host}), minimal setup"
    #     set_path_only=1
    # fi
    # # only need major.manor
    # OSREL=`echo $OSREL | awk -F '.' '{print $1"."$2}'`
    # # default CentOS is 7.8
    # if [[ $OSREL == "7."* ]] ; then
    #   OSREL="7.8"
    # else
    #   OSREL="8.1"
    # fi
    #CentOS = RHEL
    OSDIST="centos"

fi
dir=${arch}/${OSDIST}-default

if [ ! -d "${root}/${dir}" ]; then
  tmpDir=${arch}/${OSDIST}-default
  echo "## ${dir} does not exists trying default for: ${OSDIST}"
  if [ ! -d "${root}/${tmpDir}" ]; then
    echo "# Default directory (${tmpDir}) for ${OSDIST} does not exists will use common directory: ${default_dir} "
    dir=${default_dir}
  else
    echo "# Default directory (${default_dir}) for ${OSDIST} exists will use it."
    dir=${tmpDir}
  fi
fi

echo "Sourcing: ${root}/${dir}/opt/xilinx/xrt/setup.sh"
source ${root}/${dir}/opt/xilinx/xrt/setup.sh
