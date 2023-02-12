#!/bin/bash

cd build
if [ ! -f "boost_1_81_0_rc1.tar.gz" ];
then
    wget "https://boostorg.jfrog.io/artifactory/main/release/1.81.0/source/boost_1_81_0_rc1.tar.gz"
fi

curr=$(sha256sum boost_1_81_0_rc1.tar.gz)

echo $curr

if [[ "$curr" == *"205666dea9f6a7cfed87c7a6dfbeb52a2c1b9de55712c9c1a87735d7181452b6"* ]];
then
    tar xvf boost_1_81_0_rc1.tar.gz
    cd boost_1_81_0
    ./bootstrap.sh --prefix=/usr/
    ./b2
    ./b2 install
else
    echo "error checksum"
fi
