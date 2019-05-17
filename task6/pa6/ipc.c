
#include "main.h"
#include "connections.h"
#include "util.h"



void sync_time(timestamp_t time) {
    timestamp_t localTime = get_time();
    localTime = time > localTime ? time : localTime;
    localTime++;
	set_local_time(localTime);
}

int writePipe(int fd, const Message* msg) {
	if (fd == 0 || msg == NULL)
		return E_PIPE_INVALID_ARGUMENT;

	ssize_t result =  write(fd, &msg->s_header, sizeof(MessageHeader));
	if(result < 0)
        return -1;
	if (msg->s_header.s_payload_len > 0) {
        result = write(fd, msg->s_payload, msg->s_header.s_payload_len);
        if (result < 0)
            return -1;
    }
	return 0;
}

int readPipe(int fd, Message* msg, char isWait) {
	if (fd == 0 || msg == NULL)
		return E_PIPE_INVALID_ARGUMENT;
	int readed;
	while (1) {
		readed = (int)read(fd, &msg->s_header, sizeof(MessageHeader));

		if (readed <= 0) {
			if (isWait) {
			} else {
				return E_PIPE_NO_DATA;
			}
		} else {
			break;
		}
	}

	if (msg->s_header.s_payload_len > 0) {
		do {
			readed = (int)read(fd, msg->s_payload, msg->s_header.s_payload_len);
		} while (readed < 0);
	}

	return readed;
}

int send(void * self, local_id dst, const Message * msg) {
    thinker_t* thinker = self;
	return writePipe(thinker->connections[dst].write, msg);
}


int try_receive_message(thinker_t* thinker, Message* msg){
	for (int i = 1 ; i < thinker->connection_count; i++){
		if (i == thinker->id)
			continue;
		int result = readPipe(thinker->connections[i].read, msg, 0);
		if (result > 0) {
			return i;
		}
	}

    return 0;

}

int send_multicast(void * self, const Message * msg) {
	thinker_t* thinker = self;
    for (int i = 1; i < thinker->connection_count; i++) {
		if (i == thinker->id)
			continue;
		writePipe(thinker->connections[i].write, msg);
	}
	return 0;
}

int receive(void * self, local_id from, Message * msg) {
    thinker_t* thinker = self;
	return readPipe(thinker->connections[from].read, msg, 1);
}


int receive_any(void * self, Message * msg) {
    thinker_t* thinker = self;
    while (1){
        for (int i = 1 ; i < thinker->connection_count; i++){
            int result = readPipe(thinker->connections[i].read, msg, 0);
            if (result > 0)
                return i;
        }
    }
}
