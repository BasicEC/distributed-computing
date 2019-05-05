
#ifndef _UTIL
#define _UTIL

#include <bits/types/FILE.h>
#include "ipc.h"

typedef struct {
	int read;
	int write;
} connection_t;

typedef struct{
	connection_t* left_neighbor;
	connection_t* right_neighor;
} thinker_t;

typedef enum{
  LEFT,
  RIGHT,
  BOTH
} direction;


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
