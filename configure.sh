#!/bin/bash

mkdir src/c/cmake-build-debug
cd src/c
cmake -Wdev -S . -B cmake-build-debug
