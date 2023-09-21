#!/bin/bash

ml  gcc/12.1.1
#module load mvapich2/2.2
ml cmake/3.23.1

#rm -rf build-corona 
#mkdir build-corona

#rm -rf build-quartz
#mkdir build-quartz
cd build-quartz

ccmake ../src/ -DCMAKE_INSTALL_PREFIX:PATH=/usr/gapps/bbs/TOSS-4/conveyorlc_contrib/conduit/ -DHDF5_DIR=/usr/gapps/bbs/TOSS-4/conveyorlc_contrib/hdf5-1.10.4/ -DHDF5_ROOT=/usr/gapps/bbs/TOSS-4/conveyorlc_contrib/hdf5-1.10.4/
#ccmake ../src/ -DCMAKE_INSTALL_PREFIX:PATH=/usr/gapps/bbs/TOSS-3/conduit-intel -DHDF5_DIR=/usr/gapps/aha/quartz/hdf5-1.10.4/ -DHDF5_ROOT=/usr/gapps/aha/quartz/hdf5-1.10.4/
