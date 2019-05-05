
#ifndef _UTIL
#define _UTIL

#include <bits/types/FILE.h>
#include "ipc.h"


typedef struct {
	int dirty;
	int enabled;
} fork_t;

typedef struct {
	int read;
	int write;
} connection_t;

typedef struct{
	connection_t* left_neighbor;
	connection_t* right_neighbor;
	fork_t* left_fork;
	fork_t* right_fork;
} thinker_t;

typedef enum{
  DIRECTION_LEFT,
  DIRECTION_RIGHT,
  DIRECTION_BOTH
} direction;

typedef struct{
	Message msg;
	direction dir;
} message_info_t;

typedef struct{
	int thinkers_count;
	thinker_t* thinkers;
} table_t;

timestamp_t get_time();
void set_local_time(timestamp_t time);
void register_event();
void set_childCount(int childCount);
int get_childCount();
FILE* get_pipefile();
void set_pipefile(FILE*);


#endif
