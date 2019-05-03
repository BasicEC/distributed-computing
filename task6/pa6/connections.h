#ifndef _CONNECTIONS
#define _CONNECTIONS

typedef struct {
	int input;
	int output;
} PipeLineInfo;

void initPipeLines(int processCount);
int getOpenedPipesFDCount();
void closeUnusedPipes(int selfId);
void freePipeLines();
void closePipe(int fd);
int get_pipeline_width();
PipeLineInfo*  get_pPipeLines();


#endif
