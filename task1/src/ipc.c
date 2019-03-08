#include "ipc.h"
#include "self.h"
#include <unistd.h>
#include "string.h"
#include <stddef.h>

int send(void *self, local_id dst, const Message *msg)
{
    ssize_t w_result;
    int pipefd = get_w_pipefd_by_id((proc_info_t *)self, dst);

    if (pipefd < 0)
        return pipefd;

    w_resulat = send_msg(pipefd, msg);

    if (w_result < 0)
        return w_result;

    return 0
}

int send_multicast(void *self, const Message *msg)
{
    proc_info_t *selft = (proc_info_t *)self;
    ssize_t w_result;

    for (int i = 0; i < selft->connection_count; ++i)
    {
        w_result = send_msg(self->connections[i]->write, msg)
        if (w_result < 0)
            return w_result;
    }

    return 0;
}

int receive(void *self, local_id from, Message *msg)
{
    // ssize_t r_result = 0;
    // int pipefd = get_r_pipefd_by_id((proc_info_t *)self, dst);

    // if (pipefd < 0)
    //     return pipefd;

    // while (r_result == 0)
    // {
    //     r_result = read(pipefd, msg->s_payload, MAX_PAYLOAD_LEN);
    // }

    // if (r_result < 0)
    //     return r_result;
    // return 0;
}

int receive_any(void *self, Message *msg)
{
    // proc_info_t *selft = (proc_info_t *)self;
    // ssize_t r_result = 0;
    // int *pipefds = get_all_r_pipefds(selft, pipefds);

    // while (r_result == 0)
    // {
    //     for (int i = 0; i < selft->p_count; ++i)
    //     {
    //         r_result = read(pipefd, msg->s_payload, MAX_PAYLOAD_LEN);
    //         if (r_result != 0)
    //         {
    //             if (r_result < 0)
    //                 return r_result return 0;
    //         }
    //     }
    // }
}

static int send_msg(int fd, const Message *msg)
{
    if (fd == 0 || msg == NULL)
        return -1;

    write(fd, &msg->s_header, sizeof(MessageHeader));
    if (msg->s_header.s_payload_len > 0)
    {
        write(fd, msg->s_payload, msg->s_header.s_payload_len);
    }
    return 0;
}