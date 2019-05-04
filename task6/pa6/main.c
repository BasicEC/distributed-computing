#define _GNU_SOURCE

#include "main.h"
#include "connections.h"
#include "util.h"
#include <getopt.h>

pid_t parentPid;
FILE* pLogFile;

int processID;

void copy(void* from, void* to, size_t count) {
	for (int i = 0; i < count; i++) {
		((char*)to)[i] = ((char*)from)[i];
	}
}

void zero(void* from, size_t count) {
	for (int i = 0; i < count; i++) {
		((char*)from)[i] = 0;
	}
}


/*
 * Child workflow
 */

int system_done(pid_t pid, int selfId) {
	register_event();
	// sync
	Message msg;
	sprintf(msg.s_payload, log_done_fmt, get_time(), selfId, 0);

	msg.s_header.s_local_time = get_time();
	msg.s_header.s_magic = MESSAGE_MAGIC;
	msg.s_header.s_payload_len = strlen(msg.s_payload) + 1;
	msg.s_header.s_type = DONE;

	DataInfo info;
	info.senderId = selfId;
	send_multicast(&info, &msg);
	for (int id = 0; id < get_childCount() + 1; id++) {
		if (id != PARENT_ID && id != selfId) {
			receive(&info, id, &msg);
		}
	}

	// sync ended
	fprintf(pLogFile, log_received_all_done_fmt, get_time(), selfId);
	fflush(pLogFile);

	return 0;
}

int system_work(pid_t pid, int selfId) {
	// some work
	// char isWorked = 1;

	// work is done
	fprintf(pLogFile, log_done_fmt, get_time(), selfId, 0);
	fflush(pLogFile);
	return system_done(pid, selfId);
}

int system_started(pid_t pid, int selfId) {
	register_event();
	// sync
	fprintf(pLogFile, log_started_fmt, get_time(), selfId, pid, parentPid, 0);
	fflush(pLogFile);

	Message msg;
	sprintf(msg.s_payload, log_started_fmt, get_time(), selfId, pid, parentPid, 0);
	msg.s_header.s_local_time = get_time();
	msg.s_header.s_magic = MESSAGE_MAGIC;
	msg.s_header.s_payload_len = strlen(msg.s_payload) + 1;
	msg.s_header.s_type = STARTED;

	DataInfo info;
	info.senderId = selfId;
	
	send_multicast(&info, &msg);
	//printf("kek\n");
	for (int id = 0; id < get_childCount() + 1; id++) {
		if (id != PARENT_ID && id != selfId) {
			receive(&info, id, &msg);
		}
	}
	// sync complete
	fprintf(pLogFile, log_received_all_started_fmt, get_time(), selfId);
	fflush(pLogFile);

	return system_work(pid, selfId);
}

arguments_t parse_command_line_argument(int argc, char** argv){
	arguments_t arguments;
	arguments.count  = atoi(argv[2]);
	arguments.mutex = 0;
	if (argc == 4)
		arguments.mutex = 1;
	return arguments;
}


int main(int argc, char **argv) {
	arguments_t arguments = parse_command_line_argument(argc, argv);
	int childCount = arguments.count;
	set_childCount(childCount);
	processID = 0;
	parentPid = getpid();

	pLogFile = fopen(evengs_log, "w+");
	set_pipefile(fopen(pipes_log, "w+"));

	initPipeLines(childCount + 1);

	pid_t childPid = 0;
	for (int id = 1; id < childCount + 1; id++) {
		childPid = fork();
		if (childPid > 0) {

			processID = id;
//			closeUnusedPipes(id);
			system_started(getpid(), id);
			break;

		} else if (childPid < 0) {
			return -1;
		}
	}
	if (childPid != 0) {
		closeUnusedPipes(PARENT_ID);

		DataInfo info;
		Message msg;
		info.senderId = PARENT_ID;

		for (int idx = 1; idx <= childCount; idx++) {
			receive(&info, idx, &msg);
		}

		for (int idx = 1; idx < childCount + 1; idx++) {
			receive(&info, idx, &msg);
		}

		int status;
		while (wait(&status) > 0);
	}

	freePipeLines();
	fclose(get_pipefile());
	fclose(pLogFile);

	return 0;
}
