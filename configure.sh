#!/bin/bash

mkdir src/c/cmake-build-debug
cd src/c
cmake -Wdev . -B cmake-build-debug
