#include "ipc.h"
#include "ipc2.h"
#include "pa2345.h"
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>

void fill_balance(BalanceHistory *history, int currentBalance, timestamp_t currentTime);

parsed_arguments parse_arguments(char **args)
{
    int process_count = -1;
    void *ptr = NULL;
    if (strcmp(args[1], "-p") == 0 || strcmp(args[1], "-P") == 0)
    {
        process_count = (short)strtol(args[2], ptr, 10);
    }

    balance_t *balances = (balance_t *)malloc(sizeof(balance_t) * (process_count + 1));

    for (int i = 1; i <= process_count; i++)
        balances[i] = (balance_t)atoi(args[i + 2]);
    parsed_arguments arguments;
    arguments.balances = balances;
    arguments.process_count = process_count;
    return arguments;
}

void establis_connection(int i, int j, Connections_t io, FILE* logFile){
    io.fds[i][j] = (int *) calloc(2, sizeof(int));
    pipe(io.fds[i][j]);
    fcntl(io.fds[i][j][0], F_SETFL, O_NONBLOCK);
    fcntl(io.fds[i][j][1], F_SETFL, O_NONBLOCK);
    fprintf(logFile, "pipe between %d %d was opened\n", i, j);
}

void establish_connections(Connections_t io){
    FILE *logFile = fopen(pipes_log, "a+");
    for (int i = 0; i <= io.procCount; ++i) {
        io.fds[i] = (int **) calloc((io.procCount + 1), sizeof(int *));
        for (int j = 0; j <= io.procCount; ++j) {
            if (i == j) continue;
            establis_connection(i,j,io,logFile);
        }
    }
}


prepared_history prepare_to_work(int proc_id, FILE* logfile, parsed_arguments arguments, Connections_t connections){
    balance_t* balances = arguments.balances;
    int procCount = arguments.process_count;
    BalanceHistory history;
    history.s_id = proc_id;
    BalanceState initState = {balances[proc_id], get_physical_time(), 0};
    history.s_history[0] = initState;
    history.s_history_len = 1;
    proc_info info = {connections, proc_id};
    close_connections(&info, proc_id);

    fprintf(logfile, log_started_fmt, get_physical_time(), proc_id, getpid(), getppid(), balances[proc_id]);
    fflush(logfile);

    Message msg;
    sprintf(msg.s_payload, log_started_fmt, get_physical_time(), proc_id, getpid(), getppid(), balances[proc_id]);
    createMessageHeader(&msg, STARTED);
    send_multicast(&info, &msg);

    Message start_msgs[procCount + 1];
    receive_all(&info, start_msgs, STARTED);
    fprintf(logfile, log_received_all_started_fmt, get_physical_time(), proc_id);
    fflush(logfile);
    prepared_history preparedHistory;
    preparedHistory.history = history;
    preparedHistory.info = info;
    return  preparedHistory;
}

void stop_child(FILE* logfile, int proc_id, proc_info sio, balance_t* balances){
    Message done_msg;
    sprintf(done_msg.s_payload, log_done_fmt, get_physical_time(), proc_id, balances[proc_id]);
    createMessageHeader(&done_msg, DONE);
    fprintf(logfile, log_done_fmt, get_physical_time(), proc_id, balances[proc_id]);
    send_multicast(&sio, &done_msg);
}


