#include "ipc2.h"
#include "banking.h"
#include <fcntl.h>
#include <stdio.h>
#include <string.h>

int send(void *self, local_id dst, const Message *msg) {
    proc_info *info = (proc_info *) self;
    if (info->self == dst) {
        return -1;
    }
    write(info->connections.fds[info->self][dst][1], msg, sizeof msg->s_header + msg->s_header.s_payload_len);
    return 0;
}

int send_multicast(void *self, const Message *msg) {
    proc_info *info = (proc_info *) self;
    for (int i = 0; i <= info->connections.procCount; ++i) {
        if (i != info->self)
            send(self, i, msg);
    }
    return 0;
}

int receive(void *self, local_id from, Message *msg) {
    proc_info *info = (proc_info *) self;
    int fd = info->connections.fds[from][info->self][0];
    while (1) {
        int sum, sum1;
        if ((sum = read(fd, &msg->s_header, sizeof(MessageHeader))) == -1) {
            usleep(1000);
            continue;
        }
        if (msg->s_header.s_payload_len > 0) {
            sum1 = read(fd, msg->s_payload, msg->s_header.s_payload_len);
        }
        return 0;
    }
}

int receive_any(void *self, Message *msg) {
    proc_info *info = (proc_info *) self;
    while (1) {
        for (int i = 0; i <= info->connections.procCount; ++i) {
            if (i == info->self) continue;

            int fd = info->connections.fds[i][info->self][0];
            int sum, sum1;
            int flags = fcntl(fd, F_GETFL, 0);
            fcntl(fd, F_SETFL, flags | O_NONBLOCK);
            sum = read(fd, &msg->s_header, sizeof(MessageHeader));
            if (sum == -1) {
                continue;
            }
            if (msg->s_header.s_payload_len > 0) {
                sum1 = read(fd, msg->s_payload, msg->s_header.s_payload_len);
            }
            fflush(stdout);
            return 0;
        }
        usleep(500);
    }
}

//static void close_connection(int i, int j, proc_info* info, int proc_id){
//    if (proc_id == i) {
//        close(info->connections.fds[i][j][0]);
//    } else if (proc_id == j) {
//        close(info->connections.fds[i][j][1]);
//    } else {
//        close(info->connections.fds[i][j][0]);
//        close(info->connections.fds[i][j][1]);
//    }
//}
//
//void close_connections(void *self, int proc_id) {
//    proc_info *info = (proc_info *) self;
//    for (int i = 0; i <= info->connections.procCount; ++i) {
//        for (int j = 0; j <= info->connections.procCount; ++j) {
//            if (i == j) continue;
//            close_connection(i,j,info, proc_id);
//        }
//    }
//}

void close_connections(void *self, int proc) {
    proc_info *sio = (proc_info *) self;
    for (int i = 0; i <= sio->connections.procCount; ++i) {
        for (int j = 0; j <= sio->connections.procCount; ++j) {
            if (i == j) continue;
            if (proc == i) {
                close(sio->connections.fds[i][j][0]);
            } else if (proc == j) {
                close(sio->connections.fds[i][j][1]);
            } else {
                close(sio->connections.fds[i][j][0]);
                close(sio->connections.fds[i][j][1]);
            }
        }
    }
}

int receive_all(void *self, Message msgs[], MessageType type) {
   proc_info *info = (proc_info *) self;
    for (int i = 1; i <= info->connections.procCount; ++i) {
        if (i == info->self) continue;
        do {
            receive(self, i, &msgs[i]);
        } while (msgs[i].s_header.s_type != type);
    }

    return 0;
}

void createMessageHeader(Message *msg, MessageType messageType) {
    msg->s_header.s_magic = MESSAGE_MAGIC;
    msg->s_header.s_type = messageType;
    msg->s_header.s_local_time = get_physical_time();
    msg->s_header.s_payload_len = strlen(msg->s_payload) + 1;
}

void transfer(void *parent_data, local_id src, local_id dst, balance_t amount) {
    Message msg;
    TransferOrder order = {src, dst, amount};
    memcpy(msg.s_payload, &order, sizeof(order));
    createMessageHeader(&msg, TRANSFER);
    msg.s_header.s_payload_len = sizeof(order);
    send(parent_data, src, &msg);

    Message result_msg;
    result_msg.s_header.s_type = DONE;
    while (result_msg.s_header.s_type != ACK)
        receive(parent_data, dst, &result_msg);
    printf("%d$ transfered from %d to %d\n",amount, src, dst);
    fflush(stdout);
    usleep(1000);
}
