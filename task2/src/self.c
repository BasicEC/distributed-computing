#include "self.h"
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include "logs.h"

void disable_blocks(int fd)
{
    int flags = fcntl(fd, F_GETFL);
    fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

int create_pipe_without_blocks(int *fd)
{
    if (pipe(fd) != 0)
        return -1;
    disable_blocks(*fd);
    disable_blocks(fd[1]);
    return 0;
}



void unidirectional_connection(proc_info_t *send, proc_info_t *receive)
{
    int fd[2];
    create_pipe_without_blocks(fd);
    receive->connections[send->id].read = fd[0];
    send->connections[receive->id].write = fd[1];
    log_pipe(OPEN, 0, send->id, receive->id, fd[1]);
}

void establish_all_connections(System_t *sys)
{
    int i, j;
    for (i = 0; i < sys->process_count; i++)
    {
        for (j = 0; j < sys->process_count; j++)
        {

            if (i != j)
            {
                unidirectional_connection((sys->processes + i), (sys->processes + j));
            }
        }
    }
}




int get_w_pipefd_by_id(proc_info_t *self, local_id dst)
{
    return self->connections[dst].write;
}

int get_r_pipefd_by_id(proc_info_t *self, local_id dst)
{
    return self->connections[dst].read;
}

void initialize_proc(proc_info_t *proc, process_task task, int conn_count)
{
    proc->task = task;
    proc->connection_count = conn_count;
    proc->connections = malloc(sizeof(connection_t) * (conn_count));
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
    establish_all_connections(sys);
    return sys;
}
