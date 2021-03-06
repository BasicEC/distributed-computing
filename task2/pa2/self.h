#ifndef __IFMO_DISTRIBUTED_CLASS_SELF__H
#define __IFMO_DISTRIBUTED_CLASS_SELF__H

#include <sys/types.h>
#include "ipc.h"
#include "banking.h"


//typedef void (*process_task)();
typedef void (*process_task)(pid_t id);

typedef struct
{
    int read;
    int write;
} connection_t;

typedef struct
{
    local_id id;
    balance_t balance;
    process_task task;
    int connection_count;
    connection_t *connections;
} proc_info_t;

typedef struct
{
    local_id process_count;
    proc_info_t *processes;
} System_t;


System_t* SYSTEM;



System_t *initialize_System(process_task c_task, process_task p_task, int proc_count, balance_t *balances);

int get_w_pipefd_by_id(proc_info_t *self, local_id dst);

int get_r_pipefd_by_id(proc_info_t *self, local_id dst);

#endif //__IFMO_DISTRIBUTED_CLASS_SELF__H
