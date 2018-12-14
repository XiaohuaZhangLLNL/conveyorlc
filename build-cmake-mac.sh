#!/bin/bash

# For Mac, the openMPI should use instead of mpich and boost should build with openMPI.
CC=/Users/zhang30/inst/macports/bin/clang-mp-7.0 CXX=/Users/zhang30/inst/macports/bin/clang++-mp-7.0 ccmake ./ -DCMAKE_INSTALL_PREFIX=/Users/zhang30/inst/conveyorlc -DBOOST_ROOT=/Users/zhang30/inst/macports/ -DHDF5_ROOT=/Users/zhang30/inst/macports/ -DCONDUIT_DIR=/Users/zhang30/inst/conveyorlc_contrib/conduit/
#CC=/opt/local/bin/gcc-mp-4.9 CXX=/opt/local/bin/g++-mp-4.9 cmake ../  -DCMAKE_INSTALL_PREFIX=/Users/zhang30/inst/conveyorlc -DBOOST_ROOT=/Users/zhang30/inst/boost_1_61_0

#cmake ../ -DCMAKE_C_COMPILER=/opt/local/bin/gcc-mp-4.9 -DCMAKE_CXX_COMPILER=/opt/local/bin/g++-mp-4.9 -DCMAKE_INSTALL_PREFIX=/Users/zhang30/inst/conveyorlc -DBOOST_ROOT=/Users/zhang30/inst/boost_1_61_0
