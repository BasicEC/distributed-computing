#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <wait.h>
#include "ipc.h"
#include "common.h"
#include "self.h"
#include "banking.h"
#include "logs.h"

int PROCESS_COUNT;
int CONNECTIONS_COUNT;

void do_smth(){}

balance_t *parse_arguments(char **args)
{
    void *ptr = NULL;
    if (strcmp(args[1], "-p") == 0 || strcmp(args[1], "-P") == 0)
    {
        PROCESS_COUNT = (short)strtol(args[2], ptr, 10) + 1;
        CONNECTIONS_COUNT = PROCESS_COUNT;
    }

    balance_t *balances = (balance_t *)malloc(sizeof(balance_t) * (PROCESS_COUNT - 1));

    for (int i = 0; i < PROCESS_COUNT - 1; i++)
        balances[i] = (balance_t)atoi(args[i + 3]);
}


void initialize_child(proc_info_t *child, process_task task)
{
    child->task = task;
    child->connections = malloc(sizeof(connection_t) * (CONNECTIONS_COUNT));
    child->connection_count = CONNECTIONS_COUNT;
}

System_t *initialize_System(process_task task)
{
    System_t *sys = (System_t *)malloc(sizeof(System_t));
    sys->process_count = PROCESS_COUNT;
    proc_info_t *children = (proc_info_t *)malloc(sizeof(proc_info_t) * sys->process_count);
    sys->processes = children;
    local_id i;
    for (i = 0; i < sys->process_count; i++)
    {
        sys->processes[i].id = (local_id)(i);
        initialize_child(&sys->processes[i], task);
    }
    return sys;
}

int main(int argc, char **argv)
{
    balance_t* balances = parse_arguments(argv);
    open_log_files();
    System_t *sys = initialize_System(do_smth);

    /*
        code here
        OKEY
    */

    close_log_files();
    return 0;
}