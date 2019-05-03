#!/bin/bash

export LD_PRELOAD_PATH="$PWD"

LD_PRELOAD="$PWD/libruntime.so" ./main $@
