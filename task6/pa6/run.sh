#!/bin/bash

export LD_LIBRARY_PATH="$PWD"

LD_PRELOAD="$PWD/libruntime.so" ./main $@

# run from source : source tun.sh -p N --mutexl
