#!/bin/bash

export LD_LIBRARY_PATH="$PWD"

LD_PRELOAD="$PWD/libruntime.so" ./main $@

# USAGE : ./run -p N (1 .. N)
