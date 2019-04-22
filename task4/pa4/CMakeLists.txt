cmake_minimum_required(VERSION 3.12)
project(pa2 C)

set(CMAKE_C_STANDARD 11)

include_directories(.)

add_executable(pa2
        bank_robbery.c
        banking.h
        common.h
        ipc.c
        ipc.h
        ipc2.h
        pa23.c
        pa2345.h)
