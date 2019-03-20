#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <wait.h>
#include "ipc.h"
#include "common.h"
#include "pa1.h"
#include "self.h"

int PROCESS_COUNT;


int send_to_all_and_wait_all(proc_info_t *proc, char *text, MessageType type)
{
    Message *msg = (Message *)malloc(sizeof(Message));
    MessageHeader *header = (MessageHeader *)malloc(sizeof(MessageHeader));
    header->s_magic = MESSAGE_MAGIC;
    header->s_payload_len = (uint16_t)strlen(text);
    header->s_type = type;
    // header->s_local_time wtf, what is that???
    msg->s_header = *header;
    strcpy(msg->s_payload, text);

    send_multicast(proc, msg);

    Message* msgs[proc->connection_count];
    for (int i = 0; i < proc->connection_count; i++)
    {
        msgs[i] = (Message *)malloc(sizeof(Message));
        receive_any(proc, msgs[i]);
    }

//    fprintf(fevents_log, log_received_all_started_fmt, proc->id);
    return 0;
}

void do_smth()
{
}

static int pipes_log_descriptor = -1;

int get_pipes_log_descriptor()
{
    if (pipes_log_descriptor < 0)
    {
        pipes_log_descriptor = open(pipes_log, O_CREAT | O_APPEND | O_WRONLY | O_TRUNC, 0777);
    }
    return pipes_log_descriptor;
}

void log_pipe_event(char *fmt, local_id node_id, int fd)
{
    char formated_message[64];
    int len = sprintf(formated_message, fmt, node_id, fd);

    puts(formated_message);
    write(get_pipes_log_descriptor(), formated_message, (size_t)len);
}

void log_pipe_read(local_id node_id, int fd)
{
    log_pipe_event("Node %d read from %d file descriptor\n", node_id, fd);
}

void log_pipe_write(local_id node_id, int fd)
{
    log_pipe_event("Node %d write to %d file descriptor\n", node_id, fd);
}

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
    log_pipe_read(receive->id, fd[0]);
    send->connections[receive->id].write = fd[1];
    log_pipe_write(send->id, fd[1]);
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


void close_connection(connection_t* connection){
    close(connection->read);
    close(connection->write);
}

void close_all_unused_connections(System_t* sys, int index){
    int i;
    for (i = 0; i < sys->process_count; i++){
        if (i == index) continue;
        int j;
        proc_info_t* info_i = sys->processes + i;
        for (j = 0; j < sys->process_count; j++){
            proc_info_t* info_j = sys->processes + j;
            if (i!=j && j != index) {
                close_connection(info_i->connections + j);
                close_connection(info_j->connections + i);
            }
            else if (j == index)
                close_connection(info_i->connections + j);
        }
    }
}

int create_process(System_t *sys, int index)
{
//    pid_t id = fork();
//    if (id < 0)
//    {
//        perror("unable to create process");
//        errno = -1;
//        _exit(errno);
//    }
//    if (id == 0)
//    {
//        usleep((9-index)*10000);
        proc_info_t* proc = sys->processes + index;
//        close_all_unused_connections(sys, index);
        send_to_all_and_wait_all(proc, "hello", STARTED);
        (*proc).task();
        send_to_all_and_wait_all(proc, "done", DONE);
        _exit(0);
//    }
//    return id;
return 0;
}

void initialize_child(proc_info_t *child, process_task task)
{
    child->task = task;
    child->connections = malloc(sizeof(connection_t) * (PROCESS_COUNT));
    child->connection_count = PROCESS_COUNT;
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
        sys->processes[i].id = i;
        initialize_child(&sys->processes[i], task);
    }
    return sys;
}

void run(System_t *sys)
{
    int i;
    int pid_arr[PROCESS_COUNT];
    for (i = 0; i < PROCESS_COUNT; i++)
    {
        pid_arr[i] = create_process(sys, i);
    }
    for (i = 0; i < PROCESS_COUNT; i++)
    {
        wait(NULL);
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
    System_t *sys = initialize_System(do_smth);
    establish_all_connections(sys);
    run(sys);
    return 0;
}
