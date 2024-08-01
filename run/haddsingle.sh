#!/bin/bash


source /opt/sphenix/core/bin/sphenix_setup.sh -n
source /opt/sphenix/core/bin/setup_local.sh "/sphenix/user/jocl/projects/testinstall"
export HOME=/sphenix/u/jocl
rm -f "output/sumroot/summed_${1}_base.root"
hadd "output/sumroot/summed_${1}_base.root" `ls output/root/*${1}*fullfile*`
