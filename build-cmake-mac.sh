#!/bin/bash

# For Mac, the openMPI should use instead of mpich and boost should build with openMPI.
cmake ../ -DCMAKE_INSTALL_PREFIX=/Users/zhang30/inst/medcm -DBOOST_ROOT=/Users/zhang30/inst/medcm/contrib/
#CC=/opt/local/bin/gcc-mp-4.9 CXX=/opt/local/bin/g++-mp-4.9 cmake ../  -DCMAKE_INSTALL_PREFIX=/Users/zhang30/inst/medcm -DBOOST_ROOT=/Users/zhang30/inst/boost_1_61_0

#cmake ../ -DCMAKE_C_COMPILER=/opt/local/bin/gcc-mp-4.9 -DCMAKE_CXX_COMPILER=/opt/local/bin/g++-mp-4.9 -DCMAKE_INSTALL_PREFIX=/Users/zhang30/inst/medcm -DBOOST_ROOT=/Users/zhang30/inst/boost_1_61_0
