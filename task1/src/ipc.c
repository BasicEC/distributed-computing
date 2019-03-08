#include "ipc.h"
#include "self.h"
#include <unistd.h>
#include "string.h"
#include <stddef.h>

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
        w_result = send_msg(self->connections[i]->write, msg) if (w_result < 0) return w_result;
    }

    return 0;
}

int receive(void *self, local_id from, Message *msg)
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
            r_result = read_msg(selft->connections[i]->read, msg);
            if (r_result != 0)
            {
                if (r_result < 0)
                    return r_result;
                return 0;
            }
        }
    }
}

static int send_msg(int fd, const Message *msg)
{
    if (fd == 0 || msg == NULL)
        return -1;

    int result = write(tmp->fd_write, msg, sizeof(MessageHeader) + msg->s_header.s_payload_len);

    if (result < 0)
        return result;

    return 0;
}

static int read_msg(int fd, Message *msg)
{
    if (fd == 0 || msg == NULL)
        return -1;

    ssize_t result0;
    ssize_t result1;
    MessageHeader mh;

    result = read(fd, &mh, sizeof(MessageHeader));
    if (result < 0)
        return result;

    msg->s_header = mh;

    result1 = read(fd, msg->s_payload, mh.s_payload_len);
    if (result1 < 0)
        return result1;

    return result0 + result1;
}