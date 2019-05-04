
#include "main.h"
#include "connections.h"
#include "util.h"



void sync_time(timestamp_t time) {
    timestamp_t localTime = get_time();
    localTime = time > localTime ? time : localTime;
    localTime++;
	set_local_time(localTime);
}

int getPipeId(int from, int to) {
//	if (to > from) to--;
//	return from * get_pipeline_width() + to;
 	return 0;
}

int writePipe(int fd, const Message* msg) {
	if (fd == 0 || msg == NULL)
		return E_PIPE_INVALID_ARGUMENT;

	write(fd, &msg->s_header, sizeof(MessageHeader));
	if (msg->s_header.s_payload_len > 0) {
		write(fd, msg->s_payload, msg->s_header.s_payload_len);
	}

	return 0;
}

int readPipe(int fd, Message* msg, char isWait) {
	if (fd == 0 || msg == NULL)
		return E_PIPE_INVALID_ARGUMENT;

	while (1) {
		int readed = read(fd, &msg->s_header, sizeof(MessageHeader));

		if (readed < 0) {
			if (isWait) {
			} else {
				return E_PIPE_NO_DATA;
			}
		} else {
			break;
		}
	}

	if (msg->s_header.s_payload_len > 0) {
		int readed = 0;
		do {
			readed = read(fd, msg->s_payload, msg->s_header.s_payload_len);
		} while (readed < 0);
	}

	return 0;
}

int send(void * self, local_id dst, const Message * msg) {
//	DataInfo* info = (DataInfo*)self;
//    connection_t* pPipeLines = get_pPipeLines();
//	int pipeId = pPipeLines[getPipeId(info->senderId, dst)].write;
//	return writePipe(pipeId, msg);
	return 0;
}


int send_multicast(void * self, const Message * msg) {
//	DataInfo* info = (DataInfo*)self;
//    connection_t* pPipeLines = get_pPipeLines();
//	for (int id = 0; id < get_childCount() + 1; id++) {
//		if (id != info->senderId) {
//			int pipeId = pPipeLines[getPipeId(info->senderId, id)].write;
//			if (writePipe(pipeId, msg) < 0)
//				return -1;
//		}
//	}

	return 0;
}

int receive(void * self, local_id from, Message * msg) {
//	DataInfo* info = (DataInfo*)self;
//    connection_t* pPipeLines = get_pPipeLines();
//	int pipeId = pPipeLines[getPipeId(from, info->senderId)].read;
//	int result = readPipe(pipeId, msg, 1);
//
//	if (result >= 0) {
//		sync_time(msg->s_header.s_local_time);
//	}

//	return result;
    return 0;
}


int receive_any(void * self, Message * msg) {
//	DataInfo* info = (DataInfo*)self;
//    connection_t* pPipeLines = get_pPipeLines();
//	for (int id = 0; id <= get_childCount(); id++) {
//		if (id != info->senderId) {
//			int pipeId = pPipeLines[getPipeId(id, info->senderId)].read;
//
//			if (readPipe(pipeId, msg, 0) == 0) {
//				info->receiveId = id;
//				sync_time(msg->s_header.s_local_time);
//
//				return 0;
//			}
//		}
//		//sleep(1);
//	}

    return -1;
}
