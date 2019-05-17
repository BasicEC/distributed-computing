#define _GNU_SOURCE

#include "main.h"
#include "connections.h"
#include "util.h"
#include "pa2345.h"
#include <getopt.h>
#include <time.h>

pid_t parentPid;
FILE* pLogFile;

table_t* table;
thinker_t* thinker;
int* delayed_transfers;
int done_count = 0;
int mutexl;



int ask_for_fork(int dir,pid_t pid, int selfId){
	Message msg;
	sprintf(msg.s_payload, log_transfer_in_fmt, get_time(), selfId, pid, dir);
	msg.s_header.s_local_time = get_time();
	msg.s_header.s_magic = MESSAGE_MAGIC;
	msg.s_header.s_payload_len = (uint16_t)strlen(msg.s_payload) + (uint16_t)1;
	msg.s_header.s_type = ACK;
	return send(thinker, (local_id)dir, &msg);
}

int compare_time(timestamp_t time, int dir, int selfId){
	timestamp_t localtime = get_time();
	if (time != localtime)
		return time - localtime;
	return dir - selfId;
}

Message* prepare_ack_message(pid_t pid, int selfId){
	Message* msg = malloc(sizeof(Message));
	sprintf(msg->s_payload, log_transfer_in_fmt, get_time(), selfId, pid, 0);
	msg->s_header.s_local_time = get_time();
	msg->s_header.s_magic = MESSAGE_MAGIC;
	msg->s_header.s_payload_len = (uint16_t)(strlen(msg->s_payload) + 1);
	msg->s_header.s_type = ACK;
	return msg;
}

Message* prepare_transfer_message(pid_t pid, int selfId){
	Message* msg = malloc(sizeof(Message));
	sprintf(msg->s_payload, log_transfer_out_fmt, get_time(), selfId, pid, 0);
	msg->s_header.s_local_time = get_time();
	msg->s_header.s_magic = MESSAGE_MAGIC;
	msg->s_header.s_payload_len = (uint16_t)(strlen(msg->s_payload) + 1);
	msg->s_header.s_type = TRANSFER;
	return msg;
}

