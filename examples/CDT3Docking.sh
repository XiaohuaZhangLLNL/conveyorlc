#!/bin/bash

export LBindData=/usr/gapps/aha/quartz/conveyorlc_10/data
export PATH=/usr/gapps/aha/quartz/conveyorlc_10/bin:/usr/gapps/aha/quartz/bin:$PATH
export AMBERHOME=/usr/gapps/aha/quartz/amber16
export AMBERHOME10=/usr/gapps/aha/quartz/amber10
export PATH=$AMBERHOME/bin/:$AMBERHOME10/bin/:$PATH

srun -N 1 -n 4 -c 4  -ppdebug CDT3Docking 
