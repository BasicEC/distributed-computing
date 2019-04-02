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
int CONNECTIONS_COUNT;

int event_log_descriptor = -1;

int get_events_log_descriptor()
{
    if (event_log_descriptor == -1)
        event_log_descriptor = open(events_log, O_CREAT | O_APPEND | O_WRONLY, 0777);
    return event_log_descriptor;
}

void log_events_event(char *fmt, local_id node_id, int fd, int client_id)
{
    char formated_message[64];
    int len = sprintf(formated_message, fmt, node_id, fd, client_id);

    puts(formated_message);
    write(get_events_log_descriptor(), formated_message, len);
}

void log_events_read(local_id node_id, int fd, int client_id)
{
    log_events_event("Node %d receive message from %d file descriptor (node %d)\n", node_id, fd, client_id);
}

void log_events_write(local_id node_id, int fd, int client_id)
{
    log_events_event("Node %d send message to %d file descriptor (node %d)\n", node_id, fd, client_id);
}

static int send_msg(int fd, const Message *msg)
{
    if (fd == 0 || msg == NULL)
        return -1;

    ssize_t result = write(fd, msg, sizeof(MessageHeader) + msg->s_header.s_payload_len);

    return (int)result;
}

static int can_read(int fd)
{
    long cur = lseek(fd, 0, SEEK_CUR);
    long end = lseek(fd, 0, SEEK_END);

    if (end > cur)
    {
        lseek(fd, cur, SEEK_SET);
        return 0;
    }
    return 1;
}

static int read_msg(int fd, Message *msg)
{
    if (fd == 0 || msg == NULL)
        return -1;

    ssize_t result0;
    ssize_t result1;
    MessageHeader mh;

    result0 = read(fd, &mh, sizeof(MessageHeader));
    if (result0 < 0)
        return (int)result0;

    msg->s_header = mh;

    char buf[mh.s_payload_len];
    result1 = read(fd, buf, mh.s_payload_len);
    if (result1 < 0)
        return (int)result1;
    strcpy(msg->s_payload, buf);

    return (int)(result0 + result1);
}

int send(void *self, local_id dst, const Message *msg)
{
    proc_info_t *selft = (proc_info_t *)self;
    ssize_t w_result;
    int pipefd = get_w_pipefd_by_id((proc_info_t *)selft, dst);

    if (pipefd < 0)
        return pipefd;

    w_result = send_msg(pipefd, msg);

    if (w_result < 0)
        return w_result;

    return 0;
}

