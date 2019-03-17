#include "ipc.h"
#include "self.h"
#include <unistd.h>
#include "string.h"
#include <stddef.h>
#include <stdio.h>
#include <fcntl.h>
#include <common.h>


int event_log_descriptor = -1;

int get_events_log_descriptor()
{
    if(event_log_descriptor == -1)
        event_log_descriptor = open(events_log, O_CREAT | O_APPEND | O_WRONLY, 0777);
    return event_log_descriptor;
}

void log_events_event(char *fmt, local_id node_id, int fd)
{
    char formated_message[64];
    int len = sprintf(formated_message, fmt, node_id, fd);

    puts(formated_message);
    write(get_events_log_descriptor(), formated_message, len);
}

void log_events_read(local_id node_id, int fd)
{
    log_events_event("Node %d read from %d file descriptor\n", node_id, fd);
}

void log_events_write(local_id node_id, int fd)
{
    log_events_event("Node %d write to %d file descriptor\n", node_id, fd);
}


static int send_msg(int fd, const Message *msg)
{
    if (fd == 0 || msg == NULL)
        return -1;

    ssize_t result = write(fd, msg, sizeof(MessageHeader) + msg->s_header.s_payload_len);
    if (result < 0)
        return (int)result;

    return 0;
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
        log_events_write(selft->id,selft->connections[i].write);
        if (w_result < 0)
            return w_result;
    }

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

    while (r_result == 0)
    {
        for (int i = 0; i < selft->connection_count; ++i)
        {
            r_result = read_msg(selft->connections[i].read, msg);
            if (r_result != 0)
            {
                if (r_result < 0)
                    return (int)r_result;
                log_events_read(selft->id, selft->connections[i].read);
                return 0;
            }
        }
    }
    return 0;
}