//void transfer_to(Message* workMsg, int proc_id, proc_info* info, FILE* logfile, BalanceHistory* history){
//    TransferOrder order;
//    memcpy(&order, &workMsg.s_payload, workMsg.s_header.s_payload_len);
//    if (order.s_src == i) {
//        workMsg.s_header.s_local_time = get_physical_time();
//        fill_balance(&history, -order.s_amount, workMsg.s_header.s_local_time);
//        send(&info, order.s_dst, &workMsg);
//        fprintf(logfile, log_transfer_out_fmt, get_physical_time(), order.s_src, order.s_amount,
//                order.s_dst);
//        fflush(logfile);
//    } else if (order.s_dst == i) {
//        fill_balance(&history, order.s_amount, workMsg.s_header.s_local_time);
//        printf("history written %d %d\n", history.s_history[history.s_history_len - 1].s_balance,
//               history.s_history[history.s_history_len - 1].s_time);
//        fprintf(logfile, log_transfer_in_fmt, get_physical_time(), order.s_src, order.s_amount,
//                order.s_dst);
//        fflush(logfile);
//        Message ackMsg;
//        createMessageHeader(&ackMsg, ACK);
//        send(&info, 0, &ackMsg);
//        printf("sent %d\n", i);
//        fflush(stdout);
//    }
//}


void run_system(FILE* logfile, Connections_t connections, parsed_arguments arguments){
    int procCount = arguments.process_count;

    for (local_id i = 1; i <= procCount; ++i) {
        if (fork() == 0) {
            balance_t* balances = arguments.balances;

            prepared_history preparedHistory  = prepare_to_work(i,logfile,arguments,connections);

            proc_info info = preparedHistory.info;
            BalanceHistory history = preparedHistory.history;

            int done = 1;
            Message workMsg;
            while (1) {
                receive_any(&info, &workMsg);
                if (workMsg.s_header.s_type == STOP) {
                    stop_child(logfile, i, info, balances);
                }
                if (workMsg.s_header.s_type == TRANSFER) {
                    TransferOrder order;
                    memcpy(&order, &workMsg.s_payload, workMsg.s_header.s_payload_len);
                    if (order.s_src == i) {
                        workMsg.s_header.s_local_time = get_physical_time();
                        fill_balance(&history, -order.s_amount, workMsg.s_header.s_local_time);
                        send(&info, order.s_dst, &workMsg);
                        fprintf(logfile, log_transfer_out_fmt, get_physical_time(), order.s_src, order.s_amount,
                                order.s_dst);
                        fflush(logfile);
                    } else if (order.s_dst == i) {
                        fill_balance(&history, order.s_amount, workMsg.s_header.s_local_time);
                        printf("history written by %d %d\n", history.s_history[history.s_history_len - 1].s_balance,
                               history.s_history[history.s_history_len - 1].s_time);
                        fprintf(logfile, log_transfer_in_fmt, get_physical_time(), order.s_src, order.s_amount,
                                order.s_dst);
                        fflush(logfile);
                        Message ackMsg;
                        createMessageHeader(&ackMsg, ACK);
                        send(&info, 0, &ackMsg);
                        printf("sent to %d\n", i);
                        fflush(stdout);
                    }
                }
                if (workMsg.s_header.s_type == DONE) {
                    done++;
                    if (done == procCount) {
                        fill_balance(&history, 0, get_physical_time());
                        goto stop;
                    }
                }
            }
            stop : ;
            Message historyMsg;
            memcpy(historyMsg.s_payload, &history, sizeof(BalanceHistory));
            createMessageHeader(&historyMsg, BALANCE_HISTORY);
            historyMsg.s_header.s_payload_len = sizeof(BalanceHistory);
            printf("%d\n", history.s_history[2].s_balance);

            send(&info, 0, &historyMsg);
            return;
        }
    }
}


void parent_task(Connections_t io, FILE* logfile, int proc_count){
    proc_info info = {io, 0};
    close_connections(&info, 0);
    Message msgs[proc_count + 1];
    receive_all(&info, msgs, STARTED);
    fprintf(logfile, log_received_all_started_fmt, get_physical_time(), 0);
    fflush(logfile);

    int max_id = proc_count;
    bank_robbery(&info, max_id);

    Message stopMsg;
    createMessageHeader(&stopMsg, STOP);
    stopMsg.s_header.s_payload_len = 0;
    send_multicast(&info, &stopMsg);

    receive_all(&info, msgs, DONE);
    fprintf(logfile, log_received_all_done_fmt, get_physical_time(), 0);
    fflush(logfile);
    for (int i = 0; i < info.connections.procCount; i++)
        wait(NULL);

    Message history_msgs[proc_count + 1];
    receive_all(&info, history_msgs, BALANCE_HISTORY);

    AllHistory allHistory;
    allHistory.s_history_len = proc_count;
    for (int i = 0; i < proc_count; i++) {
        memcpy(&allHistory.s_history[i], &history_msgs[i + 1].s_payload, sizeof(BalanceHistory));
        printf("%d\n", allHistory.s_history[i].s_history[2].s_balance);
    }
    print_history(&allHistory);
}


