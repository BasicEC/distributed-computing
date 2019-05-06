#define _GNU_SOURCE

#include "main.h"
#include "connections.h"
#include "util.h"
#include "pa6.h"
#include <getopt.h>
#include <time.h>
pid_t parentPid;
FILE* pLogFile;

table_t* table;
thinker_t* thinker;
delayed_transfer_t* delayed_transfer;

/*
 * Child workflow
 */

int system_done(pid_t pid, int selfId) {
	register_event();
	// sync
	message_info_t msg;
	sprintf(msg.msg.s_payload, log_done_fmt, get_time(), selfId);

	msg.msg.s_header.s_local_time = get_time();
	msg.msg.s_header.s_magic = MESSAGE_MAGIC;
	msg.msg.s_header.s_payload_len = strlen(msg.msg.s_payload) + 1;
	msg.msg.s_header.s_type = DONE;

	send_to_neighbor(thinker, DIRECTION_BOTH, &msg.msg);

	receive_from_neighbor(thinker, DIRECTION_RIGHT, &msg);
	receive_from_neighbor(thinker, DIRECTION_LEFT, &msg);

	// sync ended
	fprintf(pLogFile, log_received_all_done_fmt, get_time(), selfId);
	fflush(pLogFile);

	return 0;
}

int ask_for_fork(direction dir,pid_t pid, int selfId){
	Message msg;
	sprintf(msg.s_payload, log_request_for_fork_fmt, get_time(), selfId, pid);
	msg.s_header.s_local_time = get_time();
	msg.s_header.s_magic = MESSAGE_MAGIC;
	msg.s_header.s_payload_len = strlen(msg.s_payload) + 1;
	msg.s_header.s_type = ACK;
	return send_to_neighbor(thinker, dir, &msg);
}

int compare_time(timestamp_t time, direction dir, int selfId){
	timestamp_t localtime = get_time();
	if (time != localtime)
		return time - localtime;
	switch (dir){
		case DIRECTION_RIGHT:
			return selfId == 1 ? 1 : -1;
		case DIRECTION_LEFT:
			return selfId == table->thinkers_count - 1 ? -1 : 1;
		default:
			return 0;
	}
}

Message* prepare_transfer_message(pid_t pid, int selfId){
	Message* msg = malloc(sizeof(Message));
	sprintf(msg->s_payload, log_responce_with_fork_fmt, get_time(), selfId, pid);
	msg->s_header.s_local_time = get_time();
	msg->s_header.s_magic = MESSAGE_MAGIC;
	msg->s_header.s_payload_len = (uint16_t)(strlen(msg->s_payload) + 1);
	msg->s_header.s_type = TRANSFER;
	return msg;
}

int process_message_info(message_info_t* info, int selfId, pid_t pid){
	switch (info->msg.s_header.s_type){
		case TRANSFER:{
			switch (info->dir){
				case DIRECTION_LEFT:{
					thinker->left_fork->enabled = 1;
					thinker->left_fork->dirty = 0;
					break;
				}
				case DIRECTION_RIGHT:{
					thinker->right_fork->enabled = 1;
					thinker->right_fork->dirty = 0;
					break;
				}
				default:
					return -1;
			}
			break;
		}
		case ACK:{
			if (compare_time(info->msg.s_header.s_local_time,info->dir, selfId) < 0){
				switch (info->dir){
					case DIRECTION_BOTH:
						return -1;
					case DIRECTION_LEFT:{
						if (!thinker->left_fork->enabled)
							return -1;
						thinker->left_fork->enabled = 0;
						thinker->left_fork->dirty = 0;
						Message* msg = prepare_transfer_message(pid, selfId);
						send_to_neighbor(thinker, DIRECTION_LEFT, msg);
						break;
					}
					case DIRECTION_RIGHT:{
						if (!thinker->right_fork->enabled)
							return -1;
						thinker->right_fork->enabled = 0;
						thinker->right_fork->dirty = 0;
						Message* msg = prepare_transfer_message(pid, selfId);
						send_to_neighbor(thinker, DIRECTION_RIGHT, msg);
						break;
					}
				}
			} else {
				switch(info->dir){
					case DIRECTION_LEFT:{
						delayed_transfer->left_neighbor = 1;
						break;
					}
					case DIRECTION_RIGHT:{
						delayed_transfer->right_neighbor = 1;
						break;
					}
					default:
						return -1;
				}
			}
			break;
		}
		default:
			return -1;
	}
	return 0;
}

int check_delayed_transfers(pid_t pid, int selfId){
	if (delayed_transfer->right_neighbor || delayed_transfer->left_neighbor){
		Message* msg = prepare_transfer_message(pid, selfId);
		if (delayed_transfer->left_neighbor){
		    send_to_neighbor(thinker, DIRECTION_LEFT, msg);
			thinker->left_fork->enabled = 0;
			thinker->left_fork->dirty = 0;
		}
		if (delayed_transfer->right_neighbor){
			send_to_neighbor(thinker, DIRECTION_RIGHT, msg);
			thinker->right_fork->enabled = 0;
			thinker->right_fork->dirty = 0;
		}
	}
	return 0;
}

