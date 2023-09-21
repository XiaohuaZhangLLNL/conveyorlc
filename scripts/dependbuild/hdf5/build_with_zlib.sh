#!/bin/bash

ml  gcc/10.3.1
module load mvapich2/2.3.7
module load cmake/3.23.1

rm -rf build
mkdir -p build
cd build

#ccmake ../  -DCMAKE_INSTALL_PREFIX=/usr/gapps/bbs/TOSS-4/hdfx_contrib/hdf5-1.14.2/ -DZLIB_DIR=/usr/gapps/bbs/TOSS-4/hdfx_contrib/zlib-1.3 

cmake ../ -DCMAKE_INSTALL_PREFIX=/usr/gapps/bbs/TOSS-4/hdfx_contrib/hdf5-1.14.2/ \
  -DHDF5_ENABLE_Z_LIB_SUPPORT=ON \
  -DCMAKE_PREFIX_PATH=/usr/gapps/bbs/TOSS-4/hdfx_contrib/zlib-1.3
 
