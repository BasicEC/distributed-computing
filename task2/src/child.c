#include "child.h"

static int send_to_all_and_wait_all(proc_info_t *proc, MessageType type)
{
    char payload[COMMON_PAYLOAD_LEN];
    int len = sprintf(payload, log_started_fmt, get_physical_time(), id, getpid(), getppid(), proc->balance);

    Message msg = create_message(payload, len, STARTED);

    send_multicast(proc, msg);
    // usleep(1);
    Message *msgs[proc->connection_count];
    for (int i = 1; i < proc->connection_count - 1; i++)
    {
        msgs[i] = (Message *)malloc(sizeof(Message));
        //не receive_any потому что какой-то процесс может успеть послать 2 сообщения?
        receive(proc, proc->id, msgs[i]); 
    }

    return 0;
}

void child_work(System_t *sys, local_id id)
{
    proc_info_t *cur_proc = sys->processes + id;

    close_all_unused_connections(sys, id);
    send_to_all_and_wait_all(cur_proc, type);
}
