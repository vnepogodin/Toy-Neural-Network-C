#!/bin/bash

wget https://github.com/json-c/json-c/archive/json-c-0.14-20200419.tar.gz
tar -tzvf json-c-0.14-20200419.tar.gz
find -iname "CMakeLists.txt"
mkdir json-c-build && cd json-c-build
find -iname "CMakeLists.txt"
#cmake ../../../json-c-json-c-0.14-20200419
#make
#make install
