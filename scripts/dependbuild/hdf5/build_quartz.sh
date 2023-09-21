#!/bin/bash

ml  gcc/12.1.1
#module load mvapich2/2.2
ml cmake/3.23.1

rm -rf build-quartz
mkdir -p build-quartz
cd build-quartz

ccmake ../  -DCMAKE_INSTALL_PREFIX=/usr/gapps/bbs/TOSS-4/conveyorlc_contrib/hdf5-1.10.4/ 
