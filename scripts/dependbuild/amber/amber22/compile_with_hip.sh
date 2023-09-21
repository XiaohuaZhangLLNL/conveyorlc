#!/bin/bash

#This script compiles AMBER to be executed on AMD GPUs.

#NOTES:

#  - Make sure you have rocm installed in your system ($ROCM_PATH should point to rocm install directory).

#  - AMBER has been tested with ROCM version 5.0 and below.

#  - If you want to build AMBER on NAVI cards, add "-D HIP_WARP64=OFF" to cmake.

#  - For parallel run with MPI, OpenMPI with UCX and ROCM support should be installed on your system.

#  - For parallel run with MPI, add "-D MPI=ON" to cmake command.

#  - For parallel runs with MPI, please set HSA_FORCE_FINE_GRAIN_PCIE=1 as an environment variable.

#  - At runtime, use HIP_VISIBLE_DEVICES to select the GPU(s).

#  - For maximum performance, run "rocm-smi --showtopo" and "lscpu" to get numa affinity of GPUs and 
#    use "numactl -C " to bind cores that are closer to selected GPUs .


PREFIX_DIR=/usr/gapps/bbs/TOSS-4-Tioga/amber22

cmake \
    -D CMAKE_BUILD_TYPE=Release  \
    -D COMPILER=AUTO \
    -D CMAKE_C_COMPILER=gcc \
    -D CMAKE_CXX_COMPILER=g++ \
    -D CMAKE_Fortran_COMPILER=gfortran \
    -D CMAKE_CXX_FLAGS="-O3 " \
    -D CMAKE_C_FLAGS="-O3 " \
    -D CMAKE_Fortran_FLAGS="-O3"  \
    -D DISABLE_WARNINGS=ON \
    -D USE_FFT=ON \
    -D BUILD_PYTHON=OFF \
    -D CHECK_UPDATES=OFF \
    -D DOWNLOAD_MINICONDA=OFF \
    -D HIP=ON \
    -D GTI=TRUE \
    -D VKFFT=ON \
    -D HIP_RDC=ON \
    -D HIP_TOOLKIT_ROOT_DIR=$ROCM_PATH \
    -D HIPCUDA_EMULATE_VERSION="10.1" \
    -D BUILD_HOST_TOOLS=ON \
    -D CMAKE_INSTALL_PREFIX=$PREFIX_DIR/tools \
    -S . \
    -B build/tools


cmake \
    --build build/tools \
    --target install \
    -j16


cmake \
    -D CMAKE_BUILD_TYPE=Release  \
    -D COMPILER=AUTO \
    -D CMAKE_C_COMPILER=gcc \
    -D CMAKE_CXX_COMPILER=g++ \
    -D CMAKE_Fortran_COMPILER=gfortran \
    -D CMAKE_CXX_FLAGS="-O3 " \
    -D CMAKE_C_FLAGS="-O3 " \
    -D CMAKE_Fortran_FLAGS="-O3" \
    -D DISABLE_WARNINGS=ON \
    -D USE_FFT=ON \
    -D BUILD_PYTHON=OFF \
    -D CHECK_UPDATES=OFF \
    -D DOWNLOAD_MINICONDA=OFF \
    -D HIP=ON \
    -D GTI=TRUE \
    -D HIP_RDC=ON \
    -D VKFFT=ON \
    -D HIP_TOOLKIT_ROOT_DIR=$ROCM_PATH \
    -D HIPCUDA_EMULATE_VERSION="10.1" \
    -D BUILD_HOST_TOOLS=OFF \
    -D USE_HOST_TOOLS=ON \
    -D HOST_TOOLS_DIR=$PREFIX_DIR/tools \
    -D CMAKE_INSTALL_PREFIX=$PREFIX_DIR \
    -S . \
    -B build/amber \

cmake \
    --build build/amber \
    --target xblas_build \

cmake \
    --build build/amber \
    --target install \
    -j16

