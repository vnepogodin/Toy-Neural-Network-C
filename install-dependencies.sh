#!/bin/bash

git clone https://github.com/json-c/json-c.git ../../json-c
mkdir json-c-build && cd json-c-build
cmake ../../../json-c -DCMAKE_BUILD_TYPE="release"
make
sudo make install
