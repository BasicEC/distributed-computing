#ifndef __IFMO_DISTRIBUTED_CLASS_IPC2__H
#define __IFMO_DISTRIBUTED_CLASS_IPC2__H

#include "ipc.h"
#include "banking.h"
#include <unistd.h>

int usleep(__useconds_t usec);

void close_connections(void *self, int proc);

//int usleep(__useconds_t usec);
int receive_all(void *self, Message *msgs, MessageType type);

void createMessageHeader(Message *msg, MessageType type);

typedef struct {
    int procCount;
    int ***fds;
} Connections_t;


typedef struct {
    Connections_t connections;
    local_id self;
} proc_info;


typedef struct {
    int process_count;
    balance_t* balances;
} parsed_arguments;


typedef struct {
    proc_info info;
    BalanceHistory history;
} prepared_history;
#endif
