#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdio.h>
#include <sys/types.h>
#include <time.h>
#include <string.h>
#include "ipc.h"
#include "common.h"
#include "pa2345.h"
#include "banking.h"
#include "messager.h"

#ifndef __IFMO_DISTRIBUTED_CLASS_CHILD__H
#define __IFMO_DISTRIBUTED_CLASS_CHILD__H

//не уверен что хватит, вдруг время которое передается в сообщении занимает 100500 символов
#define COMMON_PAYLOAD_LEN 256 

//void child_work(System_t* proc_info, local_id id);
//void main_work(proc_info_t* info    );

void child_work();
void main_work();
#endif
