#!/bin/bash -e

THISDIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
SUBMODULES=$THISDIR/../../example/benchmark-submodules
PROJECTDIR=$SUBMODULES/$1
STAGE=$THISDIR/build/benchmark
BUILDDIR=$STAGE/$1

if [[ -z $1 ]]; then
    >&2 echo "the project name is needed as first parameter"
    exit 1
fi

if  [[ ! -d  $PROJECTDIR ]]; then
    >&2 echo "non existing project: $PROJECTDIR"
    exit 1
fi

rm -rf $BUILDDIR
mkdir -p $BUILDDIR
cd $BUILDDIR
pwd
cmake -DCMAKE_INSTALL_PREFIX=/install $PROJECTDIR
make
DESTDIR=$STAGE make install