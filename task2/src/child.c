#include "child.h"

static void send_to_all_and_wait_all(proc_info_t *proc, MessageType type)
{
    char payload[COMMON_PAYLOAD_LEN];
    int len = sprintf(payload, log_started_fmt, get_physical_time(), id, getpid(), getppid(), proc->balance);

    Message msg = create_message(payload, len, STARTED);

    send_multicast(proc, msg);
    Message *msgs[proc->connection_count];
    for (int i = 1; i < proc->connection_count - 1; i++)
    {
        msgs[i] = (Message *)malloc(sizeof(Message));
        //не receive_any потому что какой-то процесс может успеть послать 2 сообщения?
        receive(proc, proc->id, msgs[i]);
    }
}

static void main_work(proc_info_t *proc)
{
    BalanceHistory history;
    BalanceState state;
    Message msg;
    timestamp_t last_time = 0;
    int done_count = 0;
    int balance = proc->balance;
    history.s_id = proc->id;
    state.s_balance = balance;
    state.s_time = 0;
    state.s_balance_pending_in = 0;
    history.s_history[0] = state;
    int proc->connection_count;
    while (1)
    {
        receive_any(proc, &msg);
        switch (msg.s_header.s_type)
        {
        case TRANSFER:
            on_transfer();
            break;
        case STOP:
            on_stop();
            break;
        case DONE:
            on_done();
            break;
        default:
            break;
        }
    }
}

void child_work(System_t *sys, local_id id)
{
    proc_info_t *cur_proc = sys->processes + id;

    close_all_unused_connections(sys, id);
    send_to_all_and_wait_all(cur_proc, type);
}
