#ifndef _CONNECTIONS
#define _CONNECTIONS


#include "util.h"

void initPipeLines(table_t*);
int getOpenedPipesFDCount();
void closeUnusedPipes(int selfId, table_t*);
void freePipeLines();
void closePipe(int fd);
int try_receive_message(thinker_t*, Message*);
void close_pipes_by_parent(table_t* table);

#endif
