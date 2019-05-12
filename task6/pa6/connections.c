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
		table->thinkers[i].right_neighbor = malloc(sizeof(connection_t));
		table->thinkers[i].left_neighbor = malloc(sizeof(connection_t));
	}
}


void initPipeLines(table_t* table, int* children) {
	allocate_connections(table);
	for (int i = 0; i < table->thinkers_count; i++) {
		thinker_t* source = &table->thinkers[i];
		thinker_t* destination_left = &table->thinkers[i+1 == table->thinkers_count ? 0 : i+1];
		thinker_t* destination_right = &table->thinkers[i-1 == -1 ? table->thinkers_count - 1 : i-1];
		int pipe_ids[2];
		pipe2(pipe_ids, O_NONBLOCK);
		source->right_neighbor->read = pipe_ids[0];
		destination_right->left_neighbor->write = pipe_ids[1];
		pipe2(pipe_ids, O_NONBLOCK);
		source->left_neighbor->read = pipe_ids[0];
		destination_left->right_neighbor->write = pipe_ids[1];
		pipe2(pipe_ids, O_NONBLOCK);
		children[i] = pipe_ids[0];
		source->pipe_to_parent = pipe_ids[1];

		int useless_nodes_count = table->thinkers_count - 3;
		useless_nodes_count = useless_nodes_count < 0 ? 0 : useless_nodes_count;
		source->useless_pipes = malloc(sizeof(int*) * useless_nodes_count);
		for (int j = 0 ; j < useless_nodes_count ; j++ ){
			source->useless_pipes[j] = malloc(sizeof(int)*2);
			pipe2(source->useless_pipes[j], O_NONBLOCK);
		}
	}
}

void closeUnusedPipes(int selfId, table_t* table) {
	for (int i = 0; i < table->thinkers_count; i++){
		if(i == selfId)
			continue;
		closePipe(table->thinkers[i].left_neighbor->write);
		closePipe(table->thinkers[i].left_neighbor->read);
		closePipe(table->thinkers[i].right_neighbor->write);
		closePipe(table->thinkers[i].right_neighbor->read);
	}
}


void freePipeLines() {
}

void closePipe(int fd) {
	if (fd != 0) {
		close(fd);
	}
}