void fill_balance(BalanceHistory *history, int amount, timestamp_t current_time) {
    balance_t balance = history->s_history[history->s_history_len - 1].s_balance;
    for (timestamp_t t = history->s_history_len; t <= current_time; t++) {
        BalanceState state = {balance, t, 0};
        history->s_history[t] = state;
        history->s_history_len++;
    }
    printf("balance %d amount %d\n", history->s_history[current_time].s_balance, amount);
    history->s_history[current_time].s_balance += amount;
}

int main(int argc, char *argv[]) {

    parsed_arguments args = parse_arguments(argv);
    int proc_count = args.process_count;
    Connections_t connections;
    balance_t* balances = args.balances;
    connections.procCount = proc_count;
    connections.fds = (int ***) calloc((proc_count + 1), sizeof(int **));

    establish_connections(connections);
    FILE *logfile;
    logfile = fopen(events_log, "a+");

    for (local_id i = 1; i <= proc_count; ++i) {
        if (fork() == 0) {

            prepared_history preparedHistory = prepare_to_work(i, logfile, args, connections);
            proc_info info = preparedHistory.info;
            BalanceHistory history = preparedHistory.history;

            int done = 1;
            Message workMsg;
            while (1) {
                receive_any(&info, &workMsg);
                switch (workMsg.s_header.s_type) {
                    case STOP:
                        stop_child(logfile, i, info, balances);
                        break;
                    case TRANSFER:;
                        TransferOrder order;
                        memcpy(&order, &workMsg.s_payload, workMsg.s_header.s_payload_len);
                        if (order.s_src == i) {
                            workMsg.s_header.s_local_time = get_physical_time();
                            fill_balance(&history, -order.s_amount, workMsg.s_header.s_local_time);
                            send(&info, order.s_dst, &workMsg);
                            fprintf(logfile, log_transfer_out_fmt, get_physical_time(), order.s_src, order.s_amount,
                                    order.s_dst);
                            fflush(logfile);
                        } else if (order.s_dst == i) {
                            fill_balance(&history, order.s_amount, workMsg.s_header.s_local_time);
                            printf("history written %d %d\n", history.s_history[history.s_history_len - 1].s_balance,
                                   history.s_history[history.s_history_len - 1].s_time);
                            fprintf(logfile, log_transfer_in_fmt, get_physical_time(), order.s_src, order.s_amount,
                                    order.s_dst);
                            fflush(logfile);
                            Message ackMsg;
                            createMessageHeader(&ackMsg, ACK);
                            send(&info, 0, &ackMsg);
                            printf("sent %d\n", i);
                            fflush(stdout);
                        }
//                        transfer_to;
                        break;
                    case DONE:
                        done++;
                        if (done == proc_count) {
                            fill_balance(&history, 0, get_physical_time());
                            goto stop;
                        }
                }
            }
            stop:;
            Message historyMsg;
            memcpy(historyMsg.s_payload, &history, sizeof(BalanceHistory));
            createMessageHeader(&historyMsg, BALANCE_HISTORY);
            historyMsg.s_header.s_payload_len = sizeof(BalanceHistory);
            printf("%d\n", history.s_history[2].s_balance);

            send(&info, 0, &historyMsg);
            return 0;
        }
    }

    parent_task(connections, logfile, proc_count);
    return 0;
}


