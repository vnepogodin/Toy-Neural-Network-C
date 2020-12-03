#!/bin/bash

cmake --build build --parallel 10
cmake --build build --target run --parallel 10
