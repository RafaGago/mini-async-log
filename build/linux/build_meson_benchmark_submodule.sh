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

if [[ -z $2 ]]; then
    >&2 echo "the build type is needed as a second parameter"
    exit 1
fi

if  [[ ! -d  $PROJECTDIR ]]; then
    >&2 echo "non existing project: $PROJECTDIR"
    exit 1
fi

rm -rf $BUILDDIR
mkdir -p $BUILDDIR
cd $PROJECTDIR
meson $BUILDDIR --prefix=/install --buildtype=$2
ninja -C $BUILDDIR
DESTDIR=$STAGE ninja -C $BUILDDIR install
