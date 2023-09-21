#!/bin/bash

export CUDA_HOME=/usr/tce/packages/cuda/cuda-9.2.148/
CC=/usr/tce/packages/gcc/gcc-7.3.1/bin/gcc CXX=/usr/tce/packages/gcc/gcc-7.3.1/bin/g++ FC=/usr/tce/packages/gcc/gcc-7.3.1/bin/gfortran ./configure -cuda -noX11 -nosse  gnu
