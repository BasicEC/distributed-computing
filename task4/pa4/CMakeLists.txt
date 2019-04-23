cmake_minimum_required(VERSION 3.12)
project(pa2)

set(CMAKE_CXX_STANDARD 14)

include_directories(.)

add_executable(pa2
        bank_robbery.c
        banking.c
        banking.h
        child.c
        child.h
        common.h
        ipc.c
        ipc.h
        libruntime.so
        logs.c
        logs.h
        main.c
        messager.c
        messager.h
        pa2345.h
        self.c
        self.h)
