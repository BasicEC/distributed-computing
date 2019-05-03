#ifndef _UTIL
#define _UTIL

#include "ipc.h"
#include<stdio.h>

timestamp_t get_time();
void set_local_time(timestamp_t time);
void register_event();
void set_childCount(int childCount);
int get_childCount();
FILE* get_pipefile();
void set_pipefile(FILE*);
#endif
