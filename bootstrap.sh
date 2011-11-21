#!/bin/sh

set -e
set -x

git submodule sync
git submodule update

cd boost.m4
./bootstrap
./configure
make

cd ../jansson
autoreconf -fvi
./configure
make

