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
#include "child.h"

int PROCESS_COUNT;

pid_t create_process(System_t *sys, local_id index)
{
    pid_t id = fork();
    if (id < 0)
    {
        perror("unable to create process");
        errno = -1;
        exit(errno);
    }
    if (id == 0)
    {
        proc_info_t *proc = sys->processes + index;
        (*proc).task(index);
        exit(0);
    }
    return id;
}

pid_t* fork_children(System_t *sys)
{
    pid_t* children = (pid_t*)malloc(sizeof(pid_t)*(PROCESS_COUNT - 1));
    for (local_id i = 1; i < PROCESS_COUNT; i++)
    {
        children[i-1] = create_process(sys, i);

    }
//        proc_info_t *proc = sys->processes + PROCESS_COUNT - 1;
//        (*proc).task(PROCESS_COUNT - 1);
    return children;
}

balance_t *parse_arguments(char **args)
{
    void *ptr = NULL;
    if (strcmp(args[1], "-p") == 0 || strcmp(args[1], "-P") == 0)
    {
        PROCESS_COUNT = (short)strtol(args[2], ptr, 10) + 1;
    }

    balance_t *balances = (balance_t *)malloc(sizeof(balance_t) * (PROCESS_COUNT - 1));

    for (int i = 0; i < PROCESS_COUNT - 1; i++)
        balances[i] = (balance_t)atoi(args[i + 3]);
    return balances;
}


int main(int argc, char **argv)
{
    balance_t *balances = parse_arguments(argv);
    open_log_files();
    SYSTEM = initialize_System(child_work, parent_work, PROCESS_COUNT, balances); //todo implement parent_work, child_work!!!
    fork_children(SYSTEM);
    parent_work(0);
    close_log_files();
    return 0;
}
