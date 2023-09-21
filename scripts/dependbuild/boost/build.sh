#!/bin/bash

ml  gcc/12.1.1
#module load mvapich2/2.2
ml cmake/3.23.1

#./b2 -d+2 --reconfigure --debug-configuration 2>&1 | tee b2.out
#./b2 install --prefix=/usr/gapps/kras/quartz/boost-1.62.0
./b2 install --prefix=/usr/gapps/bbs/TOSS-4/conveyorlc_contrib/boost-1.72.0
