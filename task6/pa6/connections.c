#define _GNU_SOURCE

#include"connections.h"
#include "main.h"
#include "util.h"
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>



void initPipeLines(table_t* table) {
	for (int i = 0; i < table->thinkers_count; i++)
		table->thinkers[i].connections = malloc(sizeof(connection_t) * table->thinkers_count);

	for (int i = 0; i < table->thinkers_count; i++){
		for (int j = 0; j < table->thinkers_count; j++){
			if (i == j)
				continue;
			int arr[2];
			pipe2(arr, O_NONBLOCK);
			table->thinkers[i].connections[j].read = arr[0];
			table->thinkers[j].connections[i].write = arr[1];
		}
	}
}


void closeUnusedPipes(int selfId, table_t* table) {
	for (int i = 0; i < table->thinkers_count; i++){
		if (i == selfId)
			continue;
		for (int j = 0; j < table->thinkers_count; j++){
		    if (i == j)
				continue;
			closePipe(table->thinkers[i].connections[j].read);
			closePipe(table->thinkers[i].connections[j].write);
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



