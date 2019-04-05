
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include "self.h"
#include "ipc.h"
#include "common.h"
#include "pa2345.h"

#ifndef __IFMO_DISTRIBUTED_CLASS_MESSAGER__H
#define __IFMO_DISTRIBUTED_CLASS_MESSAGER__H

void close_all_unused_connections(System_t *sys, int index);

Message create_message(char* payload, uint16_t len, int16_t type);

#endif
