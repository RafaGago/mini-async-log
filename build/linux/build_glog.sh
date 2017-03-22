#!/bin/bash -e

THISDIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
SUBMODULES=$THISDIR/../../example/benchmark-submodules
cd $SUBMODULES/glog
rm -rf build && mkdir build && cd build
cmake -DCMAKE_INSTALL_PREFIX=/install ..
make
DESTDIR=$SUBMODULES/glog/ make install
