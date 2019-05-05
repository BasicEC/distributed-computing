#ifndef _CONNECTIONS
#define _CONNECTIONS


#include "util.h"

void initPipeLines(table_t*);
int getOpenedPipesFDCount();
void closeUnusedPipes(int selfId, table_t* table);
void freePipeLines();
void closePipe(int fd);
int send_to_neighbor(thinker_t*, direction, Message*);

#endif
