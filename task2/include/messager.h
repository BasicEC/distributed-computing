
#ifndef __IFMO_DISTRIBUTED_CLASS_MESSAGER__H
#define __IFMO_DISTRIBUTED_CLASS_MESSAGER__H

#include "child.h"
#include "self.h"


void close_all_unused_connections(System_t *sys, int index);

Message create_message(char* payload, uint16_t len, int16_t type);

#endif
