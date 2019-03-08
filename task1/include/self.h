#ifndef __IFMO_DISTRIBUTED_CLASS_SELF__H
#define __IFMO_DISTRIBUTED_CLASS_SELF__H

#include "ipc.h"

typedef struct {
    intlocal_id current_pid;
    int p_count;
    int* pipefds;
} self_t;

//return pipefd or -1 if err 
int get_w_pipefd_by_id(self_t* self, local_id dst);

int get_r_pipefd_by_id(self_t* self, local_id dst);

//return count of pipefds or -1 if err.
//pipefds must point to arr with pipefds in the end of method
int get_all_w_pipefds(self_t* self, int* pipefds);

int get_all_r_pipefds(self_t* self, int* pipefds);

#endif //__IFMO_DISTRIBUTED_CLASS_SELF__H