void eat(pid_t pid, int selfId){
	if (!thinker->left_fork->enabled)
		ask_for_fork(DIRECTION_LEFT, pid, selfId);
	if (!thinker->right_fork->enabled)
		ask_for_fork(DIRECTION_RIGHT, pid, selfId);
	fprintf(pLogFile, "process - %d wants to get message\n", selfId);
	fflush(pLogFile);
	while (!thinker->left_fork->enabled || !thinker->right_fork->enabled){
		message_info_t msg;
		receive_from_neighbor(thinker, DIRECTION_BOTH, &msg);
		fprintf(pLogFile, "process - %d have got message\n", selfId);
		fflush(pLogFile);
		process_message_info(&msg, selfId, pid);
	}
    fprintf(pLogFile, "process - %d has EAT\n", selfId);
	fflush(pLogFile);
	//EAT
	thinker->right_fork->dirty = 1;
	thinker->left_fork->dirty = 1;
	check_delayed_transfers(pid, selfId);
}

time_t get_end_time(){
	clock_t start_time;
	start_time = clock();
	int delay = ((rand() * 10) % 1500000) + 100000;
	if (delay < 0) delay = -delay;
//	printf("delay - %d\n", delay);
	return start_time + delay;
}


int process_request(message_info_t* info, pid_t pid, int selfId){
	switch (info->dir){
		case DIRECTION_BOTH:
			return -1;
		case DIRECTION_LEFT:{
			if (!thinker->left_fork->enabled)
				return -1;
			thinker->left_fork->enabled = 0;
			thinker->left_fork->dirty = 0;
			Message* msg = prepare_transfer_message(pid, selfId);
			send_to_neighbor(thinker, DIRECTION_LEFT, msg);
			break;
		}
		case DIRECTION_RIGHT:{
			if (!thinker->right_fork->enabled)
				return -1;
			thinker->right_fork->enabled = 0;
			thinker->right_fork->dirty = 0;
			Message* msg = prepare_transfer_message(pid, selfId);
			send_to_neighbor(thinker, DIRECTION_RIGHT, msg);
			break;
		}
	}
	return 0;
}

void think(pid_t pid, int selfId){
	time_t end_time = get_end_time();
	message_info_t msg;
	while (clock() < end_time){
		if (try_receive_message(thinker, &msg) <= 0)
			continue;
		process_request(&msg, pid, selfId);
	}
}

int thinker_work(pid_t pid, int selfId) {

	for (int i = 0; i < 5; i++){
		think(pid, selfId);
		eat(pid, selfId);
//		fprintf(pLogFile, "process - %d has EAT\n", selfId);
	}
	// work is done
	fprintf(pLogFile, log_done_fmt, get_time(), selfId);
	fflush(pLogFile);
	return system_done(pid, selfId);
}

int system_started(pid_t pid, int selfId) {
	thinker = &table->thinkers[selfId];
	delayed_transfer = malloc(sizeof(delayed_transfer_t));
	register_event();

	message_info_t msg;
	sprintf(msg.msg.s_payload, log_started_fmt, get_time(), selfId, pid, parentPid);
	msg.msg.s_header.s_local_time = get_time();
	msg.msg.s_header.s_magic = MESSAGE_MAGIC;
	msg.msg.s_header.s_payload_len = (uint16_t)(strlen(msg.msg.s_payload) + 1);
	msg.msg.s_header.s_type = STARTED;
	send_to_neighbor(thinker, DIRECTION_BOTH, &msg.msg);

	receive_from_neighbor(thinker, DIRECTION_RIGHT, &msg);
	receive_from_neighbor(thinker, DIRECTION_LEFT, &msg);

	fprintf(pLogFile, log_started_fmt, get_time(), selfId, pid, parentPid);
	fflush(pLogFile);

	return thinker_work(pid, selfId);
}

arguments_t parse_command_line_argument(int argc, char** argv){
	arguments_t arguments;
	arguments.count  = atoi(argv[2]);
	arguments.mutex = 0;
	if (argc == 4)
		arguments.mutex = 1;
	return arguments;
}

void init_forks(){
	for (int i = 0 ; i < table->thinkers_count; i++){
		table->thinkers[i].left_fork = malloc(sizeof(fork_t));
		table->thinkers[i].left_fork->dirty = 1;
		table->thinkers[i].left_fork->enabled = 0;
		table->thinkers[i].right_fork = malloc(sizeof(fork_t));
		table->thinkers[i].right_fork->dirty = 1;
		table->thinkers[i].right_fork->enabled = 1;
	}
}

int main(int argc, char **argv) {
	arguments_t arguments = parse_command_line_argument(argc, argv);
	int childCount = arguments.count;
	set_childCount(childCount);
	parentPid = getpid();

	pLogFile = fopen(evengs_log, "w+");
	set_pipefile(fopen(pipes_log, "w+"));
	table = (table_t*)malloc(sizeof(table_t));
	table->thinkers_count = childCount;
	table->thinkers = (thinker_t*)malloc(sizeof(thinker_t)*childCount);
	initPipeLines(table);
	init_forks();
	pid_t childPid = 0;
	int id;
	for (id = 0; id < childCount - 1; id++) {
		childPid = fork();
		if (!childPid) {

			closeUnusedPipes(id, table);
			system_started(getpid(), id);
			break;

		} else if (childPid < 0) {
			return -1;
		}
	}
	if (id == 4) {
		closeUnusedPipes(4, table);
		system_started(getpid(), 4);
	}
//	if (childPid != 0) {
//		int status;
//		while (wait(&status) > 0);
//	}

	freePipeLines();
	fclose(get_pipefile());
	fclose(pLogFile);

	return 0;
}
