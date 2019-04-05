#include "ipc.h"
#include "self.h"
#include <unistd.h>
#include "string.h"
#include <stddef.h>
#include <stdio.h>
#include <fcntl.h>
#include "common.h"
#include "logs.h"

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
    ssize_t w_result = 0;
    int pipefd = get_w_pipefd_by_id((proc_info_t *)selft, dst);

    if (pipefd < 0)
        return pipefd;

//    w_result = send_msg(pipefd, msg);

    if (w_result < 0)
        return w_result;
    //log
    return 0;
}

int send_multicast(void *self, const Message *msg)
{
    proc_info_t *selft = (proc_info_t *)self;
    ssize_t w_result = 0;

    for (int i = 0; i < selft->connection_count; ++i)
    {
        if (i == selft->id)
            continue;
//        w_result = send_msg(selft->connections[i].write, msg);
        //log
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
    //log
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
                //log
                return 0;
            }
        }
    }
}
