#include "self.h"
#include "self.h"
#include <stdlib.h>

int get_w_pipefd_by_id(proc_info_t *self, local_id dst)
{
    return self->connections[dst].write;
}

int get_r_pipefd_by_id(proc_info_t *self, local_id dst)
{
    return self->connections[dst].read;
}

static void initialize_child(proc_info_t *child, process_task task, int conn_count, balance_t balance, local_id id)
{
    child->id = id;
    child->balance = balance;
    initialize_proc(child, task, conn_count);
}

static void initialize_parent(proc_info_t *parent, process_task task, int conn_count)
{
    parent->id = 0;
    initialize_proc(parent, task, conn_count);
}

//void initialize_proc(proc_info_t *proc, process_task task, int conn_count)
//{
//    proc->task = task;
//    proc->connection_count = conn_count;
//    proc->connections = malloc(sizeof(connection_t) * (parent->connection_count));
//}

System_t *initialize_System(process_task c_task, process_task p_task, int proc_count, balance_t *balances)
{
    System_t *sys = (System_t *)malloc(sizeof(System_t));
    sys->process_count = proc_count;
    proc_info_t *children = (proc_info_t *)malloc(sizeof(proc_info_t) * sys->process_count);
    sys->processes = children;

    initialize_parent(&sys->processes[0], p_task, proc_count);
    
    int i;
    for (i = 1; i < sys->process_count; i++)
    {
        initialize_child(&sys->processes[i], c_task, proc_count, balances[i - 1], (local_id)i);
    }
    return sys;
}