int process_message_info(Message* msg, int selfId, pid_t pid, int from){
	switch (msg->s_header.s_type){
		case TRANSFER:{
				thinker->forks[from].enabled = 1;
				thinker->forks[from].dirty = 0;
				break;
			}
		case ACK:{
			if (compare_time(msg->s_header.s_local_time,from, selfId) < 0){
				if (!thinker->forks[from].enabled)
					return -1;
				thinker->forks[from].enabled = 0;
				Message* message = prepare_transfer_message(pid, selfId);
				send(thinker,(local_id)from, message);
				message = prepare_ack_message(pid, selfId);
				send(thinker,(local_id)from, message);
			} else
			    delayed_transfers[from] = 1;

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
	for (int i = 1; i < thinker->connection_count; i++){
		if (delayed_transfers[i]){
			Message* msg = prepare_transfer_message(pid, selfId);
			thinker->forks[i].enabled = 0;
			send(thinker, (local_id)i, msg);
			delayed_transfers[i] = 0;
		}
	}
	return 0;
}

void ask_for_forks(pid_t pid){
	for (int i = 1 ; i < table->thinkers_count; i++){
		if (!thinker->forks[i].enabled)
			ask_for_fork(i, pid, thinker->id);
	}
}

int is_all_forks_enabled(){
	for (int i = 1 ; i < table->thinkers_count; i++)
		if (!thinker->forks[i].enabled)
			return 0;
	return 1;
}

int request_cs(const void* self){
	pid_t* pid = (pid_t*)self;
	ask_for_forks(*pid);
	while (!is_all_forks_enabled()) {
		Message msg;
		int from = receive_any(thinker, &msg);
		char *arr = msg.s_header.s_type == ACK ? "ASK" : "TRANSFER";
		fprintf(pLogFile, "process - %d have got %s message from %d\n", thinker->id, arr, from);
		fflush(pLogFile);
		process_message_info(&msg, thinker->id, *pid, from);
	}
	return  0;
}

int release_cs(const void* self){
    pid_t* pid = (pid_t*)self;
	check_delayed_transfers(*pid, thinker->id);
	return 0;
}

void eat_core(int iteration){
	fprintf(pLogFile, "process - %d now can EAT %d time\n", thinker->id, iteration);
	fflush(pLogFile);
	//EAT
	char arr[strlen(log_loop_operation_fmt)];
	sprintf(arr, log_loop_operation_fmt, thinker->id, iteration + 1, 5 * thinker->id);
	print(arr);
	for (int i = 1; i < thinker->connection_count; i++)
		thinker->forks[i].dirty = 1;
}

void eat(pid_t pid, int iteration){
    if (mutexl) {
    	request_cs(&pid);
    	eat_core(iteration);
    	release_cs(&pid);
	}
    else
    	eat_core(iteration);
	register_event();
}

time_t get_end_time(){
	clock_t start_time;
	start_time = clock();
	int delay = ((rand() * 10) % 150000) + 10000;
	if (delay < 0) delay = -delay;
	return start_time + delay;
}


int process_request(Message* msg, pid_t pid, int selfId, int from){
	switch (msg->s_header.s_type){
		case DONE:{
			done_count++;
			return 0;
		}
		case ACK:{
			if (!thinker->forks[from].enabled)
				return -1;
			thinker->forks[from].enabled = 0;
			thinker->forks[from].dirty = 0;
			Message* message = prepare_transfer_message(pid, selfId);
			send(thinker, (local_id)from, message);
			return 0;
		}
		default:
			return -1;
	}
}

void think(pid_t pid, int selfId){
	time_t end_time = get_end_time();
	Message msg;
	while (clock() < end_time){
		int result = try_receive_message(thinker, &msg);
		if (result <= 0)
			continue;
		char* arr = msg.s_header.s_type == ACK ? "ASK" : "TRANSFER";
		fprintf(pLogFile, "process - %d have got %s message from %d (while think)\n", selfId, arr, result);
		fflush(pLogFile);
		process_request(&msg, pid, selfId, result);
	}
}


int system_done(pid_t pid, int selfId) {
	register_event();
	// sync
	Message msg;
	sprintf(msg.s_payload, log_done_fmt, get_time(), selfId,0);

	msg.s_header.s_local_time = get_time();
	msg.s_header.s_magic = MESSAGE_MAGIC;
	msg.s_header.s_payload_len = (uint16_t)strlen(msg.s_payload) + (uint16_t)1;
	msg.s_header.s_type = DONE;

	send_multicast(thinker, &msg);
	while (done_count < table->thinkers_count - 2){
		int from = receive_any(thinker, &msg);
		process_request(&msg,pid,selfId, from);
	}

	fprintf(pLogFile, log_received_all_done_fmt, get_time(), selfId);
	fflush(pLogFile);

	return 0;
}

int thinker_work(pid_t pid, int selfId) {

	for (int i = 0; i < 5 * thinker->id; i++){
		think(pid, selfId);
		eat(pid, i);
	}
	// work is done
	fprintf(pLogFile, log_done_fmt, get_time(), selfId,0);
	fflush(pLogFile);
	return system_done(pid, selfId);
}

int system_started(pid_t pid, int selfId) {
	thinker = &table->thinkers[selfId];
	thinker->id = selfId;
	delayed_transfers = malloc(sizeof(int) * get_childCount());
	register_event();
    fprintf(pLogFile, log_started_fmt, get_time(), selfId, pid, parentPid,0);
    fflush(pLogFile);

	Message msg;
	sprintf(msg.s_payload, log_started_fmt, get_time(), selfId, pid, parentPid,0);
	msg.s_header.s_local_time = get_time();
	msg.s_header.s_magic = MESSAGE_MAGIC;
	msg.s_header.s_payload_len = (uint16_t)(strlen(msg.s_payload) + 1);
	msg.s_header.s_type = STARTED;
	send_multicast(thinker, &msg);

	for(int i = 1 ; i < table->thinkers_count; i++){
		if (i == thinker->id)
			continue;
		receive(thinker, (local_id)i, &msg);
	}

    fprintf(pLogFile, log_received_all_started_fmt, get_time(), selfId);
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
		table->thinkers[i].forks = malloc(sizeof(fork_t) * table->thinkers_count);
		table->thinkers[i].connection_count = table->thinkers_count;
		for (int j = i; j < table->thinkers_count; j++)
			table->thinkers[i].forks[j].enabled = 1;
	}
}


int main(int argc, char **argv) {
	arguments_t arguments = parse_command_line_argument(argc, argv);
	int childCount = arguments.count + 1;
	mutexl = arguments.mutex;

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
	for (id = 1; id < childCount; id++) {
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

	for (int i = 1 ; i < table->thinkers_count; i++)
		wait(0);
    closeUnusedPipes(0, table);

	freePipeLines();
	fclose(get_pipefile());
	fclose(pLogFile);

	return 0;
}
