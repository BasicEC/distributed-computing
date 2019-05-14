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
fork_t* forks;
int* delayed_transfers;
int done_count = 0;

void print(char*);


int ask_for_fork(direction dir,pid_t pid, int selfId){
	Message msg;
	sprintf(msg.s_payload, log_request_for_fork_fmt, get_time(), selfId, pid);
	msg.s_header.s_local_time = get_time();
	msg.s_header.s_magic = MESSAGE_MAGIC;
	msg.s_header.s_payload_len = strlen(msg.s_payload) + 1;
	msg.s_header.s_type = ACK;
	return send(thinker, dir, &msg);
}

int compare_time(timestamp_t time, direction dir, int selfId){
	timestamp_t localtime = get_time();
	if (time != localtime)
		return time - localtime;
	switch (dir){
		case DIRECTION_RIGHT:
			return selfId == 0 ? 1 : -1;
		case DIRECTION_LEFT:
			return selfId == table->thinkers_count - 1 ? -1 : 1;
		default:
			return 0;
	}
}

Message* prepare_ack_message(pid_t pid, int selfId){
	Message* msg = malloc(sizeof(Message));
	sprintf(msg->s_payload, log_request_for_fork_fmt, get_time(), selfId, pid);
	msg->s_header.s_local_time = get_time();
	msg->s_header.s_magic = MESSAGE_MAGIC;
	msg->s_header.s_payload_len = (uint16_t)(strlen(msg->s_payload) + 1);
	msg->s_header.s_type = ACK;
	return msg;
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
						msg = prepare_ack_message(pid, selfId);
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
						msg = prepare_ack_message(pid, selfId);
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
		case DONE:{
			done_count++;
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
			delayed_transfer->left_neighbor = 0;
		}
		if (delayed_transfer->right_neighbor){
			send_to_neighbor(thinker, DIRECTION_RIGHT, msg);
			thinker->right_fork->enabled = 0;
			thinker->right_fork->dirty = 0;
			delayed_transfer->right_neighbor = 0;
		}
	}
	return 0;
}

void ask_for_forks(pid_t pid){
	for (int i = 0 ; i < table->thinkers_count; i++){
		if (!forks[i].enabled)
			ask_for_fork(i, pid, thinker->id);
	}
}

int is_all_forks_enabled(){
	for (int i = 0 ; i < table->thinkers_count; i++)
		if (!forks[i].enabled)
			return 0;
	return 1;
}

void eat(pid_t pid, int iteration){
    ask_for_forks(pid);
	while (!is_all_forks_enabled()){
		message_info_t msg;
		receive_any(thinker, DIRECTION_BOTH, &msg);
		char* arr = msg.msg.s_header.s_type == ACK ? "ASK" : "TRANSFER";
		char* from = msg.dir == DIRECTION_LEFT ? "LEFT" : "RIGHT";
		fprintf(pLogFile, "process - %d have got %s message from %s\n", thinker->id, arr, from);
		fflush(pLogFile);
		process_message_info(&msg, thinker->id, pid);
	}
    fprintf(pLogFile, "process - %d now can EAT\n", thinker->id);
	fflush(pLogFile);
	//EAT
	char arr[100];
	sprintf(arr, log_loop_operation_fmt, thinker->id + 1, iteration + 1, 5);
	print(arr);


	thinker->right_fork->dirty = 1;
	thinker->left_fork->dirty = 1;
	check_delayed_transfers(pid, thinker->id);
	register_event();
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
	switch (info->msg.s_header.s_type){
		case DONE:{
			done_count++;
			return 0;
		}
		case ACK:{
			if (!forks[info->dir].enabled)
				return -1;
			forks[info->dir].enabled = 0;
			forks[info->dir].dirty = 0;
			Message* msg = prepare_transfer_message(pid, selfId);
			send(thinker, info->dir, msg);
			return 0;
		}
		default:
			return -1;
	}
}

void think(pid_t pid, int selfId){
	time_t end_time = get_end_time();
	message_info_t msg;
	while (clock() < end_time){
		if (try_receive_message(table, &msg, selfId) <= 0)
			continue;
		char* arr = msg.msg.s_header.s_type == ACK ? "ASK" : "TRANSFER";
		char* from = msg.dir == DIRECTION_LEFT ? "LEFT" : "RIGHT";
		fprintf(pLogFile, "process - %d have got %s message from %s (while think)\n", selfId, arr, from);
		fflush(pLogFile);
		process_request(&msg, pid, selfId);
	}
}


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
	while (done_count < 2){
		receive_from_neighbor(thinker, DIRECTION_BOTH, &msg);
		process_message_info(&msg,selfId,pid);
	}

	fprintf(pLogFile, log_received_all_done_fmt, get_time(), selfId);
	fflush(pLogFile);

	return 0;
}

int thinker_work(pid_t pid, int selfId) {

	for (int i = 0; i < 5; i++){
		think(pid, selfId);
		eat(pid, i);
	}
	// work is done
	fprintf(pLogFile, log_done_fmt, get_time(), selfId);
	fflush(pLogFile);
	return system_done(pid, selfId);
}

int system_started(pid_t pid, int selfId) {
	thinker = &table->thinkers[selfId];
	delayed_transfers = malloc(sizeof(int) * get_childCount());
	forks = malloc(sizeof(fork_t) * get_childCount());
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

	pLogFile = fopen(events_log, "w+");
	set_pipefile(fopen(pipes_log, "w+"));
	table = (table_t*)malloc(sizeof(table_t));
	table->thinkers_count = childCount;
	table->thinkers = (thinker_t*)malloc(sizeof(thinker_t)*childCount);
	initPipeLines(table);

	init_forks();
	pid_t childPid = 0;
	int id;
	for (id = 0; id < childCount; id++) {
		childPid = fork();
		if (!childPid) {
			closeUnusedPipes(id, table);
			system_started(getpid(), id);
			freePipeLines();
			fclose(get_pipefile());
			fclose(pLogFile);
			exit(0);
		} else if (childPid < 0) {
			return -1;
		}
	}

	for (int i = 0 ; i < table->thinkers_count; i++)
		wait(0);
    closeUnusedPipes(0, table);

	freePipeLines();
	fclose(get_pipefile());
	fclose(pLogFile);

	return 0;
}
