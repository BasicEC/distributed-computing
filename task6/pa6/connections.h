#ifndef _CONNECTIONS
#define _CONNECTIONS


#include "util.h"

void initPipeLines(table_t*);
int getOpenedPipesFDCount();
void closeUnusedPipes(int selfId, table_t*);
void freePipeLines();
void closePipe(int fd);
int send_to_neighbor(thinker_t*, direction, Message*);
int receive_from_neighbor(thinker_t*, direction, message_info_t*);
int try_receive_message(table_t*, message_info_t*, int);
void close_pipes_by_parent(table_t* table);

#endif
