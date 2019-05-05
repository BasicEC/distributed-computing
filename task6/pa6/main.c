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

/*
 * Child workflow
 */

int system_done(pid_t pid, int selfId) {
	register_event();
	// sync
	Message msg;
	sprintf(msg.s_payload, log_done_fmt, get_time(), selfId);

	msg.s_header.s_local_time = get_time();
	msg.s_header.s_magic = MESSAGE_MAGIC;
	msg.s_header.s_payload_len = strlen(msg.s_payload) + 1;
	msg.s_header.s_type = DONE;

	send_to_neighbor(thinker, DIRECTION_BOTH, &msg);

	receive_from_neighbor(thinker, DIRECTION_RIGHT, &msg);
	receive_from_neighbor(thinker, DIRECTION_LEFT, &msg);

	// sync ended
	fprintf(pLogFile, log_received_all_done_fmt, get_time(), selfId);
	fflush(pLogFile);

	return 0;
}

void eat(){

}

time_t get_end_time(){
	clock_t start_time;
	start_time = clock();
	int delay = ((rand() * 10) % 1500000) + 100000;
	if (delay < 0) delay = -delay;
	printf("delay - %d\n", delay);
	return start_time + delay;
}


void think(){
	time_t end_time = get_end_time();
	while (clock() < end_time);
}

int thinker_work(pid_t pid, int selfId) {

	for (int i = 0; i < 5; i++){
		think();
		eat();
	}
	// work is done
	fprintf(pLogFile, log_done_fmt, get_time(), selfId);
	fflush(pLogFile);
	return system_done(pid, selfId);
}

int system_started(pid_t pid, int selfId) {
	thinker = &table->thinkers[selfId];
	register_event();

	Message msg;
	sprintf(msg.s_payload, log_started_fmt, get_time(), selfId, pid, parentPid);
	msg.s_header.s_local_time = get_time();
	msg.s_header.s_magic = MESSAGE_MAGIC;
	msg.s_header.s_payload_len = (uint16_t)(strlen(msg.s_payload) + 1);
	msg.s_header.s_type = STARTED;
	send_to_neighbor(thinker, DIRECTION_BOTH, &msg);

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
