#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include "ipc.h"
#include "common.h"
#include "pa1.h"

int PROCESS_COUNT;

void do_smth(){

}

int create_process(){
   pid_t id = fork();
   if (id < 0) {
       perror("unable to create process");
       errno = -1;
       _exit(-1);
   }
   if (id == 0){
       do_smth();
       _exit(0);
   }
    return id;
}

void run(){
    int i;
    int pid_arr[PROCESS_COUNT];
    for(i = 0; i < PROCESS_COUNT; i++){
        pid_arr[i] = create_process();
    }
}

void parse_arguments(char** args)
{
    void * ptr = NULL;
    if (strcmp(args[1],"-p") == 0 || strcmp(args[1], "-P") == 0)
        PROCESS_COUNT = (short)strtol(args[2],ptr,10);
}

int main(int argc, char** argv)
{
    parse_arguments(argv);
    printf("%d",PROCESS_COUNT);
    run();
    return 0;
}

