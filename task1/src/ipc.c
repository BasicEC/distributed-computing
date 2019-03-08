#include "ipc.h"
#include "self.h"
#include <unistd.h>
#include "string.h"
#include <stddef.h>

int send(void *self, local_id dst, const Message *msg)
{
    int pipefd = get_w_pipefd_by_id(self, dst);

    if (pipefd < 0)
        return pipefd;

    return write(pipefd, msg->s_payload, strlen(msg->s_payload));
}

int send_multicast(void *self, const Message *msg)
{
    int *pipefds;
    ssize_t w_result;
    int pipefds_count = get_all_w_pipefds(self, pipefds);

    if (pipefds_count < 0)
        return pipefds_count;

    for (int i = 0; i < pipefds_count; ++i)
    {
        w_result = write(pipefd, msg->s_payload, strlen(msg->s_payload));
        if (w_result < 0)
            return w_result;
    }

    return 0;
}

int receive(void *self, local_id from, Message *msg)
{
    ssize_t r_result = 0;
    int pipefd = get_r_pipefd_by_id(self, dst);

    if (pipefd < 0)
        return pipefd;

    while (r_result == 0)
    {
        r_result = read(pipefd, msg->s_payload, MAX_PAYLOAD_LEN);
    }

    if (r_result < 0)
        return r_result;
    return 0;
}

int receive_any(void *self, Message *msg)
{
    int *pipefds;
    ssize_t r_result = 0;
    int pipefds_count = get_all_r_pipefds(self, pipefds);

    if (pipefds_count < 0)
        return pipefds_count;

    while (r_result == 0)
    {
        for (int i = 0; i < pipefds_count; ++i)
        {
            r_result = read(pipefd, msg->s_payload, MAX_PAYLOAD_LEN);
            if (r_result != 0)
            {
                if (r_result < 0)
                    return r_result return 0;
            }
        }
    }
}