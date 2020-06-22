#!/bin/bash

git clone https://github.com/json-c/json-c.git -b json-c-0.14-20200419 ../../json-c
mkdir json-c-build && cd json-c-build
cmake ../../../json-c -DCMAKE_BUILD_TYPE="release" -DBUILD_SHARED_LIBS=OFF
make
sudo make install
