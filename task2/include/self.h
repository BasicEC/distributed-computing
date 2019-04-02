#ifndef __IFMO_DISTRIBUTED_CLASS_SELF__H
#define __IFMO_DISTRIBUTED_CLASS_SELF__H

#include "ipc.h"
#include "banking.h"

typedef void (*process_task)(System_t* proc_info, local_id id);


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
    int process_count;
    proc_info_t *processes;
} System_t;

System_t *initialize_System(process_task c_task, process_task p_task, int proc_count, balance_t *balances);

int get_w_pipefd_by_id(proc_info_t *self, local_id dst);

int get_r_pipefd_by_id(proc_info_t *self, local_id dst);

#endif //__IFMO_DISTRIBUTED_CLASS_SELF__H
