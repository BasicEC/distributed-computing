#define _GNU_SOURCE

#include"connections.h"
#include "main.h"
#include "util.h"
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>



int pipeLinesWidth = 0;
PipeLineInfo* pPipeLines = 0;

void initPipeLines(int processCount) {
	pipeLinesWidth = processCount - 1;
	int pipesCount = pipeLinesWidth * processCount;
	pPipeLines = (PipeLineInfo*)malloc(sizeof(PipeLineInfo) * pipesCount);

	for (int id = 0; id < pipesCount; id++) {
		int pipe_ids[2];
		pipe2(pipe_ids, O_NONBLOCK);
		pPipeLines[id].input = pipe_ids[0];
		pPipeLines[id].output = pipe_ids[1];
	}
}

int getOpenedPipesFDCount() {
	int pipesCount = pipeLinesWidth * (pipeLinesWidth + 1);
	int result = 0;
	for (int idx = 0; idx < pipesCount; idx++) {
		if (pPipeLines[idx].input)
			result++;
		if (pPipeLines[idx].output)
			result++;
	}

	return result++;
}

void closeUnusedPipes(int selfId) {
	int pipesCount = pipeLinesWidth * (pipeLinesWidth + 1);
	for (int idx = 0; idx < pipesCount; idx++) {
		int fromProcessId = idx / pipeLinesWidth;
		int toProcessId = idx % pipeLinesWidth;
		if (toProcessId >= fromProcessId) toProcessId++;

		if (fromProcessId == selfId) {
			closePipe(pPipeLines[idx].input);
			pPipeLines[idx].input = 0;
		} else if (toProcessId == selfId) {
			closePipe(pPipeLines[idx].output);
			pPipeLines[idx].output = 0;
		} else {
			closePipe(pPipeLines[idx].input);
			pPipeLines[idx].input = 0;
			closePipe(pPipeLines[idx].output);
			pPipeLines[idx].output = 0;
		}
	}

	fprintf(get_pipefile(), "Opened for %d PipesFD == %d\n", selfId, getOpenedPipesFDCount());
}


void freePipeLines() {
	if (pPipeLines != NULL) {
		int pipesCount = pipeLinesWidth * (pipeLinesWidth + 1);
		for (int id = 0; id < pipesCount; id++) {
			closePipe(pPipeLines[id].input);
			closePipe(pPipeLines[id].output);
			pPipeLines[id].input = 0;
			pPipeLines[id].output = 0;
		}

		free(pPipeLines);
		pPipeLines = NULL;
	}
}

void closePipe(int fd) {
	if (fd != 0) {
		close(fd);
	}
}

int get_pipeline_width(){
    return pipeLinesWidth;
}

PipeLineInfo*  get_pPipeLines(){
    return pPipeLines;
}



