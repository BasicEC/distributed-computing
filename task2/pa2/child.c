#include "child.h"
#include "messager.h"
#include "logs.h"
#include "banking.h"
#include "common.h"
#include "ipc.h"

static void send_to_all_and_wait_all(proc_info_t *proc)
{
    char payload[COMMON_PAYLOAD_LEN];
    int len = sprintf(payload, log_started_fmt, get_physical_time(), 0, getpid(), getppid(), proc->balance);//change 0 to id

    Message msg = create_message(MESSAGE_MAGIC, payload, (uint16_t)len, STARTED);
    send_multicast(proc, &msg);

    log_event(_STARTED, proc->id, 0, proc->balance);

    for (int i = 1; i < proc->connection_count; i++)
    {
        if (i == proc->id)
            continue;
        //не receive_any потому что какой-то процесс может успеть послать 2 сообщения?
        receive(proc, (local_id)i, &msg);
    }
    log_event(_RECEIVED_ALL_STARTED, proc->id, 0, proc->balance);
}

//static void send_to_all(){
//
//}

static timestamp_t on_transfer(proc_info_t *proc, Message *msg, BalanceHistory *history, timestamp_t last_time)
{
    TransferOrder to;
    memcpy(&to, msg->s_payload, sizeof(TransferOrder));
    timestamp_t new_time = get_physical_time();

    if (to.s_src == proc->id)
    {
        proc->balance -= to.s_amount;
        send(proc, to.s_dst, msg);
        log_event(_TRANSFER_OUT, proc->id, to.s_dst, to.s_amount);
    }
    else if (to.s_dst == proc->id)
    {
        proc->balance += to.s_amount;
        Message reply = create_message(MESSAGE_MAGIC,NULL, 0, ACK);
        send(proc, 0, &reply);
        log_event(_TRANSFER_IN, proc->id, to.s_src, to.s_amount);
    }

//    BalanceState state;
//    state.s_balance = proc->balance;
//    state.s_time = 0;
//    state.s_balance_pending_in = 0;
//    history[new_time] = state;
//
//    for (timestamp_t i = last_time + 1; last_time < new_time; last_time++)
//    {
//        history[i] = history[last_time];
//        history[i].s_time = i;
//    }

    return new_time;
}

static void send_history(proc_info_t *proc, BalanceHistory *history)
{
    char payload[MAX_PAYLOAD_LEN];
    int len = sizeof(BalanceHistory);
    memcpy(&payload, history, (size_t)len);
    Message history_msg = create_message(MESSAGE_MAGIC, payload, (uint16_t)len, BALANCE_HISTORY);
    send(proc, 0, &history_msg); //0 - parent id
}

static int send_done(proc_info_t *proc)
{
    char payload[COMMON_PAYLOAD_LEN];
    int len = sprintf(payload, log_done_fmt, get_physical_time(), proc->id, proc->balance);
    Message msg = create_message(MESSAGE_MAGIC,payload, (uint16_t)len, DONE);
    send(proc, 0, &msg); //send to parent done
    log_event(_DONE, proc->id, 0, proc->balance);
    return 0;
}

void main_work(proc_info_t *proc)
{
    BalanceHistory history;
    BalanceState state;
    Message msg;
    timestamp_t last_time = 0;
    history.s_id = proc->id;
    state.s_balance = proc->balance;
    state.s_time = 0;
    state.s_balance_pending_in = 0;
    history.s_history[0] = state;
//    int i = proc->connection_count;
    while (1)
    {
        receive_any(proc, &msg);
        switch (msg.s_header.s_type)
        {
        case TRANSFER:
            last_time = on_transfer(proc, &msg, &history, last_time);
            break;
        case STOP:
            send_done(proc);              //send done to parent
            send_history(proc, &history); //send history
            return;
        default:
            break;
        }
    }
}

void child_work(pid_t id)
{
    proc_info_t *cur_proc = SYSTEM->processes + id;

    close_all_unused_connections(SYSTEM, id);
    send_to_all_and_wait_all(cur_proc);
    main_work(cur_proc);
    exit(0);
}

void parent_work(pid_t children)
{
    proc_info_t* proc = SYSTEM->processes;
    Message message;
    for (int i = 1; i < proc->connection_count; i++)
    {
        receive(proc, (local_id)i, &message);
    }
    log_event(_RECEIVED_ALL_STARTED, proc->id, 0, proc->balance);
    bank_robbery(SYSTEM->processes, (local_id)(SYSTEM->process_count - 1));
    Message msg = create_message(MESSAGE_MAGIC, NULL, 0, STOP);
    send_multicast(SYSTEM->processes, &msg);
}

