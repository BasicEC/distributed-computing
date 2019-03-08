#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include "ipc.h"
#include "common.h"
#include "pa1.h"

int PROCESS_COUNT;

typedef void (*process_task)();

typedef struct
{
    int read;
    int write;
} connection;

typedef struct
{
    int id;
    process_task task;
    int connection_count;
    connection *connections;
} proc_info;

typedef struct
{
    int process_count;
    proc_info *processes;
} System;

int send_greeting()
{
    return 0;
}

int send_parting()
{
    return 0;
}

void do_smth()
{
    send_greeting();
    send_parting();
}

void disable_blocks(int fd){
    int flags = fcntl(fd, F_GETFL);
    fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

int create_pipe_without_blocks(int* fd){
    if (pipe(fd) != 0)
        return -1;
    disable_blocks(*fd);
    disable_blocks(fd[1]);
    return 0;
}

void unidirectional_connection(proc_info* send, proc_info* receive){
    int fd[2];
    create_pipe_without_blocks(fd);
    receive->connections[send->id].read = fd[0];
    send->connections[receive->id].write = fd[1];
}


void establish_connection(proc_info* send, proc_info* receive)
{
    unidirectional_connection(send, receive);
    unidirectional_connection(receive,send);
}

void establish_all_connections(System *sys)
{
    int i, j;
    for (i = 0; i < sys->process_count; i++)
    {
        for (j = 0; j < sys->process_count; j++)
        {
            establish_connection((sys->processes + i), (sys->processes + j));
        }
    }
}

int create_process(proc_info* proc)
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
        (*proc).task();
        _exit(0);
    }
    return id;
}

void initialize_child(proc_info *child, process_task task)
{
    child->task = task;
    child->connections = malloc(sizeof(connection) * (PROCESS_COUNT - 1));
}

System *initialize_System(process_task task)
{
    System *sys = (System *)malloc(sizeof(System));
    sys->process_count = PROCESS_COUNT;
    proc_info *children = (proc_info *)malloc(sizeof(proc_info));
    sys->processes = children;
    int i;
    for (i = 0; i < sys->process_count; i++)
        initialize_child(sys->processes + i, task);
    return sys;
}

void run(System* sys)
{
    int i;
    int pid_arr[PROCESS_COUNT];
    for (i = 0; i < PROCESS_COUNT; i++)
    {
        pid_arr[i] = create_process(sys->processes + i);
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
    System *sys = initialize_System(do_smth);
    establish_all_connections(sys);
    run(sys);
    return 0;
}
