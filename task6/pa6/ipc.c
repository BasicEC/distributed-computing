
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

int send_to_neighbor(thinker_t* source, direction dir, Message* msg){
    switch(dir){
        case DIRECTION_BOTH: {
            if (writePipe(source->left_neighbor->write, msg) < 0)
                return -1;
            return writePipe(source->right_neighbor->write, msg);
        }
        case DIRECTION_LEFT:
            return writePipe(source->left_neighbor->write, msg);
        case DIRECTION_RIGHT:
            return writePipe(source->right_neighbor->write, msg);
    }
    return 0;
}

int receive_from_neighbor(thinker_t* source, direction dir, Message* msg){
    int result = 0;
    switch (dir){
        case DIRECTION_BOTH:{
            while(1){
                result = readPipe(source->right_neighbor->read, msg, 0);
                if (result >=0)
                    break;
                result = readPipe(source->left_neighbor->read, msg, 0);
                if (result >=0)
                    break;
            }
            break;
        }
        case DIRECTION_RIGHT:{
            result = readPipe(source->right_neighbor->read, msg, 1);
            break;
        }
        case DIRECTION_LEFT:{
            result = readPipe(source->left_neighbor->read, msg, 1);
            break;
        }
    }
    return result;
}

int send_multicast(void * self, const Message * msg) {
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
