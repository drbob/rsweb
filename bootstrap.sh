#!/bin/sh

set -e
set -x

PREFIX=$(readlink -f build-env)

git submodule sync
git submodule update

cd boost.m4
./bootstrap
./configure --prefix="$PREFIX"
make
make install

cd ../jansson
autoreconf -fvi
./configure --prefix="$PREFIX"
make
make install

cd ../
hg clone https://code.google.com/p/pagedown/ || (cd pagedown && hg pull && hg update && cd ../)

wget http://freefr.dl.sourceforge.net/project/threadpool/threadpool/0.2.5%20%28Stable%29/threadpool-0_2_5-src.zip
unzip threadpool-0_2_5-src.zip
cd threadpool
cp -vR boost "$PREFIX/include"
cd ../

wget https://www.openssl.org/source/openssl-0.9.8r.tar.gz
tar -zxvf openssl-0.9.8r.tar.gz
cd ../openssl-0.9.8r
./config --prefix="$PREFIX" threads shared static zlib-dynamic
make
make test
make install


