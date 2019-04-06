#include <string.h>
#include <unistd.h>
#include <logs.h>
#include "self.h"



static void close_connection(connection_t *connection)
{
    close(connection->read);
    close(connection->write);
}

void close_all_unused_connections(System_t* sys, int index)
{
    int i;
    for (i = 0; i < sys->process_count; i++)
    {
        if (i == index)
            continue;
        int j;
        proc_info_t *info_i = sys->processes + i;
        for (j = 0; j < sys->process_count; j++)
        {
            proc_info_t *info_j = sys->processes + j;
            if (i != j && j != index)
            {
                close_connection(info_i->connections + j);
//                log_pipe(CLOSE, index, i, j, fd[0]);
                close_connection(info_j->connections + i);
//                log_pipe(CLOSE, index, j, i, fd[0]);
            }
            else if (j == index){
                close_connection(info_i->connections + j);
//                log_pipe(CLOSE, index, j, i, fd[0]);
            }
        }
    }
}

Message create_message(char *payload, uint16_t len, int16_t type)
{
    Message msg;
    if (payload != NULL)
        memcpy(&msg.s_payload, payload, len);
//    msg.s_header = create_message_header(magic, len, type);
    return msg;
}

MessageHeader create_message_header(uint16_t len, int16_t type)
{
    MessageHeader header;
    header.s_magic = MESSAGE_MAGIC;
    header.s_payload_len = len;
    header.s_type = type;
    header.s_local_time = get_physical_time();
    return header;
}