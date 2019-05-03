#include "util.h"

timestamp_t localTime = 0;
int childCount = 0;
FILE* pPipeFile;

void set_local_time(timestamp_t time){
    localTime = time;
}

timestamp_t get_time() {
	return localTime;
}

void register_event() {
	localTime++;
}

void set_childCount(int count){
    childCount = count;
}

int get_childCount(){
    return childCount;
}

FILE* get_pipefile(){
    return pPipeFile;
}

void set_pipefile(FILE* file){
    pPipeFile = file;
}

