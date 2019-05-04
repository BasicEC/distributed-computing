#ifndef _CONNECTIONS
#define _CONNECTIONS

#include "util.h"

typedef struct {
	int read;
	int write;
} connection_t;

void initPipeLines(table_t*);
int getOpenedPipesFDCount();
void closeUnusedPipes(int selfId);
void freePipeLines();
void closePipe(int fd);


#endif
