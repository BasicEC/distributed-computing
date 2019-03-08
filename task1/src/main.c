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

FILE *fevents_log;
FILE *fpipes_log;

int send_to_all_and_wait_all(proc_info_t *proc, char* text)
{
    Message *msg = (Message *)malloc(sizeof(Message));
    MessageHeader header = (MessageHeader *)malloc(sizeof(MessageHeader));
    header->s_magic = MESSAGE_MAGIC;
    header->s_payload_len = 3;
    header->s_type = STARTED;
    // header->s_local_time wtf, what is that???
    msg->s_header = header;
    strcpy(msg->s_payload, text);

    send_multicast(proc, msg);

    Message msg[proc->connection_count];
    for (int i = 0; i < proc->connection_count; i++)
    {
        msg[i] = (Message *)malloc(sizeof(Message));
        receive_any(proc, &msg[i]);
    }

    fprintf(fevents_log, log_received_all_started_fmt, proc->id);
    return 0;
}

void do_smth()
{
}

static int pipes_log_descriptor = -1;

int get_pipes_log_descriptor() {
    if (pipes_log_descriptor < 0) {
        pipes_log_descriptor = open(pipes_log, O_CREAT | O_APPEND | O_WRONLY | O_TRUNC, 0777);
    }
    return pipes_log_descriptor;
}

void log_pipe_event(char *fmt, local_id node_id, int fd) {
    char formated_message[64];
    int len = sprintf(formated_message, fmt, node_id, fd);

    puts(formated_message);
    write(get_pipes_log_descriptor(), formated_message, len);
}

void log_pipe_read(local_id node_id, int fd) {
    log_pipe_event("Node %d read from %d file descriptor\n", node_id, fd);
}

void log_pipe_write(local_id node_id, int fd) {
    log_pipe_event("Node %d write to %d file descriptor\n", node_id, fd);
}


void disable_blocks(int fd){
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

            if (i != j) {
                unidirectional_connection((sys->processes + i), (sys->processes + j));
            }
        }
    }

}

int create_process(proc_info_t *proc)
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
        send_greeting(proc);
        (*proc).task();
        send_parting(proc);
        _exit(0);
    }
    return id;
}

void initialize_child(proc_info_t *child, process_task task)
{
    child->task = task;
    child->connections = malloc(sizeof(connection_t) * (PROCESS_COUNT - 1));
    child->connection_count = PROCESS_COUNT - 1;
}

System_t *initialize_System(process_task task)
{
    System_t *sys = (System_t *)malloc(sizeof(System_t));
    sys->process_count = PROCESS_COUNT;
    proc_info_t *children = (proc_info_t *)malloc(sizeof(proc_info_t) * sys->process_count);
    sys->processes = children;
    local_id i;
    for (i = 0; i < sys->process_count; i++) {
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
        pid_arr[i] = create_process(sys->processes + i);
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
    fevents_log = fopen(events_log, "w+");
	fpipes_log = fopen(pipes_log, "w+");
    System_t *sys = initialize_System(do_smth);
    establish_all_connections(sys);
    run(sys);
    return 0;
}
