#!/bin/bash

ml  gcc/12.1.1
#module load mvapich2/2.2
ml cmake/3.23.1

export PATH="/usr/gapps/bbs/TOSS-4/swig-4.0.1/bin/:/usr/gapps/bbs/TOSS-4/anaconda3/bin:$PATH"

rm -rf build-quartz
mkdir -p build-quartz
cd build-quartz
### with python
#ccmake ../ -DCMAKE_INSTALL_PREFIX:PATH=/usr/gapps/aha/quartz/ -DEIGEN3_INCLUDE_DIR=/usr/gapps/aha/quartz/include/eigen3/ -DPYTHON_BINDINGS=ON -DRUN_SWIG=ON

### without python

ccmake ../ -DCMAKE_INSTALL_PREFIX:PATH=/usr/gapps/bbs/TOSS-4/conveyorlc_contrib/openbabel.3.1.1 -DEIGEN3_INCLUDE_DIR=/usr/gapps/bbs/TOSS-4/eigen_3.3.7/include/eigen3/ -DBOOST_ROOT=/usr/gapps/bbs/TOSS-4/conveyorlc_contrib/boost-1.72.0 -DBoost_DIR=/usr/gapps/bbs/TOSS-4/conveyorlc_contrib/boost-1.72.0 -Dboost_zlib_DIR=/lib64/
