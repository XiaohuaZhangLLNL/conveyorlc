#!/bin/bash

#set up mpi path
ml gcc-tce/10.2.1
module load mvapich2-tce/2.3.6
#module load cmake/3.12.1
module load cmake/3.21.1

rm -rf build-corona
mkdir -p build-corona
cd build-corona
ccmake ../ -DCMAKE_INSTALL_PREFIX:PATH=/usr/gapps/bbs/TOSS-4/eigen_3.3.7 -DBoost_DIR=/usr/gapps/bbs/TOSS-4-Corona/boost-1.72.0
