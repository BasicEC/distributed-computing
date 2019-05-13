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

void establish_useless_connections(table_t* table){
	for (int i = 0; i < table->thinkers_count + 1; i++){
		for (int j = 0; j < table->thinkers_count + 1; j++){
			if (i == j)
				continue;
			int arr[2];
			pipe2(arr, O_NONBLOCK);
			table->connections[i][j].read = arr[0];
			table->connections[j][i].write = arr[1];
		}
	}
}


void initPipeLines(table_t* table) {
    table->connections = malloc(sizeof(connection_t*) * (table->thinkers_count + 1));
    for (int i = 0; i < table->thinkers_count + 1; i++){
    	table->connections[i] = malloc(sizeof(connection_t) * table->thinkers_count + 1);
    }
    establish_useless_connections(table);
	allocate_connections(table);
	for (int i = 1; i <= table->thinkers_count; i++) {
		thinker_t* source = &table->thinkers[i - 1];
		int j = i - 1;
		thinker_t* destination_left = &table->thinkers[j+1 == table->thinkers_count ? 0 : j+1];
		thinker_t* destination_right = &table->thinkers[j-1 == -1 ? table->thinkers_count - 1 : j-1];
		int left_id = i+1 == table->thinkers_count + 1? 1 : i+1;
		int right_id = i-1 == 0 ? table->thinkers_count : i-1;
		source->right_neighbor->read = table->connections[i][right_id].read;
		destination_right->left_neighbor->write = table->connections[right_id][i].write;
		source->left_neighbor->read = table->connections[i][left_id].read;
		destination_left->right_neighbor->write = table->connections[left_id][i].write;
	}
}


void closeUnusedPipes(int selfId, table_t* table) {
	for (int i = 0; i < table->thinkers_count + 1; i++){
		if (i == selfId + 1)
			continue;
		for (int j = 0 ; j < table->thinkers_count + 1; j++){
		    if (i == j)
				continue;
		    if (j == selfId + 1){
				closePipe(table->connections[i][j].write);
		    }
		    else {
				closePipe(table->connections[i][j].read);
				closePipe(table->connections[i][j].write);
			}
		}
	}
}



void freePipeLines() {
}

void closePipe(int fd) {
	if (fd != 0) {
		close(fd);
	}
}



