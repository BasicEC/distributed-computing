#ifndef MAIN_H_
#define MAIN_H_

#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>

#include <fcntl.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <string.h>

#include "ipc.h"
#include "pa6.h"
#include "common.h"
#include "connections.h"

typedef struct {
	int senderId;
	int receiveId;
} DataInfo;

typedef struct{
	int count;
	int mutex;
} arguments_t;



#define E_PIPE_INVALID_ARGUMENT -1
#define E_PIPE_NO_DATA -2
#define E_PIPE_ERROR -3


#endif /* MAIN_H_ */