int send_multicast(void *self, const Message *msg)
{
    proc_info_t *selft = (proc_info_t *)self;
    ssize_t w_result;

    for (int i = 0; i < selft->connection_count; ++i)
    {
        if (i == selft->id)
            continue;
        w_result = send_msg(selft->connections[i].write, msg);
        log_events_write(selft->id, selft->connections[i].write, i);
        if (w_result < 0)
            return w_result;


    return 0;
}

int receive(void *self, local_id dst, Message *msg)
{
    ssize_t r_result = 0;
    int pipefd = get_r_pipefd_by_id((proc_info_t *)self, dst);

    while (r_result == 0)
    {
        r_result = read_msg(pipefd, msg);
    }

    if (r_result < 0)
        return r_result;

    return 0;
}

int receive_any(void *self, Message *msg)
{
    proc_info_t *selft = (proc_info_t *)self;
    ssize_t r_result = 0;

    while (1)
    {
        for (int i = selft->connection_count; i > 0; --i)
        {
            if (i == selft->id)
                continue;
            if (can_read(selft->connections[i].read))
            {
                r_result = read_msg(selft->connections[i].read, msg);
                if (r_result <= 0)
                    continue;
                if (!r_result)
                    continue;
                log_events_read(selft->id, selft->connections[i].read, i);
                return 0;
            }
        }
    }
}

int send_to_all_and_wait_all(proc_info_t *proc, char *text, MessageType type)
{
    Message *msg = (Message *)malloc(sizeof(Message));
    MessageHeader *header = (MessageHeader *)malloc(sizeof(MessageHeader));
    header->s_magic = MESSAGE_MAGIC;
    header->s_payload_len = (uint16_t)strlen(text);
    //    header->s_payload_len = 0;
    header->s_type = type;
    // header->s_local_time
    msg->s_header = *header;
    //    strcpy(msg->s_payload, text);

    send_multicast(proc, msg);
    usleep(1);
    Message *msgs[proc->connection_count];
    for (int i = 1; i < proc->connection_count - 1; i++)
    {
        msgs[i] = (Message *)malloc(sizeof(Message));
        receive_any(proc, msgs[i]);
    }

    return 0;
}

void do_smth()
{
    //    sleep(2);
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

void log_pipe_event(char *fmt, local_id node_id, int fd, int client_id)
{
    char formated_message[64];
    int len = sprintf(formated_message, fmt, node_id, fd, client_id);

    puts(formated_message);
    write(get_pipes_log_descriptor(), formated_message, (size_t)len);
}

void log_pipe_read(local_id node_id, int fd, local_id client_id)
{
    log_pipe_event("Node %d read from %d file descriptor (node %d)\n", node_id, fd, client_id);
}

void log_pipe_write(local_id node_id, int fd, local_id client_id)
{
    log_pipe_event("Node %d write to %d file descriptor (node %d)\n", node_id, fd, client_id);
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
    log_pipe_read(receive->id, fd[0], send->id);
    send->connections[receive->id].write = fd[1];
    log_pipe_write(send->id, fd[1], receive->id);
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

void close_connection(connection_t *connection)
{
    close(connection->read);
    close(connection->write);
}

void close_all_unused_connections(System_t *sys, int index)
{
    int i;
    for (i = 0; i < sys->process_count; i++)
    {
        if (i == index)
            continue;
        int j;
        proc_info_t *info_i = sys->processes + i;
        for (j = 0; j < sys->process_count; j++)
        {
            proc_info_t *info_j = sys->processes + j;
            if (i != j && j != index)
            {
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
    pid_t id = fork();
    if (id < 0)
    {
        perror("unable to create process");
        errno = -1;
        _exit(errno);
    }
    if (id == 0)
    {
        //        usleep((9-index)*10000);
        proc_info_t *proc = sys->processes + index;
        close_all_unused_connections(sys, index);
        send_to_all_and_wait_all(proc, "hellohellohellohellohellohellohellohellohelloyes", STARTED);
        (*proc).task();
        send_to_all_and_wait_all(proc, "donehellohellohellohellohello", DONE);
        _exit(0);
    }
    return id;
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

void run(System_t *sys)
{
    int i;
    int pid_arr[PROCESS_COUNT];
    for (i = 1; i < PROCESS_COUNT; i++)
    {
        pid_arr[i] = create_process(sys, i);
    }
    close_all_unused_connections(sys, 0);
    Message msg_start[PROCESS_COUNT - 1];
    for (i = 0; i < PROCESS_COUNT - 1; i++)
    {
        proc_info_t *info = sys->processes;
        receive_any(info, (msg_start + i));
    }

    Message msg_end[PROCESS_COUNT - 1];
    for (i = 0; i < PROCESS_COUNT - 1; i++)
    {
        proc_info_t *info = sys->processes;
        receive_any(info, (msg_end + i));
    }
    for (i = 0; i < PROCESS_COUNT - 1; i++)
    {
        wait(NULL);
    }
}

void parse_arguments(char **args)
{
    void *ptr = NULL;
    if (strcmp(args[1], "-p") == 0 || strcmp(args[1], "-P") == 0)
    {
        PROCESS_COUNT = (short)strtol(args[2], ptr, 10) + 1;
        CONNECTIONS_COUNT = PROCESS_COUNT;
    }
}

int main(int argc, char **argv)
{
    parse_arguments(argv);
    get_events_log_descriptor();
    System_t *sys = initialize_System(do_smth);
    establish_all_connections(sys);
    run(sys);
    return 0;
}
