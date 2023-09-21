#!/bin/bash

ml  gcc/12.1.1
ml cmake/3.23.1


rm -rf build-quartz
mkdir build-quartz
cd build-quartz

../configure --prefix=/usr/gapps/bbs/TOSS-4/swig-4.0.1
