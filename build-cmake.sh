#!/bin/bash

export BOOST_ROOT=/usr/gapps/bbs/jade/

ccmake ../ -DCMAKE_INSTALL_PREFIX=/usr/gapps/kras/quartz/medcm -DBoost_DIR=/usr/gapps/bbs/jade/ -DBoost_INCLUDE_DIR=/usr/gapps/bbs/jade/include
