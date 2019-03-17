#ifndef __IFMO_DISTRIBUTED_CLASS_SELF__H
#define __IFMO_DISTRIBUTED_CLASS_SELF__H

#include "ipc.h"

typedef void (*process_task)();


typedef struct
{
    int read;
    int write;
} connection_t;

typedef struct
{
    local_id id;
    process_task task;
    int connection_count;
    connection_t *connections;
} proc_info_t;

typedef struct
{
    int process_count;
    proc_info_t *processes;
} System_t;

int get_w_pipefd_by_id(proc_info_t *self, local_id dst);

int get_r_pipefd_by_id(proc_info_t *self, local_id dst);

#endif //__IFMO_DISTRIBUTED_CLASS_SELF__H
