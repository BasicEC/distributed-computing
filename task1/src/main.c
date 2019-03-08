#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include "ipc.h"
#include "common.h"
#include "pa1.h"
#include "self.h"

int PROCESS_COUNT;

int send_greeting()
{
}

int send_parting()
{
}

void do_smth()
{
    send_greeting();
    send_parting();
}

void establish_connection()
{
}

void establish_all_connections(System_t *sys)
{
    int i, j;
    for (i = 0; i < sys->process_count; i++)
    {
        for (j = 0; j < sys->process_count; j++)
        {
            establish_connection();
        }
    }
}

int create_process(process_task task)
{
    pid_t id = fork();
    if (id < 0)
    {
        perror("unable to create process");
        errno = -1;
        _exit(-1);
    }
    if (id == 0)
    {
        task();
        _exit(0);
    }
    return id;
}

void initialize_child(proc_info_t *child)
{
    child->connections = malloc(sizeof(connection_t) * (PROCESS_COUNT - 1));
}

System_t *initialize_System()
{
    System_t *sys = (System_t *)malloc(sizeof(System_t));
    sys->process_count = PROCESS_COUNT;
    proc_info_t *children = (proc_info_t *)malloc(sizeof(proc_info_t));
    sys->processes = children;
    int i;
    for (i = 0; i < sys->process_count; i++)
        initialize_child(sys->processes + i);
}

void run(process_task task)
{
    int i;
    int pid_arr[PROCESS_COUNT];
    for (i = 0; i < PROCESS_COUNT; i++)
    {
        pid_arr[i] = create_process(task);
    }
}

void parse_arguments(char **args)
{
    void *ptr = NULL;
    if (strcmp(args[1], "-p") == 0 || strcmp(args[1], "-P") == 0)
        PROCESS_COUNT = (short)strtol(args[2], ptr, 10);
}

int main(int argc, char **argv)
{
    parse_arguments(argv);
    System_t *sys = initialize_System();
    establish_all_connections(sys);
    run(do_smth);
    return 0;
}
