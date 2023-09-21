#!/bin/bash

module load gcc/7.3.1
module load cuda/9.2.148
export CUDA_HOME=/usr/tce/packages/cuda/cuda-9.2.148/
export PATH="/usr/gapps/bbs/CORAL-Lassen/amber18_volta/miniconda/bin:$PATH"
CC=/usr/tce/packages/gcc/gcc-7.3.1/bin/gcc CXX=/usr/tce/packages/gcc/gcc-7.3.1/bin/g++ FC=/usr/tce/packages/gcc/gcc-7.3.1/bin/gfortran ./configure -cuda -volta -noX11 -nosse  gnu
