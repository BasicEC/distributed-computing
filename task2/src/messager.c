#include <string.h>
#include <unistd.h>
#include <logs.h>
#include "self.h"


static void close_write_connection(connection_t *connection)
{
    close(connection->write);
}

static void close_read_connection(connection_t *connection)
{
    close(connection->read);
}

static void close_connection(connection_t *connection)
{
    close_read_connection(connection);
    close_write_connection(connection);
}

void close_all_unused_connections(System_t *sys, int index)
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
                close_connection(info_j->connections + i);
            }
//            else if (j == index)
//                close_connection(info_i->connections + j);
        }
    }
}

MessageHeader create_message_header(uint16_t magic, uint16_t len, int16_t type)
{
    MessageHeader header;
    header.s_magic = MESSAGE_MAGIC;
    header.s_payload_len = len;
    header.s_type = type;
    header.s_local_time = get_physical_time();
    return header;
}

Message create_message(uint16_t magic,char *payload, uint16_t len, int16_t type)
{
    Message msg;
    if (payload != NULL)
        memcpy(&msg.s_payload, payload, len);
    msg.s_header = create_message_header(magic, len, type);
    return msg;
}

