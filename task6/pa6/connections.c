#define _GNU_SOURCE

#include"connections.h"
#include "main.h"
#include "util.h"
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>


void allocate_connections(table_t* table){
	for (int i = 0 ; i < table->thinkers_count; i++){
		table->thinkers[i].right_neighor = malloc(sizeof(connection_t));
		table->thinkers[i].left_neighbor = malloc(sizeof(connection_t));
	}
}


void initPipeLines(table_t* table) {
	allocate_connections(table);
	for (int i = 0; i < table->thinkers_count; i++) {
		thinker_t source = table->thinkers[i];
		thinker_t destination_left = table->thinkers[i+1 == table->thinkers_count ? 0 : i+1];
		thinker_t destination_right = table->thinkers[i-1 == -1 ? table->thinkers_count - 1 : i-1];
		int pipe_ids[2];
		pipe2(pipe_ids, O_NONBLOCK);
		source.right_neighor->write = pipe_ids[0];
		destination_right.left_neighbor->read = pipe_ids[1];
		pipe2(pipe_ids, O_NONBLOCK);
		source.left_neighbor->write = pipe_ids[0];
		destination_left.right_neighor->read = pipe_ids[1];
	}
}

void closeUnusedPipes(int selfId, table_t* table) {
	int left = selfId + 1 == table->thinkers_count ? 0 : selfId + 1;
	int right = selfId - 1 == -1 ? table->thinkers_count - 1 : selfId - 1;
	for (int i = 0; i < table->thinkers_count; i++){
		if(i == selfId)
			continue;
		if (i == right){
			closePipe(table->thinkers[right].right_neighor->write);
			closePipe(table->thinkers[right].right_neighor->read);
			closePipe(table->thinkers[right].left_neighbor->write);
			continue;
		}
		if(i == left){
			closePipe(table->thinkers[left].left_neighbor->write);
			closePipe(table->thinkers[left].left_neighbor->read);
			closePipe(table->thinkers[left].right_neighor->write);
			continue;
		}
		closePipe(table->thinkers[i].left_neighbor->write);
		closePipe(table->thinkers[i].left_neighbor->read);
		closePipe(table->thinkers[i].right_neighor->write);
		closePipe(table->thinkers[i].right_neighor->read);
	}
}


void freePipeLines() {
}

void closePipe(int fd) {
	if (fd != 0) {
		close(fd);
	}
}



