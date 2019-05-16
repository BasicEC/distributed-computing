
#ifndef _UTIL
#define _UTIL

#include "ipc.h"
#include <stdio.h>


typedef struct {
	int dirty;
	int enabled;
} fork_t;

typedef struct {
	int read;
	int write;
} connection_t;

typedef struct{
	int id;
	connection_t* connections;
	int connection_count;
	fork_t* forks;
} thinker_t;


typedef struct{
	Message msg;
	int dir;
} message_info_t;

typedef struct{
	int thinkers_count;
	thinker_t* thinkers;
	connection_t* parent_connections;
} table_t;


typedef struct{
    int left_neighbor;
	int right_neighbor;
} delayed_transfer_t;

timestamp_t get_time();
void set_local_time(timestamp_t time);
void register_event();
void set_childCount(int childCount);
int get_childCount();
FILE* get_pipefile();
void set_pipefile(FILE*);


#endif
