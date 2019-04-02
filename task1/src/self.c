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

void initialize_child(proc_info_t *child, process_task task, int conn_count)
{
    child->task = task;
    child->connection_count = conn_count;
    child->connections = malloc(sizeof(connection_t) * (child->connection_count));
}

System_t *initialize_System(process_task task, int proc_count)
{
    System_t *sys = (System_t *)malloc(sizeof(System_t));
    sys->process_count = proc_count;
    proc_info_t *children = (proc_info_t *)malloc(sizeof(proc_info_t) * sys->process_count);
    sys->processes = children;
    local_id i;
    for (i = 0; i < sys->process_count; i++)
    {
        sys->processes[i].id = (local_id)(i);
        initialize_child(&sys->processes[i], task, proc_count + 1);
    }
    return sys;
}