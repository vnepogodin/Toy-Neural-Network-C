#!/bin/bash

sudo apt install cmake
wget https://github.com/json-c/json-c/archive/json-c-0.14-20200419.tar.gz
tar -tzvf json-c-0.14-20200419.tar.gz
mv json-c-json-c-0.14-20200419/ json-c/
mkdir json-c-build
cd json-c-build
cmake ../json-c
make
make install
